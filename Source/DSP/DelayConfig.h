#pragma once

namespace delayconfig {

constexpr float feedbackMin = 0.0f;
constexpr float feedbackMax = 0.85f;
constexpr float feedbackDefault = 0.25f;

constexpr float mixMin = 0.0f;
constexpr float mixMax = 1.0f;
constexpr float mixDefault = 0.0f;

constexpr double tempoMinBpm = 40.0;
constexpr double tempoMaxBpm = 240.0;
constexpr double tempoDefaultBpm = 120.0;

// Longest approved Stage 4 division is dotted 1/4. At 40 BPM that is 2.25s;
// 3s leaves headroom for tempo smoothing and host tempo edge cases.
constexpr double maxDelaySeconds = 3.0;

// 4kHz keeps feedback repeats darker without turning the delay into a muffled
// special effect. Stage 6 can retune this inside Character snapshots.
constexpr float feedbackLowpassCutoffHz = 4000.0f;

// Tempo/division automation should glide quickly enough to avoid zippering
// without making rhythmic changes feel disconnected from the control gesture.
constexpr double delayTimeSmoothingSeconds = 0.02;
constexpr double parameterSmoothingTimeSeconds = 0.01;

constexpr float parameterEpsilon = 0.000001f;
constexpr float outputBoundForTests = 4.0f;

} // namespace delayconfig
