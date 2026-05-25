#pragma once

#include "FXModule.h"

#include <array>

// Clean-room BBD-flavoured chorus scaffold. Implementation notes and references:
// docs/research/chorus.md; Zolzer DAFX Chapter 2; Pirkle modulated delay
// effects; Smith PASP delay lines; Raffel/Smith BBD circuit modelling.
class Chorus final : public FXModule {
  public:
    Chorus() = default;
    explicit Chorus(const juce::dsp::ProcessSpec& spec);

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void process(juce::dsp::AudioBlock<float>& block) override;
    void reset() override;

    [[nodiscard]] int getLatencySamples() const override;

    void setDepth(float newDepth) noexcept;
    void setRateHz(float newRateHz) noexcept;
    void setMix(float newMix) noexcept;

    [[nodiscard]] float getDepth() const noexcept {
        return depth;
    }

    [[nodiscard]] float getRateHz() const noexcept {
        return rateHz;
    }

    [[nodiscard]] float getMix() const noexcept {
        return mix;
    }

  private:
    class ChorusVoice {
      public:
        ChorusVoice() = default;

        void prepare(const juce::dsp::ProcessSpec& spec, float phaseOffsetRadians);
        void reset() noexcept;

      private:
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayLine;
        double sampleRate = 48000.0;
        float phaseRadians = 0.0f;
        float phaseOffset = 0.0f;
    };

    static constexpr std::size_t voiceCount = 2;

    std::array<ChorusVoice, voiceCount> voices;
    double sampleRate = 48000.0;
    juce::uint32 maximumBlockSize = 0;
    juce::uint32 channelCount = 0;
    float depth = 0.0f;
    float rateHz = 0.25f;
    float mix = 0.0f;
};
