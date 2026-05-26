#pragma once

#include "ConvReverbConfig.h"
#include "FXModule.h"

#include <array>
#include <vector>

// Convolution reverb wrapper with fixed pre-convolution wet-path modulation.
// References: docs/decisions.md 2026-05-26 Q-IR-2/Q-CHAR-2/Q-IR-4;
// JUCE `juce::dsp::Convolution`; Zolzer DAFX Chapter 2.
class ConvReverb final : public FXModule {
  public:
    ConvReverb() = default;
    explicit ConvReverb(const juce::dsp::ProcessSpec& spec);

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void process(juce::dsp::AudioBlock<float>& block) override;
    void reset() override;

    [[nodiscard]] int getLatencySamples() const override;

    void setWetMix(float newWetMix) noexcept;
    void loadImpulseResponse(juce::AudioBuffer<float>&& impulseResponse, double impulseSampleRate);

    [[nodiscard]] float getWetMix() const noexcept {
        return wetMix;
    }

    [[nodiscard]] int getCurrentIRSize() const;
    [[nodiscard]] static juce::AudioBuffer<float> createFallbackImpulseResponse();

  private:
    [[nodiscard]] float readModulationDelay(int channel, float delaySamples) const noexcept;
    [[nodiscard]] float getWetPathModulationDelaySamples() const noexcept;
    [[nodiscard]] float getWetPathModulationDepthSamples() const noexcept;
    [[nodiscard]] float getSmoothedWetMix() noexcept;
    void processWetPathModulation(juce::AudioBuffer<float>& buffer, int numSamples) noexcept;
    void advanceModulationPhase() noexcept;
    void prepareWetPathBuffers(const juce::dsp::ProcessSpec& spec);
    void loadFallbackImpulseResponse();
    void advanceModulationWriteIndex() noexcept;

    juce::dsp::Convolution convolution{juce::dsp::Convolution::Latency{0}};
    juce::AudioBuffer<float> wetInputBuffer;
    juce::AudioBuffer<float> wetOutputBuffer;
    std::array<std::vector<float>, 2> modulationDelayBuffer;
    double sampleRate = 48000.0;
    int maximumBlockSize = 0;
    int modulationDelayBufferSize = 1;
    int modulationWriteIndex = 0;
    float modulationPhaseRadians = 0.0f;
    float wetMix = convreverbconfig::wetMixDefault;
    bool customImpulseLoaded = false;
    juce::SmoothedValue<float> wetMixSmoothed;
};
