# Convolution Reverb - research and implementation notes

The Convolution Reverb stage sits after Delay at the end of the fixed v1 chain:

`Tilt EQ -> Saturation -> Chorus -> Filter -> Delay -> ConvReverb`

Its job is cinematic space. This is the stage that should make Florescence feel
expensive, physical, and placed inside a nocturnal scene rather than just
processed.

## Locked Stage 4 decisions

See `docs/decisions.md` 2026-05-26.

- **Algorithm:** convolution reverb via `juce::dsp::Convolution`, not an
  algorithmic FDN.
- **IR library size:** 10 IRs in the separate initial library PR: 3 plate,
  2 hall, 2 chamber, and 3 designed.
- **Character behaviour:** all IRs are available in all Character modes.
  Character biases default IR choice per Atmosphere zone; the user can
  override later if the UI exposes that path.
- **Front panel:** IR selection is not a separate v1 front-panel control.
- **Wet path modulation:** fixed subtle pre-convolution chorus, single voice,
  depth around 0.2 percent, rate around 0.4 Hz, not user-exposed.

## Implementation

`Source/DSP/ConvReverb.{h,cpp}` implements `FXModule`.

Signal flow:

```text
input
   |
   +--> dry path ----------------------------------------------------+
   |                                                                |
   +--> fixed wet modulation -> juce::dsp::Convolution -> wet mix --+--> output
```

The module loads a deterministic generated fallback IR until the licensed IR
library lands. This keeps the chain testable without placing unverified audio
assets in `Resources/IRs/`.

The wet-path modulation is intentionally fixed and subtle. It uses one shared
LFO phase for both channels, so it does not create extra stereo decorrelation;
the goal is a slow breathing input into the convolution, not a chorus effect
that the user consciously hears.

`process()` performs no allocations, locks, or file I/O. IR loading is kept out
of the processing path. The public `loadImpulseResponse()` wrapper accepts an
owned `juce::AudioBuffer<float>` so callers can prepare generated or loaded IRs
off the audio thread before handing them to JUCE's convolution engine.

## Tests

`Tests/test_convreverb.cpp` covers:

- fallback IR loading
- custom identity IR behaviour
- determinism
- silence
- zero-mix dry pass-through
- reset clearing wet-path state
- finite, bounded output under wet-mix sweeps

## References

- JUCE `juce::dsp::Convolution` documentation and implementation:
  `external/JUCE/modules/juce_dsp/frequency/juce_Convolution.h`
- Zolzer, U. (ed.) - *DAFX: Digital Audio Effects*, 2nd edition, Wiley, 2011.
  Chapter 2, "Filters and delays." Official book page:
  https://dafx.de/DAFX_Book_Page_2nd_edition/index.html
