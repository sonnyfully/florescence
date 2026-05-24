# ARCHITECTURE.md

How the plugin is laid out and how the pieces fit together. Read this after `docs/CONTEXT.md`.

## High-level signal flow

```
   Stereo audio in (from DAW)
            │
            ▼
   ┌─────────────────────────┐
   │  Input gain + L/R       │
   │  alignment              │
   └─────────────────────────┘
            │
            ▼
   ┌─────────────────────────┐
   │  Tilt EQ / pre-shape    │   subtle pre-colouring before saturation
   │  (DSP/TiltEQ)           │
   └─────────────────────────┘
            │
            ▼
   ┌─────────────────────────┐
   │  Saturation             │   tape-style, asymmetric, oversampled 2-4x
   │  (DSP/Saturation)       │   ← v2 neural voicing stage inserts AFTER this
   └─────────────────────────┘
            │
            ▼
   ┌─────────────────────────┐
   │  Chorus                 │   Juno-style, true stereo, BBD-flavoured
   │  (DSP/Chorus)           │
   └─────────────────────────┘
            │
            ▼
   ┌─────────────────────────┐
   │  Filter + env follower  │   gentle SVF low-pass, slow envelope sweeps
   │  (DSP/Filter)           │
   └─────────────────────────┘
            │
            ▼
   ┌─────────────────────────┐
   │  Stereo delay           │   BPM-synced, filtered feedback, ping-pong opt
   │  (DSP/Delay)            │
   └─────────────────────────┘
            │
            ▼
   ┌─────────────────────────┐
   │  Convolution reverb     │   juce::dsp::Convolution, curated IRs
   │  (DSP/ConvReverb)       │   stereo-true, modulated wet path
   └─────────────────────────┘
            │
            ▼
   ┌─────────────────────────┐
   │  Dry/wet mix + width    │
   │  + output trim          │
   └─────────────────────────┘
            │
            ▼
   Stereo audio out (to DAW)
```

The signal chain is **fixed order, non-reorderable**. This is the product choice — the opinion is the value. Reorderability is a v2.x conversation if (and only if) customers prove they want it.

## Module contract

Every DSP stage implements a single interface:

```cpp
class FXModule {
public:
    virtual void prepare(const juce::dsp::ProcessSpec& spec) = 0;
    virtual void process(juce::dsp::AudioBlock<float>& block) = 0;
    virtual void reset() = 0;
    virtual int getLatencySamples() const { return 0; }
    virtual ~FXModule() = default;
};
```

The chain is just a `std::vector<std::unique_ptr<FXModule>>` processed in order in `PluginProcessor::processBlock`. Total reported latency is the sum of `getLatencySamples()` across modules, reported to the host via `setLatencySamples()` so the DAW can compensate.

**Why this matters for v2:** the neural voicing stage in v2 implements the same `FXModule` interface and inserts into the vector between Saturation and Chorus. No other code changes. No rewrite. v1 architecture is designed to make v2 a drop-in, not a refactor.

## Repo layout

```
character-fx/
├── AGENTS.md                  # Conventions for AI coding agents (Codex entry point)
├── README.md                  # Status, install, build, license
├── CMakeLists.txt             # Top-level build
├── .gitignore
│
├── docs/
│   ├── CONTEXT.md             # Project briefing — read first
│   ├── ARCHITECTURE.md        # This file
│   ├── roadmap.md             # Staged build plan with goals + end states per stage
│   ├── decisions.md           # Append-only decision log
│   ├── open_questions.md      # Hard-stop points — agent must check before each module
│   ├── customers.md           # Private: named v1 customer list
│   ├── aesthetic/
│   │   ├── reference-tracks.md
│   │   └── teardowns/         # Per-track analysis: what's the FX move?
│   ├── presets/               # Preset design notes per category
│   │   ├── vocals.md
│   │   ├── synths.md
│   │   └── drums.md
│   └── research/              # One markdown per technique/paper
│       ├── tape-saturation.md
│       ├── bbd-chorus.md
│       └── ...
│
├── Source/
│   ├── PluginProcessor.cpp/h  # juce::AudioProcessor — chain owner, processBlock
│   ├── PluginEditor.cpp/h     # juce::AudioProcessorEditor — main GUI
│   │
│   ├── DSP/
│   │   ├── FXModule.h         # The interface above
│   │   ├── TiltEQ.cpp/h       # Pre-saturation EQ shape
│   │   ├── Saturation.cpp/h   # Tape-style, oversampled
│   │   ├── Chorus.cpp/h       # BBD-flavoured, true stereo
│   │   ├── Filter.cpp/h       # SVF + envelope follower
│   │   ├── Delay.cpp/h        # BPM-synced stereo delay
│   │   └── ConvReverb.cpp/h   # juce::dsp::Convolution wrapper + IR mgmt
│   │
│   ├── Params/
│   │   ├── ParameterLayout.cpp/h  # juce::AudioProcessorValueTreeState setup
│   │   ├── MacroMapping.cpp/h     # Macro → internal param curves
│   │   └── PresetManager.cpp/h    # JSON load/save, factory bank loader
│   │
│   ├── GUI/
│   │   ├── MainComponent.cpp/h
│   │   ├── KnobComponent.cpp/h
│   │   ├── PresetBrowser.cpp/h
│   │   ├── LookAndFeel.cpp/h      # Custom paint, fonts, colours
│   │   └── Resources.h            # Binary resources (fonts, SVGs)
│   │
│   └── Licensing/
│       ├── Activation.cpp/h       # Online activation client
│       └── MachineID.cpp/h        # Machine fingerprinting
│
├── Resources/
│   ├── IRs/                   # Curated impulse responses (.wav)
│   ├── Presets/               # Factory bank (.json)
│   ├── Fonts/
│   └── Graphics/              # SVGs, exported from Figma
│
├── Tests/
│   ├── CMakeLists.txt
│   ├── test_saturation.cpp    # Catch2
│   ├── test_filter.cpp
│   ├── test_delay.cpp
│   └── ...
│
└── .github/
    └── workflows/
        ├── build-mac.yml      # CI: build, test, sign, notarize
        └── release.yml        # On tag: package installer, upload
```

## Module responsibilities

### `Source/DSP/`

Pure DSP. No JUCE GUI dependencies, no parameter system dependencies — these modules take raw values (frequencies, gains, mix amounts) and process audio. Parameter mapping happens one layer up.

- **`TiltEQ.cpp/h`** — Low-shelf + high-shelf tied to a single tilt parameter. Pre-saturation only — purpose is to *steer* saturation behaviour, not to be a user-facing EQ.
- **`Saturation.cpp/h`** — Asymmetric soft-clip with tape-style hysteresis. Oversampled 2x or 4x (decide in week 2 — 4x sounds better, 2x leaves CPU headroom for the convolution reverb). Reference: chowdsp_utils saturation primitives, Pirkle Chapter 19, *DAFX* Chapter 4.
- **`Chorus.cpp/h`** — Multi-voice modulated delay line, BBD-flavoured (slight high-frequency loss, slight pitch instability). True stereo: L and R have decorrelated LFO phases for width. Reference: Juno-60 chorus circuit analysis (multiple writeups online), chowdsp BBD model.
- **`Filter.cpp/h`** — State-variable filter (TPT topology), low-pass primary. Envelope follower on input signal modulates cutoff. Slow attack/release on the follower — this is the "duck" feel, not a sharp sidechain.
- **`Delay.cpp/h`** — Stereo delay lines with independent L/R times for ping-pong. BPM sync via host transport. Feedback path goes through a one-pole low-pass (filtered feedback — repeats get darker, not brighter). Reference: standard delay design, *DAFX* Chapter 2.
- **`ConvReverb.cpp/h`** — Wraps `juce::dsp::Convolution`. Loads IRs from `Resources/IRs/`. Crossfades between IRs when Space macro moves between categories. Modulation on wet path (slight chorus before convolution) to break up the static nature of convolution reverb — this is the "alive" trick.

### `Source/Params/`

- **`ParameterLayout.cpp/h`** — Defines the `AudioProcessorValueTreeState` with all 7 macros + sync toggle as automatable parameters. These are what the DAW sees.
- **`MacroMapping.cpp/h`** — The taste-encoding layer. Each macro is a function from `[0, 1]` to a vector of internal DSP parameter values. Curves are non-linear and hand-designed per macro. *This is where half the product lives.* (See `docs/decisions.md` for design notes per macro.)
- **`PresetManager.cpp/h`** — JSON preset format, versioned. Factory bank loaded from `Resources/Presets/` at startup. User presets saved to platform-standard location (`~/Library/Audio/Presets/<vendor>/<plugin>/`).

### `Source/GUI/`

JUCE-native rendering. Custom LookAndFeel for the visual identity (this is where the Figma work lands). Components are kept dumb — they observe the parameter tree, they don't own state.

### `Source/Licensing/`

Online activation. Machine-locked. Talks to a small Node/Python API on a VPS that issues and validates license keys. Simple, not bulletproof — piracy at v1 scale isn't the threat model; the threat model is making legitimate purchase frictionless.

### `Resources/IRs/`

8–12 curated impulse responses, all stereo `.wav` at 48kHz. Mix of:
- **Real spaces** — captured or licensed (Samplicity, Voxengo free IRs, or DIY captures of interesting rooms).
- **Classic hardware** — Lexicon 480L, Bricasti M7, EMT 140 plate impulses (available from various IR packs, license terms vary — check before shipping).
- **Designed IRs** — long synthetic tails with modulation baked in (these are where you get the "this doesn't sound like a normal reverb" character).

Naming convention: `<category>_<descriptor>_<length>.wav`. Example: `plate_emt140_2.4s.wav`. Categories: `plate`, `hall`, `chamber`, `space`, `room`.

### `Resources/Presets/`

60-preset factory bank as versioned JSON. Three subdirectories:
- `Vocals/` — 20 presets
- `Synths/` — 20 presets
- `Drums/` — 20 presets

Each preset includes a `schema_version` field. v2 presets will add neural voicing parameters; v1 hosts loading v2 presets ignore unknown fields; v2 hosts loading v1 presets use defaults.

## Preset JSON schema (v1)

```json
{
  "schema_version": 1,
  "name": "Velvet Vocal Ad-Lib",
  "category": "Vocals",
  "author": "Factory",
  "description": "Wide, wet, slightly saturated — for short vocal stabs.",
  "macros": {
    "character": 0.42,
    "movement": 0.31,
    "space": 0.78,
    "depth": 0.55,
    "tone": 0.67,
    "width": 0.85,
    "mix": 1.0
  },
  "settings": {
    "sync_division": "dotted_eighth"
  }
}
```

Macros are 0–1. The mapping from macro value to internal DSP parameters lives in code (`MacroMapping.cpp`), not in the preset. This means **changes to the macro mapping curves change every preset's sound**. That's intentional pre-v1 (it's how taste-tuning works) and locked-down at v1 ship (changing it post-ship invalidates customer presets).

## Latency

- TiltEQ: 0
- Saturation: 0 if 2x oversampling with linear-phase filter, or report ~1–2ms if minimum-phase
- Chorus: 0 (delay-line based, no lookahead)
- Filter: 0
- Delay: 0 (the delay time is musical, not algorithmic latency)
- ConvReverb: 0 in zero-latency mode (uniformly partitioned), or report partition size in non-zero-latency mode

Target total reported latency: **0 samples** at v1. Zero-latency convolution mode is the cost (slightly higher CPU) but worth it for the UX promise of "drop it in, no compensation issues."

v2 neural voicing will report ~5–20ms — the plumbing for `getLatencySamples()` exists from day one so v2 just returns its value.

## Threading

Standard JUCE pattern:
- **Audio thread** — `processBlock` only. No allocations, no locks, no file I/O.
- **Message thread** — GUI, parameter changes, preset loading.
- **Background thread** — IR loading on startup, activation API calls, preset bank refresh.

Preset switches are the trickiest: when a preset changes 7 macros which each touch dozens of internal params, the change has to be smooth (no zipper noise, no clicks). Handled by parameter smoothing in each DSP module — every internal parameter has a `juce::SmoothedValue` wrapper with a few-ms ramp time.

## Build system

Single top-level `CMakeLists.txt` using JUCE's CMake integration. Build targets:
- `CharacterFX_AU` — Audio Unit
- `CharacterFX_VST3` — VST3
- `CharacterFX_Standalone` — Standalone app for development/debugging
- `CharacterFX_Tests` — Catch2 test suite

CI on GitHub Actions: matrix on (Debug, Release), single OS (macOS-latest), runs full build + tests + AU validation (`auval`) + VST3 validation on every push. Release workflow on tag also signs, notarizes, and uploads installer artifacts.

## What lives outside this repo

- **Figma file** — GUI design source of truth. SVG exports come into `Resources/Graphics/`.
- **Marketing site repo** — separate, links to Gumroad/Lemon Squeezy for purchase.
- **Activation API** — separate small repo, runs on a $5/month VPS.
- **IR source files / DAW sessions used to generate them** — local only or in cloud backup, not in plugin repo.
- **Preset design DAW sessions** — local, not committed (huge files, customer audio).

## How a typical work session flows

1. Open repo. Codex reads `AGENTS.md` automatically. Confirm it's seen `CONTEXT.md` and `decisions.md`.
2. Check current branch and open PR. Resume from there, or start a new branch with a written PR description.
3. **DSP work:** open notebook (Python or Jupyter for plotting), prototype the algorithm, plot it against reference signals. Once stable, port to C++ module with tests. Build the Standalone target, A/B against reference tracks in Ableton.
4. **GUI work:** open Figma, iterate the design. Export changed SVGs to `Resources/Graphics/`. Build Standalone, see it. Iterate.
5. **Preset work:** open Ableton, load the Standalone or the AU in a real session with real source material (vocal stems, synth bounces, drum loops). Tune macros until the preset sits. Save to `Resources/Presets/<category>/<name>.json`.
6. PR with full description (what / why / alternatives / tested how / follow-ups). Squash-merge.
7. Append to `decisions.md` if anything non-obvious was decided.
