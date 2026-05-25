# Stage 3 Chorus Implementation

## What changed

This PR implements the Stage 3 clean-room Chorus DSP behind the existing `FXModule` scaffold. It replaces the failing contract stubs in `Tests/test_chorus.cpp` with executable assertions, adds the requested extra chorus tests, adds notebook visualisation artefacts, and records the mono-sum tradeoff note required by the roadmap.

The Chorus module is also wired into the temporary Stage 3 plugin chain after Saturation, with direct Depth / Rate / Mix parameters and rough GUI sliders until the later macro layer replaces direct module controls.

## Why

The Stage 3 chorus decisions are locked: v1 uses a two-voice Juno-style, BBD-flavoured modulated delay line with no GPL-derived DSP. This PR turns the verification contract into the first working Chorus module so the chain can start feeling alive before Filter work begins.

## Considered and rejected

- Companding: rejected for this PR because the Stage 3 decision parks it as optional Stage 6 listening work.
- Delay-path saturation: rejected for the same reason; the baseline is delay modulation plus wet-path HF loss.
- Third voice or density mode: rejected because Q-CHOR-2 locks v1 to two voices.
- New dependencies or ChowDSP references: rejected; implementation stays JUCE / clean-room only.

## Testing

Current:

- `cmake --build build-juce8 --target CharacterFX_Tests`
- `cmake --build build-juce8 --target CharacterFX_All CharacterFX_Tests CharacterFX_PluginSmokeTests`
- `ctest --test-dir build-juce8 --output-on-failure`

All automated tests pass locally: 29/29.

Notebook plots live in `notebooks/chorus_visualization.ipynb`.

Not yet completed in this environment: human standalone A/B with an actual synth pad source. The standalone target builds and exposes temporary Chorus controls, but the listening check still needs ears on a local audio source before merge if we want to treat it as an acceptance gate.

## Stop points

Already resolved in `docs/decisions.md`:

- Q-CHOR-1-LIC: ChowDSP BBD / `chowdsp_dsp_utils` unavailable for v1.
- Q-CHOR-1: clean-room BBD-flavoured delay-line modulation.
- Q-CHOR-2: 2 voices, Juno-style, decorrelated L/R LFO phases.

## Follow-ups

- Stage 6 tuning should revisit whether the fixed 7 ms center, 4 ms modulation range, and 6 kHz wet-path rolloff feel right across Velvet / Onyx / Chrome presets.
