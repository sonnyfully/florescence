# ARCHITECTURE.md

How the plugin is laid out and how the pieces fit together. Read this after `docs/CONTEXT.md`.

## High-level signal flow

The control surface sits above the modules. Modules remain fixed-order DSP machinery; the producer-facing model is macro + mode first.

```
   Host automation / GUI
            │
            ▼
   ┌────────────────────────────────────────────┐
   │  v1 control surface                         │
   │  Atmosphere · Burn · Pulse                  │
   │  Character: Velvet / Onyx / Chrome          │
   │  Day/Night · Output · Dry/Wet · preset       │
   └────────────────────────────────────────────┘
            │
            ▼
   ┌────────────────────────────────────────────┐
   │  MacroMapping + CharacterPreset             │
   │  mode snapshot + macro offsets + smoothing  │
   └────────────────────────────────────────────┘
            │
            ▼
   Internal DSP parameter targets
```

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
   │  Saturation             │   clean-room Jiles-Atherton + HF loss, 4x oversampled
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
   │  (DSP/ConvReverb)       │   stereo-true, Pulse-modulated wet path
   └─────────────────────────┘
            │
            ▼
   ┌─────────────────────────┐
   │  Day/Night brightness   │
   │  + dry/wet + output     │
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

Modules are no longer top-level product concepts in the UI. The fixed chain is implementation detail behind Atmosphere, Burn, Pulse, and the Character switch. DSP modules should expose raw, stable, smoothed internal parameters; `Source/Params/MacroMapping.cpp` decides how the user-facing controls reach them.

### `Source/DSP/`

Pure DSP. No JUCE GUI dependencies, no parameter system dependencies — these modules take raw values (frequencies, gains, mix amounts) and process audio. Parameter mapping happens one layer up.

- **`TiltEQ.cpp/h`** — Low-shelf + high-shelf tied to a single tilt parameter. Pre-saturation only — purpose is to *steer* saturation behaviour, not to be a user-facing EQ.
- **`Saturation.cpp/h`** — Clean-room Jiles-Atherton tape hysteresis from Jiles/Atherton 1986 and Chowdhury DAFx 2019, wrapped with drive-coupled dynamic HF loss, fixed 8kHz post rolloff, DC blocking, and 4x oversampling. Burn drives the input push into the model; Character mode changes the curve/emphasis around it later. Reference: 2026-05-25 clean-room saturation decision and `docs/research/saturation.md`.
- **`Chorus.cpp/h`** — Multi-voice modulated delay line, BBD-flavoured if licensing/implementation permits (see Q-CHOR-1-LIC). True stereo: L and R have decorrelated LFO phases for width. Pulse is the primary user-facing motion control.
- **`Filter.cpp/h`** — State-variable filter (TPT topology), low-pass primary. Envelope follower on input signal modulates cutoff. Slow attack/release on the follower — this is the "duck" feel, not a sharp sidechain.
- **`Delay.cpp/h`** — Stereo delay lines with independent L/R times for ping-pong. BPM sync via host transport. Feedback path goes through a one-pole low-pass (filtered feedback — repeats get darker, not brighter). Reference: standard delay design, *DAFX* Chapter 2.
- **`ConvReverb.cpp/h`** — Wraps `juce::dsp::Convolution`. Loads IRs from `Resources/IRs/`. Atmosphere drives wet level; Character mode influences IR character per Q-CHAR-2. Pulse drives wet-path modulation to break up static convolution.

### `Source/Params/`

- **`ParameterLayout.cpp/h`** — Defines the `AudioProcessorValueTreeState` with Atmosphere, Burn, Pulse, Character mode, Day/Night, Output, Dry/Wet, and any decided stereo width control. These are what the DAW sees.
- **`MacroMapping.cpp/h`** — The taste-encoding layer. Each macro is a function from `[0, 1]` to a vector of internal DSP parameter offsets inside the active Character mode. Curves are non-linear and hand-designed per macro. *This is where half the product lives.* (See `docs/decisions.md` for design notes per macro.)
- **`PresetManager.cpp/h`** — JSON preset format, versioned. Factory bank loaded from `Resources/Presets/` at startup. User presets saved to platform-standard location (`~/Library/Audio/Presets/<vendor>/<plugin>/`).

## Macro and Character switch architecture

The Character switch provides the baseline; macros modulate within that baseline. In other words, Burn at 0.7 in Velvet and Burn at 0.7 in Onyx are the same user gesture but intentionally different internal parameter values.

### Atmosphere mapping

Atmosphere is a meta-macro. It should not own a single DSP module; it raises the curated "more atmosphere" baseline across multiple targets.

| Target | Proposed mapping | Curve status |
| --- | --- | --- |
| Burn baseline | Adds positive offset into Burn's effective value | TBD in Q-MAC-3 |
| Pulse baseline | Adds positive offset into Pulse's effective value | TBD in Q-MAC-3 |
| Reverb wet | Near-zero at 0, obviously wet at 1 | TBD in Q-MAC-3 |
| Delay send / feedback | Possible subtle lift if later listening proves it belongs | Not locked; surface before implementation |

### Burn mapping

Burn is the density axis. It replaces the old Character macro name but narrows its scope to saturation-adjacent behaviour.

| Target | Proposed mapping | Curve status |
| --- | --- | --- |
| Saturation drive | Primary Burn target | Q-SAT-2, renamed to Burn drive curve |
| Dynamic LPF drive coupling | Follows saturation drive, with mode-specific range | Q-SAT-2 + CharacterPreset |
| Post-saturation low-mid emphasis | Positive lift as Burn increases | Hand-tuned under Q-MAC-1 |
| Chorus depth | Only if Q-MAC-4 resolves yes | Open |

### Pulse mapping

Pulse is the motion axis.

| Target | Proposed mapping | Curve status |
| --- | --- | --- |
| Chorus rate | Mode-specific rate range, increased by Pulse | Hand-tuned under Q-MAC-1 |
| Chorus depth | Primary depth movement target | Hand-tuned under Q-MAC-1 |
| Delay modulation depth | Increases life in repeats | Hand-tuned under Q-MAC-1 |
| Convolution wet-path modulation | Drives the "alive" trick from Q-IR-4 | Q-IR-4 + Q-MAC-1 |

### CharacterPreset data structure

Proposed implementation shape for the later C++ PR:

```cpp
enum class CharacterMode
{
    Velvet,
    Onyx,
    Chrome
};

struct CharacterPreset
{
    CharacterMode mode;

    SaturationTargets saturation;
    PostSaturationToneTargets postSaturationTone;
    ChorusTargets chorus;
    FilterTargets filter;
    DelayTargets delay;
    ReverbTargets reverb;
    BrightnessTargets brightness;
};
```

`CharacterPreset` holds per-mode baselines and legal ranges for every internal parameter affected by the Character switch. `MacroMapping` then computes effective targets like:

```cpp
effective = modeBaseline + macroOffset(mode, macroValue);
```

The exact combination rules are part of the future implementation PR and must be reviewed carefully because Atmosphere intentionally overlaps Burn, Pulse, and reverb wet (Q-MAC-3).

### Switching behaviour

Switching Velvet / Onyx / Chrome applies a different parameter snapshot. Whether that snapshot is instant, short-crossfaded, or long-morphed is open in Q-CHAR-1. Whether Character mode swaps the available reverb IR set or biases default IR choice is open in Q-CHAR-2.

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

60-preset factory bank as versioned JSON. The old source-typed split (`Vocals/`, `Synths/`, `Drums/`) is superseded by Q-PRE-1-REVISED. Proposed themed directories:
- `Spaces/` — heavy reverb-led presets
- `Movement/` — heavy Pulse-led presets
- `Heat/` — heavy Burn-led presets
- `Atmospheres/` — full-stack maximalist presets
- `Subtle/` — light-touch presets across all macros

Each preset includes a `schema_version` field. v2 presets will add neural voicing parameters; v1 hosts loading v2 presets ignore unknown fields; v2 hosts loading v1 presets use defaults.

## Preset JSON schema (v1)

```json
{
  "schema_version": 1,
  "name": "Cold Cathedral",
  "category": "Spaces",
  "author": "Factory",
  "description": "Evocative factory preset for a large, cold nocturnal space.",
  "macros": {
    "atmosphere": 0.78,
    "burn": 0.42,
    "pulse": 0.31,
    "mix": 1.0,
    "output_db": 0.0
  },
  "settings": {
    "character_mode": "Chrome",
    "day_night": "Night",
    "sync_division": "dotted_eighth"
  }
}
```

Macros are 0–1. The mapping from macro value to internal DSP parameters lives in code (`MacroMapping.cpp`), not in the preset. Character mode stores a discrete mode, not raw DSP values. This means **changes to the macro mapping curves or Character snapshots change every preset's sound**. That's intentional pre-v1 (it's how taste-tuning works) and locked-down at v1 ship (changing it post-ship invalidates customer presets).

## Latency

- TiltEQ: 0
- Saturation: 0 with the Stage 2 4x zero-latency oversampling choice
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

Preset and Character switches are the trickiest: when a preset changes macros or the Character switch applies a new snapshot, dozens of internal params may move at once. The change has to be smooth (no zipper noise, no clicks). Handled by parameter smoothing in each DSP module — every internal parameter has a `juce::SmoothedValue` wrapper with a few-ms ramp time, plus any Character crossfade/morph behaviour resolved in Q-CHAR-1.

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
