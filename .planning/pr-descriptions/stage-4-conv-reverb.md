# Implement Stage 4 convolution reverb module

## What changed

This PR adds the Stage 4 `ConvReverb` DSP module and wires it into the fixed chain after `Delay`. It implements the convolution wrapper, wet/dry mixing, fixed subtle pre-convolution modulation, and deterministic tests without adding the licensed IR library yet.

## Why

Convolution reverb is the cinematic-space stage in Florescence. The module needs to exist before the separate IR library PR can populate `Resources/IRs/` and before Stage 5 maps Atmosphere, Pulse, and Character mode onto reverb behaviour.

## Considered and rejected

- Algorithmic FDN reverb remains rejected for v1; this PR uses `juce::dsp::Convolution`.
- Real IR assets are not added here because Q-IR-1 requires licence verification before any IR enters `Resources/IRs/`.
- User-facing IR controls are deferred because Q-CHAR-2 keeps IR selection behind Atmosphere and Character biasing for v1.

## Testing

- Add deterministic Catch2 coverage for silence, impulse loading, wet/dry mix, fixed pre-convolution modulation bounds, reset behaviour, and bounded processing.
- Run `cmake --build build-juce8`.
- Run `ctest --test-dir build-juce8 --output-on-failure`.

## Follow-ups

- `feature/initial-ir-library` will add licensed/captured IR files and metadata.
- Stage 5 will map Atmosphere, Pulse, and Character mode onto wet level, modulation depth scaling if approved, and IR selection bias.
