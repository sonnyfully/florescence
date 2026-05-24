#include "TiltEQ.h"

#include <algorithm>
#include <cmath>
#include <complex>

namespace {
constexpr double twoPi = 6.28318530717958647692;

[[nodiscard]] double clampFrequency(const double sampleRate, const double frequencyHz) noexcept {
    return std::clamp(frequencyHz, static_cast<double>(dspconfig::minFilterFrequencyHz),
                      sampleRate * static_cast<double>(dspconfig::nyquistGuard));
}
} // namespace

void TiltEQ::prepare(const juce::dsp::ProcessSpec& spec) {
    sampleRate = std::max(1.0, spec.sampleRate);
    channels.assign(spec.numChannels, {});
    updateCoefficients();
    reset();
}

void TiltEQ::process(juce::dsp::AudioBlock<float>& block) {
    const auto channelCount = std::min<std::size_t>(block.getNumChannels(), channels.size());

    for (std::size_t channel = 0; channel < channelCount; ++channel) {
        auto* samples = block.getChannelPointer(channel);

        for (std::size_t sample = 0; sample < block.getNumSamples(); ++sample)
            samples[sample] = processSample(channel, samples[sample]);
    }
}

void TiltEQ::reset() {
    for (auto& channel : channels) {
        channel.lowShelf.reset();
        channel.highShelf.reset();
    }
}

void TiltEQ::setTiltDb(const float newTiltDb) noexcept {
    const auto clampedTilt = std::clamp(newTiltDb, dspconfig::tiltEqMinDb, dspconfig::tiltEqMaxDb);

    if (std::abs(clampedTilt - tiltDb) < dspconfig::tiltEqParameterEpsilonDb)
        return;

    tiltDb = clampedTilt;
    updateCoefficients();
}

float TiltEQ::processSample(const std::size_t channel, const float input) noexcept {
    if (channel >= channels.size())
        return input;

    auto& state = channels[channel];
    const auto lowShaped = state.lowShelf.process(input, lowShelfCoefficients);
    return state.highShelf.process(lowShaped, highShelfCoefficients);
}

double TiltEQ::getMagnitudeDb(const double frequencyHz) const {
    const auto lowMagnitude = getMagnitude(lowShelfCoefficients, sampleRate, frequencyHz);
    const auto highMagnitude = getMagnitude(highShelfCoefficients, sampleRate, frequencyHz);
    return juce::Decibels::gainToDecibels(lowMagnitude * highMagnitude);
}

void TiltEQ::BiquadState::reset() noexcept {
    z1 = 0.0;
    z2 = 0.0;
}

float TiltEQ::BiquadState::process(const float input,
                                   const BiquadCoefficients& coefficients) noexcept {
    const auto output = coefficients.b0 * input + z1;
    z1 = coefficients.b1 * input - coefficients.a1 * output + z2;
    z2 = coefficients.b2 * input - coefficients.a2 * output;
    return static_cast<float>(output);
}

TiltEQ::BiquadCoefficients TiltEQ::makeLowShelf(const double sampleRate, const double frequencyHz,
                                                const double gainDb) noexcept {
    const auto frequency = clampFrequency(sampleRate, frequencyHz);
    const auto a = std::pow(10.0, gainDb / 40.0);
    const auto omega = twoPi * frequency / sampleRate;
    const auto sinOmega = std::sin(omega);
    const auto cosOmega = std::cos(omega);
    const auto sqrtA = std::sqrt(a);
    const auto alpha =
        sinOmega / 2.0 * std::sqrt((a + 1.0 / a) * (1.0 / dspconfig::tiltEqShelfSlope - 1.0) + 2.0);

    const auto b0 = a * ((a + 1.0) - (a - 1.0) * cosOmega + 2.0 * sqrtA * alpha);
    const auto b1 = 2.0 * a * ((a - 1.0) - (a + 1.0) * cosOmega);
    const auto b2 = a * ((a + 1.0) - (a - 1.0) * cosOmega - 2.0 * sqrtA * alpha);
    const auto a0 = (a + 1.0) + (a - 1.0) * cosOmega + 2.0 * sqrtA * alpha;
    const auto a1 = -2.0 * ((a - 1.0) + (a + 1.0) * cosOmega);
    const auto a2 = (a + 1.0) + (a - 1.0) * cosOmega - 2.0 * sqrtA * alpha;

    return {b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0};
}

TiltEQ::BiquadCoefficients TiltEQ::makeHighShelf(const double sampleRate, const double frequencyHz,
                                                 const double gainDb) noexcept {
    const auto frequency = clampFrequency(sampleRate, frequencyHz);
    const auto a = std::pow(10.0, gainDb / 40.0);
    const auto omega = twoPi * frequency / sampleRate;
    const auto sinOmega = std::sin(omega);
    const auto cosOmega = std::cos(omega);
    const auto sqrtA = std::sqrt(a);
    const auto alpha =
        sinOmega / 2.0 * std::sqrt((a + 1.0 / a) * (1.0 / dspconfig::tiltEqShelfSlope - 1.0) + 2.0);

    const auto b0 = a * ((a + 1.0) + (a - 1.0) * cosOmega + 2.0 * sqrtA * alpha);
    const auto b1 = -2.0 * a * ((a - 1.0) + (a + 1.0) * cosOmega);
    const auto b2 = a * ((a + 1.0) + (a - 1.0) * cosOmega - 2.0 * sqrtA * alpha);
    const auto a0 = (a + 1.0) - (a - 1.0) * cosOmega + 2.0 * sqrtA * alpha;
    const auto a1 = 2.0 * ((a - 1.0) - (a + 1.0) * cosOmega);
    const auto a2 = (a + 1.0) - (a - 1.0) * cosOmega - 2.0 * sqrtA * alpha;

    return {b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0};
}

double TiltEQ::getMagnitude(const BiquadCoefficients& coefficients, const double sampleRate,
                            const double frequencyHz) noexcept {
    const auto omega = twoPi * clampFrequency(sampleRate, frequencyHz) / sampleRate;
    const auto z1 = std::exp(std::complex<double>(0.0, -omega));
    const auto z2 = std::exp(std::complex<double>(0.0, -2.0 * omega));
    const auto numerator = coefficients.b0 + coefficients.b1 * z1 + coefficients.b2 * z2;
    const auto denominator = 1.0 + coefficients.a1 * z1 + coefficients.a2 * z2;
    return std::abs(numerator / denominator);
}

void TiltEQ::updateCoefficients() noexcept {
    const auto halfTiltDb = static_cast<double>(tiltDb) * 0.5;
    lowShelfCoefficients = makeLowShelf(sampleRate, dspconfig::tiltEqPivotHz, -halfTiltDb);
    highShelfCoefficients = makeHighShelf(sampleRate, dspconfig::tiltEqPivotHz, halfTiltDb);
}
