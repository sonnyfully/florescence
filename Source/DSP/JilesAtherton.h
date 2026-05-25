#pragma once

#include <array>
#include <cstddef>

// Clean-room Jiles-Atherton tape hysteresis model.
// References:
// - Jiles & Atherton, "Theory of ferromagnetic hysteresis", JMMM 61, 1986.
// - Chowdhury, "Real-time Physical Modelling for Analog Tape Machines", DAFx 2019.
// No GPL tape-model source was consulted or copied.
class JilesAtherton {
  public:
    struct Parameters {
        float saturationMagnetisation = 1.0f;
        float pinning = 0.47f;
        float anhystereticShape = 22000.0f;
        float domainCoupling = 1.6e-3f;
        float reversibility = 1.7e-1f;
        float inputFieldScale = 22000.0f;
        float maxDriveGain = 3.0f;
        // Normalized audio lacks a tape-machine bias path; this record-path asymmetry
        // gives the v1 model even-harmonic emphasis and is a Stage 6 tuning value.
        float recordAsymmetry = 1.0f;
    };

    JilesAtherton() = default;

    void prepare(double newSampleRate);
    void prepare(double newSampleRate, Parameters newParameters);
    void reset() noexcept;

    [[nodiscard]] float processSample(float input, float drive) noexcept;

    [[nodiscard]] float getMagnetisation() const noexcept {
        return magnetisation;
    }

    [[nodiscard]] static float langevin(float x) noexcept;
    [[nodiscard]] static float langevinDerivative(float x) noexcept;

  private:
    struct DerivativeInput {
        float h = 0.0f;
        float hDot = 0.0f;
    };

    [[nodiscard]] float inputToField(float input, float drive) const noexcept;
    [[nodiscard]] float lookupLangevin(float q) const noexcept;
    [[nodiscard]] float lookupLangevinDerivative(float q) const noexcept;
    void updateLookupTables() noexcept;
    [[nodiscard]] float derivative(float m, DerivativeInput input) const noexcept;
    [[nodiscard]] float solveRk4(float currentField, float currentFieldDerivative) noexcept;

    static constexpr std::size_t lookupTableSize = 2049;
    static constexpr float lookupMin = -8.0f;
    static constexpr float lookupMax = 8.0f;

    Parameters parameters;
    double sampleRate = 48000.0;
    float timeStep = 1.0f / 48000.0f;
    float previousField = 0.0f;
    float previousFieldDerivative = 0.0f;
    float magnetisation = 0.0f;
    std::array<float, lookupTableSize> langevinTable{};
    std::array<float, lookupTableSize> langevinDerivativeTable{};
};
