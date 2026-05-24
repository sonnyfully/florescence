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

### Q-SAT-1: Oversampling factor (2x vs 4x vs variable)
- Where it lives: `Source/DSP/Saturation.cpp`
- Triggered when: First A/B test of saturation on reference material in week 2
- Options:
  - 2x: lower CPU, possible HF aliasing
  - 4x: ~2x CPU, cleaner top end
  - Variable: 2x at low drive, 4x at high drive
- Default if pushed: 4x as compile-time constant, revisit
- Owner decision needed by: End of week 2

### Q-SAT-2: Drive curve shape (Character macro → drive amount)
- Where it lives: `Source/Params/MacroMapping.cpp`
- Triggered when: First preset attempts in week 5–6 reveal the macro feeling "dead" at low values or "abrupt" at high
- Options: linear, exponential, S-curve, log-shaped
- Default if pushed: linear placeholder — **but flag explicitly**, this is almost certainly wrong as final
- Owner decision needed by: Mid-week 6 (preset tuning)

### Q-SAT-3: Hysteresis model parameter values (if using chowdsp tape model)
- Where it lives: `Source/DSP/Saturation.cpp` (private constants)
- Triggered when: Tuning pass during preset design, week 6
- Options: chowdsp defaults, or hand-tuned values per A/B with reference tracks
- Default if pushed: chowdsp defaults
- Owner decision needed by: End of week 6

### Q-SAT-4: Post-saturation EQ shaping (internal HF rolloff)
- Where it lives: `Source/DSP/Saturation.cpp`
- Triggered when: Implementing the module body in week 2
- Options:
  - A: pure saturation, no internal EQ
  - B: small fixed HF rolloff (~-3dB at 8kHz) to fake tape head losses
- Default if pushed: B (matches tape behaviour, makes stage feel complete in isolation)
- Owner decision needed by: Week 2

### Q-SAT-5: Bypass behaviour at Character = 0
- Where it lives: `Source/DSP/Saturation.cpp`
- Triggered when: Implementing the module body in week 2
- Options: true bypass, unity-saturation (always-on with zero drive), crossfade
- Default if pushed: unity-saturation with parameter smoothing
- Owner decision needed by: Week 2

---

## Convolution Reverb module

Q-IR-1 moved to `docs/decisions.md` on 2026-05-24. Licence terms must still be re-verified in Stage 4 before any IR enters `Resources/IRs/`.

### Q-IR-2: IR count and category split
- Where it lives: `docs/research/ir-sourcing.md`
- Triggered when: Designing the Space macro behaviour
- Options:
  - 8 IRs: 2 plate, 2 hall, 2 chamber, 2 designed
  - 12 IRs: more variety per category
  - 16 IRs: maximum variety, more binary size
- Default if pushed: 8 (minimal viable), expand if Space macro feels limited
- Owner decision needed by: Week 4

### Q-IR-3: Space macro behaviour — selection vs morphing
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

### Q-CHOR-1: BBD model fidelity
- Where it lives: `Source/DSP/Chorus.cpp`
- Triggered when: Implementing in week 3
- Options:
  - Pure delay-line modulation (cheapest, sounds generic)
  - BBD-style with high-frequency loss + companding (more authentic, more code)
  - chowdsp BBD primitives (good middle ground, MIT-licensed)
- Default if pushed: chowdsp BBD primitives
- Owner decision needed by: Week 3

### Q-CHOR-2: Voice count
- Where it lives: `Source/DSP/Chorus.cpp`
- Triggered when: Implementing
- Options: 2 (classic Juno), 3, 4 (denser)
- Default if pushed: 2, Juno-style
- Owner decision needed by: Week 3

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

### Q-MAC-2: Macro range overlap
- Where it lives: `Source/Params/MacroMapping.cpp`
- Triggered when: Designing each macro's parameter touches
- Should multiple macros touch the same internal parameter? E.g. both Character and Tone touching the filter cutoff?
  - Yes, with explicit additive/multiplicative combination rules
  - No, each internal parameter owned by exactly one macro
- Default if pushed: no overlap at v1 (simpler reasoning, fewer surprise interactions)
- Owner decision needed by: Week 5

---

## Preset design

### Q-PRE-1: 60-preset category split
- Where it lives: `Resources/Presets/`
- Triggered when: Week 6 preset week starts
- Options:
  - 20/20/20 split (Vocals/Synths/Drums)
  - Weighted to expected use (likely Vocals 25, Synths 25, Drums 10)
  - Themed instead of source-typed (Atmospheres / Movement / Texture / etc.)
- Default if pushed: 20/20/20
- Owner decision needed by: Start of week 6

### Q-PRE-2: Preset naming convention
- Where it lives: `Resources/Presets/`
- Triggered when: Week 6
- Options:
  - Descriptive ("Bright Wide Plate")
  - Evocative ("Velvet Curtain")
  - Mixed (descriptive prefix + evocative suffix)
- Default if pushed: mixed
- Owner decision needed by: Start of week 6

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
