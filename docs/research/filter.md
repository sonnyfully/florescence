# Filter - research and implementation notes

The Filter stage sits after Chorus in the fixed v1 chain:

`Tilt EQ -> Saturation -> Chorus -> Filter`

Its job is gentle low-pass movement: the source gets darker under louder,
denser passages and opens back up through quieter or sustained passages. It is
not a character filter product, ladder emulation, or multimode synth filter.

## Locked Stage 3 decisions

See `docs/decisions.md` 2026-05-26.

- **Topology:** `juce::dsp::StateVariableTPTFilter`, low-pass only.
- **Envelope scope:** input-following only for v1. No sidechain bus, routing,
  sidechain/internal toggle, or hidden pro mode.
- **Follower tap:** the signal entering `Filter`, downstream of Saturation in
  the fixed chain. In the current Stage 3 chain that also means post-Chorus.
- **Tuning:** attack 10ms, release 150ms, depth 0.5 octaves are audition
  baselines. Q-FILT-2-TUNING tracks Stage 6 listening review.

## Implementation

`Source/DSP/Filter.{h,cpp}` implements `FXModule`.

Signal flow:

```text
filter input
   |
   +--> peak envelope follower -> cutoff modulation
   |
   +--> TPT SVF low-pass -> output
```

The envelope is peak-following across channels. Loud input lowers the cutoff by
an octave-scaled amount:

```text
cutoff = baseCutoff * 2^(-depthOctaves * envelope)
```

At the audition baseline depth of 0.5 and a normalized envelope of 1, the
cutoff ducks by a half octave. This is deliberately conservative; Stage 6 can
retune depth and timing once the full chain is audible.

## Tests

`Tests/test_filter.cpp` covers:

- cutoff modulation math
- determinism
- silence
- stereo coherence
- low-pass attenuation
- envelope-driven high-frequency ducking
- smooth parameter ramps
- zero latency
- finite, bounded output under parameter sweeps

## References

- JUCE `juce::dsp::StateVariableTPTFilter`: https://docs.juce.com/master/classjuce_1_1dsp_1_1StateVariableTPTFilter.html
- Zolzer, U. (ed.) - *DAFX: Digital Audio Effects*, 2nd edition, Wiley, 2011. Chapter 2, "Filters and delays." Official book page: https://dafx.de/DAFX_Book_Page_2nd_edition/index.html
- Zavalishin, V. - *The Art of VA Filter Design*, Native Instruments, 2012.
