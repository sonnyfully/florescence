#include "Chorus.h"

#include <algorithm>

namespace {
constexpr float minUnitValue = 0.0f;
constexpr float maxUnitValue = 1.0f;
constexpr float minRateHz = 0.01f;
constexpr float maxRateHz = 10.0f;
constexpr float leftVoicePhaseOffsetRadians = 0.0f;
constexpr float rightVoicePhaseOffsetRadians = juce::MathConstants<float>::pi;
constexpr int maxDelaySamplesForScaffold = 1;

[[nodiscard]] float clampUnitValue(const float value) noexcept {
    return std::clamp(value, minUnitValue, maxUnitValue);
}
} // namespace

Chorus::Chorus(const juce::dsp::ProcessSpec& spec) {
    prepare(spec);
}

void Chorus::prepare(const juce::dsp::ProcessSpec& spec) {
    sampleRate = std::max(1.0, spec.sampleRate);
    maximumBlockSize = spec.maximumBlockSize;
    channelCount = spec.numChannels;

    voices[0].prepare(spec, leftVoicePhaseOffsetRadians);
    voices[1].prepare(spec, rightVoicePhaseOffsetRadians);
}

void Chorus::process(juce::dsp::AudioBlock<float>& block) {
    juce::ignoreUnused(block);
    // PR 2 is scaffolding only. The real Stage 3 implementation will route L/R
    // through the two decorrelated wet voices described in docs/research/chorus.md.
}

void Chorus::reset() {
    for (auto& voice : voices)
        voice.reset();
}

int Chorus::getLatencySamples() const {
    return 0;
}

void Chorus::setDepth(const float newDepth) noexcept {
    depth = clampUnitValue(newDepth);
}

void Chorus::setRateHz(const float newRateHz) noexcept {
    rateHz = std::clamp(newRateHz, minRateHz, maxRateHz);
}

void Chorus::setMix(const float newMix) noexcept {
    mix = clampUnitValue(newMix);
}

void Chorus::ChorusVoice::prepare(const juce::dsp::ProcessSpec& spec,
                                  const float phaseOffsetRadians) {
    sampleRate = std::max(1.0, spec.sampleRate);
    phaseOffset = phaseOffsetRadians;
    delayLine.setMaximumDelayInSamples(maxDelaySamplesForScaffold);
    delayLine.prepare(spec);
    reset();
}

void Chorus::ChorusVoice::reset() noexcept {
    phaseRadians = phaseOffset;
    delayLine.reset();
}
