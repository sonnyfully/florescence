#include "Filter.h"

#include "DSPConfig.h"

#include <algorithm>
#include <cmath>

namespace {
[[nodiscard]] float clampRange(const float value, const float min, const float max) noexcept {
    return std::clamp(value, min, max);
}
} // namespace

void Filter::prepare(const juce::dsp::ProcessSpec& spec) {
    sampleRate = std::max(1.0, spec.sampleRate);

    lowPass.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    lowPass.prepare(spec);

    smoothedCutoffHz.reset(sampleRate, filterconfig::parameterSmoothingTimeSeconds);
    smoothedResonance.reset(sampleRate, filterconfig::parameterSmoothingTimeSeconds);
    smoothedEnvelopeDepthOctaves.reset(sampleRate, filterconfig::parameterSmoothingTimeSeconds);
    smoothedCutoffHz.setCurrentAndTargetValue(targetCutoffHz);
    smoothedResonance.setCurrentAndTargetValue(targetResonance);
    smoothedEnvelopeDepthOctaves.setCurrentAndTargetValue(targetEnvelopeDepthOctaves);

    attackCoefficient = getDetectorCoefficient(sampleRate, filterconfig::envelopeAttackMs);
    releaseCoefficient = getDetectorCoefficient(sampleRate, filterconfig::envelopeReleaseMs);

    updateFilterTargets(targetCutoffHz, targetResonance);
    reset();
}

void Filter::process(juce::dsp::AudioBlock<float>& block) {
    const auto numChannels = block.getNumChannels();
    const auto numSamples = block.getNumSamples();

    for (std::size_t sample = 0; sample < numSamples; ++sample) {
        const auto detectorInput = getInputEnvelope(block, sample);
        const auto coefficient = detectorInput > envelope ? attackCoefficient : releaseCoefficient;
        envelope = coefficient * envelope + (1.0f - coefficient) * detectorInput;

        const auto cutoffHz = smoothedCutoffHz.getNextValue();
        const auto resonance = smoothedResonance.getNextValue();
        const auto depthOctaves = smoothedEnvelopeDepthOctaves.getNextValue();
        const auto modulatedCutoffHz =
            getModulatedCutoffHz(cutoffHz, envelope, depthOctaves, sampleRate);

        updateFilterTargets(modulatedCutoffHz, resonance);

        for (std::size_t channel = 0; channel < numChannels; ++channel) {
            auto* samples = block.getChannelPointer(channel);
            samples[sample] = lowPass.processSample(static_cast<int>(channel), samples[sample]);
        }
    }

    lowPass.snapToZero();
}

void Filter::reset() {
    envelope = 0.0f;
    smoothedCutoffHz.setCurrentAndTargetValue(targetCutoffHz);
    smoothedResonance.setCurrentAndTargetValue(targetResonance);
    smoothedEnvelopeDepthOctaves.setCurrentAndTargetValue(targetEnvelopeDepthOctaves);
    lowPass.reset();
    updateFilterTargets(targetCutoffHz, targetResonance);
}

int Filter::getLatencySamples() const {
    return 0;
}

void Filter::setCutoffHz(const float newCutoffHz) noexcept {
    const auto clampedCutoff = clampCutoff(newCutoffHz);

    if (std::abs(clampedCutoff - targetCutoffHz) < filterconfig::parameterEpsilon)
        return;

    targetCutoffHz = clampedCutoff;
    smoothedCutoffHz.setTargetValue(targetCutoffHz);
}

void Filter::setResonance(const float newResonance) noexcept {
    const auto clampedResonance =
        clampRange(newResonance, filterconfig::resonanceMin, filterconfig::resonanceMax);

    if (std::abs(clampedResonance - targetResonance) < filterconfig::parameterEpsilon)
        return;

    targetResonance = clampedResonance;
    smoothedResonance.setTargetValue(targetResonance);
}

void Filter::setEnvelopeDepthOctaves(const float newDepthOctaves) noexcept {
    const auto clampedDepth = clampRange(newDepthOctaves, filterconfig::envelopeDepthMinOctaves,
                                         filterconfig::envelopeDepthMaxOctaves);

    if (std::abs(clampedDepth - targetEnvelopeDepthOctaves) < filterconfig::parameterEpsilon)
        return;

    targetEnvelopeDepthOctaves = clampedDepth;
    smoothedEnvelopeDepthOctaves.setTargetValue(targetEnvelopeDepthOctaves);
}

float Filter::getDetectorCoefficient(const double detectorSampleRate, const float timeMs) noexcept {
    const auto seconds = std::max(0.001f, timeMs) * 0.001f;
    return std::exp(-1.0f / static_cast<float>(detectorSampleRate * seconds));
}

float Filter::getModulatedCutoffHz(const float baseCutoffHz, const float envelope,
                                   const float depthOctaves, const double sampleRate) noexcept {
    const auto normalizedEnvelope = clampRange(envelope, 0.0f, 1.0f);
    const auto octaveOffset = -std::max(0.0f, depthOctaves) * normalizedEnvelope;
    const auto cutoff = baseCutoffHz * std::pow(2.0f, octaveOffset);
    const auto nyquistLimited = static_cast<float>(sampleRate * dspconfig::nyquistGuard);
    return clampRange(cutoff, filterconfig::cutoffMinHz, nyquistLimited);
}

float Filter::getInputEnvelope(juce::dsp::AudioBlock<float>& block,
                               const std::size_t sample) noexcept {
    auto peak = 0.0f;

    for (std::size_t channel = 0; channel < block.getNumChannels(); ++channel)
        peak = std::max(peak, std::abs(block.getChannelPointer(channel)[sample]));

    return std::min(peak, 1.0f);
}

float Filter::clampCutoff(const float cutoffHz) const noexcept {
    const auto nyquistLimited = static_cast<float>(sampleRate * dspconfig::nyquistGuard);
    return clampRange(cutoffHz, filterconfig::cutoffMinHz, nyquistLimited);
}

void Filter::updateFilterTargets(const float cutoffHz, const float resonance) noexcept {
    lowPass.setCutoffFrequency(clampCutoff(cutoffHz));
    lowPass.setResonance(
        clampRange(resonance, filterconfig::resonanceMin, filterconfig::resonanceMax));
}
