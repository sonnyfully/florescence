#include "Chorus.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr float leftVoicePhaseOffsetRadians = 0.0f;
constexpr float rightVoicePhaseOffsetRadians = juce::MathConstants<float>::pi;
constexpr juce::uint32 monoVoiceChannelCount = 1;
constexpr int delayLineSafetySamples = 4;

[[nodiscard]] float clampUnitValue(const float value) noexcept {
    return std::clamp(value, chorusconfig::depthMin, chorusconfig::depthMax);
}

[[nodiscard]] float millisecondsToSamples(const float milliseconds,
                                          const double sampleRate) noexcept {
    return milliseconds * static_cast<float>(sampleRate) * 0.001f;
}
} // namespace

Chorus::Chorus(const juce::dsp::ProcessSpec& spec) {
    prepare(spec);
}

void Chorus::prepare(const juce::dsp::ProcessSpec& spec) {
    sampleRate = std::max(1.0, spec.sampleRate);

    depthSmoothed.reset(sampleRate, chorusconfig::parameterSmoothingTimeSeconds);
    rateSmoothed.reset(sampleRate, chorusconfig::parameterSmoothingTimeSeconds);
    mixSmoothed.reset(sampleRate, chorusconfig::parameterSmoothingTimeSeconds);
    depthSmoothed.setCurrentAndTargetValue(depth);
    rateSmoothed.setCurrentAndTargetValue(rateHz);
    mixSmoothed.setCurrentAndTargetValue(mix);

    prepareDryDelay(spec);
    voices[0].prepare(spec, leftVoicePhaseOffsetRadians);
    voices[1].prepare(spec, rightVoicePhaseOffsetRadians);
}

void Chorus::process(juce::dsp::AudioBlock<float>& block) {
    const auto numChannels = static_cast<int>(block.getNumChannels());
    const auto numSamples = static_cast<int>(block.getNumSamples());

    if (numChannels == 0)
        return;

    auto* left = block.getChannelPointer(0);
    auto* right = numChannels > 1 ? block.getChannelPointer(1) : left;

    for (auto sample = 0; sample < numSamples; ++sample) {
        const auto inputLeft = left[sample];
        const auto inputRight = right[sample];
        const auto monoInput = 0.5f * (inputLeft + inputRight);

        const auto smoothedDepth = getSmoothedDepth();
        const auto smoothedRateHz = getSmoothedRateHz();
        const auto smoothedMix = getSmoothedMix();

        const auto wetLeft = voices[0].processSample(monoInput, smoothedDepth, smoothedRateHz);
        const auto wetRight = voices[1].processSample(monoInput, smoothedDepth, smoothedRateHz);

        dryDelayLine.pushSample(0, inputLeft);
        const auto dryLeft =
            dryDelayLine.popSample(0, static_cast<float>(getLatencySamples()), true);

        auto dryRight = dryLeft;
        if (numChannels > 1) {
            dryDelayLine.pushSample(1, inputRight);
            dryRight = dryDelayLine.popSample(1, static_cast<float>(getLatencySamples()), true);
        }

        left[sample] = dryLeft + smoothedMix * (wetLeft - dryLeft);

        if (numChannels > 1)
            right[sample] = dryRight + smoothedMix * (wetRight - dryRight);
    }
}

void Chorus::reset() {
    for (auto& voice : voices)
        voice.reset();

    dryDelayLine.reset();
    depthSmoothed.setCurrentAndTargetValue(depth);
    rateSmoothed.setCurrentAndTargetValue(rateHz);
    mixSmoothed.setCurrentAndTargetValue(mix);
}

int Chorus::getLatencySamples() const {
    return getCenterDelaySamples(sampleRate);
}

void Chorus::setDepth(const float newDepth) noexcept {
    depth = clampUnitValue(newDepth);
    depthSmoothed.setTargetValue(depth);
}

void Chorus::setRateHz(const float newRateHz) noexcept {
    rateHz = std::clamp(newRateHz, chorusconfig::rateMinHz, chorusconfig::rateMaxHz);
    rateSmoothed.setTargetValue(rateHz);
}

void Chorus::setMix(const float newMix) noexcept {
    mix = clampUnitValue(newMix);
    mixSmoothed.setTargetValue(mix);
}

int Chorus::getCenterDelaySamples(const double sampleRate) noexcept {
    return static_cast<int>(
        std::lround(millisecondsToSamples(chorusconfig::centerDelayMs, std::max(1.0, sampleRate))));
}

float Chorus::getSmoothedDepth() noexcept {
    return depthSmoothed.getNextValue();
}

float Chorus::getSmoothedRateHz() noexcept {
    return rateSmoothed.getNextValue();
}

float Chorus::getSmoothedMix() noexcept {
    return mixSmoothed.getNextValue();
}

void Chorus::prepareDryDelay(const juce::dsp::ProcessSpec& spec) {
    const auto maxDelaySamples =
        static_cast<int>(std::ceil(millisecondsToSamples(
            chorusconfig::centerDelayMs + chorusconfig::modulationRangeMs, sampleRate))) +
        delayLineSafetySamples;

    dryDelayLine.setMaximumDelayInSamples(maxDelaySamples);
    dryDelayLine.prepare(spec);
    dryDelayLine.reset();
}

void Chorus::ChorusVoice::prepare(const juce::dsp::ProcessSpec& spec,
                                  const float phaseOffsetRadians) {
    sampleRate = std::max(1.0, spec.sampleRate);
    phaseOffset = phaseOffsetRadians;

    const auto maxDelaySamples =
        static_cast<int>(std::ceil(millisecondsToSamples(
            chorusconfig::centerDelayMs + chorusconfig::modulationRangeMs, sampleRate))) +
        delayLineSafetySamples;
    const juce::dsp::ProcessSpec monoSpec{sampleRate, spec.maximumBlockSize, monoVoiceChannelCount};

    delayLine.setMaximumDelayInSamples(maxDelaySamples);
    delayLine.prepare(monoSpec);

    lowpassCoefficient =
        1.0f - std::exp(-juce::MathConstants<float>::twoPi * chorusconfig::wetLowpassCutoffHz /
                        static_cast<float>(sampleRate));
    reset();
}

void Chorus::ChorusVoice::reset() noexcept {
    phaseRadians = phaseOffset;
    lowpassState = 0.0f;
    delayLine.reset();
}

float Chorus::ChorusVoice::processSample(const float input, const float depthAmount,
                                         const float rate) noexcept {
    const auto delaySamples = getDelaySamples(depthAmount);

    delayLine.pushSample(0, input);
    const auto delayed = delayLine.popSample(0, delaySamples, true);
    const auto wet = processWetLowpass(delayed);

    advancePhase(rate);
    return wet;
}

float Chorus::ChorusVoice::getDelaySamples(const float depthAmount) const noexcept {
    const auto lfo = std::sin(phaseRadians);
    const auto centerDelaySamples = static_cast<float>(Chorus::getCenterDelaySamples(sampleRate));
    const auto modulationRangeSamples =
        millisecondsToSamples(chorusconfig::modulationRangeMs, sampleRate);

    return centerDelaySamples + depthAmount * modulationRangeSamples * lfo;
}

float Chorus::ChorusVoice::processWetLowpass(const float sample) noexcept {
    lowpassState += lowpassCoefficient * (sample - lowpassState);
    return lowpassState;
}

void Chorus::ChorusVoice::advancePhase(const float rate) noexcept {
    phaseRadians += juce::MathConstants<float>::twoPi * rate / static_cast<float>(sampleRate);

    if (phaseRadians >= juce::MathConstants<float>::twoPi)
        phaseRadians = std::fmod(phaseRadians, juce::MathConstants<float>::twoPi);
}
