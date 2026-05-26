#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "DSP/ConvReverb.h"
#include "DSP/ConvReverbConfig.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <algorithm>
#include <cmath>
#include <limits>

namespace {
constexpr double sampleRate = 48000.0;
constexpr juce::uint32 blockSize = 8192;
constexpr juce::uint32 channelCount = 2;

void prepareReverb(ConvReverb& reverb, const juce::uint32 samples = blockSize) {
    reverb.prepare({sampleRate, samples, channelCount});
}

juce::AudioBuffer<float> makeIdentityImpulse() {
    juce::AudioBuffer<float> impulse(static_cast<int>(channelCount), 1);
    impulse.clear();
    impulse.setSample(0, 0, 1.0f);
    impulse.setSample(1, 0, 1.0f);
    return impulse;
}

void fillSine(juce::AudioBuffer<float>& buffer, const float frequencyHz,
              const float amplitude = 0.5f) {
    for (auto sample = 0; sample < buffer.getNumSamples(); ++sample) {
        const auto phase = juce::MathConstants<float>::twoPi * frequencyHz *
                           static_cast<float>(sample) / static_cast<float>(sampleRate);
        const auto value = amplitude * std::sin(phase);

        buffer.setSample(0, sample, value);
        buffer.setSample(1, sample, -value);
    }
}

float getPeakDifference(const juce::AudioBuffer<float>& a, const juce::AudioBuffer<float>& b) {
    auto peak = 0.0f;

    for (auto channel = 0; channel < a.getNumChannels(); ++channel) {
        for (auto sample = 0; sample < a.getNumSamples(); ++sample) {
            peak = std::max(peak,
                            std::abs(a.getSample(channel, sample) - b.getSample(channel, sample)));
        }
    }

    return peak;
}

float getPeakInWindow(const juce::AudioBuffer<float>& buffer, const int channel, const int start,
                      const int end) {
    auto peak = 0.0f;

    for (auto sample = std::max(0, start); sample < std::min(buffer.getNumSamples(), end); ++sample)
        peak = std::max(peak, std::abs(buffer.getSample(channel, sample)));

    return peak;
}
} // namespace

TEST_CASE("ConvReverb fallback IR loads during prepare", "[convreverb]") {
    ConvReverb reverb;
    prepareReverb(reverb);

    REQUIRE(reverb.getCurrentIRSize() == convreverbconfig::fallbackImpulseLengthSamples);
    REQUIRE(reverb.getLatencySamples() == 0);
}

TEST_CASE("ConvReverb custom identity IR produces delayed wet impulse", "[convreverb]") {
    ConvReverb reverb;
    reverb.loadImpulseResponse(makeIdentityImpulse(), sampleRate);
    prepareReverb(reverb);
    reverb.setWetMix(1.0f);
    reverb.reset();

    juce::AudioBuffer<float> buffer(static_cast<int>(channelCount), static_cast<int>(blockSize));
    buffer.clear();
    buffer.setSample(0, 0, 1.0f);
    buffer.setSample(1, 0, 1.0f);

    auto block = juce::dsp::AudioBlock<float>(buffer);
    reverb.process(block);

    const auto expectedCenter = static_cast<int>(
        std::lround(convreverbconfig::wetPathModulationCenterDelayMs * 0.001 * sampleRate));

    REQUIRE(std::abs(buffer.getSample(0, 0)) <= std::numeric_limits<float>::epsilon());
    REQUIRE(getPeakInWindow(buffer, 0, expectedCenter - 16, expectedCenter + 16) > 0.85f);
    REQUIRE(getPeakInWindow(buffer, 1, expectedCenter - 16, expectedCenter + 16) > 0.85f);
}

TEST_CASE("ConvReverb is deterministic for the same input", "[convreverb]") {
    ConvReverb first;
    ConvReverb second;
    first.loadImpulseResponse(makeIdentityImpulse(), sampleRate);
    second.loadImpulseResponse(makeIdentityImpulse(), sampleRate);
    prepareReverb(first);
    prepareReverb(second);
    first.setWetMix(0.75f);
    second.setWetMix(0.75f);

    juce::AudioBuffer<float> firstBuffer(static_cast<int>(channelCount),
                                         static_cast<int>(blockSize));
    juce::AudioBuffer<float> secondBuffer(static_cast<int>(channelCount),
                                          static_cast<int>(blockSize));
    fillSine(firstBuffer, 220.0f);
    secondBuffer.makeCopyOf(firstBuffer);

    auto firstBlock = juce::dsp::AudioBlock<float>(firstBuffer);
    auto secondBlock = juce::dsp::AudioBlock<float>(secondBuffer);
    first.process(firstBlock);
    second.process(secondBlock);

    REQUIRE(getPeakDifference(firstBuffer, secondBuffer) <= 0.000001f);
}

TEST_CASE("ConvReverb keeps silence silent", "[convreverb]") {
    ConvReverb reverb;
    prepareReverb(reverb);
    reverb.setWetMix(1.0f);
    reverb.reset();

    juce::AudioBuffer<float> buffer(static_cast<int>(channelCount), static_cast<int>(blockSize));
    buffer.clear();

    auto block = juce::dsp::AudioBlock<float>(buffer);
    reverb.process(block);

    REQUIRE(buffer.getMagnitude(0, buffer.getNumSamples()) <=
            std::numeric_limits<float>::epsilon());
}

TEST_CASE("ConvReverb zero mix preserves dry input", "[convreverb]") {
    ConvReverb reverb;
    prepareReverb(reverb);
    reverb.setWetMix(0.0f);

    juce::AudioBuffer<float> input(static_cast<int>(channelCount), static_cast<int>(blockSize));
    fillSine(input, 330.0f, 0.4f);

    juce::AudioBuffer<float> processed;
    processed.makeCopyOf(input);
    auto block = juce::dsp::AudioBlock<float>(processed);
    reverb.process(block);

    REQUIRE(getPeakDifference(input, processed) <= 0.000001f);
}

TEST_CASE("ConvReverb reset clears wet-path state", "[convreverb]") {
    ConvReverb reverb;
    reverb.loadImpulseResponse(makeIdentityImpulse(), sampleRate);
    prepareReverb(reverb);
    reverb.setWetMix(1.0f);
    reverb.reset();

    juce::AudioBuffer<float> impulse(static_cast<int>(channelCount), static_cast<int>(blockSize));
    impulse.clear();
    impulse.setSample(0, 0, 1.0f);
    impulse.setSample(1, 0, 1.0f);
    auto impulseBlock = juce::dsp::AudioBlock<float>(impulse);
    reverb.process(impulseBlock);

    reverb.reset();

    juce::AudioBuffer<float> silence(static_cast<int>(channelCount), static_cast<int>(blockSize));
    silence.clear();
    auto silenceBlock = juce::dsp::AudioBlock<float>(silence);
    reverb.process(silenceBlock);

    REQUIRE(silence.getMagnitude(0, silence.getNumSamples()) <=
            std::numeric_limits<float>::epsilon());
}

TEST_CASE("ConvReverb stays finite and bounded across wet mix sweeps", "[convreverb]") {
    ConvReverb reverb;
    reverb.loadImpulseResponse(makeIdentityImpulse(), sampleRate);
    reverb.prepare({sampleRate, 1, channelCount});

    for (auto sample = 0; sample < 12000; ++sample) {
        const auto amount = static_cast<float>(sample % 4000) / 3999.0f;
        reverb.setWetMix(amount);

        juce::AudioBuffer<float> buffer(static_cast<int>(channelCount), 1);
        const auto input =
            0.5f * std::sin(juce::MathConstants<float>::twoPi * 110.0f *
                            static_cast<float>(sample) / static_cast<float>(sampleRate));
        buffer.setSample(0, 0, input);
        buffer.setSample(1, 0, input);

        auto block = juce::dsp::AudioBlock<float>(buffer);
        reverb.process(block);

        REQUIRE(std::isfinite(buffer.getSample(0, 0)));
        REQUIRE(std::isfinite(buffer.getSample(1, 0)));
        REQUIRE(std::abs(buffer.getSample(0, 0)) < convreverbconfig::outputBoundForTests);
        REQUIRE(std::abs(buffer.getSample(1, 0)) < convreverbconfig::outputBoundForTests);
    }
}
