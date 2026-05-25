# CONTEXT.md

The briefing doc. If you're an agent or a collaborator joining this project, read this end-to-end before doing anything.

## Thesis

Florescence is an atmosphere processor for nocturnal electronic music.

It is a stereo bus tool for synth bus, vocal bus, and mix bus: a way for producers making dark, cinematic electronic music to make sources feel like they are inside a scene at night. The product is not the individual DSP modules. The product is the way saturation, chorus, filtering, delay, and convolution reverb combine into one aesthetic register, then surface as a small set of expressive controls.

The differentiation is the front panel: three atmosphere-named macros plus a three-mode Character switch. Producers do not reach for "the chorus knob" or "the tape module"; they reach for Atmosphere, Burn, Pulse, and a mode like Velvet, Onyx, or Chrome.

This is **Track 2 v1** — the secondary track. The primary track is the audio-reactive visuals work (`audio-visual-project/`). Track 2 exists to scratch the itch to build and sell, keep business sense sharp, and ship a real artifact that proves the JUCE/signing/notarization pipeline before any neural DSP work goes near a customer. Track 1 stays the psychic priority.

## What it is

- A stereo in / stereo out audio effect.
- Fixed-order FX chain: tilt EQ → saturation → chorus → filter → delay → convolution reverb.
- Macro-driven: three large macros, one three-position Character switch, Day/Night, Output, Dry/Wet, and a small preset dropdown. Stereo width handling is unresolved in Q-GUI-3.
- 60-preset factory bank, with themed categories to be resolved in Q-PRE-1-REVISED.
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
- **Not a tape saturator.** Saturation is one part of the atmosphere engine, not the product category.
- **Not lo-fi tape/cassette emulation.** Texture is allowed; cassette nostalgia is not the center of gravity.
- **Not a producer's first plugin.** It is an aesthetic-led purchase, not a general workhorse.
- **Not feature-creep-friendly.** If a "small addition" appears mid-build, it goes in v1.1 unless it's a bug.

## Aesthetic target

Same palette as the Track 1 visual work — the audio-side companion to the same world.

- **The Weeknd, Trilogy / Kissland / After Hours / Dawn FM** — synth-driven, atmospheric, lo-fi-but-luxurious. Reverb forward in the mix, controlled palette, neon-through-haze.
- **Illangelo / Doc McKinney production signatures** — sidechain-pumped pads, gated reverbs, pitched-down vocal stabs, "wet but clear" mix balance.
- **Gesaffelstein / industrial nocturnal electronics** — menacing, synthetic, hard-edged, cold, scale-heavy.
- **Drive (2011), Blade Runner 2049, The Batman (2022)** — soundtrack-adjacent, cinematic synthwave, atmospheric tension.
- **Burial / lo-fi nocturnal electronic** — the texture end of the palette, vinyl crackle, distant detuned chords.

The broader axis is nocturnal-electronic-cinematic: menacing industrial at one end, seductive glossy R&B/pop in the middle, cold futurist synth cinema at the other. The Character switch maps to these sub-regions: Velvet for soft/seductive/warm, Onyx for cold/hard/menacing, Chrome for clean/polished/futurist.

What this is **not**: bright modern pop FX, utility channel strips, surgical mastering tools, EDM punch processors, lo-fi-hiphop tape sims. Adjacent territory but wrong center of gravity.

Pinned reference tracks (the A/B targets for preset design — listed in `docs/aesthetic/reference-tracks.md`).

## Stack — decided

- **Framework: JUCE 8.x.** C++20. Industry standard for AU/VST3, mature, well-documented, free for revenue under $50k.
- **Build: CMake.** Modern JUCE supports it cleanly, scales to CI.
- **DSP utilities: JUCE first, clean-room from papers second.** Licence review found only `chowdsp_buffers`, `chowdsp_math`, and `chowdsp_simd` BSD-safe; the DSP modules needed for v1 are GPLv3-blocked. Do not add chowdsp DSP modules to `Source/DSP/`.
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

- **Phase:** Start of Stage 3 — north-star alignment before any Chorus or Filter code.
- **Active branch:** `docs/revamp-north-star-alignment`
- **Module being worked on:** none in this PR; C++ changes are explicitly deferred.
- **Open questions being surfaced:** Q-MAC-3, Q-MAC-4, Q-CHAR-1, Q-CHAR-2, Q-GUI-3, Q-GUI-4, Q-PRE-1-REVISED, Q-PRE-2-REVISED.
- **Blockers:** the new Stage 3+ control-surface stop points must be resolved before macro, Character switch, IR, GUI, and preset implementation PRs.

## Control surface

The v1 surface is locked around three large macros, one three-position Character switch, one global brightness toggle, Output, and Dry/Wet. Stereo width is unresolved in Q-GUI-3.

### Atmosphere

Atmosphere is the headline "more of this" control. It is a meta-macro that scales Burn, Pulse, and convolution reverb wet level together along a curated, non-linear curve. At 0, the sub-macros sit at their lowest meaningful values and reverb wet is near zero. At 1, Burn and Pulse are pushed toward their upper ranges and reverb wet is obviously wet.

Sub-macros remain independently adjustable: Atmosphere sets a baseline, Burn and Pulse offset from that baseline. Producer mental model: turn Atmosphere up for more atmosphere, then dial Burn and Pulse to taste.

### Burn

Burn is the density, thickness, and saturation axis. It drives saturation amount, low-mid emphasis in post-saturation tone shaping, and possibly chorus depth at high values depending on Q-MAC-4. Producer mental model: how thick, present, and saturated.

### Pulse

Pulse is the motion and life axis. It drives chorus rate/depth, delay modulation depth, convolution reverb wet-path modulation, and any other time-varying behaviour. Producer mental model: how alive, breathing, and moving.

### Character switch

The Character switch is a three-position segmented control. It changes the emotional register of the whole chain by applying a curated parameter snapshot; macros then modulate within that mode.

- **Velvet** — soft, seductive, warm. Kissland / After Hours register. Gentler asymmetric saturation, warmer upper-mid emphasis, plate/warm chamber IRs, slower deeper chorus, gentle filter moves.
- **Onyx** — cold, hard, menacing. Gesaffelstein / industrial register. Harder saturation, upper-mid bite, concrete/metallic/large dark spaces, tighter mechanical chorus, more aggressive filter resonance options.
- **Chrome** — clean, polished, futurist. Dawn FM / Blade Runner 2049 register. Cleanest saturation based on the Stage 2 clean-room Jiles-Atherton baseline, clean halls/bright chambers, lush glossy chorus, neutral controlled filtering.

### Global controls

- **Day/Night** — small top-level brightness toggle. Day applies a brighter, slightly more open high shelf around 8kHz. Night applies a darker, slightly compressed top end. When hidden from the main UI via settings, brightness sits neutral with no shelf.
- **Output** — vertical output trim fader, -inf to +12dB, default 0dB.
- **Dry/Wet** — mix knob, 0% to 100%, default 100% wet because Florescence is designed as a fully wet atmosphere tool; lower values support parallel setups.

## Scope and timeline

- **Target ship: 7 weeks of focused work** (~4 days/week minimum). 8–9 with realistic buffer.
- **Below 4 days/week, do not start.** Spreading this across 14 weeks at 2 days/week kills momentum and the plugin never ships.
- **Track 1 has priority.** If Track 1 is hot, this delays to autumn. The whole point of v1 being small is that it can be paused cleanly and resumed.

Rough week-by-week (full version in `docs/build-plan.md`):

1. JUCE plumbing, throwaway plugin, signing pipeline working end-to-end
2. Repo skeleton, Tilt EQ + Saturation modules
3. North-star alignment, then Chorus + Filter modules, true stereo locked down
4. Delay + Convolution Reverb, IR curation
5. Macro and Character switch mapping system, parameter automation, first GUI pass
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
- **80+ parameter exposure** — the user-facing surface is a small expressive control set, not raw module parameters.
- **Per-module bypass buttons on the main UI** — modules are implementation detail, not the mental model.
- **Sub-parameter exposure on the main UI** — no individual delay times, chorus rates, or filter cutoffs as primary controls.
- **Audio visualisations** — no spectrum analyser, waveform display, or gain-reduction meter in v1.
- **Prominent preset browser** — small dropdown at the top of the UI only.
- **XY pad at v1** — designed out for v1; may return in v1.x if the product needs a performance surface.
- **Neural DSP in audio path at v1** — banked for v2, architecture pre-empts it.
- **Buying into a multi-tool workflow** — one repo, JUCE + CMake, no exotic tooling; any optional DSP utility must clear licensing first.
