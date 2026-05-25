#pragma once

namespace filterconfig {

constexpr float cutoffMinHz = 200.0f;
constexpr float cutoffMaxHz = 20000.0f;
constexpr float cutoffDefaultHz = 12000.0f;

constexpr float resonanceMin = 0.1f;
constexpr float resonanceMax = 1.0f;
constexpr float resonanceDefault = 0.5f;

constexpr float envelopeDepthMinOctaves = 0.0f;
constexpr float envelopeDepthMaxOctaves = 2.0f;

// Audition baseline values - final tuning in Stage 6 (Q-FILT-2-TUNING).
constexpr float envelopeAttackMs = 10.0f;
constexpr float envelopeReleaseMs = 150.0f;
constexpr float envelopeDefaultDepthOctaves = 0.5f;

constexpr double parameterSmoothingTimeSeconds = 0.01;
constexpr float parameterEpsilon = 0.000001f;
constexpr float outputBoundForTests = 2.0f;

} // namespace filterconfig
