#pragma once

#include "FXModule.h"
#include "FilterConfig.h"

// TPT state-variable low-pass with input envelope cutoff modulation.
// References: JUCE `juce::dsp::StateVariableTPTFilter`; Zolzer, DAFX,
// Chapter 2; Vadim Zavalishin, The Art of VA Filter Design.
class Filter final : public FXModule {
  public:
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void process(juce::dsp::AudioBlock<float>& block) override;
    void reset() override;

    [[nodiscard]] int getLatencySamples() const override;

    void setCutoffHz(float newCutoffHz) noexcept;
    void setResonance(float newResonance) noexcept;
    void setEnvelopeDepthOctaves(float newDepthOctaves) noexcept;

    [[nodiscard]] float getCutoffHz() const noexcept {
        return targetCutoffHz;
    }

    [[nodiscard]] float getResonance() const noexcept {
        return targetResonance;
    }

    [[nodiscard]] float getEnvelopeDepthOctaves() const noexcept {
        return targetEnvelopeDepthOctaves;
    }

    [[nodiscard]] static float getDetectorCoefficient(double sampleRate, float timeMs) noexcept;
    [[nodiscard]] static float getModulatedCutoffHz(float baseCutoffHz, float envelope,
                                                    float depthOctaves, double sampleRate) noexcept;

  private:
    [[nodiscard]] float getInputEnvelope(juce::dsp::AudioBlock<float>& block,
                                         std::size_t sample) noexcept;
    [[nodiscard]] float clampCutoff(float cutoffHz) const noexcept;
    void updateFilterTargets(float cutoffHz, float resonance) noexcept;

    double sampleRate = 48000.0;
    float targetCutoffHz = filterconfig::cutoffDefaultHz;
    float targetResonance = filterconfig::resonanceDefault;
    float targetEnvelopeDepthOctaves = filterconfig::envelopeDefaultDepthOctaves;
    float envelope = 0.0f;
    float attackCoefficient = 0.0f;
    float releaseCoefficient = 0.0f;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedCutoffHz;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedResonance;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedEnvelopeDepthOctaves;
    juce::dsp::StateVariableTPTFilter<float> lowPass;
};
