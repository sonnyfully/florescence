# Stage 1 Plumbing

## What changed

This PR starts the Stage 1 plumbing branch for the throwaway JUCE plugin: CMake/JUCE setup, AU/VST3/Standalone targets, a minimal gain + one-pole low-pass processor, default GUI controls, and a trivial Catch2 test target.

## Why

Stage 1 exists to prove the build -> load -> iterate loop before any product DSP lands. The bundle identity resolved in Stage 0 (`com.solace.florescence`, vendor `Solace`, plugin `Florescence`) is the metadata this branch will cement once the project file is added.

## Stage 0 gate status

Stage 0 is not fully complete yet. The unresolved items in `docs/decisions.md` are external/taste deliverables: registered `.com` domain, pinned reference playlist, private beta customers, Figma link, and Lemon Squeezy account. Because `docs/roadmap.md` says stages are sequential, this PR should not be merged until those are resolved or the roadmap is explicitly amended.

## Considered and rejected

- Starting real DSP now: rejected because Stage 1 is only plumbing, and Stage 2 owns the first product DSP.
- Adding chowdsp_utils now: rejected because Stage 1 does not need it and the roadmap defers it to Stage 2.
- Custom UI work now: rejected because the Stage 1 GUI is deliberately throwaway.

## Testing

Planned:

- Configure and build local CMake project on Apple Silicon.
- Run trivial Catch2 test target.
- Run `auval` for the AU once the plugin builds.
- Run VST3 validator if the JUCE checkout provides the validator or a local SDK validator is available.

Current:

- Catch2 test-only configure path added as `-DCHARACTERFX_BUILD_PLUGIN=OFF`.
- Q-BUILD-1 resolved by moving the JUCE submodule to current stable `8.0.12`.
- `cmake --build build-juce8 --target CharacterFX_All` builds AU, VST3, and Standalone artifacts.
- `ctest --test-dir build-juce8 --output-on-failure` passes.
- `auval -v aufx flrs Slce` passes when run outside the workspace sandbox.
- `pluginval --validate build-juce8/CharacterFX_artefacts/VST3/Florescence.vst3 --strictness-level 5 --skip-gui-tests --timeout-ms 30000` passes. The bundled Steinberg VST3 validator path was not available, so pluginval skipped that optional sub-check.
- Ableton Live 11.3.43 scanner log confirms the VST3 is discovered as `Florescence`, vendor `Solace`, from `~/Library/Audio/Plug-Ins/VST3/Florescence.vst3`.
- Human DAW check: Sonny confirmed the plugin loads in Ableton.

## Follow-ups

- Resolve remaining Stage 0 external deliverables before merging this PR.
