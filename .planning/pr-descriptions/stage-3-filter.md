# Stage 3 Filter

## Status

This PR completes the remaining Stage 3 gap after Chorus merged. Q-FILT-1 and Q-FILT-2 are now resolved and moved to `docs/decisions.md`.

## Stage 3 gap

Stage 3 is not fully implemented yet:

- Done: north-star docs/planning PR.
- Done: Chorus module, tests, visualization notebook, and mono-sum decision.
- Missing: `Source/DSP/Filter.{h,cpp}`.
- Missing: `Tests/test_filter.cpp`.
- Missing: filter frequency-response / envelope-following notebook.
- Missing: chain integration after Chorus.
- Missing: Stage 3 A/B pass for Tilt -> Saturation -> Chorus -> Filter.

## Stop points resolved

### Q-FILT-1: Filter topology

- Decision: TPT SVF low-pass only.
- Implementation: `juce::dsp::StateVariableTPTFilter`, `StateVariableTPTFilterType::lowpass`.
- Rejected: ladder, biquad, alternate filter modes, and ChowDSP/GPL filter modules.

### Q-FILT-2: Envelope follower scope

- Decision: input-following only for v1.
- No sidechain bus, sidechain UI, or internal/sidechain mode toggle.
- Follower tap: the signal entering `Filter`, downstream of Saturation in the fixed chain. In Stage 3 this is post-Saturation/post-Chorus audio, not raw plugin input.

## New open question

Q-FILT-2-TUNING tracks Stage 6 by-ear tuning for the audition-baseline envelope attack/release/depth values.

## Implementation

- Add `FilterConfig.h` for named constants and references.
- Implement `Filter` as an `FXModule`.
- Expose cutoff, resonance, envelope amount, and follower timing internally.
- Use audio-thread-safe smoothing; no allocations, locks, file I/O, or logging in `process`.
- Add deterministic Catch2 coverage for silence, stereo coherence, cutoff behavior, resonance bounds, envelope modulation, smooth ramps, finite output, and latency reporting.
- Add a notebook for frequency response and envelope-following plots.
- Wire the module after Chorus in `PluginProcessor`.

## Testing plan

- `cmake --build build-juce8 --target CharacterFX_All CharacterFX_Tests CharacterFX_PluginSmokeTests`
- `ctest --test-dir build-juce8 --output-on-failure`
- Notebook plots for frequency response and envelope response.
- A/B listening check on a sustained synth/pad or bus source after the implementation PR becomes audible.

## Current testing

- `cmake --build build-juce8 --target CharacterFX_Tests`
- `cmake --build build-juce8 --target CharacterFX_All CharacterFX_Tests CharacterFX_PluginSmokeTests`
- `ctest --test-dir build-juce8 --output-on-failure`

Current result: 38/38 tests pass after adding `Tests/test_filter.cpp`, including the VST3 smoke test with Stage 3 filter parameters.

Not yet completed in this environment: human standalone A/B on a sustained synth/pad source. The standalone target builds and exposes temporary Filter controls, but the final Stage 3 listening check still needs ears.
