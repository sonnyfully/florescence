#include "Delay.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr int interpolationSafetySamples = 2;

[[nodiscard]] float clampUnitRange(const float value, const float min, const float max) noexcept {
    return std::clamp(value, min, max);
}

[[nodiscard]] double clampTempo(const double bpm) noexcept {
    return std::clamp(bpm, delayconfig::tempoMinBpm, delayconfig::tempoMaxBpm);
}
} // namespace

Delay::Delay(const juce::dsp::ProcessSpec& spec) {
    prepare(spec);
}

void Delay::prepare(const juce::dsp::ProcessSpec& spec) {
    sampleRate = std::max(1.0, spec.sampleRate);
    maxDelaySamples = static_cast<int>(std::ceil(delayconfig::maxDelaySeconds * sampleRate)) +
                      interpolationSafetySamples;

    for (auto& channelBuffer : delayBuffer)
        channelBuffer.assign(static_cast<std::size_t>(maxDelaySamples), 0.0f);

    feedbackLowpassCoefficient =
        1.0f - std::exp(-juce::MathConstants<float>::twoPi * delayconfig::feedbackLowpassCutoffHz /
                        static_cast<float>(sampleRate));

    delaySamplesSmoothed.reset(sampleRate, delayconfig::delayTimeSmoothingSeconds);
    feedbackSmoothed.reset(sampleRate, delayconfig::parameterSmoothingTimeSeconds);
    mixSmoothed.reset(sampleRate, delayconfig::parameterSmoothingTimeSeconds);

    const auto currentDelay = getDelaySamplesForDivision(syncDivision, tempoBpm, sampleRate);
    delaySamplesSmoothed.setCurrentAndTargetValue(currentDelay);
    feedbackSmoothed.setCurrentAndTargetValue(feedback);
    mixSmoothed.setCurrentAndTargetValue(mix);

    reset();
}

void Delay::process(juce::dsp::AudioBlock<float>& block) {
    const auto numChannels = static_cast<int>(block.getNumChannels());
    const auto numSamples = static_cast<int>(block.getNumSamples());

    if (numChannels == 0)
        return;

    auto* left = block.getChannelPointer(0);
    auto* right = numChannels > 1 ? block.getChannelPointer(1) : left;

    for (auto sample = 0; sample < numSamples; ++sample) {
        const auto inputLeft = left[sample];
        const auto inputRight = right[sample];
        const auto delaySamples = getSmoothedDelaySamples();
        const auto feedbackAmount = getSmoothedFeedback();
        const auto mixAmount = getSmoothedMix();

        const auto delayedLeft = readDelayedSample(0, delaySamples);
        const auto delayedRight = readDelayedSample(1, delaySamples);

        const auto feedbackSourceLeft = topology == Topology::PingPong ? delayedRight : delayedLeft;
        const auto feedbackSourceRight =
            topology == Topology::PingPong ? delayedLeft : delayedRight;

        const auto feedbackLeft = processFeedbackLowpass(0, feedbackSourceLeft);
        const auto feedbackRight = processFeedbackLowpass(1, feedbackSourceRight);

        delayBuffer[0][static_cast<std::size_t>(writeIndex)] =
            inputLeft + feedbackAmount * feedbackLeft;
        delayBuffer[1][static_cast<std::size_t>(writeIndex)] =
            inputRight + feedbackAmount * feedbackRight;

        left[sample] = inputLeft + mixAmount * (delayedLeft - inputLeft);

        if (numChannels > 1)
            right[sample] = inputRight + mixAmount * (delayedRight - inputRight);

        advanceWriteIndex();
    }
}

void Delay::reset() {
    for (auto& channelBuffer : delayBuffer)
        std::fill(channelBuffer.begin(), channelBuffer.end(), 0.0f);

    feedbackLowpassState = {0.0f, 0.0f};
    writeIndex = 0;

    const auto currentDelay = getDelaySamplesForDivision(syncDivision, tempoBpm, sampleRate);
    delaySamplesSmoothed.setCurrentAndTargetValue(currentDelay);
    feedbackSmoothed.setCurrentAndTargetValue(feedback);
    mixSmoothed.setCurrentAndTargetValue(mix);
}

int Delay::getLatencySamples() const {
    return 0;
}

void Delay::setTempoBpm(const double newTempoBpm) noexcept {
    const auto clampedTempo = clampTempo(newTempoBpm);

    if (std::abs(clampedTempo - tempoBpm) < static_cast<double>(delayconfig::parameterEpsilon))
        return;

    tempoBpm = clampedTempo;
    updateDelayTarget();
}

void Delay::setSyncDivision(const SyncDivision newDivision) noexcept {
    if (newDivision == syncDivision)
        return;

    syncDivision = newDivision;
    updateDelayTarget();
}

void Delay::setFeedback(const float newFeedback) noexcept {
    feedback = clampUnitRange(newFeedback, delayconfig::feedbackMin, delayconfig::feedbackMax);
    feedbackSmoothed.setTargetValue(feedback);
}

void Delay::setMix(const float newMix) noexcept {
    mix = clampUnitRange(newMix, delayconfig::mixMin, delayconfig::mixMax);
    mixSmoothed.setTargetValue(mix);
}

void Delay::setTopology(const Topology newTopology) noexcept {
    topology = newTopology;
}

Delay::SyncDivision Delay::syncDivisionFromIndex(const int index) noexcept {
    const auto clampedIndex = std::clamp(index, 0, getSyncDivisionCount() - 1);
    return static_cast<SyncDivision>(clampedIndex);
}

int Delay::syncDivisionToIndex(const SyncDivision division) noexcept {
    return std::clamp(static_cast<int>(division), 0, getSyncDivisionCount() - 1);
}

const char* Delay::getSyncDivisionName(const SyncDivision division) noexcept {
    switch (division) {
    case SyncDivision::Quarter:
        return "1/4";
    case SyncDivision::DottedQuarter:
        return "1/4 dotted";
    case SyncDivision::QuarterTriplet:
        return "1/4 triplet";
    case SyncDivision::Eighth:
        return "1/8";
    case SyncDivision::DottedEighth:
        return "1/8 dotted";
    case SyncDivision::EighthTriplet:
        return "1/8 triplet";
    case SyncDivision::Sixteenth:
        return "1/16";
    case SyncDivision::DottedSixteenth:
        return "1/16 dotted";
    case SyncDivision::SixteenthTriplet:
        return "1/16 triplet";
    }

    return "1/8 dotted";
}

float Delay::getSyncDivisionBeats(const SyncDivision division) noexcept {
    switch (division) {
    case SyncDivision::Quarter:
        return 1.0f;
    case SyncDivision::DottedQuarter:
        return 1.5f;
    case SyncDivision::QuarterTriplet:
        return 2.0f / 3.0f;
    case SyncDivision::Eighth:
        return 0.5f;
    case SyncDivision::DottedEighth:
        return 0.75f;
    case SyncDivision::EighthTriplet:
        return 1.0f / 3.0f;
    case SyncDivision::Sixteenth:
        return 0.25f;
    case SyncDivision::DottedSixteenth:
        return 0.375f;
    case SyncDivision::SixteenthTriplet:
        return 1.0f / 6.0f;
    }

    return 0.75f;
}

float Delay::getDelaySamplesForDivision(const SyncDivision division, const double bpm,
                                        const double targetSampleRate) noexcept {
    const auto secondsPerBeat = 60.0 / clampTempo(bpm);
    const auto seconds = secondsPerBeat * static_cast<double>(getSyncDivisionBeats(division));
    return static_cast<float>(seconds * std::max(1.0, targetSampleRate));
}

float Delay::readDelayedSample(const int channel, const float delaySamples) const noexcept {
    const auto clampedDelay =
        std::clamp(delaySamples, 1.0f, static_cast<float>(maxDelaySamples - 2));
    auto readPosition = static_cast<float>(writeIndex) - clampedDelay;

    while (readPosition < 0.0f)
        readPosition += static_cast<float>(maxDelaySamples);

    const auto index0 = static_cast<int>(std::floor(readPosition));
    const auto index1 = (index0 + 1) % maxDelaySamples;
    const auto fraction = readPosition - static_cast<float>(index0);
    const auto& buffer = delayBuffer[static_cast<std::size_t>(channel)];

    return buffer[static_cast<std::size_t>(index0)] +
           fraction * (buffer[static_cast<std::size_t>(index1)] -
                       buffer[static_cast<std::size_t>(index0)]);
}

float Delay::processFeedbackLowpass(const int channel, const float sample) noexcept {
    auto& state = feedbackLowpassState[static_cast<std::size_t>(channel)];
    state += feedbackLowpassCoefficient * (sample - state);
    return state;
}

float Delay::getSmoothedDelaySamples() noexcept {
    return delaySamplesSmoothed.getNextValue();
}

float Delay::getSmoothedFeedback() noexcept {
    return feedbackSmoothed.getNextValue();
}

float Delay::getSmoothedMix() noexcept {
    return mixSmoothed.getNextValue();
}

void Delay::updateDelayTarget() noexcept {
    delaySamplesSmoothed.setTargetValue(
        getDelaySamplesForDivision(syncDivision, tempoBpm, sampleRate));
}

void Delay::advanceWriteIndex() noexcept {
    writeIndex = (writeIndex + 1) % maxDelaySamples;
}
