#pragma once

namespace chorusconfig {

constexpr float depthMin = 0.0f;
constexpr float depthMax = 1.0f;
constexpr float mixMin = 0.0f;
constexpr float mixMax = 1.0f;
constexpr float rateMinHz = 0.1f;
constexpr float rateMaxHz = 10.0f;

// Juno-style chorus starting point from the public Stage 3 spec and the
// Juno-60 / Juno-106 service-analysis trail at:
// https://www.florian-anwander.de/roland_string_choruses/
constexpr float centerDelayMs = 7.0f;

// Full-depth modulation spans roughly 3ms..11ms around the 7ms centre, staying
// in chorus territory rather than sub-millisecond flanging.
// https://www.florian-anwander.de/roland_string_choruses/
constexpr float modulationRangeMs = 4.0f;

// Wet-path bandwidth loss approximates the filtering around BBD chorus lines;
// Raffel/Smith discuss practical BBD circuits as delay plus surrounding filters.
// https://www.dafx.de/paper-archive/details/JhVfAOFXD1lAtctkMTUODg
constexpr float wetLowpassCutoffHz = 6000.0f;

// 10ms is fast enough for parameter automation to feel responsive while avoiding
// obvious zippering on depth, rate, and mix moves.
constexpr double parameterSmoothingTimeSeconds = 0.01;

constexpr float outputBoundForTests = 2.0f;

} // namespace chorusconfig
