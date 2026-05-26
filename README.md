# Florescence

Florescence by Solace is a stereo atmosphere processor for nocturnal electronic music.
It is currently a Mac JUCE plugin project targeting AU, VST3, and Standalone builds.

The current codebase has Stage 1 plumbing, Stage 2 Tilt EQ + Saturation, and the
Stage 3 Chorus + Filter modules implemented and covered by Catch2 tests. The
remaining staged work is tracked in `docs/roadmap.md`.

## Current Status

- Canonical docs live in `docs/`.
- Open stop points live in `docs/open_questions.md`.
- Stage status and remaining work live in `docs/roadmap.md`.
- Bundle ID: `com.solace.florescence`.
- Framework: JUCE 8.0.12 via `external/JUCE`.
- Private beta-customer notes should use local `docs/customers.md`, which is ignored by git.
- IRs, presets, graphics, licensing, and final GUI assets are still future-stage work.

## Build

Configure and build all plugin formats:

```sh
cmake -S . -B build-juce8 -G "Unix Makefiles"
cmake --build build-juce8 --target CharacterFX_All
```

Run tests:

```sh
cmake --build build-juce8 --target CharacterFX_Tests CharacterFX_PluginSmokeTests
ctest --test-dir build-juce8 --output-on-failure
```

The local build writes artifacts under `build-juce8/CharacterFX_artefacts/`.

Validate local artifacts:

```sh
auval -v aufx flrs Slce
/Applications/pluginval.app/Contents/MacOS/pluginval --validate build-juce8/CharacterFX_artefacts/VST3/Florescence.vst3 --strictness-level 5 --skip-gui-tests --timeout-ms 30000
```

## Agent Entry Point

Read `AGENTS.md` first, then follow the read order listed there.
