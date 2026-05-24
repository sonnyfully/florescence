# roadmap.md

Staged build plan for v1. Read after `docs/CONTEXT.md`, `docs/ARCHITECTURE.md`, and `docs/open_questions.md`.

## How to read this doc

The build is broken into **8 stages**, mapped to ~7 weeks of focused work (4 days/week minimum) with ~1 week of buffer. Each stage has:

- **Goal** — the one thing this stage exists to achieve. If the goal isn't met, the stage isn't done. Adding things beyond the goal is scope creep.
- **End state** — a concrete, checkable definition of done. The agent and the human should both be able to answer "is this stage finished?" with yes/no, not "mostly."
- **Stop points** — Q-IDs from `open_questions.md` that must be resolved during this stage. The agent surfaces these proactively, not at the end.
- **Deliverables** — files committed, PRs merged, artefacts created.
- **What's NOT in scope** — things that *feel* like they belong here but are deliberately deferred. Most stages have one or two of these. They exist to short-circuit scope-creep instincts.
- **Risks** — the most likely ways the stage overruns, and the mitigation.

Stages are sequential — don't start stage N+1 until stage N's end state is met. If a stage is taking longer than its budget, surface why before extending. If a stage finishes faster, the extra time goes into the next stage's buffer, not into expanding the current stage.

---

## Stage 0 — Pre-flight (this week, before stage 1 starts)

**Goal:** Resolve every decision that gates stage 1 so the build starts pointed at a known target. No code in this stage.

**End state:**
- Plugin name chosen and `.com` registered (Q-NAME-1).
- Bundle ID and vendor name decided (Q-NAME-2).
- 5 reference tracks pinned in a playlist + `docs/aesthetic/reference-tracks.md` written.
- 5 named beta customers written down in `docs/customers.md` (private).
- IR sourcing strategy decided in principle (Q-IR-1) — paid library / free library / self-capture / mix. Actual IR acquisition happens in stage 5.
- Figma file started — colour palette, typography, knob style, rough layout for the 7 macros. Doesn't need to be pixel-final.
- Apple Developer enrolment **not started yet** (deferred to end of stage 4 per Q-LIC-3).
- Lemon Squeezy account created (no products set up yet).

**Stop points to resolve in this stage:**
- Q-NAME-1 (plugin name)
- Q-NAME-2 (bundle ID / vendor)
- Q-IR-1 (IR sourcing strategy — in principle)

**Deliverables:**
- `docs/aesthetic/reference-tracks.md`
- `docs/customers.md` (gitignored or private branch)
- Figma file (lives outside repo, link in `docs/decisions.md`)
- Domain registered

**Not in scope:**
- Any C++ code
- Apple Developer enrolment
- Actual IR files in the repo
- Marketing site

**Risks:**
- *"I can't pick a name"* — set a 2-hour timebox and pick. The name is changeable until stage 1 commits it to the bundle ID. After that it's painful but not impossible.
- *"I can't list 5 customers"* — real signal worth noticing. If genuinely can't, ask whether v1 should ship at all, or whether you need to spend stage 0 broadening the network first.

---

## Stage 1 — Plumbing and throwaway plugin (Week 1)

**Goal:** Prove the entire build → load → iterate loop works on a trivial plugin, before any real DSP lands.

**End state:**
- Current JUCE stable checked out, CMake project building `CharacterFX_AU`, `CharacterFX_VST3`, and `CharacterFX_Standalone` targets.
- Throwaway plugin: gain + 1-pole low-pass filter, two knobs, ugly default GUI.
- Builds clean on local Mac (Apple Silicon).
- Loads in Ableton Live as AU and VST3, audio passes through, knobs work, parameter automation works.
- `auval -v <type> <subtype> <manufacturer>` passes for the AU.
- VST3 validator passes.
- Catch2 test suite runs (one trivial test).
- `.gitignore` correct, repo committed.
- `AGENTS.md`, `docs/CONTEXT.md`, `docs/ARCHITECTURE.md`, `docs/open_questions.md` all in repo.
- `docs/decisions.md` created with stage 0's resolved decisions.
- GitHub repo private, first PR (the plumbing) merged to main via squash.

**Stop points to resolve in this stage:**
- None expected — but Q-NAME-2 (bundle ID) gets cemented here. If it wasn't resolved in stage 0, surface immediately.

**Deliverables:**
- Working repo at the layout in `ARCHITECTURE.md`
- One PR merged: `feature/plumbing`
- README.md with build instructions

**Not in scope:**
- Custom LookAndFeel — JUCE defaults are fine, this stage is about plumbing
- Any DSP from the v1 chain (no saturation, no chorus, etc.)
- Code signing — Apple Dev account not enrolled yet
- Preset system — comes in stage 6
- chowdsp_utils integration — comes in stage 2 when first needed

**Risks:**
- *CMake fights JUCE* — budget a day for this. Jules Storer's JUCE CMake examples on GitHub are the reference; copy structure rather than improvise.
- *Apple Silicon vs Intel issues* — build Universal Binary from day one. `CMAKE_OSX_ARCHITECTURES="arm64;x86_64"`.
- *AU validation fails* — common, fix early before the chain is large enough to make debugging hard.

---

## Stage 2 — Tilt EQ + Saturation (Week 2)

**Goal:** First two real DSP modules in the chain, audibly working on real audio. Saturation is the aesthetic anchor — if it doesn't sound right, nothing downstream saves it.

**End state:**
- `FXModule` interface defined in `Source/DSP/FXModule.h` per `ARCHITECTURE.md`.
- `TiltEQ` module: stereo, tilt parameter -12dB to +12dB at pivot ~1kHz, unit-tested for frequency response.
- `Saturation` module: chowdsp `ChowTapeModel` wrapped, drive parameter 0–1, oversampled (factor per Q-SAT-1), DC blocker, optional internal HF rolloff per Q-SAT-4, bypass behaviour per Q-SAT-5.
- Both modules plug into a minimal chain in `PluginProcessor::processBlock`.
- Plugin's two front-panel knobs (still ugly GUI) are mapped to Tilt amount and Saturation drive — direct mapping, no Macro layer yet.
- A/B test conducted on at least 3 reference tracks (vocal, synth, drum loop) — saturation character feels right or open question raised.
- Unit tests in `Tests/test_tilteq.cpp` and `Tests/test_saturation.cpp` pass.
- chowdsp_utils added as CMake dependency with explicit `decisions.md` entry.

**Stop points to resolve in this stage:**
- Q-SAT-1 (oversampling factor) — A/B at week 2 end
- Q-SAT-4 (internal HF rolloff yes/no)
- Q-SAT-5 (bypass behaviour at Character = 0)

**Stop points deferred (to surface but not yet resolve):**
- Q-SAT-2 (drive curve shape) — implement linear placeholder, surface that it needs revisit in stage 6
- Q-SAT-3 (hysteresis param tuning) — use chowdsp defaults, surface for stage 6 tuning pass

**Deliverables:**
- `Source/DSP/FXModule.h`
- `Source/DSP/TiltEQ.{cpp,h}`
- `Source/DSP/Saturation.{cpp,h}`
- `Tests/test_tilteq.cpp`, `Tests/test_saturation.cpp`
- One Jupyter notebook plotting saturation curves and harmonic spectrum
- Two PRs: `feature/tilt-eq`, `feature/saturation`
- `docs/decisions.md` entries for chowdsp dependency, Q-SAT-1, Q-SAT-4, Q-SAT-5

**Not in scope:**
- Chorus, Filter, Delay, Reverb (later stages)
- Macro mapping layer (stage 6)
- GUI polish (stage 7)
- Drive curve tuning (stage 6)
- Multi-band saturation (not in v1 scope at all)

**Risks:**
- *Saturation doesn't sound right with chowdsp defaults* — expected at this stage; the tuning pass is stage 6. What needs to be true at end of stage 2 is "the character is in the right neighbourhood." If it's clearly wrong (e.g., sounds like a guitar pedal), surface immediately rather than pushing through.
- *Oversampling latency* — JUCE oversampling has latency in some modes. Target zero-latency mode, confirm `getLatencySamples()` returns 0.
- *chowdsp build issues* — header-only modules vs static-library modules in JUCE CMake. Document the integration pattern in `decisions.md` so it's not re-discovered later.

---

## Stage 3 — Chorus + Filter (Week 3)

**Goal:** Modulation stages working in true stereo, chain feels "alive" rather than static.

**End state:**
- `Chorus` module: BBD-flavoured (per Q-CHOR-1), N voices (per Q-CHOR-2), true stereo with decorrelated L/R LFO phases, depth and rate parameters.
- `Filter` module: SVF (TPT) low-pass, cutoff + resonance parameters, envelope follower modulating cutoff per Q-FILT-2.
- Both modules in the chain after Saturation.
- True stereo verified: identical L/R input → identical L/R output (stereo coherence test).
- A/B tested on reference material — by end of stage, the chain (Tilt → Saturation → Chorus → Filter) sounds *recognisably* in the target palette on at least one preset sketch, even if rough.
- Unit tests for both modules.
- Two PRs merged.

**Stop points to resolve in this stage:**
- Q-CHOR-1 (BBD model fidelity)
- Q-CHOR-2 (chorus voice count)
- Q-FILT-1 (filter topology — confirm SVF is right)
- Q-FILT-2 (envelope follower scope — input only vs sidechain capable)

**Deliverables:**
- `Source/DSP/Chorus.{cpp,h}`, `Source/DSP/Filter.{cpp,h}`
- `Tests/test_chorus.cpp`, `Tests/test_filter.cpp`
- Notebooks for chorus modulation visualisation and filter frequency response
- PRs: `feature/chorus`, `feature/filter`
- `decisions.md` entries for resolved Q-CHOR / Q-FILT items

**Not in scope:**
- Delay and reverb (stage 4)
- Sidechain input routing for filter (deferred to v1.x per Q-FILT-2 default)
- Macro mapping (stage 6)

**Risks:**
- *Chorus phasing artefacts when summed to mono* — verify mono-compatibility, but accept that "true stereo" presets won't be mono-perfect. Document in `decisions.md` what the mono-sum tradeoff is.
- *Envelope follower feels too snappy or too slow* — tunable, but pick reasonable defaults (attack ~10ms, release ~150ms) and revisit in stage 6.

---

## Stage 4 — Delay + Convolution Reverb (Week 4)

**Goal:** Time-based stages complete the chain. Convolution reverb is the "space" — half of why the plugin sounds expensive.

**End state:**
- `Delay` module: stereo lines with BPM sync (divisions per Q-DEL-1), filtered feedback, ping-pong support per Q-DEL-2.
- `ConvReverb` module: wraps `juce::dsp::Convolution`, loads IRs from `Resources/IRs/`, Space parameter selects/morphs between IRs per Q-IR-3, wet-path modulation per Q-IR-4.
- IR library populated with initial set (8 IRs per Q-IR-2 default, can grow to 12 if needed).
- Complete chain: Tilt → Saturation → Chorus → Filter → Delay → ConvReverb working end-to-end.
- Apple Developer enrolment initiated per Q-LIC-3 (this is the last stage where you can defer it — the next stages need signed builds for beta testing).
- A/B test on reference tracks: chain produces something that sits convincingly in the target aesthetic on ≥2 different source types.

**Stop points to resolve in this stage:**
- Q-IR-2 (IR count and category split)
- Q-IR-3 (Space macro behaviour — selection vs morphing)
- Q-IR-4 (wet path modulation approach)
- Q-DEL-1 (BPM sync divisions)
- Q-DEL-2 (ping-pong topology)
- Q-LIC-3 (Apple Dev enrolment timing — start the enrolment by end of stage)

**Stop points already resolved from earlier stage but acquiring IRs now:**
- Q-IR-1 (IR sourcing strategy — execute the plan from stage 0)

**Deliverables:**
- `Source/DSP/Delay.{cpp,h}`, `Source/DSP/ConvReverb.{cpp,h}`
- `Resources/IRs/` populated with licensed/captured IRs
- `Tests/test_delay.cpp`, `Tests/test_convreverb.cpp`
- Notebooks for delay timing verification and IR analysis
- PRs: `feature/delay`, `feature/conv-reverb`, `feature/initial-ir-library`
- Apple Developer account enrolment submitted

**Not in scope:**
- Algorithmic FDN reverb (ruled out, see CONTEXT.md)
- IR morphing via spectral interpolation (deferred unless Q-IR-3 lands there)
- Sidechain ducking on reverb tail (v1.x consideration)

**Risks:**
- *IR licensing rabbit hole* — biggest risk in this stage. Check licence terms before any IR enters the repo. If self-capturing, allow extra days. If using free libraries, double-check redistribution terms.
- *Convolution reverb CPU at zero-latency mode* — if too heavy, fall back to partitioned mode and report latency. Document in `decisions.md`.
- *Modulating convolution wet path causing weird artefacts* — pre-convolution modulation is safest. If post-convolution is needed, expect tuning work.

---

## Stage 5 — Macro mapping + Parameter system + First GUI pass (Week 5)

**Goal:** Translate the 7 macros to the dozens of internal parameters via hand-designed curves. Get a functional (not pretty) GUI showing the 7 knobs. This is when the plugin becomes a *plugin* rather than a chain of modules.

**End state:**
- `Source/Params/ParameterLayout.{cpp,h}` defines the AudioProcessorValueTreeState with 7 macros + sync toggle exposed to the host.
- `Source/Params/MacroMapping.{cpp,h}` implements each macro's mapping to internal DSP params. Curves hand-designed per Q-MAC-1, no overlap per Q-MAC-2.
- All 7 macros automatable in Ableton.
- Smoothed parameter changes (no zipper noise, no clicks) — verified on automation ramps.
- First GUI pass: 7 knobs visible, labelled, functional. Uses JUCE default LookAndFeel for now or a very thin custom one. Not the final design.
- GUI consumes Figma exports for layout reference but pixel-final design is stage 7.
- Plugin state save/load works (Ableton saves/loads the project with plugin state intact).
- Knob interaction model per Q-GUI-2.
- GUI scale strategy per Q-GUI-1.

**Stop points to resolve in this stage:**
- Q-MAC-1 (macro mapping curve design philosophy) — biggest call of the stage, half the product
- Q-MAC-2 (macro range overlap — no overlap at v1)
- Q-GUI-1 (GUI scale — fixed at v1)
- Q-GUI-2 (knob interaction model)

**Deliverables:**
- `Source/Params/ParameterLayout.{cpp,h}`
- `Source/Params/MacroMapping.{cpp,h}`
- `Source/GUI/MainComponent.{cpp,h}` (functional, not final design)
- `Source/GUI/KnobComponent.{cpp,h}`
- `Source/GUI/LookAndFeel.{cpp,h}` (stub)
- PRs: `feature/parameter-system`, `feature/macro-mapping`, `feature/gui-first-pass`
- `decisions.md` entries documenting the *initial* macro mapping decisions (these will be revisited in stage 6)

**Not in scope:**
- Preset save/load UI (stage 6)
- Final GUI design (stage 7)
- Preset browser (stage 6)
- Activation/licensing (stage 7)
- Marketing site (stage 7)

**Risks:**
- *Macro mapping feels dead at low values* — almost certain on first pass with linear curves. This is the whole point of Q-SAT-2 and similar — surface and tune in stage 6.
- *Parameter automation drops out or jitters* — usually a smoothing issue. Check `SmoothedValue` ramp times on every internal param.
- *State save/load loses values* — happens when parameter IDs change or aren't registered with the ValueTreeState. Audit before stage end.

---

## Stage 6 — Preset design + Tuning pass (Week 6)

**Goal:** The single most important stage. Build the 60-preset factory bank that *is* the product. Use real source material in a real DAW. This is where taste does the heaviest lifting and where most of the remaining open questions get resolved.

**End state:**
- 60 presets shipped in `Resources/Presets/` (category split per Q-PRE-1).
- Preset naming convention per Q-PRE-2 applied consistently.
- Preset save/load UI in the GUI — at minimum, dropdown with category sections; ideally a proper browser.
- Each preset tested against ≥1 piece of source material in the category (real vocal stems, real synth bounces, real drum loops).
- Macro curves tuned per Q-SAT-2 and similar — the macros should *all* feel responsive across their full range, on at least one preset that exercises that macro.
- Hysteresis parameter tuning resolved per Q-SAT-3.
- Each macro touches what feels right, not what was guessed in stage 5.
- The chain is *locked* — no more DSP changes after this stage. Final DSP behaviour committed.

**Stop points to resolve in this stage:**
- Q-SAT-2 (Character macro drive curve)
- Q-SAT-3 (hysteresis param tuning)
- Q-PRE-1 (preset category split)
- Q-PRE-2 (preset naming convention)
- Any straggler macro tuning surfaced during preset design

**Deliverables:**
- 60 preset JSON files in `Resources/Presets/Vocals/`, `/Synths/`, `/Drums/`
- `Source/Params/PresetManager.{cpp,h}`
- `Source/GUI/PresetBrowser.{cpp,h}`
- PRs: `feature/preset-manager`, `feature/preset-bank` (probably split into multiple PRs by category), `fix/macro-curves`
- `docs/presets/<category>.md` notes on the design philosophy per category
- `decisions.md` entries closing out tuning-related open questions

**Not in scope:**
- Final GUI design (stage 7) — preset browser can be functional, not final
- Licensing infrastructure (stage 7)
- Beta build distribution (stage 7)
- Adding new DSP features (the chain is locked at this stage's start)

**Risks:**
- *Underbudgeting preset design* — 60 presets in a week is ~12/day, ~1/hour at 8h days. Realistic. But each preset that *reveals a DSP problem* eats hours. Budget the buffer for this.
- *Scope creep into DSP changes* — strong temptation when preset designing to "fix" the saturation curve, add a parameter, etc. Resist. The chain is locked. Bugs are exceptions; "wouldn't it be cool if" goes in `v1.x_ideas.md`.
- *Preset bank feels uneven* — likely. Have a trusted ear (one of the named customers) review the bank at end of stage and flag the weakest 10. Replace or improve those before stage end.

---

## Stage 7 — GUI polish + Licensing + Launch prep (Week 7)

**Goal:** Make it look like a real product. Build the activation infrastructure. Get to a signed, notarized, sellable artefact.

**End state:**
- Final GUI: custom LookAndFeel matching the Figma design. All 7 knobs, preset browser, brand mark, version label.
- Activation system working: Lemon Squeezy license key → plugin checks via API on launch, caches result.
- Demo mode behaviour per Q-LIC-2 (whatever was decided — full demo, time-limited, or none).
- Apple Developer Account active (should be by now per stage 4 enrolment). Developer ID Application cert installed.
- Signed AU and VST3 binaries. Notarized via `notarytool`. Stapled.
- Installer (`.pkg`) built that installs to `/Library/Audio/Plug-Ins/Components/` and `/Library/Audio/Plug-Ins/VST3/`. Signed and notarized.
- Marketing site: single landing page, video demo, audio examples (before/after on real material), pricing, buy button via Lemon Squeezy.
- 5 named beta testers have the build, are giving feedback.
- Critical-path beta feedback addressed (bugs, glaring sound issues). Polish feedback parked for v1.x.

**Stop points to resolve in this stage:**
- Q-LIC-1 (activation flow specifics)
- Q-LIC-2 (demo / trial mode — must decide and ship)

**Deliverables:**
- Final `Source/GUI/LookAndFeel.{cpp,h}`, all GUI components polished
- `Source/Licensing/Activation.{cpp,h}`, `Source/Licensing/MachineID.{cpp,h}`
- Activation API repo (separate, deployed on VPS or Lemon Squeezy's hosted system)
- Built, signed, notarized `.pkg` installer
- Marketing site (separate repo, deployed)
- Lemon Squeezy product configured, license keys flowing
- PRs: `feature/final-gui`, `feature/activation`, `feature/installer`
- `decisions.md` final entries

**Not in scope:**
- Windows port (v1.1)
- AAX / Pro Tools (v1.x or later)
- Manual / user docs beyond a one-page quickstart
- Localisation
- v2 neural voicing prep (banked, separate roadmap when v1 ships)

**Risks:**
- *Notarization fails on first attempt* — almost certain. Common causes: hardened runtime missing, entitlements wrong, dependent dylibs not signed. Budget a full day for notarization debugging. Apple's docs and StackOverflow answers carry you through.
- *Beta testers find a deal-breaker* — possible. The mitigation is having 5 beta testers, not 1. If 1 hates it but 4 love it, you're fine. If 5 are lukewarm, that's the real signal.
- *GUI polish overruns* — design work is open-ended. Set a hard 3-day budget for the LookAndFeel pass. Pixel-perfect is v1.x; "looks like a real plugin" is v1.
- *Marketing site overruns* — same trap. Single page, single video, 5 audio examples, buy button. That's it. The plugin sells itself or doesn't.

---

## Stage 8 — Launch + first month (Buffer week + post-ship)

**Goal:** Ship, respond to first-week issues, learn what the next move is.

**End state:**
- Plugin live on Lemon Squeezy and marketing site.
- Announced to: 5 named customers (private intro), Twitter/X, relevant Discord/Reddit communities (KVR Audio, r/edmproduction, etc.), one or two reach-out emails to producers in the palette.
- First-week issues triaged: critical fixes go in a v1.0.1 release within 7–10 days, non-critical in v1.1.
- Sales numbers observed, not panicked about.
- `v1.x_ideas.md` populated with everything that came up during stages 6–7 that didn't ship.
- Decision made: do v1.1 (Windows port + bug fixes + small features), v2 (neural voicing stage), or pause Track 2 and return to Track 1.

**Stop points to resolve in this stage:**
- None pre-defined — emergent.

**Deliverables:**
- Public launch
- v1.0.1 hotfix (if needed)
- Post-launch retrospective in `docs/retro.md`: what worked, what didn't, what the right v2 move looks like

**Not in scope:**
- v1.1 features
- v2 neural voicing work
- A second plugin

**Risks:**
- *Sales are underwhelming* — likely for a debut plugin from a no-name shop. The right read isn't "this was a failure" — it's "what did I learn about the customer / palette / positioning?" v1's success metric isn't revenue, it's having a shipped artefact that proves you can ship and a feedback loop with real customers.
- *Sales are great and v2 momentum is irresistible* — equally a risk. Track 1 is the long-horizon work. If v1 lands, the temptation to dive into v2 will be real. Honour the original framing: Track 2 is secondary. v2 starts when Track 1 is at a natural break.

---

## Cross-cutting principles

A few rules that apply across every stage:

1. **The chain is locked at start of stage 6.** No new DSP features after that. Bugs are exceptions; "wouldn't it be cool if" goes in `v1.x_ideas.md`.
2. **Stop points are stop points.** Agent never silently picks defaults on Q-IDs marked **NO DEFAULT**. Even on those with defaults, the default is for *unblocking forward progress* not for shipping — confirm with the human within the same PR.
3. **Each stage has a buffer day built into it.** If a stage finishes on day 4, day 5 goes into the *next* stage's buffer, not into expanding the current stage.
4. **If two stages in a row overrun by more than a day, stop and re-plan.** Either the estimate is wrong or scope is wrong. Don't push through a third overrunning stage.
5. **`open_questions.md` is the master list. `decisions.md` is the audit trail.** Resolved questions move from one to the other with date and rationale. Never delete entries from either.
6. **The Track 1 boundary holds.** If Track 1 needs attention, the plugin pauses. The roadmap is designed so each stage is a clean pause point — finish the stage, commit, walk away. Resume when ready.

## Status tracking

Stage status in this file (update as work progresses):

- Stage 0 — Pre-flight — **IN PROGRESS** (2026-05-24)
- Stage 1 — Plumbing — **DONE ON BRANCH** (2026-05-24, verified locally; merge blocked by Stage 0 external deliverables)
- Stage 2 — Tilt EQ + Saturation — NOT STARTED
- Stage 3 — Chorus + Filter — NOT STARTED
- Stage 4 — Delay + ConvReverb — NOT STARTED
- Stage 5 — Macros + GUI v1 — NOT STARTED
- Stage 6 — Presets + Tuning — NOT STARTED
- Stage 7 — Polish + Launch prep — NOT STARTED
- Stage 8 — Launch + first month — NOT STARTED

When a stage starts, change to **IN PROGRESS** with date. When it ends, **DONE** with date. If it stalls, **PAUSED** with reason.
