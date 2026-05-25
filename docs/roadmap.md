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

## Current reassessment — 2026-05-25

The codebase is ahead of some old roadmap gates, while a few external product-readiness items are still open. Treat this file as the current source of truth:

- Stage 1 plumbing is merged to `main`.
- Stage 2 Tilt EQ + Saturation code exists and is wired into the minimal chain. The saturation stage has been refactored to a clean-room Jiles-Atherton implementation; `ctest --test-dir build-juce8 --output-on-failure` passes 19/19 locally, including the VST3 smoke test. The older `build/` directory is stale/misconfigured and finds no tests.
- Stage 3 is currently the `docs/revamp-north-star-alignment` docs branch. Chorus and Filter C++ work has **not** started.
- Stage 0's unresolved non-code items no longer block Stage 3 engineering, but they are still product-readiness gates with due-by milestones below.
- `README.md` is known stale about branch/stage status. Do not treat it as authoritative until a separate docs cleanup updates it.

---

## Stage 0 — Pre-flight identity decisions (engineering-unblocking)

**Goal:** Resolve identity and technical decisions that must be known before engineering can proceed. No C++ feature work in this stage.

**End state:**
- Plugin name chosen (Q-NAME-1).
- Bundle ID and vendor name decided (Q-NAME-2).
- IR sourcing strategy decided in principle (Q-IR-1) — paid library / free library / self-capture / mix. Actual IR acquisition happens in stage 4.
- Apple Developer enrolment **not started yet** (deferred to end of stage 4 per Q-LIC-3).

**Deferred product-readiness gates:**
- 5 reference tracks pinned in a playlist + `docs/aesthetic/reference-tracks.md` written before Stage 6 tuning starts.
- 5 named beta customers written down in local ignored `docs/customers.md` before Stage 7 beta distribution.
- Figma direction started before Stage 7 GUI polish: colour palette, typography, knob style, rough layout for Atmosphere, Burn, Pulse, Character, and global controls. It does not need to be pixel-final before engineering continues.
- Florescence domain registered before launch prep in Stage 7.
- Lemon Squeezy account created before activation / launch prep in Stage 7.

**Stop points to resolve in this stage:**
- Q-NAME-1 (plugin name)
- Q-NAME-2 (bundle ID / vendor)
- Q-IR-1 (IR sourcing strategy — in principle)

**Deliverables:**
- `docs/decisions.md` entries for Q-NAME-1, Q-NAME-2, and Q-IR-1.
- `docs/non-code-todos.md` tracks the deferred external items above.

**Not in scope:**
- Any C++ code
- Apple Developer enrolment
- Actual IR files in the repo
- Marketing site

**Risks:**
- *External items drift too far into the build* — they no longer block Stage 3, but they do become hard gates at the stages listed above. Do not let Stage 6 begin without taste anchors, or Stage 7 begin without beta/customer/launch infrastructure.

---

## Stage 1 — Plumbing and throwaway plugin (Week 1)

**Goal:** Prove the entire build → load → iterate loop works on a trivial plugin, before any real DSP lands.

**Current status:** Done and merged to `main` via the plumbing PR. The branch-level "blocked by Stage 0 external deliverables" status is obsolete.

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
- chowdsp_utils integration — no v1 DSP module may use GPL chowdsp modules; prefer JUCE or clean-room paper implementations

**Risks:**
- *CMake fights JUCE* — budget a day for this. Jules Storer's JUCE CMake examples on GitHub are the reference; copy structure rather than improvise.
- *Apple Silicon vs Intel issues* — build Universal Binary from day one. `CMAKE_OSX_ARCHITECTURES="arm64;x86_64"`.
- *AU validation fails* — common, fix early before the chain is large enough to make debugging hard.

---

## Stage 2 — Tilt EQ + Saturation (Week 2)

**Goal:** First two real DSP modules in the chain, audibly working on real audio. Saturation is the aesthetic anchor — if it doesn't sound right, nothing downstream saves it.

**Current status:** Done on 2026-05-25. `TiltEQ`, clean-room Jiles-Atherton `Saturation`, `FXModule`, deterministic Catch2 tests, verification plots, and the minimal chain are present. Stage 2 still leaves Q-SAT-2 open for the Stage 6 Burn curve tuning pass.

**End state:**
- `FXModule` interface defined in `Source/DSP/FXModule.h` per `ARCHITECTURE.md`.
- `TiltEQ` module: stereo, tilt parameter -12dB to +12dB at pivot ~1kHz, unit-tested for frequency response.
- `Saturation` module: clean-room Jiles-Atherton hysteresis per the 2026-05-25 decision, drive parameter 0–1, 4x oversampled per Q-SAT-1, DC blocker, drive-coupled dynamic LPF plus fixed 8kHz post-saturator rolloff per the revised Q-SAT-4 history, unity behaviour per Q-SAT-5.
- Both modules plug into a minimal chain in `PluginProcessor::processBlock`.
- Plugin's two front-panel knobs (still ugly GUI) are mapped to Tilt amount and Saturation drive — direct mapping, no Macro layer yet.
- A/B listening check conducted on representative source material (vocal, synth, drum loop) — saturation character feels right or open question raised. The formal 5-track reference list is still due before Stage 6.
- Unit tests in `Tests/test_tilteq.cpp` and `Tests/test_saturation.cpp` pass.
- GPL chowdsp/CHOW Tape dependency explicitly rejected in `decisions.md`; no new CMake dependency for saturation.

**Stop points resolved in this stage:**
- Q-SAT-1 (oversampling factor) — resolved 2026-05-25
- Q-SAT-4 (internal HF rolloff yes/no) — resolved 2026-05-25, revised to include fixed 8kHz rolloff plus drive-coupled HF loss
- Q-SAT-5 (bypass behaviour at drive/Burn = 0) — resolved 2026-05-25

**Stop points deferred (to surface but not yet resolve):**
- Q-SAT-2 (Burn macro drive curve shape) — linear placeholder is acceptable before Stage 6, but not final without human listening review
- Q-SAT-3 (hysteresis param tuning) — reopened by the clean-room Jiles-Atherton decision; fixed starting values are implemented, with listening/tuning deferred to Stage 6

**Deliverables:**
- `Source/DSP/FXModule.h`
- `Source/DSP/TiltEQ.{cpp,h}`
- `Source/DSP/Saturation.{cpp,h}`
- `Tests/test_tilteq.cpp`, `Tests/test_jiles_atherton.cpp`, `Tests/test_saturation.cpp`
- Verification plots in `docs/research/saturation/`
- Two PRs: `feature/tilt-eq`, `feature/saturation`
- `docs/decisions.md` entries for the GPL dependency rejection, clean-room Jiles-Atherton path, Q-SAT-1, Q-SAT-3, Q-SAT-4, Q-SAT-5, Q-SAT-6, Q-SAT-7, and Q-SAT-8

**Not in scope:**
- Chorus, Filter, Delay, Reverb (later stages)
- Macro mapping layer (stage 6)
- GUI polish (stage 7)
- Drive curve tuning (stage 6)
- Multi-band saturation (not in v1 scope at all)

**Risks:**
- *Saturation CPU or tone misses the v1 bar* — the clean-room Jiles-Atherton path has a higher aesthetic ceiling but a heavier RK4 core. Full Release-plugin profiling and Stage 6 listening decide whether to keep it, optimize it, or explicitly choose the asymmetric-polynomial fallback.
- *Oversampling latency* — JUCE oversampling has latency in some modes. Target zero-latency mode, confirm `getLatencySamples()` returns 0.
- *Third-party DSP licensing issues* — surfaced by saturation and now tracked for chorus as Q-CHOR-1-LIC. Do not add chowdsp modules unless their licence is resolved first.

---

## Stage 3 — North-star alignment + Chorus + Filter (Week 3)

**Goal:** Align the project to the Florescence north star before code resumes, then build modulation stages working in true stereo so the chain feels "alive" rather than static.

**Current status:** In progress as a docs/planning branch only. No Chorus or Filter C++ work should start until the Stage 3 stop points below are surfaced and resolved.

**End state:**
- North-star docs/planning PR merged before any Stage 3 C++ work.
- `Chorus` module: BBD-flavoured (per Q-CHOR-1), N voices (per Q-CHOR-2), true stereo with decorrelated L/R LFO phases, depth and rate parameters.
- `Filter` module: SVF (TPT) low-pass, cutoff + resonance parameters, envelope follower modulating cutoff per Q-FILT-2.
- Both modules in the chain after Saturation.
- True stereo verified: identical L/R input → identical L/R output (stereo coherence test).
- A/B tested on reference material — by end of stage, the chain (Tilt → Saturation → Chorus → Filter) sounds *recognisably* in the target palette on at least one preset sketch, even if rough.
- Unit tests for both modules.
- Two PRs merged.

**Stop points to resolve in this stage:**
- Q-CHOR-1-LIC (chowdsp / BBD licensing) — **must be resolved before any Chorus implementation starts**
- Q-CHOR-1 (BBD model fidelity) — resolve before Chorus PR merge
- Q-CHOR-2 (chorus voice count) — resolve before Chorus PR merge
- Q-FILT-1 (filter topology — confirm SVF is right) — resolve before Filter PR merge
- Q-FILT-2 (envelope follower scope — input only vs sidechain capable) — resolve before Filter PR merge

**Deliverables:**
- Docs/planning PR: `docs/revamp-north-star-alignment`
- `Source/DSP/Chorus.{cpp,h}`, `Source/DSP/Filter.{cpp,h}`
- `Tests/test_chorus.cpp`, `Tests/test_filter.cpp`
- Notebooks for chorus modulation visualisation and filter frequency response
- PRs: `feature/chorus`, `feature/filter`
- `decisions.md` entries for resolved Q-CHOR / Q-FILT items

**Not in scope:**
- Delay and reverb (stage 4)
- Sidechain input routing for filter (deferred to v1.x per Q-FILT-2 default)
- Macro mapping and Character switch implementation (stage 5)

**Risks:**
- *Licensing blocks the intended chorus direction* — Q-CHOR-1-LIC has no safe default. If chowdsp BBD primitives are GPLv3/incompatible, choose among a clean-room implementation, permissive Airwindows-flavoured code, pure delay-line modulation, or a commercial licence before writing code.
- *Chorus phasing artefacts when summed to mono* — verify mono-compatibility, but accept that "true stereo" presets won't be mono-perfect. Document in `decisions.md` what the mono-sum tradeoff is.
- *Envelope follower feels too snappy or too slow* — do not silently lock taste values. If Q-FILT-2 resolves to input-following, start from the documented 10ms / 150ms saturation precedent only as an audition baseline and revisit in Stage 6.

---

## Stage 4 — Delay + Convolution Reverb (Week 4)

**Goal:** Time-based stages complete the chain. Convolution reverb is the cinematic space — half of why the plugin sounds expensive.

**End state:**
- `Delay` module: stereo lines with BPM sync (divisions per Q-DEL-1), filtered feedback, ping-pong support per Q-DEL-2.
- `ConvReverb` module: wraps `juce::dsp::Convolution`, loads IRs from `Resources/IRs/`, supports Atmosphere-driven wet level and Pulse-driven wet-path modulation. Character-mode IR behaviour is resolved per Q-CHAR-2.
- IR library populated with initial set (8 IRs per Q-IR-2 default, can grow to 12 if needed).
- Complete chain: Tilt → Saturation → Chorus → Filter → Delay → ConvReverb working end-to-end.
- Apple Developer enrolment initiated per Q-LIC-3 (this is the last stage where you can defer it — the next stages need signed builds for beta testing).
- A/B test on reference tracks: chain produces something that sits convincingly in the target aesthetic on ≥2 different source types.

**Stop points to resolve in this stage:**
- Q-IR-2 (IR count and category split)
- Q-CHAR-2 (Character switch reverb IR set behaviour)
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
- IR morphing via spectral interpolation unless a future decision explicitly reopens it
- Sidechain ducking on reverb tail (v1.x consideration)

**Risks:**
- *IR licensing rabbit hole* — biggest risk in this stage. Check licence terms before any IR enters the repo. If self-capturing, allow extra days. If using free libraries, double-check redistribution terms.
- *Convolution reverb CPU at zero-latency mode* — if too heavy, fall back to partitioned mode and report latency. Document in `decisions.md`.
- *Modulating convolution wet path causing weird artefacts* — pre-convolution modulation is safest. If post-convolution is needed, expect tuning work.

---

## Stage 5 — Macro mapping + Character switch + First GUI pass (Week 5)

**Goal:** Translate Atmosphere, Burn, Pulse, and the Character switch to dozens of internal parameters via hand-designed curves. Get a functional (not pretty) GUI showing the locked v1 control surface. This is when the plugin becomes a *plugin* rather than a chain of modules.

**End state:**
- `Source/Params/ParameterLayout.{cpp,h}` defines the AudioProcessorValueTreeState with Atmosphere, Burn, Pulse, Character mode, Day/Night, Output, Dry/Wet, and any stereo-width control resolved in Q-GUI-3.
- `Source/Params/MacroMapping.{cpp,h}` implements macro offsets within each Character mode. Curves hand-designed per Q-MAC-1 and Q-MAC-3.
- `CharacterPreset` or equivalent data structure represents Velvet / Onyx / Chrome snapshots.
- All top-level controls automatable in Ableton where appropriate.
- Smoothed parameter changes (no zipper noise, no clicks) — verified on automation ramps.
- First GUI pass: three large macros, Character switch, Day/Night, Output, Dry/Wet, preset dropdown, and any resolved width control visible and functional. Uses JUCE default LookAndFeel for now or a very thin custom one. Not the final design.
- GUI follows the locked control surface even if the Figma direction is still rough. Pixel-final design is Stage 7.
- Plugin state save/load works (Ableton saves/loads the project with plugin state intact).
- Knob interaction model per Q-GUI-2.
- GUI scale strategy per Q-GUI-1.

**Stop points to resolve in this stage:**
- Q-MAC-1 (macro mapping curve design philosophy) — biggest call of the stage, half the product
- Q-MAC-3 (Atmosphere meta-macro mapping curve)
- Q-MAC-4 (whether Burn drives chorus depth at high values)
- Q-CHAR-1 (Character switch selector/crossfade/morph behaviour)
- Q-GUI-1 (GUI scale — fixed at v1)
- Q-GUI-2 (knob interaction model)
- Q-GUI-3 (stereo width control on main UI)
- Q-GUI-4 (Day/Night toggle default visibility)
Q-GUI-5, Q-CHAR-3, Q-MAC-DECOMP

**Deliverables:**
- `Source/Params/ParameterLayout.{cpp,h}`
- `Source/Params/MacroMapping.{cpp,h}`
- `Source/GUI/MainComponent.{cpp,h}` (functional, not final design)
- `Source/GUI/KnobComponent.{cpp,h}`
- `Source/GUI/LookAndFeel.{cpp,h}` (stub)
- PRs: `refactor/macro-parameter-renames`, `feature/atmosphere-routing`, `feature/character-switch`, `feature/gui-first-pass`
- `decisions.md` entries documenting the *initial* macro mapping decisions (these will be revisited in stage 6)
- Macro decomposition table (Q-MAC-DECOMP) committed to docs/decisions.md as the audit trail of macro → internal-parameter mappings. This is a *required deliverable*, not just code — Stage 6 tuning depends on it being readable and reviewable.
- Q-GUI-5 resolved (Day/Night semantic behaviour) and reflected in MacroMapping.
- Q-CHAR-3 resolved (CharacterPreset state shape) and reflected in the CharacterPreset / equivalent data structure.

**Not in scope:**
- Preset save/load UI (stage 6)
- Final GUI design (stage 7)
- Preset browser (stage 6)
- Activation/licensing (stage 7)
- Marketing site (stage 7)
- Per-module controls, per-module bypass, waveform/spectrum visualisations, or a prominent preset browser (explicitly cut from v1)

**Risks:**
- *Macro mapping feels dead at low values* — almost certain on first pass with linear curves. This is the whole point of Q-SAT-2 and similar — surface and tune in stage 6.
- *Parameter automation drops out or jitters* — usually a smoothing issue. Check `SmoothedValue` ramp times on every internal param.
- *State save/load loses values* — happens when parameter IDs change or aren't registered with the ValueTreeState. Audit before stage end.

---

## Stage 6 — Preset design + Tuning pass (Week 6)

**Goal:** The single most important stage. Build the 60-preset factory bank that *is* the product. Use real source material in a real DAW. This is where taste does the heaviest lifting and where most of the remaining open questions get resolved.

**End state:**
- 5 reference tracks are pinned and written up in `docs/aesthetic/reference-tracks.md` before tuning starts.
- 60 presets shipped in `Resources/Presets/` (themed category split per Q-PRE-1-REVISED).
- Preset naming convention per Q-PRE-2-REVISED applied consistently.
- Preset save/load UI in the GUI — at minimum, dropdown with category sections; ideally a proper browser.
- Each preset tested against real source material across synth bus, vocal bus, and mix bus use cases.
- Macro curves tuned per Q-SAT-2 and similar — Atmosphere, Burn, and Pulse should all feel responsive across their full range, on at least one preset that exercises that macro.
- Each macro touches what feels right, not what was guessed in stage 5.
- The chain is *locked* — no more DSP changes after this stage. Final DSP behaviour committed.

**Stop points to resolve in this stage:**
- Q-SAT-2 (Burn macro drive curve)
- Q-PRE-1-REVISED (themed preset split)
- Q-PRE-2-REVISED (evocative preset naming)
- Any straggler macro tuning surfaced during preset design

**Deliverables:**
- `docs/aesthetic/reference-tracks.md` completed with the 5 taste anchors used for tuning
- 60 preset JSON files in themed directories resolved by Q-PRE-1-REVISED
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
- *Reference tracks are still TBD* — Stage 6 must not start without them. The product is taste-led, so tuning without explicit anchors would reintroduce guessing.
- *Underbudgeting preset design* — 60 presets in a week is ~12/day, ~1/hour at 8h days. Realistic. But each preset that *reveals a DSP problem* eats hours. Budget the buffer for this.
- *Scope creep into DSP changes* — strong temptation when preset designing to "fix" the saturation curve, add a parameter, etc. Resist. The chain is locked. Bugs are exceptions; "wouldn't it be cool if" goes in `v1.x_ideas.md`.
- *Preset bank feels uneven* — likely. Have a trusted ear (one of the named customers) review the bank at end of stage and flag the weakest 10. Replace or improve those before stage end.

---

## Stage 7 — GUI polish + Licensing + Launch prep (Week 7)

**Goal:** Make it look like a real product. Build the activation infrastructure. Get to a signed, notarized, sellable artefact.

**End state:**
- Figma direction exists before GUI polish starts: colour palette, typography, knob style, and rough layout for Atmosphere, Burn, Pulse, Character, Day/Night, Output, Dry/Wet, and the small preset dropdown.
- Final GUI: custom LookAndFeel matching the Figma direction. Three macro knobs, Character switch, global controls, small preset dropdown, brand mark, version label.
- Activation system working: Lemon Squeezy license key → plugin checks via API on launch, caches result.
- Demo mode behaviour per Q-LIC-2 (whatever was decided — full demo, time-limited, or none).
- Apple Developer Account active (should be by now per stage 4 enrolment). Developer ID Application cert installed.
- Signed AU and VST3 binaries. Notarized via `notarytool`. Stapled.
- Installer (`.pkg`) built that installs to `/Library/Audio/Plug-Ins/Components/` and `/Library/Audio/Plug-Ins/VST3/`. Signed and notarized.
- Marketing site: single landing page, video demo, audio examples (before/after on real material), pricing, buy button via Lemon Squeezy.
- Florescence domain registered and pointed at the marketing site.
- Lemon Squeezy account and product configured before activation/checkout work is considered complete.
- 5 named beta testers are listed in local ignored `docs/customers.md`, have the build, and are giving feedback.
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
- Domain configured for the marketing site
- Local/private `docs/customers.md` filled with 5 beta testers
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

- Stage 0 — Pre-flight identity decisions — **DONE FOR ENGINEERING** (2026-05-24; remaining external items moved to due-by product-readiness gates)
- Stage 1 — Plumbing — **DONE / MERGED** (2026-05-24)
- Stage 2 — Tilt EQ + Saturation — **DONE** (2026-05-25, implementation verified and human A/B listening test passed)
- Stage 3 — North-star alignment + Chorus + Filter — **IN PROGRESS** (2026-05-25; docs alignment branch only, Chorus/Filter code not started)
- Stage 4 — Delay + ConvReverb — NOT STARTED
- Stage 5 — Macros + GUI v1 — NOT STARTED
- Stage 6 — Presets + Tuning — NOT STARTED
- Stage 7 — Polish + Launch prep — NOT STARTED
- Stage 8 — Launch + first month — NOT STARTED

When a stage starts, change to **IN PROGRESS** with date. When it ends, **DONE** with date. If it stalls, **PAUSED** with reason.
