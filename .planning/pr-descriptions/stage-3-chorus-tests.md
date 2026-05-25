# Stage 3 Chorus Tests

## What changed

This PR adds `Tests/test_chorus.cpp` as the verification contract for the upcoming Chorus implementation. The file contains the requested failing Catch2 stubs for determinism, silence handling, zero-depth stereo coherence, smooth parameter ramps, and latency reporting.

## Why

PR 2 established the Chorus module scaffold without implementing the BBD-flavoured DSP. This PR defines the tests that the later `feature/chorus-implementation` branch must make pass, keeping the implementation work honest and scoped.

## Considered and rejected

- Implementing assertions against the current pass-through scaffold: rejected because that would mostly prove the scaffold is inert, not that the real chorus behaviour is correct.
- Adding actual chorus DSP now: rejected because this PR is the test contract only.
- Adding new dependencies or reference code: rejected; the Stage 3 chorus decision remains JUCE / clean-room only.

## Testing

Expected current state:

- Build should compile the new test file.
- `ctest --test-dir build-juce8 --output-on-failure` is expected to fail on the five Chorus contract stubs until `feature/chorus-implementation` replaces the stubs with executable assertions.

## Stop points

Already resolved in `docs/decisions.md`:

- Q-CHOR-1-LIC: ChowDSP BBD / `chowdsp_dsp_utils` unavailable for v1.
- Q-CHOR-1: clean-room BBD-flavoured delay-line modulation.
- Q-CHOR-2: 2 voices, Juno-style, decorrelated L/R LFO phases.

## Follow-ups

- `feature/chorus-implementation`: replace the failing stubs with real assertions and implement the Chorus module until they pass.
