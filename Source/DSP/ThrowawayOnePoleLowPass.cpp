#include "ThrowawayOnePoleLowPass.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr double twoPi = 6.28318530717958647692;
constexpr float minCutoffHz = 20.0f;
constexpr float maxCutoffHz = 20000.0f;
} // namespace

void ThrowawayOnePoleLowPass::prepare(const double newSampleRate, const std::size_t channelCount) {
    sampleRate = std::max(1.0, newSampleRate);
    z1.assign(channelCount, 0.0f);
    updateCoefficient();
}

void ThrowawayOnePoleLowPass::reset() noexcept {
    std::fill(z1.begin(), z1.end(), 0.0f);
}

void ThrowawayOnePoleLowPass::setCutoffHz(const float newCutoffHz) noexcept {
    cutoffHz = std::clamp(newCutoffHz, minCutoffHz, maxCutoffHz);
    updateCoefficient();
}

float ThrowawayOnePoleLowPass::processSample(const std::size_t channel,
                                             const float input) noexcept {
    if (channel >= z1.size())
        return input;

    auto& state = z1[channel];
    state += coefficient * (input - state);
    return state;
}

void ThrowawayOnePoleLowPass::updateCoefficient() noexcept {
    const auto nyquistLimitedCutoff = std::min<double>(cutoffHz, sampleRate * 0.49);
    coefficient = static_cast<float>(1.0 - std::exp(-twoPi * nyquistLimitedCutoff / sampleRate));
}
