# Delay - research and implementation notes

The Delay stage sits after Filter and before Convolution Reverb in the fixed v1
chain:

`Tilt EQ -> Saturation -> Chorus -> Filter -> Delay -> ConvReverb`

Its job is a tempo-synced character delay: rhythmic repeats that darken as they
feed back, with enough stereo movement to support the nocturnal atmosphere
without becoming a full-featured utility delay.

## Locked Stage 4 decisions

See `docs/decisions.md` 2026-05-26.

- **Sync divisions:** 1/4, dotted 1/4, 1/4 triplet, 1/8, dotted 1/8,
  1/8 triplet, 1/16, dotted 1/16, and 1/16 triplet.
- **Default division:** dotted 1/8.
- **Skipped divisions:** 1/2 and 1/1, because they are too long for a character
  delay.
- **Topology:** internal `Stereo` / `PingPong` mode toggle, automatable but not
  shown on the v1 main GUI.
- **Feedback:** filtered feedback so repeats get darker rather than brighter.

## Implementation

`Source/DSP/Delay.{h,cpp}` implements `FXModule`.

Signal flow:

```text
tempo + division -> delay samples

input L/R
   |
   +--> dry/wet mix -----------------------------------> output L/R
   |
   +--> delay lines -> filtered feedback -> write input + feedback
```

Stereo topology keeps each channel's feedback independent. Ping-pong topology
crossfeeds the delayed output: left repeat feeds the right delay line and right
repeat feeds the left delay line.

The module owns circular buffers allocated in `prepare()`. `process()` performs
no allocations, locks, or file I/O. Fractional read positions use linear
interpolation, which is sufficient for tempo-sync delay times and smoothed host
tempo/division changes.

The initial feedback low-pass cutoff is 4kHz. This is a deliberately simple
starting value that keeps repeats behind the dry source; Stage 6 can retune it
per Character mode if listening shows the repeats are too dull or too bright.

## Tests

`Tests/test_delay.cpp` covers:

- resolved Stage 4 division set and default
- dotted 1/8 timing at 120 BPM
- determinism
- silence
- zero-mix dry pass-through
- stereo topology channel independence
- ping-pong crossfeed
- reset clearing repeats
- zero host-visible latency
- finite, bounded output under parameter sweeps

## References

- Zolzer, U. (ed.) - *DAFX: Digital Audio Effects*, 2nd edition, Wiley, 2011.
  Chapter 2, "Filters and delays." Official book page:
  https://dafx.de/DAFX_Book_Page_2nd_edition/index.html
- Smith, J. O. - *Physical Audio Signal Processing*, online book, "Delay
  Lines." https://ccrma.stanford.edu/~jos/pasp/Delay_Lines.html
