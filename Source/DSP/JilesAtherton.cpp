#include "JilesAtherton.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr float minDenominator = 1.0e-6f;
constexpr float maxDerivative = 2.0e4f;
constexpr float maxMagnetisationScale = 1.25f;
constexpr float minDrive = 1.0e-6f;

[[nodiscard]] float safeDivide(const float numerator, const float denominator) noexcept {
    const auto signedMinimum = denominator < 0.0f ? -minDenominator : minDenominator;
    const auto safeDenominator =
        std::abs(denominator) < minDenominator ? signedMinimum : denominator;
    return numerator / safeDenominator;
}

[[nodiscard]] float signum(const float value) noexcept {
    if (value > 0.0f)
        return 1.0f;
    if (value < 0.0f)
        return -1.0f;
    return 0.0f;
}
} // namespace

void JilesAtherton::prepare(const double newSampleRate) {
    prepare(newSampleRate, {});
}

void JilesAtherton::prepare(const double newSampleRate, Parameters newParameters) {
    sampleRate = std::max(1.0, newSampleRate);
    timeStep = 1.0f / static_cast<float>(sampleRate);
    parameters = newParameters;
    updateLookupTables();
    reset();
}

void JilesAtherton::reset() noexcept {
    previousField = 0.0f;
    previousFieldDerivative = 0.0f;
    magnetisation = 0.0f;
}

float JilesAtherton::processSample(const float input, const float drive) noexcept {
    const auto driveAmount = std::clamp(drive, 0.0f, 1.0f);

    if (driveAmount <= minDrive) {
        previousField = 0.0f;
        previousFieldDerivative = 0.0f;
        magnetisation = 0.0f;
        return input;
    }

    const auto field = inputToField(input, driveAmount);
    const auto fieldDerivative = (field - previousField) / timeStep;

    const auto solvedMagnetisation = solveRk4(field, fieldDerivative);

    previousField = field;
    previousFieldDerivative = fieldDerivative;
    magnetisation =
        std::clamp(solvedMagnetisation, -parameters.saturationMagnetisation * maxMagnetisationScale,
                   parameters.saturationMagnetisation * maxMagnetisationScale);

    return magnetisation / std::max(parameters.saturationMagnetisation, minDenominator);
}

float JilesAtherton::langevin(const float x) noexcept {
    const auto absX = std::abs(x);

    if (absX < 1.0e-4f)
        return x / 3.0f - (x * x * x) / 45.0f;

    return 1.0f / std::tanh(x) - 1.0f / x;
}

float JilesAtherton::langevinDerivative(const float x) noexcept {
    const auto absX = std::abs(x);

    if (absX < 1.0e-4f)
        return 1.0f / 3.0f - (x * x) / 15.0f;

    const auto sinhX = std::sinh(x);
    return 1.0f / (x * x) - 1.0f / (sinhX * sinhX);
}

float JilesAtherton::inputToField(const float input, const float drive) const noexcept {
    const auto gain = 1.0f + drive * parameters.maxDriveGain;
    const auto asymmetry = parameters.recordAsymmetry * drive;
    const auto polarityScale = input >= 0.0f ? 1.0f + asymmetry : 1.0f - asymmetry;

    return input * parameters.inputFieldScale * gain * polarityScale;
}

float JilesAtherton::lookupLangevin(const float q) const noexcept {
    if (q <= lookupMin || q >= lookupMax)
        return langevin(q);

    const auto normalised = (q - lookupMin) / (lookupMax - lookupMin);
    const auto scaled = normalised * static_cast<float>(lookupTableSize - 1);
    const auto lowerIndex = static_cast<std::size_t>(scaled);
    const auto upperIndex = std::min(lowerIndex + 1, lookupTableSize - 1);
    const auto fraction = scaled - static_cast<float>(lowerIndex);

    return langevinTable[lowerIndex] +
           fraction * (langevinTable[upperIndex] - langevinTable[lowerIndex]);
}

float JilesAtherton::lookupLangevinDerivative(const float q) const noexcept {
    if (q <= lookupMin || q >= lookupMax)
        return langevinDerivative(q);

    const auto normalised = (q - lookupMin) / (lookupMax - lookupMin);
    const auto scaled = normalised * static_cast<float>(lookupTableSize - 1);
    const auto lowerIndex = static_cast<std::size_t>(scaled);
    const auto upperIndex = std::min(lowerIndex + 1, lookupTableSize - 1);
    const auto fraction = scaled - static_cast<float>(lowerIndex);

    return langevinDerivativeTable[lowerIndex] +
           fraction * (langevinDerivativeTable[upperIndex] - langevinDerivativeTable[lowerIndex]);
}

void JilesAtherton::updateLookupTables() noexcept {
    for (std::size_t index = 0; index < lookupTableSize; ++index) {
        const auto q = lookupMin + (lookupMax - lookupMin) * static_cast<float>(index) /
                                       static_cast<float>(lookupTableSize - 1);
        langevinTable[index] = langevin(q);
        langevinDerivativeTable[index] = langevinDerivative(q);
    }
}

float JilesAtherton::derivative(const float m, const DerivativeInput input) const noexcept {
    // Clean-room Jiles-Atherton model from Jiles/Atherton 1986 and Chowdhury DAFx 2019:
    // M_an = M_s L((H + alpha M) / a); dM/dt follows Chowdhury's RK-ready form of
    // dM/dH with irreversible pinning, reversible magnetisation, and dH/dt.
    const auto effectiveField = input.h + parameters.domainCoupling * m;
    const auto q = effectiveField / std::max(parameters.anhystereticShape, minDenominator);
    const auto anhysteretic = parameters.saturationMagnetisation * lookupLangevin(q);
    const auto anhystereticDerivative = parameters.saturationMagnetisation /
                                        std::max(parameters.anhystereticShape, minDenominator) *
                                        lookupLangevinDerivative(q);

    const auto deltaS = signum(input.hDot);
    const auto magnetisationGap = anhysteretic - m;
    const auto deltaM =
        (deltaS > 0.0f && magnetisationGap > 0.0f) || (deltaS < 0.0f && magnetisationGap < 0.0f)
            ? 1.0f
            : 0.0f;
    const auto irreversibleNumerator =
        (1.0f - parameters.reversibility) * deltaM * (anhysteretic - m);
    const auto irreversibleDenominator =
        (1.0f - parameters.reversibility) * deltaS * parameters.pinning -
        parameters.domainCoupling * (anhysteretic - m);
    const auto irreversible =
        safeDivide(irreversibleNumerator, irreversibleDenominator) * input.hDot;
    const auto reversible = parameters.reversibility * anhystereticDerivative * input.hDot;
    const auto feedbackDenominator =
        1.0f - parameters.reversibility * parameters.domainCoupling * anhystereticDerivative;

    return std::clamp(safeDivide(irreversible + reversible, feedbackDenominator), -maxDerivative,
                      maxDerivative);
}

float JilesAtherton::solveRk4(const float currentField,
                              const float currentFieldDerivative) noexcept {
    const auto halfInput =
        DerivativeInput{0.5f * (previousField + currentField),
                        0.5f * (previousFieldDerivative + currentFieldDerivative)};
    const auto startInput = DerivativeInput{previousField, previousFieldDerivative};
    const auto endInput = DerivativeInput{currentField, currentFieldDerivative};

    const auto k1 = timeStep * derivative(magnetisation, startInput);
    const auto k2 = timeStep * derivative(magnetisation + 0.5f * k1, halfInput);
    const auto k3 = timeStep * derivative(magnetisation + 0.5f * k2, halfInput);
    const auto k4 = timeStep * derivative(magnetisation + k3, endInput);

    return magnetisation + (k1 + 2.0f * k2 + 2.0f * k3 + k4) / 6.0f;
}
