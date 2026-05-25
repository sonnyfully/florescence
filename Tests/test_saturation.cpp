#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "DSP/Saturation.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <algorithm>
#include <cmath>
#include <vector>

namespace {
constexpr double sampleRate = 48000.0;
constexpr juce::uint32 blockSize = 8192;
constexpr juce::uint32 channelCount = 2;

void prepareSaturation(Saturation& saturation) {
    saturation.prepare({sampleRate, blockSize, channelCount});
}

void fillSine(juce::AudioBuffer<float>& buffer, const float frequencyHz,
              const float amplitude = 0.8f) {
    for (auto sample = 0; sample < buffer.getNumSamples(); ++sample) {
        const auto phase = juce::MathConstants<float>::twoPi * frequencyHz *
                           static_cast<float>(sample) / static_cast<float>(sampleRate);
        const auto value = amplitude * std::sin(phase);

        for (auto channel = 0; channel < buffer.getNumChannels(); ++channel)
            buffer.setSample(channel, sample, value);
    }
}

float getRms(const juce::AudioBuffer<float>& buffer, const int channel, const int startSample) {
    double sumSquares = 0.0;
    const auto samplesToMeasure = buffer.getNumSamples() - startSample;

    for (auto sample = startSample; sample < buffer.getNumSamples(); ++sample) {
        const auto value = buffer.getSample(channel, sample);
        sumSquares += static_cast<double>(value * value);
    }

    return static_cast<float>(std::sqrt(sumSquares / static_cast<double>(samplesToMeasure)));
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
} // namespace

TEST_CASE("Saturation tanh shaper is transparent at zero drive") {
    REQUIRE_THAT(Saturation::shapeSample(0.75f, 0.0f),
                 Catch::Matchers::WithinAbs(0.75f, 0.000001f));
    REQUIRE_THAT(Saturation::shapeSample(-0.5f, 0.0f),
                 Catch::Matchers::WithinAbs(-0.5f, 0.000001f));
}

TEST_CASE("Saturation dynamic cutoff follows the exponential range") {
    REQUIRE_THAT(Saturation::getDynamicCutoffHz(0.0f),
                 Catch::Matchers::WithinAbs(saturationconfig::lpfMaxCutoffHz, 0.001f));
    REQUIRE_THAT(Saturation::getDynamicCutoffHz(1.0f),
                 Catch::Matchers::WithinAbs(saturationconfig::lpfMinCutoffHz, 0.001f));

    const auto midpoint = Saturation::getDynamicCutoffHz(0.5f);
    REQUIRE(midpoint < saturationconfig::lpfMaxCutoffHz);
    REQUIRE(midpoint > saturationconfig::lpfMinCutoffHz);
}

TEST_CASE("Saturation is transparent at drive zero on broadband signal") {
    Saturation saturation;
    prepareSaturation(saturation);
    saturation.setDrive(0.0f);

    juce::AudioBuffer<float> buffer(static_cast<int>(channelCount), static_cast<int>(blockSize));
    juce::AudioBuffer<float> reference(static_cast<int>(channelCount), static_cast<int>(blockSize));

    auto seed = 0x12345678u;
    for (auto sample = 0; sample < buffer.getNumSamples(); ++sample) {
        seed = 1664525u * seed + 1013904223u;
        const auto value = (static_cast<float>((seed >> 8) & 0xFFFFu) / 32768.0f - 1.0f) * 0.5f;

        for (auto channel = 0; channel < buffer.getNumChannels(); ++channel)
            buffer.setSample(channel, sample, value);
    }

    reference.makeCopyOf(buffer);

    auto block = juce::dsp::AudioBlock<float>(buffer);
    saturation.process(block);

    const auto peakDifference = getPeakDifference(buffer, reference);
    const auto allowedDifference =
        juce::Decibels::decibelsToGain(saturationconfig::transparencyToleranceDb);
    REQUIRE(peakDifference <= allowedDifference);
}

TEST_CASE("Saturation keeps silence silent") {
    Saturation saturation;
    prepareSaturation(saturation);
    saturation.setDrive(1.0f);

    juce::AudioBuffer<float> buffer(static_cast<int>(channelCount), static_cast<int>(blockSize));
    buffer.clear();

    auto block = juce::dsp::AudioBlock<float>(buffer);
    saturation.process(block);

    REQUIRE(buffer.getMagnitude(0, buffer.getNumSamples()) <= 0.000001f);
}

TEST_CASE("Saturation DC blocker removes constant bias under drive") {
    Saturation saturation;
    prepareSaturation(saturation);
    saturation.setDrive(1.0f);

    juce::AudioBuffer<float> buffer(static_cast<int>(channelCount), static_cast<int>(blockSize));
    buffer.clear();

    for (auto sample = 0; sample < buffer.getNumSamples(); ++sample) {
        for (auto channel = 0; channel < buffer.getNumChannels(); ++channel)
            buffer.setSample(channel, sample, 0.25f);
    }

    auto block = juce::dsp::AudioBlock<float>(buffer);
    saturation.process(block);

    REQUIRE(std::abs(buffer.getSample(0, buffer.getNumSamples() - 1)) < 0.01f);
    REQUIRE(std::abs(buffer.getSample(1, buffer.getNumSamples() - 1)) < 0.01f);
}

TEST_CASE("Saturation is deterministic for the same input") {
    Saturation first;
    Saturation second;
    prepareSaturation(first);
    prepareSaturation(second);
    first.setDrive(0.75f);
    second.setDrive(0.75f);

    juce::AudioBuffer<float> firstBuffer(static_cast<int>(channelCount),
                                         static_cast<int>(blockSize));
    juce::AudioBuffer<float> secondBuffer(static_cast<int>(channelCount),
                                          static_cast<int>(blockSize));
    fillSine(firstBuffer, 1000.0f);
    secondBuffer.makeCopyOf(firstBuffer);

    auto firstBlock = juce::dsp::AudioBlock<float>(firstBuffer);
    auto secondBlock = juce::dsp::AudioBlock<float>(secondBuffer);
    first.process(firstBlock);
    second.process(secondBlock);

    REQUIRE(getPeakDifference(firstBuffer, secondBuffer) <= 0.000001f);
}

TEST_CASE("Saturation preserves stereo coherence for identical input") {
    Saturation saturation;
    prepareSaturation(saturation);
    saturation.setDrive(1.0f);

    juce::AudioBuffer<float> buffer(static_cast<int>(channelCount), static_cast<int>(blockSize));
    fillSine(buffer, 440.0f);

    auto block = juce::dsp::AudioBlock<float>(buffer);
    saturation.process(block);

    for (auto sample = 0; sample < buffer.getNumSamples(); ++sample)
        REQUIRE_THAT(buffer.getSample(0, sample),
                     Catch::Matchers::WithinAbs(buffer.getSample(1, sample), 0.000001f));
}

TEST_CASE("Saturation drive reduces high-frequency level more than low-frequency level") {
    Saturation lowBand;
    Saturation highBand;
    prepareSaturation(lowBand);
    prepareSaturation(highBand);
    lowBand.setDrive(1.0f);
    highBand.setDrive(1.0f);

    juce::AudioBuffer<float> lowBuffer(static_cast<int>(channelCount), static_cast<int>(blockSize));
    juce::AudioBuffer<float> highBuffer(static_cast<int>(channelCount),
                                        static_cast<int>(blockSize));
    fillSine(lowBuffer, 1000.0f, 0.95f);
    fillSine(highBuffer, 12000.0f, 0.95f);

    const auto lowInputRms = getRms(lowBuffer, 0, static_cast<int>(blockSize / 2));
    const auto highInputRms = getRms(highBuffer, 0, static_cast<int>(blockSize / 2));

    auto lowBlock = juce::dsp::AudioBlock<float>(lowBuffer);
    auto highBlock = juce::dsp::AudioBlock<float>(highBuffer);
    lowBand.process(lowBlock);
    highBand.process(highBlock);

    const auto lowRatio = getRms(lowBuffer, 0, static_cast<int>(blockSize / 2)) / lowInputRms;
    const auto highRatio = getRms(highBuffer, 0, static_cast<int>(blockSize / 2)) / highInputRms;

    REQUIRE(highRatio < lowRatio * 0.85f);
}
