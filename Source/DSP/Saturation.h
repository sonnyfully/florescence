#pragma once

#include "FXModule.h"
#include "SaturationConfig.h"

#include <memory>
#include <vector>

class Saturation final : public FXModule {
  public:
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void process(juce::dsp::AudioBlock<float>& block) override;
    void reset() override;

    [[nodiscard]] int getLatencySamples() const override;

    void setDrive(float newDrive) noexcept;

    [[nodiscard]] float getDrive() const noexcept {
        return targetDrive;
    }

    [[nodiscard]] static float shapeSample(float input, float drive) noexcept;
    [[nodiscard]] static float getDynamicCutoffHz(float driveAmount) noexcept;

  private:
    struct ChannelState {
        float envelope = 0.0f;
        float dcInput = 0.0f;
        float dcOutput = 0.0f;
    };

    static float getDetectorCoefficient(double sampleRate, float timeMs) noexcept;
    static float getDcBlockerCoefficient(double sampleRate, float cutoffHz) noexcept;

    [[nodiscard]] float processDcBlocker(ChannelState& channel, float input) const noexcept;

    void updateFilterCutoff(float driveAmount) noexcept;

    double sampleRate = 48000.0;
    double oversampledSampleRate = sampleRate * saturationconfig::oversamplingFactor;
    float targetDrive = 0.0f;
    float attackCoefficient = 0.0f;
    float releaseCoefficient = 0.0f;
    float dcBlockerCoefficient = 0.0f;
    std::vector<ChannelState> channels;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedDrive;
    juce::dsp::StateVariableTPTFilter<float> dynamicLowPass;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
};
