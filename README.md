# Character FX

Florescence by Solace: a stereo character processor plugin for the Kissland / After Hours palette.

This repo is currently on the **Stage 1 — Plumbing** branch. The branch builds a throwaway JUCE plugin with gain + low-pass controls, but should not merge until the remaining Stage 0 external deliverables are resolved or the roadmap is explicitly amended.

## Current Status

- Canonical docs live in `docs/`.
- Open stop points live in `docs/open_questions.md`.
- Stage status lives in `docs/roadmap.md`.
- Bundle ID: `com.solace.florescence`.
- Framework: JUCE 8.0.12 via `external/JUCE`.
- Private beta-customer notes should use local `docs/customers.md`, which is ignored by git.

## Build

Configure and build all plugin formats:

```sh
cmake -S . -B build -G "Unix Makefiles"
cmake --build build --target CharacterFX_All
```

Run tests:

```sh
cmake --build build --target CharacterFX_Tests CharacterFX_PluginSmokeTests
ctest --test-dir build --output-on-failure
```

The local build writes artifacts under `build/CharacterFX_artefacts/`.

Validate local artifacts:

```sh
auval -v aufx flrs Slce
/Applications/pluginval.app/Contents/MacOS/pluginval --validate build/CharacterFX_artefacts/VST3/Florescence.vst3 --strictness-level 5 --skip-gui-tests --timeout-ms 30000
```

## Agent Entry Point

Read `AGENTS.md` first, then follow the read order listed there.
