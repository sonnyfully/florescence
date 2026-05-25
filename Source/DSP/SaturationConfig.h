#pragma once

namespace saturationconfig {

constexpr int oversamplingExponent = 2;
constexpr int oversamplingFactor = 1 << oversamplingExponent;

constexpr float driveMin = 0.0f;
constexpr float driveMax = 1.0f;
constexpr float driveSmoothingTimeSeconds = 0.02f;
constexpr float driveParameterEpsilon = 0.000001f;

constexpr float minClipDrive = 0.0001f;
constexpr float maxClipDrive = 5.0f;

constexpr float detectorAttackMs = 10.0f;
constexpr float detectorReleaseMs = 150.0f;

constexpr float lpfMaxCutoffHz = 20000.0f;
constexpr float lpfMinCutoffHz = 6000.0f;
constexpr float lpfResonance = 0.5f;

constexpr float dcBlockerCutoffHz = 10.0f;
constexpr float transparencyToleranceDb = -80.0f;

} // namespace saturationconfig
