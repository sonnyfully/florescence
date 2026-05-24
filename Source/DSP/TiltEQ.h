#pragma once

#include "DSPConfig.h"
#include "FXModule.h"

#include <vector>

class TiltEQ final : public FXModule {
  public:
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void process(juce::dsp::AudioBlock<float>& block) override;
    void reset() override;

    void setTiltDb(float newTiltDb) noexcept;

    [[nodiscard]] float processSample(std::size_t channel, float input) noexcept;
    [[nodiscard]] float getTiltDb() const noexcept {
        return tiltDb;
    }
    [[nodiscard]] double getMagnitudeDb(double frequencyHz) const;

  private:
    struct BiquadCoefficients {
        double b0 = 1.0;
        double b1 = 0.0;
        double b2 = 0.0;
        double a1 = 0.0;
        double a2 = 0.0;
    };

    struct BiquadState {
        double z1 = 0.0;
        double z2 = 0.0;

        void reset() noexcept;
        [[nodiscard]] float process(float input, const BiquadCoefficients& coefficients) noexcept;
    };

    struct ChannelState {
        BiquadState lowShelf;
        BiquadState highShelf;
    };

    static BiquadCoefficients makeLowShelf(double sampleRate, double frequencyHz,
                                           double gainDb) noexcept;
    static BiquadCoefficients makeHighShelf(double sampleRate, double frequencyHz,
                                            double gainDb) noexcept;
    static double getMagnitude(const BiquadCoefficients& coefficients, double sampleRate,
                               double frequencyHz) noexcept;

    void updateCoefficients() noexcept;

    double sampleRate = 48000.0;
    float tiltDb = 0.0f;
    BiquadCoefficients lowShelfCoefficients;
    BiquadCoefficients highShelfCoefficients;
    std::vector<ChannelState> channels;
};
