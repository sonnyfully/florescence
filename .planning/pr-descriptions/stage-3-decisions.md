# Stage 3 Chorus Decisions

## What changed

This PR locks the Stage 3 chorus direction before implementation starts. It records the Q-CHOR-1-LIC, Q-CHOR-1, and Q-CHOR-2 resolutions, adds the chorus research note, and records the v1.x follow-up for a possible third chorus voice or density mode.

## Why

The roadmap blocks Chorus C++ work until the licensing and fidelity decisions are explicit. The product needs a Juno-style true-stereo modulation stage, but the closed-source commercial model means GPLv3 ChowDSP BBD code cannot be used without a commercial licence, which is out of scope for v1.

## Considered and rejected

- Using `chowdsp_dsp_utils` BBD/delay primitives: rejected because the relevant upstream module is GPLv3 and commercial licensing is not being pursued for v1.
- Using Airwindows-flavoured chorus code: rejected for this v1 baseline because the locked target is clean-room, Juno-style BBD-flavoured delay modulation rather than adopting another implementation's sound.
- Pure generic delay-line modulation with no BBD colour: rejected because Stage 3 needs bandwidth loss / HF softening to keep the chorus inside the Florescence palette.
- Adding 3 or 4 voices now: rejected because the locked baseline is 2 voices, Juno-style; density expansion is deferred to v1.x only if Stage 6 tuning proves it necessary.

## Testing

Documentation-only PR. Verified the affected docs are internally consistent and that the implementation work remains blocked to later branches:

- `feature/chorus-scaffolding`
- `feature/chorus-tests`
- `feature/chorus-implementation`

## Stop points

Resolved in `docs/decisions.md` on 2026-05-25:

- Q-CHOR-1-LIC: ChowDSP BBD / `chowdsp_dsp_utils` unavailable for v1.
- Q-CHOR-1: clean-room BBD-flavoured delay-line modulation.
- Q-CHOR-2: 2 voices, Juno-style, decorrelated L/R LFO phases.

## Follow-ups

- Create `feature/chorus-scaffolding` after this PR is merged.
- Create `feature/chorus-tests` after scaffolding is merged.
- Implement actual BBD-flavoured DSP later on `feature/chorus-implementation`.
