# Saturation — research, decisions, open questions

The saturation stage is the single most aesthetically-loaded module in the chain. It runs after Tilt EQ, before Chorus (and in v2, before the neural voicing stage). Get it right and the whole chain has a center of gravity. Get it wrong and the plugin sounds generic.

Read this end to end before implementing `Source/DSP/Saturation.cpp`.

## What the saturation needs to do

The aesthetic target is **tape-style warmth, not transistor crunch**. Reference: how vocals sit in *The Knowing*, *Wicked Games*, *Belong to the World*; how synths feel in *Drive* (2011) cues; the analog-tape smear on Burial's *Archangel*.

Functional requirements:

1. **Asymmetric** — adds even harmonics dominantly, some odd. Symmetric saturation (pure tanh) is too clean and "digital."
2. **Frequency-dependent** — high frequencies saturate slightly more than lows. Real tape has this; it's part of why tape sounds "smooth."
3. **Dynamic / signal-dependent** — not a static waveshaper. Some hysteresis or memory so transients behave differently from sustained tones.
4. **Subtle at low drive, characterful at high drive** — most usage is 10–30% drive on the Character macro. Has to *do something* at low settings or the macro feels dead. Has to not fall apart at high settings either.
5. **Stereo coherent** — L and R saturate identically when fed identical signals (no false widening from independent processing).

## The candidate algorithms

### 1. Static waveshaping (tanh, soft-clip, polynomial)

The simplest approach. Pass the signal through a memoryless nonlinear function: `y = tanh(drive * x) / tanh(drive)` or similar.

- **Pros:** Cheap, easy to oversample, well-understood.
- **Cons:** Memoryless = no dynamics. Same input always produces same output. Sounds "static" — fine as one ingredient, weak as the whole stage. Symmetric tanh is too clean.
- **Verdict:** Use as a building block but not the whole stage. The "drive" input to whatever model we use should pass through a tanh-style shaper for the basic nonlinearity, then a dynamic stage on top.

### 2. Asymmetric polynomial / piecewise functions

Replace tanh with an asymmetric function — e.g. `y = x - 0.3 * x^2 - 0.1 * x^3` for the positive half, different curve for negative half. Adds even harmonics.

- **Pros:** Still cheap, adds the "warmth" character via even harmonics, easy to tune by eye on a curve plot.
- **Cons:** Still memoryless. Aliasing-prone — odd-degree polynomial terms create harmonics that fold back without oversampling.
- **Verdict:** Good candidate for the core static shape. Pair with oversampling and a dynamic stage.

### 3. Tape hysteresis model (Jatin Chowdhury's CHOW Tape)

A simplified Jiles-Atherton magnetic hysteresis model. The output depends on input *and* previous magnetization state — genuine memory. Open-source implementation in `chowdsp_utils` and the standalone CHOW Tape plugin.

- **Pros:** This is the gold-standard "sounds like tape" algorithm in the open-source plugin world. Dynamic, asymmetric naturally, signal-history-dependent. Matches the aesthetic exactly.
- **Cons:** Higher CPU than static shaping. More parameters to expose or hide (tape speed, bias, drive). Requires oversampling. Iterative solver = potential numerical stability concerns at extreme settings.
- **Verdict:** **Likely the right answer.** chowdsp_utils ships it. Jatin's thesis and paper are public. Multiple commercial plugins use derivatives of this approach.

### 4. Diode-clipper circuit emulation

Models the nonlinearity of diode clipper circuits (Tubescreamer-style). Either via static curve fitting or via the K-method / wave digital filter approach for circuit accuracy.

- **Pros:** Distinct character, well-researched.
- **Cons:** Sounds like a guitar pedal, not like tape. Wrong aesthetic for this plugin.
- **Verdict:** Skip. Wrong palette.

### 5. Tube/triode emulation (Koren model, etc.)

Models a vacuum tube's plate-grid characteristic. Asymmetric, dynamic if implemented with full state.

- **Pros:** Warm, asymmetric, dynamic in full state-space form.
- **Cons:** "Tube warmth" is a different sound from "tape warmth" — tube tends to have more upper-mid emphasis, tape has more upper-mid loss. Wrong center for this aesthetic.
- **Verdict:** Skip. Adjacent but off-palette.

### 6. Neural network learned saturation

Train a tiny RNN/TCN on input/output pairs from a reference chain. This is the v2 territory.

- **Verdict:** **Banked for v2.** Architecturally the v1 saturation slot is the right place to swap a learned model in eventually, but v1 is pure DSP.

## Decision

**Primary algorithm: tape hysteresis (chowdsp_utils `ChowTapeModel` or equivalent), oversampled 2x or 4x.**

Rationale:
- Matches the aesthetic target exactly (tape, not transistor, not tube)
- Asymmetric and dynamic naturally — no separate "asymmetry" stage needed
- Battle-tested in shipped open-source plugin (CHOW Tape Model)
- MIT-licensed implementation available, no need to re-derive from Jiles-Atherton paper unless tuning requires it
- Drops into the FXModule interface cleanly

**Backup option** if chowdsp's implementation has issues at extremes or doesn't sound right after A/B testing in week 2: fall back to **asymmetric polynomial waveshaper + envelope-driven drive modulation** to fake dynamics. Lower ceiling, faster to implement, less interesting sonically.

## What still needs deciding — STOP POINTS

These are the open questions where the agent must stop and ask, not guess. Each links to the parent `open_questions.md` entry for traceability.

### Q-SAT-1: Oversampling factor (2x vs 4x)

**Stop point.** Before writing the Saturation module body, decide:

- **2x:** lower CPU, may have audible aliasing on high-frequency content (hats, vocal sibilance) at high drive
- **4x:** ~2x CPU of 2x, cleaner top end, leaves less headroom for the rest of the chain (especially convolution reverb)
- **Variable:** 2x at low drive, 4x at high drive, decided per-block

Decision deferred until week 2 — needs A/B test on reference material. Agent should implement with `oversamplingFactor` as a compile-time constant initially, set to 4x, and flag this for revisit after the first preset sketches exist.

### Q-SAT-2: Drive curve shape

The Character macro maps `[0, 1]` to a drive value passed into the saturation. What curve?

- Linear: feels dead in the lower range, abrupt at the top
- Exponential: feels lively at low values, can be unusable at the top
- S-curve (sigmoid): plausible default — gentle at the extremes, responsive in the middle
- Log-shaped: most "ear-natural" since perception of distortion is log-ish

Decision deferred until the saturation algorithm is implemented and can be auditioned. **Agent must not pick this silently** — implement with a placeholder linear map, plot the perceived loudness vs macro position, and flag for human listening test.

### Q-SAT-3: Hysteresis model parameters (if using chowdsp)

The Jiles-Atherton model exposes several parameters:
- Saturation magnetization (M_s)
- Coercivity (H_c)
- Anhysteretic curve shape (a)
- Domain coupling (alpha)
- Reversibility (c)

CHOW Tape exposes these to users; this plugin won't. We need to pick fixed values that produce the right character. Two approaches:

1. Start with chowdsp defaults, A/B against reference, tune by ear
2. Tune analytically against measured tape (out of scope for v1)

**Default to (1).** Agent uses chowdsp defaults at implementation time, flags for tuning pass during week 6 (preset design week) when the chain is complete and tuning has audible consequences.

### Q-SAT-4: Pre/post EQ shaping

Tilt EQ runs before saturation. Should the saturation module also have an internal post-saturation EQ to tame harshness, or leave that to downstream stages?

Tape historically has a high-frequency rolloff after saturation (head bumps, head losses). Real tape sound includes this.

- **Option A:** Saturation is "pure" — just the nonlinearity, no internal EQ. High-frequency taming happens at the Filter stage downstream.
- **Option B:** Saturation includes a fixed gentle HF rolloff post-shaper to fake tape's head losses, making the stage feel "complete" on its own.

**Stop point.** Decide before implementing. Default recommendation: Option B (small fixed rolloff, ~-3dB at 8kHz, not user-exposed), because it makes the saturation stage feel right in isolation and matches tape behaviour. But this is a taste call.

### DC offset handling

Asymmetric saturation creates DC offset on signals with constant polarity bias. Need a DC-blocking high-pass after the saturator (typical: 1-pole high-pass at ~5–20Hz).

This isn't really a decision — it's a "don't forget." Agent must include the DC blocker. Flag if there's a reason to think it shouldn't be there.

### Q-SAT-5: Bypass behaviour

When Character macro = 0, what does saturation do?

- **True bypass:** signal passes through untouched, no DC blocker, no EQ
- **Unity-saturation:** signal still passes through the model but at zero drive, DC blocker stays, gentle HF rolloff stays
- **Crossfade:** smoothly mix between true-bypass and full saturation as drive increases

Unity-saturation is the cleanest UX (no clicks on automation), but means even at Character = 0 there's some tonal change. Crossfade is most transparent but most code.

**Default: unity-saturation with parameter smoothing.** Flag if A/B at low drive reveals audible tonal change that's a problem.

## Implementation order

1. Stub the module with a tanh shaper, verify it builds and runs in the chain.
2. Plot input/output curves in a notebook (Python or in JUCE Standalone with a sweep tone).
3. Swap in chowdsp ChowTapeModel.
4. Resolve Q-SAT-1 (oversampling) by ear-testing on hats and vocal stems.
5. Resolve Q-SAT-4 (post-saturation EQ) by ear-testing on a synth pad.
6. Defer Q-SAT-2 (drive curve) and Q-SAT-3 (hysteresis params) to week 6 tuning pass.

## Tests

DSP unit tests in `Tests/test_saturation.cpp` (Catch2):

- **Determinism:** same input vector → same output vector (run twice, assert equal).
- **Silence in → silence out:** zero-input produces zero-output (within float epsilon).
- **DC blocking:** constant DC input decays to zero within N samples.
- **Stereo coherence:** identical L and R input → identical L and R output.
- **Smoothness:** parameter ramp from 0 to 1 produces no discontinuities (sample-to-sample diff stays below threshold).
- **Bypass continuity:** parameter at exactly 0 produces output within tolerance of input.

Not tested in unit tests (requires human ear): aesthetic match, harmonic content character, dynamic response feel.

## References

- Chowdhury, J. — *Real-time Physical Modelling of Audio Distortion Circuits* (thesis). The CHOW Tape derivation.
- Jiles, D. C. & Atherton, D. L. — *Theory of ferromagnetic hysteresis* (1986). The original physics paper.
- Pirkle, W. — *Designing Audio Effect Plugins in C++*, Chapter 19 (waveshaping fundamentals).
- Zölzer, U. (ed.) — *DAFX: Digital Audio Effects*, Chapter 4 (nonlinear processing).
- chowdsp_utils source: https://github.com/Chowdhury-DSP/chowdsp_utils
- CHOW Tape Model source: https://github.com/jatinchowdhury18/AnalogTapeModel
