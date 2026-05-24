#pragma once

#include <juce_dsp/juce_dsp.h>

class FXModule {
  public:
    FXModule() = default;
    FXModule(const FXModule&) = delete;
    FXModule& operator=(const FXModule&) = delete;
    FXModule(FXModule&&) = delete;
    FXModule& operator=(FXModule&&) = delete;

    virtual void prepare(const juce::dsp::ProcessSpec& spec) = 0;
    virtual void process(juce::dsp::AudioBlock<float>& block) = 0;
    virtual void reset() = 0;

    [[nodiscard]] virtual int getLatencySamples() const {
        return 0;
    }

    virtual ~FXModule() = default;
};
