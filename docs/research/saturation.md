# Saturation Research — Clean-Room Jiles-Atherton

## Status

Stage 2 saturation now uses a clean-room Jiles-Atherton hysteresis model
implemented from papers, not from GPL code.

Do not consult or copy `chowdsp_waveshapers`, AnalogTapeModel, CHOW Tape, or
any other GPL tape-model implementation. The implementation trail is:

- D. C. Jiles and D. L. Atherton, "Theory of ferromagnetic hysteresis",
  *Journal of Magnetism and Magnetic Materials*, 61, 48-60, 1986.
- Jatin Chowdhury, "Real-time Physical Modelling for Analog Tape Machines",
  Proc. DAFx 2019.

## Model

The model maps normalized audio into a magnetic-field-like input `H`, then
integrates magnetisation `M` with a fourth-order Runge-Kutta solver.

Pseudocode:

```text
H = input * inputFieldScale * driveGain * polarityAsymmetry
H_dot = (H - previousH) / dt

M_an = M_s * L((H + alpha * M) / a)
dM_an/dH = (M_s / a) * L'((H + alpha * M) / a)

delta_s = sign(H_dot)
delta_m = 1 if sign(M_an - M) == delta_s else 0

dM_irrev/dH =
    delta_m * (M_an - M) /
    ((1 - c) * delta_s * k - alpha * (M_an - M))

dM/dt =
    ((1 - c) * dM_irrev/dH + c * dM_an/dH) /
    (1 - c * alpha * dM_an/dH) *
    H_dot

M_next = RK4(M, dM/dt)
output = M_next / M_s
```

The modified Langevin function uses a small-signal approximation near zero:

```text
L(x)  ~= x / 3 - x^3 / 45
L'(x) ~= 1 / 3 - x^2 / 15
```

Outside the near-zero region, `L(x) = coth(x) - 1/x`.

## Parameters

Starting values come from the Chowdhury DAFx 2019 paper and are fixed for v1:

| Parameter | Value | Notes |
| --- | ---: | --- |
| `M_s` | 1.0 | Normalized saturation magnetisation |
| `k` | 0.47 | Pinning / coercivity term |
| `a` | 22000 | Anhysteretic shape |
| `alpha` | 1.6e-3 | Domain coupling |
| `c` | 1.7e-1 | Reversibility |

Implementation-specific normalized-audio constants:

| Constant | Value | Notes |
| --- | ---: | --- |
| `inputFieldScale` | 22000 | Converts normalized audio to the `H` domain |
| `maxDriveGain` | 3.0 | Drive 0-1 maps to input push before hysteresis |
| `recordAsymmetry` | 1.0 | v1 bias/asymmetry amount for even-harmonic emphasis |

These are Stage 6 listening-tuning values. They are not exposed as v1
parameters.

## Saturation Chain

The module keeps the Stage 2 architecture around the new hysteresis core:

```text
input
  -> 4x oversampling
  -> Jiles-Atherton hysteresis
  -> drive-coupled dynamic low-pass
  -> fixed 8 kHz post-saturation rolloff
  -> DC blocker
  -> downsampling
```

The drive-coupled low-pass stays because tape saturation is not just magnetic
memory; pushed tape loses high-frequency energy. The hysteresis model supplies
history-dependent nonlinear behaviour, while the dynamic low-pass keeps the
top-end loss tied to drive and signal level.

## Verification

Automated tests cover:

- Determinism
- Silence in -> silence out
- DC blocking
- Stereo coherence
- Smooth drive ramps
- Hysteresis loop behavior on a slow sine
- Even-harmonic emphasis versus low odd harmonics
- Bounded output at 44.1, 48, 96, and 192 kHz

Evidence plots:

- `docs/research/saturation/hysteresis_loop.png`
- `docs/research/saturation/harmonic_content.png`

## Performance Note

The RK4 core is substantially heavier than the previous tanh shaper. A
temporary optimized micro-benchmark measured the Jiles-Atherton core at roughly
16x the cost of the old tanh core. Absolute runtime was still small for one
stereo instance, but this exceeds the original "flag if greater than 3x" target.

The implementation uses lookup tables for `L(x)` and `L'(x)` to avoid
hyperbolic functions in the audio loop. Further optimization options before
beta include:

- Benchmarking the full module in a Release plugin build instead of the core
  solver alone.
- Evaluating whether RK4 can run at the base sample rate while the surrounding
  nonlinear/filter path remains oversampled.
- Replacing RK4 with a lower-cost solver only if listening and stability tests
  prove it preserves the tape behaviour.

Do not silently swap to a simpler waveshaper. If this CPU cost is too high for
v1, surface the tradeoff and decide explicitly.
