#include "Saturation.h"

#include <algorithm>
#include <cmath>

namespace {
[[nodiscard]] float clampDrive(const float drive) noexcept {
    return std::clamp(drive, saturationconfig::driveMin, saturationconfig::driveMax);
}

[[nodiscard]] float clampCutoff(const double sampleRate, const float cutoffHz) noexcept {
    const auto nyquistLimited = static_cast<float>(sampleRate * 0.49);
    return std::clamp(cutoffHz, 20.0f, nyquistLimited);
}
} // namespace

void Saturation::prepare(const juce::dsp::ProcessSpec& spec) {
    sampleRate = std::max(1.0, spec.sampleRate);
    oversampledSampleRate = sampleRate * saturationconfig::oversamplingFactor;
    channels.assign(spec.numChannels, {});
    tapeModels.assign(spec.numChannels, {});

    for (auto& model : tapeModels)
        model.prepare(oversampledSampleRate);

    oversampler = std::make_unique<juce::dsp::Oversampling<float>>(
        spec.numChannels, saturationconfig::oversamplingExponent,
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, true);
    oversampler->initProcessing(spec.maximumBlockSize);

    dynamicLowPass.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    dynamicLowPass.setResonance(saturationconfig::lpfResonance);
    dynamicLowPass.prepare({oversampledSampleRate,
                            spec.maximumBlockSize * saturationconfig::oversamplingFactor,
                            spec.numChannels});

    fixedHighFrequencyRolloff.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    fixedHighFrequencyRolloff.setResonance(saturationconfig::fixedHfRolloffResonance);
    fixedHighFrequencyRolloff.prepare({oversampledSampleRate,
                                       spec.maximumBlockSize * saturationconfig::oversamplingFactor,
                                       spec.numChannels});
    fixedHighFrequencyRolloff.setCutoffFrequency(
        clampCutoff(oversampledSampleRate, saturationconfig::fixedHfRolloffHz));

    smoothedDrive.reset(oversampledSampleRate, saturationconfig::driveSmoothingTimeSeconds);
    smoothedDrive.setCurrentAndTargetValue(targetDrive);

    attackCoefficient =
        getDetectorCoefficient(oversampledSampleRate, saturationconfig::detectorAttackMs);
    releaseCoefficient =
        getDetectorCoefficient(oversampledSampleRate, saturationconfig::detectorReleaseMs);
    dcBlockerCoefficient =
        getDcBlockerCoefficient(oversampledSampleRate, saturationconfig::dcBlockerCutoffHz);

    updateFilterCutoff(targetDrive);
    reset();
}

void Saturation::process(juce::dsp::AudioBlock<float>& block) {
    if (oversampler == nullptr)
        return;

    if (targetDrive <= saturationconfig::minProcessDrive &&
        smoothedDrive.getCurrentValue() <= saturationconfig::minProcessDrive)
        return;

    auto oversampledBlock = oversampler->processSamplesUp(block);
    const auto channelCount =
        std::min<std::size_t>(oversampledBlock.getNumChannels(), channels.size());

    for (std::size_t sample = 0; sample < oversampledBlock.getNumSamples(); ++sample) {
        const auto drive = smoothedDrive.getNextValue();

        for (std::size_t channel = 0; channel < channelCount; ++channel) {
            auto* samples = oversampledBlock.getChannelPointer(channel);
            auto shaped = tapeModels[channel].processSample(samples[sample], drive);

            auto& envelope = channels[channel].envelope;
            const auto magnitude = std::abs(shaped);
            const auto coefficient = magnitude > envelope ? attackCoefficient : releaseCoefficient;
            envelope = coefficient * envelope + (1.0f - coefficient) * magnitude;

            // Tape loses top end as it is driven harder. The Jiles-Atherton stage supplies
            // hysteresis/nonlinear memory; this drive-coupled LPF keeps the v1 tape path
            // aligned with the documented level-dependent HF loss target.
            const auto cutoffDrive = std::clamp(drive * envelope, 0.0f, 1.0f);
            dynamicLowPass.setCutoffFrequency(
                clampCutoff(oversampledSampleRate, getDynamicCutoffHz(cutoffDrive)));
            shaped = dynamicLowPass.processSample(static_cast<int>(channel), shaped);
            shaped = fixedHighFrequencyRolloff.processSample(static_cast<int>(channel), shaped);
            samples[sample] = processDcBlocker(channels[channel], shaped);
        }
    }

    dynamicLowPass.snapToZero();
    fixedHighFrequencyRolloff.snapToZero();
    oversampler->processSamplesDown(block);
}

void Saturation::reset() {
    for (auto& channel : channels) {
        channel.envelope = 0.0f;
        channel.dcInput = 0.0f;
        channel.dcOutput = 0.0f;
    }

    for (auto& model : tapeModels)
        model.reset();

    smoothedDrive.setCurrentAndTargetValue(targetDrive);

    if (oversampler != nullptr)
        oversampler->reset();

    dynamicLowPass.reset();
    fixedHighFrequencyRolloff.reset();
}

int Saturation::getLatencySamples() const {
    // Keep v1 zero-latency for host PDC; IIR oversampling phase is part of the processed tone.
    return 0;
}

void Saturation::setDrive(const float newDrive) noexcept {
    const auto clampedDrive = clampDrive(newDrive);

    if (std::abs(clampedDrive - targetDrive) < saturationconfig::driveParameterEpsilon)
        return;

    targetDrive = clampedDrive;
    smoothedDrive.setTargetValue(targetDrive);
}

float Saturation::getDynamicCutoffHz(const float driveAmount) noexcept {
    const auto amount = clampDrive(driveAmount);
    const auto ratio = saturationconfig::lpfMinCutoffHz / saturationconfig::lpfMaxCutoffHz;
    return saturationconfig::lpfMaxCutoffHz * std::pow(ratio, amount);
}

float Saturation::getDetectorCoefficient(const double detectorSampleRate,
                                         const float timeMs) noexcept {
    const auto seconds = std::max(0.001f, timeMs) * 0.001f;
    return std::exp(-1.0f / static_cast<float>(detectorSampleRate * seconds));
}

float Saturation::getDcBlockerCoefficient(const double dcBlockerSampleRate,
                                          const float cutoffHz) noexcept {
    const auto cutoff = std::max(1.0f, cutoffHz);
    return std::exp(-juce::MathConstants<float>::twoPi * cutoff /
                    static_cast<float>(dcBlockerSampleRate));
}

float Saturation::processDcBlocker(ChannelState& channel, const float input) const noexcept {
    const auto output = dcBlockerCoefficient * (channel.dcOutput + input - channel.dcInput);
    channel.dcInput = input;
    channel.dcOutput = output;
    return output;
}

void Saturation::updateFilterCutoff(const float driveAmount) noexcept {
    dynamicLowPass.setCutoffFrequency(
        clampCutoff(oversampledSampleRate, getDynamicCutoffHz(driveAmount)));
}
