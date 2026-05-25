#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "DSP/JilesAtherton.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace {
constexpr double sampleRate = 48000.0;
constexpr auto sampleCount = 8192;

void fillSine(std::vector<float>& buffer, const float frequencyHz, const float amplitude,
              const double rate = sampleRate) {
    for (std::size_t sample = 0; sample < buffer.size(); ++sample) {
        const auto phase = 2.0f * 3.14159265358979323846f * frequencyHz *
                           static_cast<float>(sample) / static_cast<float>(rate);
        buffer[sample] = amplitude * std::sin(phase);
    }
}

std::vector<float> processBuffer(const std::vector<float>& input, const float drive,
                                 const double rate = sampleRate) {
    JilesAtherton model;
    model.prepare(rate);

    std::vector<float> output(input.size());
    for (std::size_t sample = 0; sample < input.size(); ++sample)
        output[sample] = model.processSample(input[sample], drive);

    return output;
}

float getPeakDifference(const std::vector<float>& first, const std::vector<float>& second) {
    auto peak = 0.0f;

    for (std::size_t i = 0; i < first.size(); ++i)
        peak = std::max(peak, std::abs(first[i] - second[i]));

    return peak;
}

bool isFiniteAndBounded(const std::vector<float>& buffer, const float bound) {
    return std::all_of(buffer.begin(), buffer.end(), [bound](const float value) {
        return std::isfinite(value) && std::abs(value) <= bound;
    });
}

float sampleOnRisingOrFallingEdge(const std::vector<float>& input, const std::vector<float>& output,
                                  const float targetInput, const bool rising) {
    auto bestDistance = std::numeric_limits<float>::max();
    auto bestValue = 0.0f;

    for (std::size_t sample = 1; sample < input.size(); ++sample) {
        const auto isRising = input[sample] > input[sample - 1];
        if (isRising != rising)
            continue;

        const auto distance = std::abs(input[sample] - targetInput);
        if (distance < bestDistance) {
            bestDistance = distance;
            bestValue = output[sample];
        }
    }

    return bestValue;
}
} // namespace

TEST_CASE("Jiles-Atherton Langevin uses stable small-signal approximations") {
    REQUIRE_THAT(JilesAtherton::langevin(0.0f), Catch::Matchers::WithinAbs(0.0f, 1.0e-7f));
    REQUIRE_THAT(JilesAtherton::langevinDerivative(0.0f),
                 Catch::Matchers::WithinAbs(1.0f / 3.0f, 1.0e-6f));
}

TEST_CASE("Jiles-Atherton model is deterministic for the same input") {
    std::vector<float> input(sampleCount);
    fillSine(input, 1000.0f, 0.7f);

    const auto first = processBuffer(input, 0.75f);
    const auto second = processBuffer(input, 0.75f);

    REQUIRE(getPeakDifference(first, second) <= 1.0e-6f);
}

TEST_CASE("Jiles-Atherton model keeps silence silent") {
    const std::vector<float> input(sampleCount, 0.0f);
    const auto output = processBuffer(input, 1.0f);

    REQUIRE(*std::max_element(output.begin(), output.end()) <= 1.0e-7f);
    REQUIRE(*std::min_element(output.begin(), output.end()) >= -1.0e-7f);
}

TEST_CASE("Jiles-Atherton model has memory and forms a hysteresis loop") {
    std::vector<float> input(sampleCount);
    fillSine(input, 50.0f, 0.65f);
    const auto output = processBuffer(input, 0.8f);

    const auto rising = sampleOnRisingOrFallingEdge(input, output, 0.2f, true);
    const auto falling = sampleOnRisingOrFallingEdge(input, output, 0.2f, false);

    REQUIRE(std::abs(rising - falling) > 0.01f);
}

TEST_CASE("Jiles-Atherton model remains bounded across common sample rates") {
    constexpr double rates[] = {44100.0, 48000.0, 96000.0, 192000.0};

    for (const auto rate : rates) {
        std::vector<float> input(static_cast<std::size_t>(rate / 10.0));

        for (std::size_t sample = 0; sample < input.size(); ++sample) {
            const auto position = static_cast<float>(sample) /
                                  static_cast<float>(std::max<std::size_t>(1, input.size() - 1));
            const auto level = position < 0.5f ? position * 4.0f : (1.0f - position) * 4.0f;
            input[sample] = std::clamp(level, 0.0f, 2.0f);
        }

        const auto output = processBuffer(input, 1.0f, rate);
        REQUIRE(isFiniteAndBounded(output, 1.25f));
    }
}
