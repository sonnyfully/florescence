#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "DSP/Chorus.h"
#include "DSP/ChorusConfig.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <algorithm>
#include <cmath>
#include <complex>
#include <limits>
#include <vector>

namespace {
constexpr double sampleRate = 48000.0;
constexpr juce::uint32 blockSize = 8192;
constexpr juce::uint32 channelCount = 2;

void prepareChorus(Chorus& chorus, const juce::uint32 samples = blockSize) {
    chorus.prepare({sampleRate, samples, channelCount});
}

void fillSine(juce::AudioBuffer<float>& buffer, const float frequencyHz,
              const float amplitude = 0.5f) {
    for (auto sample = 0; sample < buffer.getNumSamples(); ++sample) {
        const auto phase = juce::MathConstants<float>::twoPi * frequencyHz *
                           static_cast<float>(sample) / static_cast<float>(sampleRate);
        const auto value = amplitude * std::sin(phase);

        for (auto channel = 0; channel < buffer.getNumChannels(); ++channel)
            buffer.setSample(channel, sample, value);
    }
}

void fillWhiteNoise(juce::AudioBuffer<float>& buffer, const float amplitude = 0.35f) {
    auto seed = 0x12345678u;

    for (auto sample = 0; sample < buffer.getNumSamples(); ++sample) {
        seed = 1664525u * seed + 1013904223u;
        const auto value =
            (static_cast<float>((seed >> 8) & 0xFFFFu) / 32768.0f - 1.0f) * amplitude;

        for (auto channel = 0; channel < buffer.getNumChannels(); ++channel)
            buffer.setSample(channel, sample, value);
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

float getStereoDifferenceRms(const juce::AudioBuffer<float>& buffer, const int startSample) {
    double sumSquares = 0.0;
    auto count = 0;

    for (auto sample = startSample; sample < buffer.getNumSamples(); ++sample) {
        const auto difference = buffer.getSample(0, sample) - buffer.getSample(1, sample);
        sumSquares += static_cast<double>(difference * difference);
        ++count;
    }

    return static_cast<float>(std::sqrt(sumSquares / static_cast<double>(std::max(1, count))));
}

std::complex<double> getDftBin(const std::vector<float>& samples, const double frequencyHz,
                               const int startSample, const int samplesToMeasure) {
    std::complex<double> sum{0.0, 0.0};

    for (auto index = 0; index < samplesToMeasure; ++index) {
        const auto sample = startSample + index;
        const auto phase = -juce::MathConstants<double>::twoPi * frequencyHz *
                           static_cast<double>(index) / sampleRate;
        sum += static_cast<double>(samples[static_cast<std::size_t>(sample)]) *
               std::complex<double>{std::cos(phase), std::sin(phase)};
    }

    return sum / static_cast<double>(samplesToMeasure);
}

float getBandMagnitude(const juce::AudioBuffer<float>& buffer, const float frequencyHz,
                       const int startSample, const int samplesToMeasure) {
    std::vector<float> samples(static_cast<std::size_t>(buffer.getNumSamples()));

    for (auto sample = 0; sample < buffer.getNumSamples(); ++sample)
        samples[static_cast<std::size_t>(sample)] = buffer.getSample(0, sample);

    constexpr auto binSpacingHz = 25.0f;
    auto magnitude = 0.0;

    for (auto offset = -2; offset <= 2; ++offset)
        magnitude +=
            std::abs(getDftBin(samples, frequencyHz + binSpacingHz * static_cast<float>(offset),
                               startSample, samplesToMeasure));

    return static_cast<float>(magnitude / 5.0);
}
} // namespace

TEST_CASE("Chorus is deterministic for the same input", "[chorus]") {
    Chorus first;
    Chorus second;
    prepareChorus(first);
    prepareChorus(second);
    first.setDepth(0.75f);
    first.setRateHz(0.8f);
    first.setMix(0.65f);
    second.setDepth(0.75f);
    second.setRateHz(0.8f);
    second.setMix(0.65f);

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

TEST_CASE("Chorus keeps silence silent", "[chorus]") {
    Chorus chorus;
    prepareChorus(chorus);
    chorus.setDepth(1.0f);
    chorus.setRateHz(10.0f);
    chorus.setMix(1.0f);

    juce::AudioBuffer<float> buffer(static_cast<int>(channelCount), static_cast<int>(blockSize));
    buffer.clear();

    auto block = juce::dsp::AudioBlock<float>(buffer);
    chorus.process(block);

    REQUIRE(buffer.getMagnitude(0, buffer.getNumSamples()) <=
            std::numeric_limits<float>::epsilon());
}

TEST_CASE("Chorus preserves stereo coherence at zero depth", "[chorus]") {
    Chorus chorus;
    prepareChorus(chorus);
    chorus.setDepth(0.0f);
    chorus.setRateHz(2.0f);
    chorus.setMix(1.0f);

    juce::AudioBuffer<float> buffer(static_cast<int>(channelCount), static_cast<int>(blockSize));
    fillSine(buffer, 440.0f);

    auto block = juce::dsp::AudioBlock<float>(buffer);
    chorus.process(block);

    const auto latency = chorus.getLatencySamples();
    for (auto sample = latency + 16; sample < buffer.getNumSamples(); ++sample)
        REQUIRE_THAT(buffer.getSample(0, sample),
                     Catch::Matchers::WithinAbs(buffer.getSample(1, sample), 0.000001f));
}

TEST_CASE("Chorus parameter ramps remain smooth", "[chorus]") {
    Chorus chorus;
    chorus.prepare({sampleRate, 1, channelCount});

    auto previous = 0.0f;
    auto largestDelta = 0.0f;

    for (auto sample = 0; sample < 4096; ++sample) {
        const auto amount = static_cast<float>(sample) / 4095.0f;
        chorus.setDepth(amount);
        chorus.setRateHz(chorusconfig::rateMinHz +
                         amount * (chorusconfig::rateMaxHz - chorusconfig::rateMinHz));
        chorus.setMix(amount);

        juce::AudioBuffer<float> buffer(static_cast<int>(channelCount), 1);
        const auto input =
            0.45f * std::sin(juce::MathConstants<float>::twoPi * 80.0f *
                             static_cast<float>(sample) / static_cast<float>(sampleRate));
        buffer.setSample(0, 0, input);
        buffer.setSample(1, 0, input);

        auto block = juce::dsp::AudioBlock<float>(buffer);
        chorus.process(block);

        if (sample > chorus.getLatencySamples())
            largestDelta = std::max(largestDelta, std::abs(buffer.getSample(0, 0) - previous));

        previous = buffer.getSample(0, 0);
    }

    REQUIRE(largestDelta < 0.15f);
}

TEST_CASE("Chorus latency report matches actual delay", "[chorus]") {
    Chorus chorus;
    prepareChorus(chorus);
    chorus.setDepth(0.0f);
    chorus.setRateHz(0.1f);
    chorus.setMix(0.0f);

    REQUIRE(chorus.getLatencySamples() == Chorus::getCenterDelaySamples(sampleRate));

    juce::AudioBuffer<float> buffer(static_cast<int>(channelCount), static_cast<int>(blockSize));
    buffer.clear();
    buffer.setSample(0, 0, 1.0f);
    buffer.setSample(1, 0, 1.0f);

    auto block = juce::dsp::AudioBlock<float>(buffer);
    chorus.process(block);

    const auto latency = chorus.getLatencySamples();
    REQUIRE_THAT(buffer.getSample(0, latency), Catch::Matchers::WithinAbs(1.0f, 0.000001f));
    REQUIRE_THAT(buffer.getSample(1, latency), Catch::Matchers::WithinAbs(1.0f, 0.000001f));
}

TEST_CASE("Chorus LFO remains continuous when rate changes", "[chorus]") {
    Chorus chorus;
    chorus.prepare({sampleRate, 1, channelCount});
    chorus.setDepth(1.0f);
    chorus.setRateHz(0.2f);
    chorus.setMix(1.0f);

    auto largestDelta = 0.0f;
    auto largestDeltaAroundChange = 0.0f;
    auto previous = 0.0f;

    for (auto sample = 0; sample < 5000; ++sample) {
        if (sample == 2500)
            chorus.setRateHz(8.0f);

        juce::AudioBuffer<float> buffer(static_cast<int>(channelCount), 1);
        const auto input =
            0.4f * std::sin(juce::MathConstants<float>::twoPi * 330.0f *
                            static_cast<float>(sample) / static_cast<float>(sampleRate));
        buffer.setSample(0, 0, input);
        buffer.setSample(1, 0, input);

        auto block = juce::dsp::AudioBlock<float>(buffer);
        chorus.process(block);

        if (sample > chorus.getLatencySamples()) {
            const auto delta = std::abs(buffer.getSample(0, 0) - previous);
            largestDelta = std::max(largestDelta, delta);

            if (sample >= 2495 && sample <= 2505)
                largestDeltaAroundChange = std::max(largestDeltaAroundChange, delta);
        }

        previous = buffer.getSample(0, 0);
    }

    REQUIRE(largestDeltaAroundChange < 0.12f);
    REQUIRE(largestDelta < 0.2f);
}

TEST_CASE("Chorus turns mono input into stereo width when depth is positive", "[chorus]") {
    Chorus chorus;
    prepareChorus(chorus);
    chorus.setDepth(1.0f);
    chorus.setRateHz(0.8f);
    chorus.setMix(1.0f);

    juce::AudioBuffer<float> buffer(static_cast<int>(channelCount), static_cast<int>(blockSize));
    fillSine(buffer, 440.0f);

    auto block = juce::dsp::AudioBlock<float>(buffer);
    chorus.process(block);

    REQUIRE(getStereoDifferenceRms(buffer, chorus.getLatencySamples() + 128) > 0.0005f);
}

TEST_CASE("Chorus wet path has fixed HF rolloff near 6 kHz", "[chorus]") {
    constexpr auto noiseBlockSize = 65536;

    Chorus chorus;
    chorus.prepare({sampleRate, noiseBlockSize, channelCount});
    chorus.setDepth(0.0f);
    chorus.setRateHz(0.1f);
    chorus.setMix(1.0f);

    juce::AudioBuffer<float> buffer(static_cast<int>(channelCount), noiseBlockSize);
    fillWhiteNoise(buffer);

    auto block = juce::dsp::AudioBlock<float>(buffer);
    chorus.process(block);

    const auto startSample = chorus.getLatencySamples() + 2048;
    const auto samplesToMeasure = 32768;
    const auto lowBand = getBandMagnitude(buffer, 1000.0f, startSample, samplesToMeasure);
    const auto cutoffBand =
        getBandMagnitude(buffer, chorusconfig::wetLowpassCutoffHz, startSample, samplesToMeasure);
    const auto highBand = getBandMagnitude(buffer, 12000.0f, startSample, samplesToMeasure);

    const auto cutoffRelativeDb = juce::Decibels::gainToDecibels(cutoffBand / lowBand);
    const auto highRelativeDb = juce::Decibels::gainToDecibels(highBand / lowBand);

    REQUIRE(cutoffRelativeDb < -1.0f);
    REQUIRE(cutoffRelativeDb > -6.0f);
    REQUIRE(highRelativeDb < cutoffRelativeDb - 3.0f);
}

TEST_CASE("Chorus stays finite and bounded across parameter sweeps", "[chorus]") {
    Chorus chorus;
    chorus.prepare({sampleRate, 1, channelCount});

    for (auto sample = 0; sample < 12000; ++sample) {
        const auto amount = static_cast<float>(sample % 4000) / 3999.0f;
        chorus.setDepth(amount);
        chorus.setRateHz(chorusconfig::rateMinHz +
                         amount * (chorusconfig::rateMaxHz - chorusconfig::rateMinHz));
        chorus.setMix(1.0f - amount);

        juce::AudioBuffer<float> buffer(static_cast<int>(channelCount), 1);
        const auto input =
            0.95f * std::sin(juce::MathConstants<float>::twoPi * 997.0f *
                             static_cast<float>(sample) / static_cast<float>(sampleRate));
        buffer.setSample(0, 0, input);
        buffer.setSample(1, 0, -input);

        auto block = juce::dsp::AudioBlock<float>(buffer);
        chorus.process(block);

        for (auto channel = 0; channel < buffer.getNumChannels(); ++channel) {
            const auto value = buffer.getSample(channel, 0);
            REQUIRE(std::isfinite(value));
            REQUIRE(std::abs(value) < chorusconfig::outputBoundForTests);
        }
    }
}

TEST_CASE("Chorus mono sum keeps useful level", "[chorus]") {
    Chorus chorus;
    prepareChorus(chorus);
    chorus.setDepth(1.0f);
    chorus.setRateHz(0.8f);
    chorus.setMix(1.0f);

    juce::AudioBuffer<float> buffer(static_cast<int>(channelCount), static_cast<int>(blockSize));
    fillSine(buffer, 440.0f);

    auto block = juce::dsp::AudioBlock<float>(buffer);
    chorus.process(block);

    double stereoEnergy = 0.0;
    double monoEnergy = 0.0;
    const auto startSample = chorus.getLatencySamples() + 128;

    for (auto sample = startSample; sample < buffer.getNumSamples(); ++sample) {
        const auto left = buffer.getSample(0, sample);
        const auto right = buffer.getSample(1, sample);
        stereoEnergy += static_cast<double>(left * left + right * right);

        const auto mono = 0.5f * (left + right);
        monoEnergy += static_cast<double>(mono * mono);
    }

    const auto monoRelativeDb =
        juce::Decibels::gainToDecibels(std::sqrt(monoEnergy / (0.5 * stereoEnergy)));

    REQUIRE(monoRelativeDb > -9.0);
}
