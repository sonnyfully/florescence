# open_questions.md

**The agent treats this file as a hard-stop list.** When work reaches a question listed here, the agent must:

1. Stop.
2. Surface the question in the current PR/conversation with the relevant context.
3. Wait for a human decision.
4. Once decided, move the entry to `decisions.md` with the rationale and the date.

Do **not** silently pick a default. Do **not** guess. Do **not** defer by picking something arbitrary "to keep moving." If a stop point is hit before its expected week, surface it anyway — the schedule is wrong, not the question.

Format per question:

```
### Q-<MODULE>-<N>: Short title
- Where it lives: <file path>
- Triggered when: <condition>
- Options: <list>
- Default if pushed: <only if a default is genuinely safe>
- Owner decision needed by: <week / milestone>
```

---

## Saturation module

### ~~Q-SAT-1: Oversampling factor (2x vs 4x vs variable)~~ — RESOLVED 2026-05-25
**Resolved → 4x, compile-time constant.** See `decisions.md` 2026-05-25.

### Q-SAT-2: Drive curve shape (Burn macro → drive amount)
- Where it lives: `Source/Params/MacroMapping.cpp`
- Triggered when: First preset attempts in week 5–6 reveal Burn feeling "dead" at low values or "abrupt" at high
- Options: linear, exponential, S-curve, log-shaped
- Default if pushed: linear placeholder — **but flag explicitly**, this is almost certainly wrong as final
- Owner decision needed by: Mid-week 6 (preset tuning)

### ~~Q-SAT-3: Hysteresis model parameter values~~ — RESOLVED 2026-05-25, REVISED 2026-05-25
**Resolved → clean-room Jiles-Atherton starting values from Chowdhury DAFx 2019:**
`M_s=1.0`, `k=0.47`, `a=22000`, `alpha=1.6e-3`, `c=1.7e-1`.
Normalized-audio constants (`inputFieldScale`, `maxDriveGain`, `recordAsymmetry`) are
implementation tuning values. See `decisions.md` 2026-05-25 clean-room saturation entry.

### ~~Q-SAT-4: Post-saturation EQ shaping (internal HF rolloff)~~ — RESOLVED 2026-05-25, REVISED 2026-05-25
**Resolved → drive-coupled dynamic LPF plus fixed 8kHz post-saturator rolloff.**
The dynamic LPF models level-dependent tape HF loss; the fixed rolloff prevents fizz before
downstream modules. See `decisions.md` 2026-05-25 clean-room saturation entry.

### ~~Q-SAT-5: Bypass behaviour at Character = 0~~ — RESOLVED 2026-05-25
**Resolved → unity behaviour, no separate bypass.** With drive=0, the Jiles-Atherton
model path returns input unchanged and the module skips the saturation/filter path, so
automation into saturation is smooth while zero drive remains transparent. See `decisions.md`
2026-05-25 clean-room saturation entry.

### ~~Q-SAT-6: Soft-clip function~~ — SUPERSEDED 2026-05-25
**Superseded by clean-room Jiles-Atherton hysteresis.** No tanh soft-clipper in the
Stage 2 saturation path. See `decisions.md` 2026-05-25 clean-room saturation entry.

### ~~Q-SAT-7: Level detector type and time constants~~ — RESOLVED 2026-05-25
**Resolved → peak detector, 10ms attack / 150ms release.** Time constants are
starting values; revisit during preset tuning if behaviour feels wrong on
real material. See `decisions.md` 2026-05-25.

### ~~Q-SAT-8: LPF topology and cutoff range~~ — RESOLVED 2026-05-25
**Resolved → SVF (TPT), Q=0.5, 20kHz → 6kHz exponential mapping.**
See `decisions.md` 2026-05-25.

---

## Convolution Reverb module

Q-IR-1 moved to `docs/decisions.md` on 2026-05-24. Licence terms must still be re-verified in Stage 4 before any IR enters `Resources/IRs/`.

### Q-IR-2: IR count and category split
- Where it lives: `docs/research/ir-sourcing.md`
- Triggered when: Designing the reverb library and resolving whether Character modes swap IR sets (Q-CHAR-2)
- Options:
  - 8 IRs: 2 plate, 2 hall, 2 chamber, 2 designed
  - 12 IRs: more variety per category
  - 16 IRs: maximum variety, more binary size
- Default if pushed: 8 (minimal viable), expand if the reverb library feels too narrow under the Character modes
- Owner decision needed by: Week 4

### ~~Q-IR-3: Space macro behaviour — selection vs morphing~~ — SUPERSEDED 2026-05-25
**Superseded by the north-star control surface.** The Space macro no longer exists. Atmosphere drives reverb wet level per Q-MAC-3; IR character is tied to Character mode per Q-CHAR-2. The previous "Space selects vs morphs IRs" question no longer applies in this form.

- Where it lives: `Source/DSP/ConvReverb.cpp` + `Source/Params/MacroMapping.cpp`
- Triggered when: Implementing the Space macro mapping
- Options:
  - **Selection:** Space knob selects which IR is active (with crossfade at boundaries)
  - **Morphing:** real-time spectral interpolation between IRs (complex, possibly artefacts)
  - **Layered:** size knob controls IR length crop + wet mix, IR category exposed separately
- Default if pushed: selection with crossfade — simplest, sounds clean, easy to reason about
- Owner decision needed by: Week 4

### Q-IR-4: Wet path modulation (the "alive" trick)
- Where it lives: `Source/DSP/ConvReverb.cpp`
- Triggered when: Implementing wet path
- Options:
  - No modulation (static convolution — risk: sounds "dead")
  - Pre-convolution chorus on wet signal
  - Post-convolution chorus on wet signal
  - Subtle delay-line modulation in the wet path
- Default if pushed: subtle pre-convolution chorus, depth ~0.3%, rate ~0.5Hz
- Owner decision needed by: Week 4

---

## Chorus module

### ~~Q-CHOR-1-LIC: Re-verify chowdsp BBD primitives licence before any use~~ — RESOLVED 2026-05-25
**Resolved -> treat ChowDSP BBD / `chowdsp_dsp_utils` as unavailable for v1.**
GPLv3 licensing blocks use in Florescence's closed-source commercial model
unless a non-GPL commercial licence is obtained; commercial licensing is not
being pursued for v1. See `decisions.md` 2026-05-25.

### ~~Q-CHOR-1: BBD model fidelity~~ — RESOLVED 2026-05-25
**Resolved -> clean-room BBD-flavoured delay-line modulation.** Implement a
modulated delay line with smooth fractional interpolation and wet-path
bandwidth loss / HF softening. Optional companding and delay-path nonlinearity
are Stage 6 tuning additions, not required for the initial implementation.
No GPL-derived code and no references to `chowdsp_dsp_utils` source. See
`decisions.md` 2026-05-25 and `docs/research/chorus.md`.

### ~~Q-CHOR-2: Voice count~~ — RESOLVED 2026-05-25
**Resolved -> 2 voices, Juno-style, with decorrelated L/R LFO phases.** If
Stage 6 preset tuning proves this too narrow, a possible 3rd voice / density
mode is parked in `v1.x_ideas.md`. See `decisions.md` 2026-05-25.

---

## Filter module

### Q-FILT-1: Filter topology
- Where it lives: `Source/DSP/Filter.cpp`
- Triggered when: Implementing in week 3
- Options: SVF (TPT), ladder (Moog-style), biquad
- Default if pushed: SVF (TPT) — clean, stable, well-understood
- Owner decision needed by: Week 3

### Q-FILT-2: Envelope follower scope
- Where it lives: `Source/DSP/Filter.cpp`
- Triggered when: Implementing
- Options:
  - Follow input signal (filter ducks under loud input)
  - Follow sidechain input (true sidechain compression behaviour for filter cutoff)
  - Both, switchable
- Default if pushed: input signal only at v1, sidechain in v1.x
- Owner decision needed by: Week 3

---

## Delay module

### Q-DEL-1: BPM sync divisions
- Where it lives: `Source/DSP/Delay.cpp`
- Triggered when: Implementing in week 4
- Options:
  - Minimal: 1/4, 1/8, dotted-1/8, 1/16
  - Standard: above + triplets, dotted variants
  - Full: every common subdivision plus 1/2 and 1/1
- Default if pushed: standard set
- Owner decision needed by: Week 4

### Q-DEL-2: Ping-pong topology
- Where it lives: `Source/DSP/Delay.cpp`
- Triggered when: Implementing
- Options:
  - Independent L/R delay times, simple
  - True ping-pong (L feeds R's delay line and vice versa)
  - Mode toggle (stereo vs ping-pong)
- Default if pushed: mode toggle
- Owner decision needed by: Week 4

---

## Macro mapping

### Q-MAC-1: Mapping curve design philosophy
- Where it lives: `Source/Params/MacroMapping.cpp`
- Triggered when: Building the first mapping table in week 5
- The big question — this is half the product:
  - Curves designed by ear, tuned during preset design (hand-crafted)
  - Curves driven by perceptual loudness equalisation (algorithmic baseline, then hand-tuned)
  - Curves learned from human-rated preset interpolations (overkill for v1)
- Default if pushed: hand-crafted, but **agent must NOT just pick values silently** — every macro's mapping table goes through human review before merge
- Owner decision needed by: Week 5

### ~~Q-MAC-2: Macro range overlap~~ — SUPERSEDED 2026-05-25
**Superseded by the Atmosphere meta-macro.** Atmosphere is a meta-macro and by definition overlaps with Burn, Pulse, and reverb wet. Burn and Pulse may or may not share targets — see Q-MAC-4. The "no overlap" default no longer applies at the macro level; it applies among Burn and Pulse if Q-MAC-4 resolves to the default.

- Where it lives: `Source/Params/MacroMapping.cpp`
- Triggered when: Designing each macro's parameter touches
- Should multiple macros touch the same internal parameter? E.g. both Character and Tone touching the filter cutoff?
  - Yes, with explicit additive/multiplicative combination rules
  - No, each internal parameter owned by exactly one macro
- Default if pushed: no overlap at v1 (simpler reasoning, fewer surprise interactions)
- Owner decision needed by: Week 5

### Q-MAC-3: Atmosphere meta-macro mapping curve
- Where it lives: `Source/Params/MacroMapping.cpp`
- Triggered when: Implementing Atmosphere routing in week 5
- How does Atmosphere scale Burn, Pulse, and reverb wet?
- Options:
  - Linear across all targets
  - Exponential across all targets
  - S-curve across all targets
  - Per-target curves, each hand-tuned
- Default if pushed: each target gets its own hand-tuned curve
- Owner decision needed by: Week 5

### Q-MAC-4: Does Burn drive chorus depth at high values?
- Where it lives: `Source/Params/MacroMapping.cpp`
- Triggered when: Designing Burn and Pulse target ownership
- Options:
  - Burn affects saturation-adjacent parameters only
  - Burn also pushes chorus depth at high values
  - Burn affects chorus only in selected Character modes
- Default if pushed: Burn affects saturation parameters only; chorus depth lives under Pulse
- Owner decision needed by: Week 5

---

## Character switch

### Q-CHAR-1: Character switch — selector, crossfade, or morph?
- Where it lives: `Source/Params/MacroMapping.cpp` + affected DSP modules
- Triggered when: Implementing the Velvet / Onyx / Chrome Character switch
- Options:
  - Instant selector: click applies the new parameter snapshot immediately
  - Short crossfade: e.g. 50ms, click-free but feels instantaneous
  - Longer morph: e.g. 500ms, intentionally audible "this is changing" feel
- Default if pushed: crossfade over 50ms, click-free but feels instantaneous
- Owner decision needed by: Week 5

### Q-CHAR-2: Character switch — does it affect the reverb IR set?
- Where it lives: `Resources/IRs/`, `Source/DSP/ConvReverb.cpp`, `Source/Params/MacroMapping.cpp`
- Triggered when: Designing CharacterPreset reverb values and IR library size
- Options:
  - Each mode has its own IR list; switching mode swaps the IR set entirely
  - All IRs are available in all modes; each mode biases the default selection
- Default if pushed: all IRs available in all modes, with Character mode biasing the default selection
- Owner decision needed by: Week 4

---

## Preset design

### ~~Q-PRE-1: 60-preset category split~~ — SUPERSEDED 2026-05-25 by Q-PRE-1-REVISED
- Where it lives: `Resources/Presets/`
- Triggered when: Week 6 preset week starts
- Options:
  - 20/20/20 split (Vocals/Synths/Drums)
  - Weighted to expected use (likely Vocals 25, Synths 25, Drums 10)
  - Themed instead of source-typed (Atmospheres / Movement / Texture / etc.)
- Default if pushed: 20/20/20
- Owner decision needed by: Start of week 6

### Q-PRE-1-REVISED: Preset split — themed, not source-typed
- Where it lives: `Resources/Presets/`
- Triggered when: Week 6 preset week starts
- Background: with the atmosphere-processor framing, source-typed presets (Vocals / Synths / Drums) feel wrong. Presets should be organised by what they do to the source, not by what the source is.
- Options:
  - 5 themed categories: Spaces, Movement, Heat, Atmospheres, Subtle
  - Fewer broader categories
  - Keep source-typed categories despite the positioning shift
- Default if pushed: 5 categories, 12 presets each = 60 presets total
- Owner decision needed by: Start of Week 6

### ~~Q-PRE-2: Preset naming convention~~ — SUPERSEDED 2026-05-25 by Q-PRE-2-REVISED
- Where it lives: `Resources/Presets/`
- Triggered when: Week 6
- Options:
  - Descriptive ("Bright Wide Plate")
  - Evocative ("Velvet Curtain")
  - Mixed (descriptive prefix + evocative suffix)
- Default if pushed: mixed
- Owner decision needed by: Start of week 6

### Q-PRE-2-REVISED: Preset naming — evocative
- Where it lives: `Resources/Presets/`
- Triggered when: Week 6 preset naming starts
- Background: with the atmosphere framing, descriptive names such as "Bright Wide Plate" feel wrong. Preset names should sell the scene.
- Options:
  - Evocative only, e.g. "Velvet Curtain", "Cold Cathedral", "Late Train"
  - Mixed descriptive + evocative
  - Descriptive only
- Default if pushed: evocative-only
- Owner decision needed by: Start of Week 6

---

## GUI

### Q-GUI-1: GUI scale
- Where it lives: `Source/PluginEditor.h`
- Triggered when: First GUI pass in week 5
- Options:
  - Fixed size (simplest)
  - Resizable with discrete scale factors (75% / 100% / 125% / 150%)
  - Free-resizable
- Default if pushed: fixed at v1, discrete scaling at v1.x
- Owner decision needed by: Week 5

### Q-GUI-2: Knob interaction model
- Where it lives: `Source/GUI/KnobComponent.cpp`
- Triggered when: Implementing knobs in week 5
- Options:
  - Vertical drag (standard)
  - Rotary drag
  - Both
  - Vertical + fine mode on shift-drag + double-click to reset
- Default if pushed: vertical drag + fine mode on shift + double-click reset
- Owner decision needed by: Week 5

### Q-GUI-3: Stereo width control on main UI?
- Where it lives: `Source/GUI/MainComponent.cpp`, `Source/Params/ParameterLayout.cpp`
- Triggered when: Designing the Stage 5 first GUI pass and parameter layout
- Options:
  - 3-state button: Narrow / Normal / Wide
  - Small slider
  - No top-level control; width handled per-mode in the Character switch
  - Slider hidden behind a "more" panel
- Default if pushed: no default; this is a taste and workflow decision
- Owner decision needed by: Week 5

### Q-GUI-4: Day/Night toggle default visibility
- Where it lives: `Source/GUI/MainComponent.cpp`, `Source/Params/ParameterLayout.cpp`
- Triggered when: Designing the main UI and secondary settings panel
- Options:
  - Visible on the main UI by default, with an option to hide
  - Hidden by default, with an option to reveal
- Default if pushed: visible by default
- Owner decision needed by: Week 5

---

## Licensing / distribution

### Q-LIC-1: Activation flow
- Where it lives: `Source/Licensing/`
- Triggered when: Week 6–7 launch prep
- Options:
  - Gumroad license keys with simple validation
  - Lemon Squeezy with built-in licence management
  - Self-hosted on VPS
- Default if pushed: Lemon Squeezy (handles EU VAT)
- Owner decision needed by: Week 6

### Q-LIC-2: Demo / trial mode
- Where it lives: `Source/Licensing/`
- Triggered when: Week 6–7
- Options:
  - No demo, paid only
  - Time-limited trial (14 days, full features)
  - Feature-limited demo (no preset save, occasional silence)
  - Output-degraded demo (occasional dropout)
- Default if pushed: **NO DEFAULT** — meaningful UX decision, human call
- Owner decision needed by: Week 6

### Q-LIC-3: Apple Developer enrolment timing
- Where it lives: external (Apple)
- Triggered when: When conviction is real enough to ship
- Options:
  - Enrol at week 1 (eat the cost upfront)
  - Enrol mid-week 5 / early week 6 (after the chain sounds right but before beta)
  - Enrol week 7 (last possible moment, riskier)
- Default if pushed: end of week 4 / start of week 5 — leaves 2–3 weeks for notarization issues
- Owner decision needed by: End of week 4

---

## Name and identity

Q-NAME-1 and Q-NAME-2 moved to `docs/decisions.md` on 2026-05-24. The bundle ID and AU codes are locked when Stage 1 writes `CMakeLists.txt`.

---

## How to add new questions

When the agent surfaces a new ambiguity during work that meets the bar for human input, it:

1. Adds an entry here in the same format.
2. References it from the PR/branch where it came up.
3. Comments out or stubs the code with a `// TODO Q-XXX-N` reference.
4. Surfaces in PR description.

When questions are resolved, the agent moves them to `decisions.md` with date and rationale. Do not delete entries from this file — strike through or move, so the history is preserved.
