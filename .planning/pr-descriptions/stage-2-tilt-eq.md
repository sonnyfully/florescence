# Stage 2 Tilt EQ

## What changed

This PR starts Stage 2 by adding the shared `FXModule` interface and the first product DSP module: stereo Tilt EQ. The Tilt EQ provides a single pre-saturation tone control around a fixed pivot and replaces the throwaway low-pass stage in the minimal processor chain.

## Why

Stage 2 exists to move from plumbing into real DSP. Tilt EQ is the safest first slice because it establishes the module contract and test pattern before the more taste-heavy saturation work starts.

## Considered and rejected

- Combining Tilt EQ and Saturation in one PR: rejected because `docs/roadmap.md` calls for two Stage 2 PRs and saturation has multiple hard stop points.
- Tuning macro behaviour now: rejected because the macro mapping layer belongs to Stage 5.
- Expanding the GUI design: rejected because the current controls are still intentionally throwaway.

## Testing

Planned:

- Add deterministic unit tests for Tilt EQ frequency response and stereo coherence.
- Build the plugin targets locally.
- Run the Catch2 test target.
- Manually confirm the existing front-panel control maps directly to Tilt amount.

## Stop points

No Tilt EQ-specific stop points are listed in `docs/open_questions.md`. The saturation branch that follows must resolve or surface Q-SAT-1, Q-SAT-4, and Q-SAT-5, while explicitly deferring Q-SAT-2 and Q-SAT-3 per the roadmap.
