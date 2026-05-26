#include "ConvReverb.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr int stereoChannelCount = 2;
constexpr int modulationDelaySafetySamples = 4;

[[nodiscard]] float clampWetMix(const float value) noexcept {
    return std::clamp(value, convreverbconfig::wetMixMin, convreverbconfig::wetMixMax);
}

[[nodiscard]] float millisecondsToSamples(const float milliseconds,
                                          const double sampleRate) noexcept {
    return milliseconds * static_cast<float>(sampleRate) * 0.001f;
}
} // namespace

ConvReverb::ConvReverb(const juce::dsp::ProcessSpec& spec) {
    prepare(spec);
}

void ConvReverb::prepare(const juce::dsp::ProcessSpec& spec) {
    sampleRate = std::max(1.0, spec.sampleRate);
    maximumBlockSize = static_cast<int>(spec.maximumBlockSize);

    if (!customImpulseLoaded)
        loadFallbackImpulseResponse();

    convolution.prepare(spec);

    wetMixSmoothed.reset(sampleRate, convreverbconfig::parameterSmoothingTimeSeconds);
    wetMixSmoothed.setCurrentAndTargetValue(wetMix);

    prepareWetPathBuffers(spec);
    reset();
}

void ConvReverb::process(juce::dsp::AudioBlock<float>& block) {
    const auto numChannels = static_cast<int>(block.getNumChannels());
    const auto numSamples = static_cast<int>(block.getNumSamples());

    if (numChannels == 0 || numSamples == 0)
        return;

    jassert(numChannels <= stereoChannelCount);
    jassert(numSamples <= maximumBlockSize);

    const auto channelsToProcess = std::min(numChannels, stereoChannelCount);
    for (auto channel = 0; channel < channelsToProcess; ++channel)
        wetInputBuffer.copyFrom(
            channel, 0, block.getChannelPointer(static_cast<std::size_t>(channel)), numSamples);

    if (channelsToProcess == 1)
        wetInputBuffer.copyFrom(1, 0, block.getChannelPointer(0), numSamples);

    processWetPathModulation(wetInputBuffer, numSamples);

    for (auto channel = 0; channel < stereoChannelCount; ++channel)
        wetOutputBuffer.copyFrom(channel, 0, wetInputBuffer, channel, 0, numSamples);

    auto wetBlock = juce::dsp::AudioBlock<float>(wetOutputBuffer)
                        .getSubBlock(0, static_cast<std::size_t>(numSamples));
    auto context = juce::dsp::ProcessContextReplacing<float>(wetBlock);
    convolution.process(context);

    for (auto sample = 0; sample < numSamples; ++sample) {
        const auto smoothedMix = getSmoothedWetMix();

        for (auto channel = 0; channel < channelsToProcess; ++channel) {
            auto* samples = block.getChannelPointer(static_cast<std::size_t>(channel));
            const auto dry = samples[sample];
            const auto wet = wetOutputBuffer.getSample(channel, sample);
            samples[sample] = dry + smoothedMix * (wet - dry);
        }
    }
}

void ConvReverb::reset() {
    convolution.reset();
    wetInputBuffer.clear();
    wetOutputBuffer.clear();

    for (auto& channelBuffer : modulationDelayBuffer)
        std::fill(channelBuffer.begin(), channelBuffer.end(), 0.0f);

    modulationPhaseRadians = 0.0f;
    modulationWriteIndex = 0;
    wetMixSmoothed.setCurrentAndTargetValue(wetMix);
}

int ConvReverb::getLatencySamples() const {
    return convolution.getLatency();
}

void ConvReverb::setWetMix(const float newWetMix) noexcept {
    const auto clampedMix = clampWetMix(newWetMix);

    if (std::abs(clampedMix - wetMix) < convreverbconfig::parameterEpsilon)
        return;

    wetMix = clampedMix;
    wetMixSmoothed.setTargetValue(wetMix);
}

void ConvReverb::loadImpulseResponse(juce::AudioBuffer<float>&& impulseResponse,
                                     const double impulseSampleRate) {
    customImpulseLoaded = true;
    convolution.loadImpulseResponse(
        std::move(impulseResponse), impulseSampleRate, juce::dsp::Convolution::Stereo::yes,
        juce::dsp::Convolution::Trim::no, juce::dsp::Convolution::Normalise::no);
}

int ConvReverb::getCurrentIRSize() const {
    return convolution.getCurrentIRSize();
}

juce::AudioBuffer<float> ConvReverb::createFallbackImpulseResponse() {
    juce::AudioBuffer<float> impulse(stereoChannelCount,
                                     convreverbconfig::fallbackImpulseLengthSamples);
    impulse.clear();

    for (auto channel = 0; channel < stereoChannelCount; ++channel) {
        auto decay = 1.0f;

        for (auto sample = 0; sample < impulse.getNumSamples(); ++sample) {
            const auto tail = 0.08f * decay;
            const auto earlyReflection = sample == 180 + 41 * channel
                                             ? convreverbconfig::fallbackImpulseEarlyReflectionGain
                                             : 0.0f;
            impulse.setSample(channel, sample, sample == 0 ? 1.0f : tail + earlyReflection);
            decay *= convreverbconfig::fallbackImpulseDecay;
        }
    }

    return impulse;
}

float ConvReverb::readModulationDelay(const int channel, const float delaySamples) const noexcept {
    const auto clampedDelay =
        std::clamp(delaySamples, 1.0f, static_cast<float>(modulationDelayBufferSize - 2));
    auto readPosition = static_cast<float>(modulationWriteIndex) - clampedDelay;

    while (readPosition < 0.0f)
        readPosition += static_cast<float>(modulationDelayBufferSize);

    const auto index0 = static_cast<int>(std::floor(readPosition));
    const auto index1 = (index0 + 1) % modulationDelayBufferSize;
    const auto fraction = readPosition - static_cast<float>(index0);
    const auto& buffer = modulationDelayBuffer[static_cast<std::size_t>(channel)];

    return buffer[static_cast<std::size_t>(index0)] +
           fraction * (buffer[static_cast<std::size_t>(index1)] -
                       buffer[static_cast<std::size_t>(index0)]);
}

float ConvReverb::getWetPathModulationDelaySamples() const noexcept {
    const auto centerDelaySamples =
        millisecondsToSamples(convreverbconfig::wetPathModulationCenterDelayMs, sampleRate);
    const auto lfo = std::sin(modulationPhaseRadians);
    return centerDelaySamples + getWetPathModulationDepthSamples() * lfo;
}

float ConvReverb::getWetPathModulationDepthSamples() const noexcept {
    return convreverbconfig::wetPathModulationDepthPercent * static_cast<float>(sampleRate);
}

float ConvReverb::getSmoothedWetMix() noexcept {
    return wetMixSmoothed.getNextValue();
}

void ConvReverb::processWetPathModulation(juce::AudioBuffer<float>& buffer,
                                          const int numSamples) noexcept {
    for (auto sample = 0; sample < numSamples; ++sample) {
        const auto delaySamples = getWetPathModulationDelaySamples();

        for (auto channel = 0; channel < stereoChannelCount; ++channel) {
            const auto input = buffer.getSample(channel, sample);
            modulationDelayBuffer[static_cast<std::size_t>(channel)]
                                 [static_cast<std::size_t>(modulationWriteIndex)] = input;
            buffer.setSample(channel, sample, readModulationDelay(channel, delaySamples));
        }

        advanceModulationWriteIndex();
        advanceModulationPhase();
    }
}

void ConvReverb::advanceModulationPhase() noexcept {
    modulationPhaseRadians += juce::MathConstants<float>::twoPi *
                              convreverbconfig::wetPathModulationRateHz /
                              static_cast<float>(sampleRate);

    if (modulationPhaseRadians >= juce::MathConstants<float>::twoPi)
        modulationPhaseRadians =
            std::fmod(modulationPhaseRadians, juce::MathConstants<float>::twoPi);
}

void ConvReverb::prepareWetPathBuffers(const juce::dsp::ProcessSpec& spec) {
    wetInputBuffer.setSize(stereoChannelCount, static_cast<int>(spec.maximumBlockSize), false,
                           false, true);
    wetOutputBuffer.setSize(stereoChannelCount, static_cast<int>(spec.maximumBlockSize), false,
                            false, true);

    const auto maxModulationDelaySamples =
        static_cast<int>(std::ceil(
            millisecondsToSamples(convreverbconfig::wetPathModulationCenterDelayMs, sampleRate) +
            getWetPathModulationDepthSamples())) +
        modulationDelaySafetySamples;

    modulationDelayBufferSize = std::max(1, maxModulationDelaySamples);

    for (auto& channelBuffer : modulationDelayBuffer)
        channelBuffer.assign(static_cast<std::size_t>(modulationDelayBufferSize), 0.0f);
}

void ConvReverb::loadFallbackImpulseResponse() {
    convolution.loadImpulseResponse(
        createFallbackImpulseResponse(), sampleRate, juce::dsp::Convolution::Stereo::yes,
        juce::dsp::Convolution::Trim::no, juce::dsp::Convolution::Normalise::no);
}

void ConvReverb::advanceModulationWriteIndex() noexcept {
    modulationWriteIndex = (modulationWriteIndex + 1) % modulationDelayBufferSize;
}
