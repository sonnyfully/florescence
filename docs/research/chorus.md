# Chorus - research, decisions, open questions

The chorus stage is the first explicit motion module in the Florescence chain. It runs after Tilt EQ and Saturation, before Filter. Its job is not to expose a generic chorus effect; it should make the chain breathe in a nocturnal, stereo, Juno-adjacent way while staying behind the Atmosphere / Burn / Pulse control surface.

**Status, 2026-05-25:** Stage 3 decisions are locked. v1 uses a clean-room BBD-flavoured modulated delay line, not ChowDSP code. See `docs/decisions.md`.

Read this end to end before implementing `Source/DSP/Chorus.cpp`.

## What the chorus needs to do

The aesthetic target is **recognisable stereo motion, not obvious seasick wobble**. It should add width, gloss, and slow movement on pads, vocal buses, and mix-bus atmosphere without becoming a standalone "chorus pedal" feature.

Functional requirements:

1. **Modulated delay line** - LFO-driven fractional delay with smooth interpolation.
2. **BBD-flavoured bandwidth loss** - HF softening on the delayed path so the wet signal does not read as clean digital modulation.
3. **True stereo** - two voices with decorrelated L/R LFO phases for width.
4. **Stable at zero depth** - identical L/R input should remain coherent when depth is zero.
5. **No GPL-derived DSP** - no `chowdsp_dsp_utils`, no CHOW Tape, no ChowDSP BBD source references.

## Algorithm choice

**Decision: clean-room BBD-flavoured delay-line modulation.**

The core algorithm is a conventional chorus: an LFO modulates a short delay time, a fractional-delay interpolator reads from a delay line, and the wet output is mixed with dry. The BBD flavour comes from modelling the surrounding behaviour rather than the exact chip internals: low-pass filtering on the delayed path, and later optional companding or mild delay-path saturation if listening tests prove the baseline too clean.

This is deliberately not a full circuit model. A full BBD model would include clock-rate interaction, anti-alias filters, sampling images, compander behaviour, distortion, and noise. Raffel and Smith document that full BBD systems commonly include low-pass filters and compander circuitry around the device; v1 borrows only the parts that matter most for the product target and schedule: delay modulation plus wet-path HF loss.

### Why not ChowDSP

`chowdsp_dsp_utils` is unavailable for v1. The upstream module is GPLv3, and using GPLv3 DSP code in Florescence would conflict with the closed-source commercial model unless a non-GPL licence were obtained. Commercial licensing is not being pursued for v1.

### Why not exact Juno circuit modelling

The Juno reference matters for topology and feel: mono-to-stereo, two BBD paths, one modulation source, and inverted/decorrelated modulation across left and right. Exact circuit modelling would expand the scope into component values, clock filtering, noise, compander details, and calibration. That is better suited to a dedicated chorus emulation product, not this atmosphere processor.

## Proposed signal flow

```text
depth/rate/mix parameters
        |
        v
LFO phase A --------------------> delay time A
LFO phase B (decorrelated) -----> delay time B

input L/R
   |
   +--> dry path ------------------------------------------------------+
   |                                                                  |
   +--> voice A delay line -> fractional read -> 1-pole LPF -> wet A --+--> left output
   |                                                                  |
   +--> voice B delay line -> fractional read -> 1-pole LPF -> wet B --+--> right output

optional later, if scope allows:
input -> light compressor -> delay -> LPF -> light expander -> wet
input -> delay-path soft saturation -> LPF -> wet
```

Initial stereo mapping should keep the dry input coherent and place the two wet voices across the stereo field. For a mono or identical L/R source, voice A feeds left and voice B feeds right; for stereo input, each side keeps its own dry path while the decorrelated wet voices create width. The exact crossfeed amount is a Stage 6 tuning question, not part of this decision.

## Pseudocode sketch

```cpp
for each sample:
    lfoA = sin(phase)
    lfoB = sin(phase + phaseOffset)

    delayMsA = centreDelayMs + depth * depthRangeMs * lfoA
    delayMsB = centreDelayMs + depth * depthRangeMs * lfoB

    delaySamplesA = delayMsA * sampleRate / 1000
    delaySamplesB = delayMsB * sampleRate / 1000

    voiceA.write(inputLeftOrMono)
    voiceB.write(inputRightOrMono)

    wetA = lowpass(voiceA.readFractional(delaySamplesA))
    wetB = lowpass(voiceB.readFractional(delaySamplesB))

    leftOut = dryMix * leftIn + wetMix * wetA
    rightOut = dryMix * rightIn + wetMix * wetB

    advance phase by rateHz / sampleRate
```

Implementation may use `juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd>` or an owned circular buffer with Lagrange interpolation. The first implementation should prefer JUCE unless its API fights the test requirements.

## Stage 6 tuning questions

These are not implementation blockers, but they need listening review during preset design:

- LFO rate range: likely around the Juno-adjacent slow range, but exact macro ranges belong to Pulse and Character tuning.
- Depth range: enough to feel alive on pads without making vocal buses seasick.
- Centre delay range: short enough to chorus rather than slap, long enough for width.
- HF rolloff frequency: set the delayed path dark enough to read BBD-flavoured without dulling Chrome mode.
- Companding on/off: optional for v1; add only if the clean-room delay + LPF baseline feels too sterile.
- Delay-path saturation on/off: optional for v1; add only if the wet path needs more density after listening.
- Wet crossfeed: decide whether each voice stays hard L/R or blends a little across channels.

## Test plan

DSP unit tests in `Tests/test_chorus.cpp`:

- **Determinism:** same input vector and parameters produce the same output vector after reset.
- **Silence in -> silence out:** zero input produces zero output.
- **Stereo coherence at zero depth:** identical L/R input remains identical when depth is zero.
- **Smoothness:** depth/rate/mix ramps produce no sample-to-sample discontinuities above a documented threshold.
- **Latency reporting:** `getLatencySamples()` reports 0 unless the implementation adds real lookahead or host-visible delay.
- **Finite output:** long processing with high depth/rate settings produces no NaN or infinity values.

## Implementation notes - 2026-05-25

`Source/DSP/Chorus.{h,cpp}` now implements the Stage 3 baseline:

- Two `ChorusVoice` instances, voice A at 0 degrees and voice B at 180 degrees.
- Both voices receive the same mono-summed input `(L + R) / 2`.
- Voice A returns the left wet signal; voice B returns the right wet signal.
- Dry L/R content is delayed internally by the reported centre latency before crossfading with wet. This keeps the latency report truthful and avoids an undelayed dry path fighting the centred wet path.
- Depth, rate, and mix use `juce::SmoothedValue` with a 10ms ramp.
- Per-sample LFO delay is not smoothed directly. The smoothed depth and rate feed the LFO equation, preserving modulation while avoiding zipper noise on automation.
- Wet-path HF loss is a clean-room one-pole low-pass at 6kHz, applied only after the delay read.
- Constants live in `Source/DSP/ChorusConfig.h` with public reference URLs next to the Juno-style timing and BBD bandwidth-loss choices.

The unit suite now covers determinism, silence, zero-depth stereo coherence,
smooth parameter ramps, latency reporting, LFO continuity on rate changes, mono
input widening, HF rolloff, bounded output, and mono-sum level retention.

Not tested in unit tests:

- Whether the chorus feels Juno-adjacent enough.
- Whether the HF rolloff frequency is emotionally right in Velvet / Onyx / Chrome.
- Whether companding or wet-path saturation is worth the added complexity.

Those are Stage 6 A/B listening questions against representative synth, vocal, and mix-bus material.

## References

- Zolzer, U. (ed.) - *DAFX: Digital Audio Effects*, 2nd edition, Wiley, 2011. Chapter 2, "Filters and delays." Official book page: https://dafx.de/DAFX_Book_Page_2nd_edition/index.html
- Pirkle, W. - *Designing Audio Effect Plugins in C++: For AAX, AU, and VST3 with DSP Theory*, 2nd edition, Routledge/Focal Press, 2019. Chapter 15, "Modulated Delay Effects." Table of contents: https://www.oreilly.com/library/view/designing-audio-effect/9780429954313/
- Pirkle, W. - "ModulatedDelay" project note for Chapter 15 examples. https://www.willpirkle.com/fx-book/project-gallery/modulateddelay/
- Smith, J. O. - *Physical Audio Signal Processing*, online book, "Delay Lines." https://ccrma.stanford.edu/~jos/pasp/Delay_Lines.html
- Raffel, C. and Smith, J. O. - "Practical Modeling of Bucket-Brigade Device Circuits," Proceedings of DAFx-10, Graz, Austria, 2010. https://www.dafx.de/paper-archive/details/JhVfAOFXD1lAtctkMTUODg
- Anwander, F. - "Roland EnsembleFX Choruses," Juno-6/Juno-60 mono-to-stereo chorus notes and service-manual pointers. https://www.florian-anwander.de/roland_string_choruses/
- Roland Juno-60 overview, chorus reference notes. https://en.wikipedia.org/wiki/Roland_Juno-60
