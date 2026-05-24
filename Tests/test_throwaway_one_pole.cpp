#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "DSP/ThrowawayOnePoleLowPass.h"

TEST_CASE("throwaway one-pole low-pass approaches DC input") {
    ThrowawayOnePoleLowPass filter;
    filter.prepare(48000.0, 2);
    filter.setCutoffHz(1000.0f);

    auto output = 0.0f;
    for (auto i = 0; i < 48000; ++i)
        output = filter.processSample(0, 1.0f);

    REQUIRE(output > 0.999f);
    REQUIRE(output <= 1.0f);
}

TEST_CASE("throwaway one-pole low-pass reset clears history") {
    ThrowawayOnePoleLowPass filter;
    filter.prepare(48000.0, 1);
    filter.setCutoffHz(2000.0f);

    for (auto i = 0; i < 128; ++i)
        static_cast<void>(filter.processSample(0, 1.0f));

    filter.reset();

    const auto firstOutput = filter.processSample(0, 1.0f);
    REQUIRE_THAT(firstOutput, Catch::Matchers::WithinAbs(filter.getCoefficient(), 0.000001f));
}
