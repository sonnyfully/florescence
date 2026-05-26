#pragma once

#include "DelayConfig.h"
#include "FXModule.h"

#include <array>
#include <vector>

// Tempo-synced stereo character delay with filtered feedback. References:
// docs/decisions.md 2026-05-26 Stage 4 decisions; Zolzer DAFX Chapter 2;
// Smith PASP delay lines.
class Delay final : public FXModule {
  public:
    enum class SyncDivision {
        Quarter = 0,
        DottedQuarter,
        QuarterTriplet,
        Eighth,
        DottedEighth,
        EighthTriplet,
        Sixteenth,
        DottedSixteenth,
        SixteenthTriplet
    };

    enum class Topology { Stereo = 0, PingPong };

    Delay() = default;
    explicit Delay(const juce::dsp::ProcessSpec& spec);

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void process(juce::dsp::AudioBlock<float>& block) override;
    void reset() override;

    [[nodiscard]] int getLatencySamples() const override;

    void setTempoBpm(double newTempoBpm) noexcept;
    void setSyncDivision(SyncDivision newDivision) noexcept;
    void setFeedback(float newFeedback) noexcept;
    void setMix(float newMix) noexcept;
    void setTopology(Topology newTopology) noexcept;

    [[nodiscard]] double getTempoBpm() const noexcept {
        return tempoBpm;
    }

    [[nodiscard]] SyncDivision getSyncDivision() const noexcept {
        return syncDivision;
    }

    [[nodiscard]] float getFeedback() const noexcept {
        return feedback;
    }

    [[nodiscard]] float getMix() const noexcept {
        return mix;
    }

    [[nodiscard]] Topology getTopology() const noexcept {
        return topology;
    }

    [[nodiscard]] static constexpr int getSyncDivisionCount() noexcept {
        return 9;
    }

    [[nodiscard]] static constexpr SyncDivision getDefaultSyncDivision() noexcept {
        return SyncDivision::DottedEighth;
    }

    [[nodiscard]] static SyncDivision syncDivisionFromIndex(int index) noexcept;
    [[nodiscard]] static int syncDivisionToIndex(SyncDivision division) noexcept;
    [[nodiscard]] static const char* getSyncDivisionName(SyncDivision division) noexcept;
    [[nodiscard]] static float getSyncDivisionBeats(SyncDivision division) noexcept;
    [[nodiscard]] static float getDelaySamplesForDivision(SyncDivision division, double bpm,
                                                          double sampleRate) noexcept;

  private:
    [[nodiscard]] float readDelayedSample(int channel, float delaySamples) const noexcept;
    [[nodiscard]] float processFeedbackLowpass(int channel, float sample) noexcept;
    [[nodiscard]] float getSmoothedDelaySamples() noexcept;
    [[nodiscard]] float getSmoothedFeedback() noexcept;
    [[nodiscard]] float getSmoothedMix() noexcept;
    void updateDelayTarget() noexcept;
    void advanceWriteIndex() noexcept;

    std::array<std::vector<float>, 2> delayBuffer;
    std::array<float, 2> feedbackLowpassState{};
    double sampleRate = 48000.0;
    double tempoBpm = delayconfig::tempoDefaultBpm;
    int maxDelaySamples = 1;
    int writeIndex = 0;
    SyncDivision syncDivision = getDefaultSyncDivision();
    Topology topology = Topology::Stereo;
    float feedback = delayconfig::feedbackDefault;
    float mix = delayconfig::mixDefault;
    float feedbackLowpassCoefficient = 0.0f;
    juce::SmoothedValue<float> delaySamplesSmoothed;
    juce::SmoothedValue<float> feedbackSmoothed;
    juce::SmoothedValue<float> mixSmoothed;
};
