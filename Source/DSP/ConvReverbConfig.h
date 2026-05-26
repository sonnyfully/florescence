#pragma once

namespace convreverbconfig {

constexpr float wetMixMin = 0.0f;
constexpr float wetMixMax = 1.0f;
constexpr float wetMixDefault = 0.0f;

// Q-IR-4, resolved 2026-05-26: fixed subtle pre-convolution chorus.
// Depth is 0.2 percent of the current sample rate, i.e. roughly 2ms at 48kHz.
constexpr float wetPathModulationDepthPercent = 0.002f;
constexpr float wetPathModulationRateHz = 0.4f;
constexpr float wetPathModulationCenterDelayMs = 8.0f;

// Deterministic fallback IR used until the licensed Stage 4 IR library lands.
// The real 10-IR library is a separate PR because every asset needs licence
// verification before entering Resources/IRs.
constexpr int fallbackImpulseLengthSamples = 2048;
constexpr float fallbackImpulseDecay = 0.997f;
constexpr float fallbackImpulseEarlyReflectionGain = 0.35f;

constexpr double parameterSmoothingTimeSeconds = 0.02;
constexpr float parameterEpsilon = 0.000001f;
constexpr float outputBoundForTests = 4.0f;

} // namespace convreverbconfig
