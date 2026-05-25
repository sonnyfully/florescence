# Stage 3 Filter

## Status

This PR is started to complete the remaining Stage 3 gap after Chorus merged. It is currently blocked at the required Filter stop points in `docs/open_questions.md`; no Filter DSP code should be written until Q-FILT-1 and Q-FILT-2 are explicitly resolved and moved to `docs/decisions.md`.

## Stage 3 gap

Stage 3 is not fully implemented yet:

- Done: north-star docs/planning PR.
- Done: Chorus module, tests, visualization notebook, and mono-sum decision.
- Missing: `Source/DSP/Filter.{h,cpp}`.
- Missing: `Tests/test_filter.cpp`.
- Missing: filter frequency-response / envelope-following notebook.
- Missing: chain integration after Chorus.
- Missing: Stage 3 A/B pass for Tilt -> Saturation -> Chorus -> Filter.

## Stop points surfaced

### Q-FILT-1: Filter topology

- Where it lives: `Source/DSP/Filter.cpp`
- Why it blocks: this determines the core filter implementation, test expectations, and how modulation behaves under resonance/cutoff sweeps.
- Options: SVF (TPT), ladder (Moog-style), biquad.
- Tentative lean: SVF (TPT), because `docs/ARCHITECTURE.md` already frames the Stage 3 filter as a gentle TPT state-variable low-pass and the project already uses JUCE TPT SVF successfully in Saturation.

### Q-FILT-2: Envelope follower scope

- Where it lives: `Source/DSP/Filter.cpp`
- Why it blocks: this determines whether the module needs only internal input-following DSP or host sidechain bus plumbing.
- Options: input signal only, sidechain input, both switchable.
- Tentative lean: input signal only for v1, sidechain deferred to v1.x, because the Stage 3 goal is the slow "duck" feel and the roadmap explicitly marks sidechain routing as out of scope for Stage 3.

## Planned implementation after decisions

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
