# decisions.md

Append-only decision log. Resolved entries move here from `docs/open_questions.md` with date, rationale, and affected files.

## 2026-05-24 — Repository pre-flight scaffold

- Decision: initialize the repository in Stage 0 with documentation and directory scaffolding only.
- Rationale: Stage 0 explicitly says no code, and Q-NAME-1/Q-NAME-2 block bundle metadata and CMake/JUCE plumbing.
- Affected files: `README.md`, `.gitignore`, `docs/`, `Resources/`, `Source/`, `Tests/`, `notebooks/`, `.github/`.

## 2026-05-24 — Stage 0 identity decisions

Resolved from `docs/open_questions.md`: Q-NAME-1 and Q-NAME-2.

- Plugin name: Florescence
- Pronunciation: FLOR-ess-ence
- Vendor / shop name: Solace
- Bundle ID: `com.solace.florescence`
- AU manufacturer code: `Slce`
- AU subtype code: `flrs`
- Considered and rejected for plugin name: Penumbra, Catharsis, Apotheosis, Bloom, Empyrean, Efflorescence
- Considered and rejected for vendor name: Threshold, Parallax, Sidereal, Halcyon, Strata
- Rationale: Florescence keeps the bloom/light/decay association without the pronunciation cost of Efflorescence; Solace fits the nocturnal atmospheric palette while staying emotionally legible.
- Risks: mild confusion with "fluorescence"; SEO collision with Solace Corp, though it is enterprise software in a different industry.
- Affected files: future `CMakeLists.txt`, plugin binary metadata, AU/VST3 identifiers, marketing, docs.

Q-NAME-2 is locked when Stage 1 writes `CMakeLists.txt`.

## 2026-05-24 — Stage 0 IR sourcing strategy

Resolved from `docs/open_questions.md`: Q-IR-1.

- Decision: use a mix of self-captured IRs and free libraries.
- Self-capture sources: real spaces captured with sine sweep and/or balloon pop.
- Free-library candidates: Voxengo and Samplicity Bricasti M7.
- Considered and rejected: paid commercial libraries, due to cost; purely synthetic IRs, because they lose too much authenticity as the core library strategy.
- Rationale: the mixed strategy gives the product real-world texture while keeping Stage 4 practical and inexpensive.
- Risk: licence terms on "free" libraries can change or may not allow redistribution. Re-verify current licence terms in Stage 4 before any IR enters `Resources/IRs/`.
- Affected files: `Resources/IRs/`, `docs/research/ir-sourcing.md`, future reverb packaging/licensing notes.

## 2026-05-25 — Saturation: algorithm change to soft-clip + dynamic LPF, full module decisions

### Context

Stage 2 was originally planned around ChowDSP's `ChowTapeModel` (Jiles-Atherton magnetic
hysteresis) via the `chowdsp_utils` modules. Licence review revealed that both
the CHOW Tape source and `chowdsp_utils` are GPLv3. Linking or copying GPL code
into Florescence and distributing the plugin would require either:

1. Releasing Florescence under GPLv3 (incompatible with the closed-source
   commercial model).
2. Obtaining a commercial/non-GPL licence from ChowDSP.
3. Replacing the algorithm with one we either own or can use under a
   permissive licence.

Decision: option 3. The plugin is closed-source commercial; we don't ship GPL
code under the hood.

### Algorithm chosen

**Soft-clip + drive-coupled dynamic low-pass filter.** Architecture lineage:
Rocktron US Patent 5,596,646 (Waller & Craven, 1994, filed by Rocktron Corp).
The patent **expired in 2014** under the 20-year US patent term and the
architecture is now in the public domain. We are not copying any specific
implementation — the architecture is a generic combination of well-known DSP
primitives that the patent merely happened to claim first.

Signal flow:
input → level detector → drive amount
│
▼
input → soft-clipper (4x oversampled) → dynamic LPF (cutoff modulated by drive) → output

The dynamic LPF is what produces the perceived "tape-like" behaviour: at low
drive, the filter cutoff sits at 20kHz (effectively bypass), so the signal is
fully transparent. As drive increases, the cutoff sweeps down, mimicking the
HF loss that real tape exhibits at high record levels. Crucially, the LPF is
*part of* the saturation behaviour, not a separate static EQ — Q-SAT-4 still
holds.

### Why not Jiles-Atherton from the Chowdhury 2019 DAFx paper

The Chowdhury DAFx 2019 paper is CC BY 3.0 licensed, so re-implementing the
J-A hysteresis model from the paper (clean-room, not reading chowdsp source)
is legally fine. We chose not to for three reasons:

1. **Aesthetic mismatch.** J-A hysteresis is the algorithm for emulating
   *real magnetic tape*, including its idiosyncrasies. Our target aesthetic
   (After Hours / Dawn FM — bright, glossy, controlled) is closer to what
   2-inch tape at 30 ips sounds like at low-to-moderate drive — i.e.
   essentially transparent saturation with mild harmonic enrichment. The
   hysteresis behaviour is mostly audible at extremes and isn't core to
   the sound we want.
2. **Implementation cost.** J-A requires a nonlinear ODE solver (RK4),
   parameter identification, careful handling of singularities at
   zero-crossings, and stability tuning at high drive. Realistically 1–2
   weeks of focused DSP work plus testing. Soft-clip + dynamic LPF is a
   day's work.
3. **Schedule.** Stage 2 is week-2 work; this swap can't add weeks.

The J-A path is not closed. If post-v1 user feedback suggests a "vintage"
mode would add value, we can implement the Chowdhury paper independently and
ship it as a Character-mode toggle in v1.x. Re-stated as: hysteresis is
deferred, not ruled out.

### Resolutions to existing open questions

#### Q-SAT-1 — Oversampling factor: **4x, compile-time constant**
The soft-clipper is a memoryless nonlinearity, so the aliasing argument is
purely about how cleanly the top end stays under hard drive. 4x is the
standard answer for `tanh`-class waveshapers; CPU cost is negligible in
context (convolution will dominate). 2x risked audible aliasing on bright
sources; "variable" added complexity for no clear win. The original concern
about needing to match ChowTapeModel's internal oversampling is now moot.

#### Q-SAT-2 — Drive curve shape: **deferred to week 6 (preset tuning)**
Placeholder remains linear, but flagged as almost certainly not final. To
be tuned during preset design against reference material. No reason to
pre-commit before hearing it in context.

#### Q-SAT-3 — Hysteresis model parameters: **N/A (algorithm changed)**
Removed from the open-questions list. No hysteresis model in v1.

#### Q-SAT-4 — Post-saturation HF rolloff: **Option A (no static internal EQ)**
The dynamic LPF *is* the HF behaviour and it's drive-coupled, not static.
Module remains transparent at zero drive. No additional static rolloff. The
chain (chorus BBD-style HF loss, convolution reverb) composes character
downstream.

#### Q-SAT-5 — Bypass at Character = 0: **unity behaviour, no separate bypass**
With the new architecture, "Character = 0" naturally means: drive = 0 →
soft-clipper passes signal unchanged (it's `tanh(x)` ≈ `x` near zero), and
LPF cutoff = 20kHz → effectively bypass. No state, no discontinuity, no
zipper. The transparency-at-zero acceptance test from the prior decision
still runs as a regression check.

### New questions resolved (Q-SAT-6, 7, 8)

#### Q-SAT-6 — Soft-clip function: **tanh, with possible pre-warp evaluation in week 6**
Pure `tanh(drive * x) / tanh(drive)` (drive-normalised so output peaks at
~1.0 at all drive levels). Symmetric, smooth, well-understood, classic.
Generates odd harmonics, which read as "warmth" without buzziness — matches
the glossy aesthetic. Alternatives considered:

- `x / (1 + |x|)` — cheaper, very slightly different harmonic shape. Rejected
  because CPU isn't a concern and tanh is the better-understood reference.
- Pre-warped tanh (asymmetric) — adds even harmonics, more "tube" flavour.
  Rejected for v1 (asymmetry pushes toward warmer/tubier territory than the
  glossy target wants) but worth a listen during week 6 preset tuning. If
  the symmetric `tanh` feels too sterile on certain sources, revisit and
  consider asymmetric as a Character-mode option.

Implementation note: use a lookup table or polynomial approximation for
`tanh` if profiling shows it matters; almost certainly won't matter given
4x oversampling overhead dominates.

#### Q-SAT-7 — Level detector: **peak detector, 10ms attack, 150ms release**
Drives the dynamic LPF. Peak detector (not RMS) because:

- Faster response on transients → the LPF dips top end exactly when
  saturation is most audible (on peaks), which is the perceptually correct
  behaviour and matches what real tape does (HF loss is instantaneous at the
  point of saturation, not averaged).
- RMS would smear the response across loud passages, making the LPF
  modulation feel sluggish and "compressed-sounding" rather than dynamic.

Time constants: 10ms attack catches transients without clicking; 150ms
release lets the top end recover between hits without pumping. These are
starting values; revisit during preset design if the LPF feels too jumpy or
too sluggish on real material. Time constants exposed as private
constants in `Saturation.cpp`, documented with rationale per AGENTS.md "no
magic numbers" rule.

#### Q-SAT-8 — LPF topology and cutoff range: **SVF (TPT), 20kHz → 6kHz**
SVF (Topology-Preserving Transform) is consistent with the choice for
Q-FILT-1 and is stable under modulation at high cutoff rates, which we
need here (the level detector is modulating cutoff at audio-rate-adjacent
rates). One-pole was tempting for cost but the rolloff isn't steep enough
to read as audible HF loss at the cutoff frequencies we want. Butterworth
was overkill — too steep, would sound EQ-like rather than tape-like.

Cutoff range:

- **At drive = 0:** 20kHz (effectively bypass — confirms unity behaviour).
- **At drive = 1.0 (Character knob fully clockwise):** 6kHz.

The mapping curve from drive amount to cutoff is exponential (perceptual),
not linear: `cutoff_hz = 20000 * (6000/20000)^drive_amount`. This puts the
audible HF reduction in the upper half of the Character range, keeping the
lower half feeling transparent.

Resonance: **Q = 0.5 (Butterworth response, no resonance peak)**. Anything
higher would introduce a colouration that fights the glossy aesthetic.
Fixed, not user-exposed.

### Implementation notes

- Module name and file location unchanged: `Source/DSP/Saturation.cpp`.
- All magic numbers (cutoff range, time constants, oversampling factor)
  live in `Source/DSP/SaturationConfig.h` per AGENTS.md.
- Acceptance test (from prior Q-SAT-5 entry): transparency at drive=0
  within -80dB on broadband signal. Carries over.
- New acceptance test: signal at drive=1.0 should show measurable HF
  reduction (-3dB at ~6kHz, falling off cleanly above) and audible odd-
  harmonic content. Both verified against a 1kHz sine and a pink-noise
  broadband signal.
- No dependency on `chowdsp_utils` for saturation. The dependency may
  still be useful for other modules (BBD primitives in Chorus, see
  Q-CHOR-1) but each use needs its own licence review. **Default
  assumption: any `chowdsp_utils` module is GPLv3 unless verified
  otherwise.** Q-CHOR-1's default of "chowdsp BBD primitives" needs to
  be re-litigated under the same lens.

### Follow-ups

- Update `open_questions.md`: strike Q-SAT-3, mark Q-SAT-1/4/5/6/7/8 as
  resolved with reference to this entry. Q-SAT-2 stays open.
- Add a new open question Q-CHOR-1-LIC: revisit chowdsp BBD licensing
  before week 3.
- Decide whether v1.x hysteresis mode is on the roadmap or not. Not
  blocking v1.

## 2026-05-24 — Use current JUCE stable for Stage 1

Resolved from `docs/open_questions.md`: Q-BUILD-1.

- Decision: move the JUCE submodule from JUCE 7.0.12 to JUCE 8.0.12.
- Rationale: JUCE 7.0.12 is the latest JUCE 7 tag, but its `juceaide` helper does not compile against the installed Xcode 26/macOS SDK because `CGWindowListCreateImage` is marked unavailable. GitHub marks JUCE 8.0.12 as the latest stable release, and this is the least invasive path if it configures and builds cleanly.
- Considered and rejected: installing an older Xcode/macOS SDK as the default path, because it adds local toolchain fragility; carrying a local patch to JUCE 7, because vendor patches create maintenance drag before the plugin has shipped.
- Risk: JUCE 8 may introduce API or behaviour differences from the original JUCE 7.x plan. Keep Stage 1 minimal and document any compatibility changes as they appear.
- Affected files: `external/JUCE`, `CMakeLists.txt`, `docs/CONTEXT.md`, `docs/roadmap.md`.

## Unresolved Stage 0 Stop Points

These remain before Stage 0 is complete:

- Register plugin `.com` domain for Florescence.
- Pin 5 reference tracks in a playlist and write `docs/aesthetic/reference-tracks.md`.
- Write 5 named beta customers to local ignored `docs/customers.md`.
- Start Figma file and record the link here.
- Create Lemon Squeezy account.
