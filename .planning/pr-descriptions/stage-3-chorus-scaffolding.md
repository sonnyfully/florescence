# Stage 3 Chorus Scaffolding

## What changed

This PR adds the `Chorus` DSP module scaffold for Stage 3. The module conforms to `FXModule`, exposes depth/rate/mix setters, contains a private two-voice structure for the future clean-room BBD-flavoured implementation, and currently passes audio through unchanged.

## Why

The Stage 3 chorus decisions are now merged, so the next safe step is to prepare the module shape without starting the actual BBD DSP. This gives the implementation branch a clear landing pad while preserving the strict PR sequence.

## Considered and rejected

- Implementing modulated delay DSP now: rejected because this PR is scaffolding only; `feature/chorus-implementation` will do the real DSP after the test contract lands.
- Adding chorus tests now: rejected because PR 3 owns `Tests/test_chorus.cpp`.
- Wiring the pass-through module into `PluginProcessor`: rejected for this scaffold because it would change the processor chain before tests define the verification contract.
- Adding new dependencies: rejected; the locked decision is JUCE/clean-room only.

## Testing

Current:

- `cmake --build build-juce8 --target CharacterFX_All CharacterFX_Tests CharacterFX_PluginSmokeTests`
- `ctest --test-dir build-juce8 --output-on-failure`

All 19 discovered tests pass, including the VST3 smoke test.

## Stop points

Already resolved in `docs/decisions.md`:

- Q-CHOR-1-LIC: ChowDSP BBD / `chowdsp_dsp_utils` unavailable for v1.
- Q-CHOR-1: clean-room BBD-flavoured delay-line modulation.
- Q-CHOR-2: 2 voices, Juno-style, decorrelated L/R LFO phases.

## Follow-ups

- PR 3: add `Tests/test_chorus.cpp` as the failing verification contract.
- Later: implement actual BBD-flavoured DSP on `feature/chorus-implementation`.
