# CONTEXT.md

The briefing doc. If you're an agent or a collaborator joining this project, read this end-to-end before doing anything.

## Thesis

Build a stereo character processor plugin (AU + VST3, Mac) that bottles the Kissland / After Hours sonic palette into a single FX chain. The preset is the product — the chain is the engine, the macros let producers push presets in their direction. Drop on any source, dial a preset, tweak macros, done.

This is **Track 2 v1** — the secondary track. The primary track is the audio-reactive visuals work (`audio-visual-project/`). Track 2 exists to scratch the itch to build and sell, keep business sense sharp, and ship a real artifact that proves the JUCE/signing/notarization pipeline before any neural DSP work goes near a customer. Track 1 stays the psychic priority.

## What it is

- A stereo in / stereo out audio effect.
- Fixed-order FX chain: tilt EQ → saturation → chorus → filter → delay → convolution reverb.
- Macro-driven: 7 knobs + sync toggle on the front panel. Each macro sweeps curated ranges of dozens of internal parameters.
- 60-preset factory bank covering vocals, synths, drums/samples.
- Curated impulse response library (8–12 IRs) for the reverb.
- Convolution reverb (not algorithmic) — scope decision, see `docs/decisions.md`.
- Mac-only at v1 (AU + VST3, Universal Binary), online-activation licensing.

## What it is NOT

(So we don't re-litigate.)

- **Not a synth.** Effect, not instrument. The Weeknd-palette identity lives in voicing/space, not oscillators.
- **Not a Swiss Army knife.** Fixed signal flow, no reorderable modules, no hidden modulation matrix. Opinionated chain or it's not the product.
- **Not algorithmic reverb.** Building an FDN reverb is a 2–3 week scope-killer for a 7-week ship. Convolution + curated IRs gets there in days.
- **Not cross-platform at v1.** Mac only. Windows is a v1.1 conversation after the Mac version is shipping and earning.
- **Not neural.** v1 is pure DSP. The neural voicing stage is v2 work, banked. v1 architecture is designed to make that drop-in possible (see `docs/ARCHITECTURE.md`).
- **Not feature-creep-friendly.** If a "small addition" appears mid-build, it goes in v1.1 unless it's a bug.

## Aesthetic target

Same palette as the Track 1 visual work — the audio-side companion to the same world.

- **The Weeknd, Trilogy / Kissland / After Hours / Dawn FM** — synth-driven, atmospheric, lo-fi-but-luxurious. Reverb forward in the mix, controlled palette, neon-through-haze.
- **Illangelo / Doc McKinney production signatures** — sidechain-pumped pads, gated reverbs, pitched-down vocal stabs, "wet but clear" mix balance.
- **Drive (2011), Blade Runner 2049, The Batman (2022)** — soundtrack-adjacent, cinematic synthwave, atmospheric tension.
- **Burial / lo-fi nocturnal electronic** — the texture end of the palette, vinyl crackle, distant detuned chords.

What this is **not**: bright modern pop FX, clean digital reverb, surgical mastering tools, EDM punch processors, lo-fi-hiphop tape sims. Adjacent territory but wrong center of gravity.

Pinned reference tracks (the A/B targets for preset design — listed in `docs/aesthetic/reference-tracks.md`).

## Stack — decided

- **Framework: JUCE 8.x.** C++20. Industry standard for AU/VST3, mature, well-documented, free for revenue under $50k.
- **Build: CMake.** Modern JUCE supports it cleanly, scales to CI.
- **DSP utilities: chowdsp_utils.** Jatin Chowdhury's open-source JUCE library. Saves weeks on saturation, filter, modulation primitives. MIT licensed.
- **Convolution: `juce::dsp::Convolution`.** Partitioned convolution, zero-latency mode available, handles stereo IRs.
- **GUI: JUCE-native components + custom LookAndFeel.** No WebView at v1 — scope discipline. WebView is a v2 design uplift if it makes sense.
- **Tests: Catch2.** DSP module unit tests with deterministic inputs.
- **CI: GitHub Actions.** Mac build + sign + notarize on tag.
- **Licensing: online activation via a small VPS API.** Not iLok at v1 — overkill, adds friction.
- **Distribution: Gumroad or Lemon Squeezy initially.** They handle VAT, EU compliance, refunds. Self-hosted store is a later problem.

## Format and platform

- **AU (Audio Unit)** — required for Logic users, native on Mac. JUCE handles this from the same codebase.
- **VST3** — covers Ableton, Cubase, Studio One, Bitwig, FL on Mac. Required.
- **AAX** — not at v1. Pro Tools users wait for v1.x.
- **Mac Universal Binary** — Apple Silicon + Intel from one installer. Notarized.
- **Windows** — not at v1. v1.1 if v1 lands.

## Open questions / stop points

Hard-stop points where the agent must not guess live in `docs/open_questions.md`. Check that file at session start and before starting any new module. Stop-point protocol is in `AGENTS.md`. Resolved questions move to `decisions.md`.

## Staged build plan

Full week-by-week roadmap with goals, end states, scope, and stop points per stage lives in `docs/roadmap.md`. Read it before starting a new stage. Stages are sequential — don't start stage N+1 until stage N's end state is met.

## Current focus

- **Phase:** Stage 1 — Plumbing verified on branch; Stage 0 external deliverables still block merge.
- **Active branch:** `feature/plumbing`
- **Module being worked on:** none; Stage 1 plumbing is ready for PR review.
- **Open question being answered:** none currently.
- **Blockers:** domain registration, reference tracks, private beta customer list, Figma file, Lemon Squeezy account.

## Scope and timeline

- **Target ship: 7 weeks of focused work** (~4 days/week minimum). 8–9 with realistic buffer.
- **Below 4 days/week, do not start.** Spreading this across 14 weeks at 2 days/week kills momentum and the plugin never ships.
- **Track 1 has priority.** If Track 1 is hot, this delays to autumn. The whole point of v1 being small is that it can be paused cleanly and resumed.

Rough week-by-week (full version in `docs/build-plan.md`):

1. JUCE plumbing, throwaway plugin, signing pipeline working end-to-end
2. Repo skeleton, Tilt EQ + Saturation modules
3. Chorus + Filter modules, true stereo locked down
4. Delay + Convolution Reverb, IR curation
5. Macro mapping system, parameter automation, first GUI pass
6. Preset design (60 presets — the most important week)
7. GUI polish, beta with 5–10 producers, marketing site, launch

## Customer

The named customer for v1 is a specific 5-person list of friends/peers whose music sits in this palette. Their honest feedback in weeks 6–7 is the actual quality bar — not generic "producer" surveys, not Reddit, not Plugin Boutique reviews. Names live in `docs/customers.md` (private).

Broader market: bedroom-to-mid-tier producers working in atmospheric/dark electronic, R&B, pop with synth lean. The TAIP / Output Thermal / Soundtoys EchoBoy buyer.

## Pricing

- **Intro: £79** (first 30 days, launch promo)
- **Standard: £119**
- Comparable: Baby Audio TAIP £79, Output Thermal £149, Soundtoys EchoBoy £199.

Undercuts premium, matches indie-premium. Right slot for a debut plugin from a no-name shop.

## Long horizon

This work feeds Track 2's role in the broader thesis: scratch the build-and-sell itch, sharpen plugin/business chops, ship a foundation that v2 (neural voicing stage) drops into cleanly. v2 is banked but architecturally pre-empted in v1.

Both tracks ultimately feed the venue dream: deep technical fluency at the intersection of DSP, ML, and music taste, with shipped artifacts proving it. See `Current update how I feel` and `Projects` notes in the broader context.

## What's been ruled out

- **Algorithmic FDN reverb at v1** — scope killer, convolution + IRs instead.
- **Synth/instrument format** — effect format won on demo-ability, scope, addressable use.
- **WebView GUI at v1** — JUCE-native is faster to ship, WebView is a v2 uplift if warranted.
- **Reorderable signal chain** — opinionated fixed order is the product.
- **iLok or hardware dongle licensing** — friction outweighs piracy protection at this scale.
- **Pro Tools / AAX support at v1** — Avid SDK overhead not worth it for v1 audience.
- **Windows at v1** — Mac is 70% of producer market in this palette, scope discipline.
- **80+ parameter exposure** — 7 macros is the entire user-facing surface, by design.
- **Neural DSP in audio path at v1** — banked for v2, architecture pre-empts it.
- **Buying into a multi-tool workflow** — one repo, JUCE + chowdsp_utils + CMake, no exotic tooling.
