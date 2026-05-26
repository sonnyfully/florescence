# Resolve Stage 4 delay, reverb, and licensing decisions

## What changed

This PR resolves the six Stage 4 stop points that block Delay, Convolution Reverb, IR curation, and Apple Developer enrolment planning: Q-DEL-1, Q-DEL-2, Q-IR-2, Q-CHAR-2, Q-IR-4, and Q-LIC-3.

## Why

Stage 4 implementation should not bake taste or workflow calls into C++ without explicit decisions. These entries lock the delay division set, ping-pong topology, initial IR library shape, Character-mode reverb behaviour, wet-path modulation strategy, and signing/notarization timing before the DSP PRs start.

## Considered and rejected

- Long `1/2` and `1/1` delay divisions were rejected because Florescence needs a character delay, not a general-purpose long echo.
- Per-mode exclusive IR lists were rejected because they make Character modes feel locked off from useful spaces and inflate the resource burden.
- Post-convolution modulation and exposed wet modulation controls were rejected for v1 because the desired behaviour is subtle movement, not an audible modulation effect.

## Testing

Docs-only PR. Verified by reading the updated decision log and open-question entries for consistency.

## Follow-ups

- Implement `Delay` on `feature/delay` using the resolved division set and internal topology enum.
- Implement `ConvReverb` on `feature/conv-reverb` using the resolved IR-selection and wet-path modulation spec.
- Acquire/licence the initial 10 IRs on `feature/initial-ir-library`.
- Complete Apple Developer enrolment and document throwaway notarization by the end of Stage 4.
