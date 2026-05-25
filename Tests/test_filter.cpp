#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "DSP/Filter.h"
#include "DSP/FilterConfig.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <algorithm>
#include <cmath>
#include <complex>
#include <limits>

namespace {
constexpr double sampleRate = 48000.0;
constexpr juce::uint32 blockSize = 8192;
constexpr juce::uint32 channelCount = 2;

void prepareFilter(Filter& filter, const juce::uint32 samples = blockSize) {
    filter.prepare({sampleRate, samples, channelCount});
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

float getDftMagnitude(const juce::AudioBuffer<float>& buffer, const int channel,
                      const float frequencyHz, const int startSample, const int samplesToMeasure) {
    std::complex<double> sum{0.0, 0.0};

    for (auto index = 0; index < samplesToMeasure; ++index) {
        const auto sample = startSample + index;
        const auto phase = -juce::MathConstants<double>::twoPi * frequencyHz *
                           static_cast<double>(index) / sampleRate;
        const auto value = static_cast<double>(buffer.getSample(channel, sample));
        sum += value * std::complex<double>{std::cos(phase), std::sin(phase)};
    }

    return static_cast<float>(std::abs(sum) / static_cast<double>(samplesToMeasure));
}
} // namespace

TEST_CASE("Filter modulation cutoff follows octave depth") {
    REQUIRE_THAT(Filter::getModulatedCutoffHz(8000.0f, 0.0f, 0.5f, sampleRate),
                 Catch::Matchers::WithinAbs(8000.0f, 0.001f));

    REQUIRE_THAT(Filter::getModulatedCutoffHz(8000.0f, 1.0f, 0.5f, sampleRate),
                 Catch::Matchers::WithinAbs(8000.0f / std::sqrt(2.0f), 0.01f));
}

TEST_CASE("Filter is deterministic for the same input", "[filter]") {
    Filter first;
    Filter second;
    prepareFilter(first);
    prepareFilter(second);
    first.setCutoffHz(3000.0f);
    first.setResonance(0.7f);
    first.setEnvelopeDepthOctaves(0.75f);
    second.setCutoffHz(3000.0f);
    second.setResonance(0.7f);
    second.setEnvelopeDepthOctaves(0.75f);

    juce::AudioBuffer<float> firstBuffer(static_cast<int>(channelCount),
                                         static_cast<int>(blockSize));
    juce::AudioBuffer<float> secondBuffer(static_cast<int>(channelCount),
                                          static_cast<int>(blockSize));
    fillSine(firstBuffer, 440.0f, 0.65f);
    secondBuffer.makeCopyOf(firstBuffer);

    auto firstBlock = juce::dsp::AudioBlock<float>(firstBuffer);
    auto secondBlock = juce::dsp::AudioBlock<float>(secondBuffer);
    first.process(firstBlock);
    second.process(secondBlock);

    REQUIRE(getPeakDifference(firstBuffer, secondBuffer) <= 0.000001f);
}

TEST_CASE("Filter keeps silence silent", "[filter]") {
    Filter filter;
    prepareFilter(filter);
    filter.setCutoffHz(1000.0f);
    filter.setResonance(1.0f);
    filter.setEnvelopeDepthOctaves(2.0f);

    juce::AudioBuffer<float> buffer(static_cast<int>(channelCount), static_cast<int>(blockSize));
    buffer.clear();

    auto block = juce::dsp::AudioBlock<float>(buffer);
    filter.process(block);

    REQUIRE(buffer.getMagnitude(0, buffer.getNumSamples()) <=
            std::numeric_limits<float>::epsilon());
}

TEST_CASE("Filter preserves stereo coherence for identical input", "[filter]") {
    Filter filter;
    prepareFilter(filter);
    filter.setCutoffHz(3000.0f);
    filter.setResonance(0.5f);
    filter.setEnvelopeDepthOctaves(0.5f);

    juce::AudioBuffer<float> buffer(static_cast<int>(channelCount), static_cast<int>(blockSize));
    fillSine(buffer, 700.0f, 0.5f);

    auto block = juce::dsp::AudioBlock<float>(buffer);
    filter.process(block);

    for (auto sample = 128; sample < buffer.getNumSamples(); ++sample)
        REQUIRE_THAT(buffer.getSample(0, sample),
                     Catch::Matchers::WithinAbs(buffer.getSample(1, sample), 0.000001f));
}

TEST_CASE("Filter low-pass attenuates high frequencies more than lows", "[filter]") {
    constexpr auto analysisBlockSize = 32768;

    Filter lowFilter;
    Filter highFilter;
    lowFilter.prepare({sampleRate, analysisBlockSize, channelCount});
    highFilter.prepare({sampleRate, analysisBlockSize, channelCount});
    lowFilter.setCutoffHz(1000.0f);
    lowFilter.setResonance(0.5f);
    lowFilter.setEnvelopeDepthOctaves(0.0f);
    highFilter.setCutoffHz(16000.0f);
    highFilter.setResonance(0.5f);
    highFilter.setEnvelopeDepthOctaves(0.0f);

    juce::AudioBuffer<float> lowBuffer(static_cast<int>(channelCount), analysisBlockSize);
    juce::AudioBuffer<float> highBuffer(static_cast<int>(channelCount), analysisBlockSize);

    for (auto sample = 0; sample < analysisBlockSize; ++sample) {
        const auto low =
            0.3f * std::sin(juce::MathConstants<float>::twoPi * 250.0f *
                            static_cast<float>(sample) / static_cast<float>(sampleRate));
        const auto high =
            0.3f * std::sin(juce::MathConstants<float>::twoPi * 8000.0f *
                            static_cast<float>(sample) / static_cast<float>(sampleRate));
        const auto value = low + high;
        lowBuffer.setSample(0, sample, value);
        lowBuffer.setSample(1, sample, value);
    }
    highBuffer.makeCopyOf(lowBuffer);

    auto lowBlock = juce::dsp::AudioBlock<float>(lowBuffer);
    auto highBlock = juce::dsp::AudioBlock<float>(highBuffer);
    lowFilter.process(lowBlock);
    highFilter.process(highBlock);

    constexpr auto startSample = 4096;
    constexpr auto samplesToMeasure = 16384;
    const auto lowFilterLow = getDftMagnitude(lowBuffer, 0, 250.0f, startSample, samplesToMeasure);
    const auto lowFilterHigh =
        getDftMagnitude(lowBuffer, 0, 8000.0f, startSample, samplesToMeasure);
    const auto highFilterLow =
        getDftMagnitude(highBuffer, 0, 250.0f, startSample, samplesToMeasure);
    const auto highFilterHigh =
        getDftMagnitude(highBuffer, 0, 8000.0f, startSample, samplesToMeasure);

    REQUIRE(lowFilterHigh / lowFilterLow < 0.25f);
    REQUIRE(highFilterHigh / highFilterLow > lowFilterHigh / lowFilterLow);
}

TEST_CASE("Filter envelope depth ducks high-frequency output under loud input", "[filter]") {
    constexpr auto analysisBlockSize = 32768;

    Filter staticFilter;
    Filter modulatedFilter;
    staticFilter.prepare({sampleRate, analysisBlockSize, channelCount});
    modulatedFilter.prepare({sampleRate, analysisBlockSize, channelCount});
    staticFilter.setCutoffHz(9000.0f);
    staticFilter.setEnvelopeDepthOctaves(0.0f);
    modulatedFilter.setCutoffHz(9000.0f);
    modulatedFilter.setEnvelopeDepthOctaves(2.0f);

    juce::AudioBuffer<float> staticBuffer(static_cast<int>(channelCount), analysisBlockSize);
    juce::AudioBuffer<float> modulatedBuffer(static_cast<int>(channelCount), analysisBlockSize);
    fillSine(staticBuffer, 7000.0f, 0.95f);
    modulatedBuffer.makeCopyOf(staticBuffer);

    auto staticBlock = juce::dsp::AudioBlock<float>(staticBuffer);
    auto modulatedBlock = juce::dsp::AudioBlock<float>(modulatedBuffer);
    staticFilter.process(staticBlock);
    modulatedFilter.process(modulatedBlock);

    const auto staticRms = getRms(staticBuffer, 0, 4096);
    const auto modulatedRms = getRms(modulatedBuffer, 0, 4096);

    REQUIRE(modulatedRms < staticRms * 0.85f);
}

TEST_CASE("Filter parameter ramps remain smooth", "[filter]") {
    Filter filter;
    filter.prepare({sampleRate, 1, channelCount});

    auto previous = 0.0f;
    auto largestDelta = 0.0f;

    for (auto sample = 0; sample < 4096; ++sample) {
        const auto amount = static_cast<float>(sample) / 4095.0f;
        filter.setCutoffHz(filterconfig::cutoffMinHz +
                           amount * (filterconfig::cutoffMaxHz - filterconfig::cutoffMinHz));
        filter.setResonance(filterconfig::resonanceMin +
                            amount * (filterconfig::resonanceMax - filterconfig::resonanceMin));
        filter.setEnvelopeDepthOctaves(amount * filterconfig::envelopeDepthMaxOctaves);

        juce::AudioBuffer<float> buffer(static_cast<int>(channelCount), 1);
        const auto input =
            0.45f * std::sin(juce::MathConstants<float>::twoPi * 100.0f *
                             static_cast<float>(sample) / static_cast<float>(sampleRate));
        buffer.setSample(0, 0, input);
        buffer.setSample(1, 0, input);

        auto block = juce::dsp::AudioBlock<float>(buffer);
        filter.process(block);

        if (sample > 64)
            largestDelta = std::max(largestDelta, std::abs(buffer.getSample(0, 0) - previous));

        previous = buffer.getSample(0, 0);
    }

    REQUIRE(largestDelta < 0.15f);
}

TEST_CASE("Filter reports zero latency", "[filter]") {
    Filter filter;
    prepareFilter(filter);

    REQUIRE(filter.getLatencySamples() == 0);
}

TEST_CASE("Filter stays finite and bounded across parameter sweeps", "[filter]") {
    Filter filter;
    filter.prepare({sampleRate, 1, channelCount});

    for (auto sample = 0; sample < 12000; ++sample) {
        const auto amount = static_cast<float>(sample % 4000) / 3999.0f;
        filter.setCutoffHz(filterconfig::cutoffMinHz +
                           amount * (filterconfig::cutoffMaxHz - filterconfig::cutoffMinHz));
        filter.setResonance(filterconfig::resonanceMin +
                            amount * (filterconfig::resonanceMax - filterconfig::resonanceMin));
        filter.setEnvelopeDepthOctaves(amount * filterconfig::envelopeDepthMaxOctaves);

        juce::AudioBuffer<float> buffer(static_cast<int>(channelCount), 1);
        const auto input =
            0.95f * std::sin(juce::MathConstants<float>::twoPi * 997.0f *
                             static_cast<float>(sample) / static_cast<float>(sampleRate));
        buffer.setSample(0, 0, input);
        buffer.setSample(1, 0, -input);

        auto block = juce::dsp::AudioBlock<float>(buffer);
        filter.process(block);

        for (auto channel = 0; channel < buffer.getNumChannels(); ++channel) {
            const auto value = buffer.getSample(channel, 0);
            REQUIRE(std::isfinite(value));
            REQUIRE(std::abs(value) < filterconfig::outputBoundForTests);
        }
    }
}
