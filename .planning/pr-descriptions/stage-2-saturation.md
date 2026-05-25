# Stage 2 Saturation

## What changed

This PR completes the remaining Stage 2 DSP work by adding the Saturation module after Tilt EQ in the fixed processor chain. It exposes a direct saturation drive parameter for the throwaway Stage 2 GUI, adds deterministic DSP tests, and includes a notebook artefact for the saturation curve and harmonic behaviour.

## Why

Stage 2 is not complete with Tilt EQ alone. Saturation is the aesthetic anchor for the whole chain, and the downstream Stage 3 chorus/filter work should not start until the pre-shape and tape-style colour path is present and testable.

## Considered and rejected

- Starting Stage 3 immediately: rejected because `docs/roadmap.md` says stages are sequential and Saturation is not implemented yet.
- Hiding the saturation behind the future macro layer: rejected because macro mapping belongs to Stage 5; Stage 2 uses direct throwaway controls.
- Using ChowDSP's `ChowTapeModel`: rejected because the CHOW Tape source and relevant `chowdsp_utils` modules are GPLv3, which conflicts with the closed-source commercial plugin model unless a commercial licence is obtained.
- Picking silent defaults for saturation stop points: rejected because Q-SAT-1, Q-SAT-4, and Q-SAT-5 were explicit hard stops in `docs/open_questions.md`.

## Testing

Current:

- `cmake -S . -B build-juce8`
- `cmake --build build-juce8 --target CharacterFX_All CharacterFX_Tests CharacterFX_PluginSmokeTests`
- `ctest --test-dir build-juce8 --output-on-failure`

All 13 discovered tests pass, including saturation transparency at drive 0, silence, DC blocking, determinism, stereo coherence, dynamic cutoff mapping, and high-frequency reduction under drive.

Human A/B listening test passed on Stage 2 reference material on 2026-05-25. Saturation character is in the right neighbourhood for moving into Stage 3.

## Stop points

Resolved in `docs/decisions.md` on 2026-05-25:

- Q-SAT-1: 4x oversampling, compile-time constant.
- Q-SAT-3: N/A after algorithm change.
- Q-SAT-4: no static internal EQ.
- Q-SAT-5: unity behaviour at drive 0.
- Q-SAT-6: drive-normalised tanh.
- Q-SAT-7: peak detector, 10ms attack / 150ms release.
- Q-SAT-8: TPT SVF low-pass, Q=0.5, 20kHz to 6kHz exponential mapping.

Deferred but surfaced:

- Q-SAT-2: drive curve shape remains a linear placeholder until Stage 5/6 macro and preset tuning.
- Q-CHOR-1-LIC: chowdsp BBD licensing must be resolved before Stage 3 chorus work.
