#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "DSP/TiltEQ.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <cmath>

namespace {
constexpr double sampleRate = 48000.0;
constexpr juce::uint32 blockSize = 512;
constexpr juce::uint32 channelCount = 2;

void prepareTiltEQ(TiltEQ& tiltEq) {
    tiltEq.prepare({sampleRate, blockSize, channelCount});
}
} // namespace

TEST_CASE("Tilt EQ is flat at zero tilt") {
    TiltEQ tiltEq;
    prepareTiltEQ(tiltEq);
    tiltEq.setTiltDb(0.0f);

    REQUIRE_THAT(tiltEq.getMagnitudeDb(100.0), Catch::Matchers::WithinAbs(0.0, 0.001));
    REQUIRE_THAT(tiltEq.getMagnitudeDb(1000.0), Catch::Matchers::WithinAbs(0.0, 0.001));
    REQUIRE_THAT(tiltEq.getMagnitudeDb(10000.0), Catch::Matchers::WithinAbs(0.0, 0.001));
}

TEST_CASE("Positive Tilt EQ amount brightens highs and attenuates lows") {
    TiltEQ tiltEq;
    prepareTiltEQ(tiltEq);
    tiltEq.setTiltDb(12.0f);

    const auto lowMagnitudeDb = tiltEq.getMagnitudeDb(100.0);
    const auto highMagnitudeDb = tiltEq.getMagnitudeDb(10000.0);

    REQUIRE(lowMagnitudeDb < -4.0);
    REQUIRE(highMagnitudeDb > 4.0);
    REQUIRE(highMagnitudeDb - lowMagnitudeDb > 8.0);
}

TEST_CASE("Negative Tilt EQ amount darkens highs and lifts lows") {
    TiltEQ tiltEq;
    prepareTiltEQ(tiltEq);
    tiltEq.setTiltDb(-12.0f);

    const auto lowMagnitudeDb = tiltEq.getMagnitudeDb(100.0);
    const auto highMagnitudeDb = tiltEq.getMagnitudeDb(10000.0);

    REQUIRE(lowMagnitudeDb > 4.0);
    REQUIRE(highMagnitudeDb < -4.0);
    REQUIRE(lowMagnitudeDb - highMagnitudeDb > 8.0);
}

TEST_CASE("Tilt EQ preserves stereo coherence for identical input") {
    TiltEQ tiltEq;
    prepareTiltEQ(tiltEq);
    tiltEq.setTiltDb(8.0f);

    juce::AudioBuffer<float> buffer(static_cast<int>(channelCount), static_cast<int>(blockSize));

    for (auto sample = 0; sample < buffer.getNumSamples(); ++sample) {
        const auto value = std::sin(static_cast<float>(sample) * 0.05f);
        buffer.setSample(0, sample, value);
        buffer.setSample(1, sample, value);
    }

    auto block = juce::dsp::AudioBlock<float>(buffer);
    tiltEq.process(block);

    for (auto sample = 0; sample < buffer.getNumSamples(); ++sample)
        REQUIRE_THAT(buffer.getSample(0, sample),
                     Catch::Matchers::WithinAbs(buffer.getSample(1, sample), 0.000001f));
}
