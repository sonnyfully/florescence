#pragma once

namespace dspconfig {

constexpr float tiltEqMinDb = -12.0f;
constexpr float tiltEqMaxDb = 12.0f;
constexpr float tiltEqPivotHz = 1000.0f;
constexpr float tiltEqShelfSlope = 1.0f;
constexpr float tiltEqParameterEpsilonDb = 0.000001f;
constexpr float minFilterFrequencyHz = 20.0f;
constexpr float nyquistGuard = 0.49f;

} // namespace dspconfig
