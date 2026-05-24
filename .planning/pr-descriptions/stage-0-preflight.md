# Stage 0 Pre-flight Scaffold

## What changed

Created the repository shell, moved docs into their canonical paths, added decision/open-question tracking, and added empty resource/source/test directories for the architecture described in `docs/ARCHITECTURE.md`.

## Why

The repo needed to be normalized before Stage 0 decisions and later Stage 1 JUCE plumbing. No plugin code or bundle metadata is included because Q-NAME-1 and Q-NAME-2 are no-default stop points.

## Considered and rejected

- Adding CMake/JUCE boilerplate now: rejected because Stage 0 still needs to finish before Stage 1 begins.
- Adding placeholder IRs: rejected because Q-IR-1 requires an explicit source and licence decision.

## Tested

Documentation-only scaffold. Verified file layout and git initialization.

## Follow-ups

- Register the Florescence `.com` domain.
- Fill `docs/aesthetic/reference-tracks.md` with 5 pinned tracks.
- Create local ignored `docs/customers.md` from `docs/customers.template.md`.
- Start the Figma file and record its link in `docs/decisions.md`.
- Create the Lemon Squeezy account.
