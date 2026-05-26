#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "DSP/Delay.h"
#include "DSP/DelayConfig.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <algorithm>
#include <cmath>
#include <limits>

namespace {
constexpr double sampleRate = 48000.0;
constexpr juce::uint32 channelCount = 2;

void prepareDelay(Delay& delay, const juce::uint32 samples) {
    delay.prepare({sampleRate, samples, channelCount});
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

juce::AudioBuffer<float> makeImpulseBuffer(const int samples) {
    juce::AudioBuffer<float> buffer(static_cast<int>(channelCount), samples);
    buffer.clear();
    buffer.setSample(0, 0, 1.0f);
    buffer.setSample(1, 0, 1.0f);
    return buffer;
}
} // namespace

TEST_CASE("Delay exposes the resolved Stage 4 sync divisions", "[delay]") {
    REQUIRE(Delay::getSyncDivisionCount() == 9);
    REQUIRE(Delay::getDefaultSyncDivision() == Delay::SyncDivision::DottedEighth);

    REQUIRE_THAT(Delay::getSyncDivisionBeats(Delay::SyncDivision::Quarter),
                 Catch::Matchers::WithinAbs(1.0f, 0.000001f));
    REQUIRE_THAT(Delay::getSyncDivisionBeats(Delay::SyncDivision::DottedQuarter),
                 Catch::Matchers::WithinAbs(1.5f, 0.000001f));
    REQUIRE_THAT(Delay::getSyncDivisionBeats(Delay::SyncDivision::QuarterTriplet),
                 Catch::Matchers::WithinAbs(2.0f / 3.0f, 0.000001f));
    REQUIRE_THAT(Delay::getSyncDivisionBeats(Delay::SyncDivision::DottedEighth),
                 Catch::Matchers::WithinAbs(0.75f, 0.000001f));
    REQUIRE_THAT(Delay::getSyncDivisionBeats(Delay::SyncDivision::SixteenthTriplet),
                 Catch::Matchers::WithinAbs(1.0f / 6.0f, 0.000001f));
}

TEST_CASE("Delay dotted eighth at 120 bpm lands at the expected sample", "[delay]") {
    const auto delaySamples = static_cast<int>(std::lround(Delay::getDelaySamplesForDivision(
        Delay::SyncDivision::DottedEighth, delayconfig::tempoDefaultBpm, sampleRate)));
    const auto bufferSamples = delaySamples + 128;

    Delay delay;
    prepareDelay(delay, static_cast<juce::uint32>(bufferSamples));
    delay.setMix(1.0f);
    delay.setFeedback(0.0f);
    delay.reset();

    auto buffer = makeImpulseBuffer(bufferSamples);
    auto block = juce::dsp::AudioBlock<float>(buffer);
    delay.process(block);

    REQUIRE(delaySamples == 18000);
    REQUIRE_THAT(buffer.getSample(0, delaySamples), Catch::Matchers::WithinAbs(1.0f, 0.000001f));
    REQUIRE_THAT(buffer.getSample(1, delaySamples), Catch::Matchers::WithinAbs(1.0f, 0.000001f));
    REQUIRE(std::abs(buffer.getSample(0, 0)) <= std::numeric_limits<float>::epsilon());
}

TEST_CASE("Delay is deterministic for the same input", "[delay]") {
    constexpr auto bufferSamples = 40960;

    Delay first;
    Delay second;
    prepareDelay(first, bufferSamples);
    prepareDelay(second, bufferSamples);
    first.setMix(0.75f);
    first.setFeedback(0.35f);
    second.setMix(0.75f);
    second.setFeedback(0.35f);

    auto firstBuffer = makeImpulseBuffer(bufferSamples);
    auto secondBuffer = makeImpulseBuffer(bufferSamples);

    auto firstBlock = juce::dsp::AudioBlock<float>(firstBuffer);
    auto secondBlock = juce::dsp::AudioBlock<float>(secondBuffer);
    first.process(firstBlock);
    second.process(secondBlock);

    REQUIRE(getPeakDifference(firstBuffer, secondBuffer) <= 0.000001f);
}

TEST_CASE("Delay keeps silence silent", "[delay]") {
    constexpr auto bufferSamples = 40960;

    Delay delay;
    prepareDelay(delay, bufferSamples);
    delay.setMix(1.0f);
    delay.setFeedback(delayconfig::feedbackMax);

    juce::AudioBuffer<float> buffer(static_cast<int>(channelCount), bufferSamples);
    buffer.clear();

    auto block = juce::dsp::AudioBlock<float>(buffer);
    delay.process(block);

    REQUIRE(buffer.getMagnitude(0, buffer.getNumSamples()) <=
            std::numeric_limits<float>::epsilon());
}

TEST_CASE("Delay zero mix preserves dry input", "[delay]") {
    constexpr auto bufferSamples = 4096;

    Delay delay;
    prepareDelay(delay, bufferSamples);
    delay.setMix(0.0f);
    delay.setFeedback(delayconfig::feedbackMax);

    juce::AudioBuffer<float> input(static_cast<int>(channelCount), bufferSamples);
    for (auto sample = 0; sample < bufferSamples; ++sample) {
        const auto value =
            0.4f * std::sin(juce::MathConstants<float>::twoPi * 440.0f *
                            static_cast<float>(sample) / static_cast<float>(sampleRate));
        input.setSample(0, sample, value);
        input.setSample(1, sample, -value);
    }

    juce::AudioBuffer<float> processed;
    processed.makeCopyOf(input);
    auto block = juce::dsp::AudioBlock<float>(processed);
    delay.process(block);

    REQUIRE(getPeakDifference(input, processed) <= 0.000001f);
}

TEST_CASE("Delay stereo topology keeps channel feedback independent", "[delay]") {
    const auto delaySamples = static_cast<int>(std::lround(Delay::getDelaySamplesForDivision(
        Delay::SyncDivision::DottedEighth, delayconfig::tempoDefaultBpm, sampleRate)));
    const auto bufferSamples = delaySamples * 2 + 128;

    Delay delay;
    prepareDelay(delay, static_cast<juce::uint32>(bufferSamples));
    delay.setMix(1.0f);
    delay.setFeedback(0.5f);
    delay.setTopology(Delay::Topology::Stereo);

    juce::AudioBuffer<float> buffer(static_cast<int>(channelCount), bufferSamples);
    buffer.clear();
    buffer.setSample(0, 0, 1.0f);

    auto block = juce::dsp::AudioBlock<float>(buffer);
    delay.process(block);

    REQUIRE(buffer.getSample(0, delaySamples) > 0.99f);
    REQUIRE(std::abs(buffer.getSample(1, delaySamples)) <= 0.000001f);
    REQUIRE(std::abs(buffer.getSample(1, delaySamples * 2)) <= 0.000001f);
}

TEST_CASE("Delay ping-pong topology crossfeeds repeats", "[delay]") {
    const auto delaySamples = static_cast<int>(std::lround(Delay::getDelaySamplesForDivision(
        Delay::SyncDivision::DottedEighth, delayconfig::tempoDefaultBpm, sampleRate)));
    const auto bufferSamples = delaySamples * 2 + 128;

    Delay delay;
    prepareDelay(delay, static_cast<juce::uint32>(bufferSamples));
    delay.setMix(1.0f);
    delay.setFeedback(0.5f);
    delay.setTopology(Delay::Topology::PingPong);

    juce::AudioBuffer<float> buffer(static_cast<int>(channelCount), bufferSamples);
    buffer.clear();
    buffer.setSample(0, 0, 1.0f);

    auto block = juce::dsp::AudioBlock<float>(buffer);
    delay.process(block);

    REQUIRE(buffer.getSample(0, delaySamples) > 0.99f);
    REQUIRE(std::abs(buffer.getSample(1, delaySamples)) <= 0.000001f);
    REQUIRE(buffer.getSample(1, delaySamples * 2) > 0.05f);
    REQUIRE(buffer.getSample(1, delaySamples * 2) < 0.5f);
}

TEST_CASE("Delay reset clears repeats", "[delay]") {
    const auto delaySamples = static_cast<int>(std::lround(Delay::getDelaySamplesForDivision(
        Delay::SyncDivision::DottedEighth, delayconfig::tempoDefaultBpm, sampleRate)));
    const auto bufferSamples = delaySamples + 128;

    Delay delay;
    prepareDelay(delay, static_cast<juce::uint32>(bufferSamples));
    delay.setMix(1.0f);

    auto impulse = makeImpulseBuffer(bufferSamples);
    auto impulseBlock = juce::dsp::AudioBlock<float>(impulse);
    delay.process(impulseBlock);
    REQUIRE(impulse.getSample(0, delaySamples) > 0.99f);

    delay.reset();

    juce::AudioBuffer<float> silence(static_cast<int>(channelCount), bufferSamples);
    silence.clear();
    auto silenceBlock = juce::dsp::AudioBlock<float>(silence);
    delay.process(silenceBlock);

    REQUIRE(silence.getMagnitude(0, silence.getNumSamples()) <=
            std::numeric_limits<float>::epsilon());
}

TEST_CASE("Delay reports zero latency", "[delay]") {
    Delay delay;
    prepareDelay(delay, 512);

    REQUIRE(delay.getLatencySamples() == 0);
}

TEST_CASE("Delay stays finite and bounded across parameter sweeps", "[delay]") {
    Delay delay;
    delay.prepare({sampleRate, 1, channelCount});

    for (auto sample = 0; sample < 60000; ++sample) {
        const auto amount = static_cast<float>(sample % 4000) / 3999.0f;
        delay.setMix(amount);
        delay.setFeedback(amount * delayconfig::feedbackMax);
        delay.setTempoBpm(60.0 + 120.0 * static_cast<double>(amount));
        delay.setSyncDivision(Delay::syncDivisionFromIndex(sample % Delay::getSyncDivisionCount()));
        delay.setTopology(sample % 2 == 0 ? Delay::Topology::Stereo : Delay::Topology::PingPong);

        juce::AudioBuffer<float> buffer(static_cast<int>(channelCount), 1);
        const auto input =
            0.5f * std::sin(juce::MathConstants<float>::twoPi * 110.0f *
                            static_cast<float>(sample) / static_cast<float>(sampleRate));
        buffer.setSample(0, 0, input);
        buffer.setSample(1, 0, -input);

        auto block = juce::dsp::AudioBlock<float>(buffer);
        delay.process(block);

        REQUIRE(std::isfinite(buffer.getSample(0, 0)));
        REQUIRE(std::isfinite(buffer.getSample(1, 0)));
        REQUIRE(std::abs(buffer.getSample(0, 0)) < delayconfig::outputBoundForTests);
        REQUIRE(std::abs(buffer.getSample(1, 0)) < delayconfig::outputBoundForTests);
    }
}
