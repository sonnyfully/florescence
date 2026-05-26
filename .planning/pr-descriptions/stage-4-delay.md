# Implement Stage 4 stereo delay module

## What changed

This PR adds the Stage 4 `Delay` DSP module and wires it into the fixed chain after `Filter`, before convolution reverb lands in the next PR. It also adds delay research notes, a timing notebook, and deterministic tests.

## Why

Delay is the first Stage 4 time-based stage. It provides tempo-synced repeats, darker feedback, and internal stereo/ping-pong topology support while keeping the front-panel surface deferred to Stage 5 macro mapping.

## Considered and rejected

- Free-time delay controls are rejected for v1 because Stage 4 resolved tempo-synced character-delay divisions.
- `1/2` and `1/1` divisions are rejected because they are too long for Florescence's character-delay role.
- Exposing ping-pong on the main GUI is rejected in this PR because Q-DEL-2 makes it internal/automatable only, with Stage 5 deciding the control surface.

## Testing

- `cmake --build build-juce8`
- `ctest --test-dir build-juce8 --output-on-failure`

## Follow-ups

- `feature/conv-reverb` will add the convolution reverb stage after Delay.
- Stage 5 will decide how Character mode, Atmosphere, or settings control delay topology and division selection.
