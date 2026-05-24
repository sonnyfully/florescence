# ir-sourcing.md

Research notes for Q-IR-1: IR sourcing strategy.

Q-IR-1 was resolved on 2026-05-24 in `docs/decisions.md`: use a mix of self-captured IRs and free libraries.

Do not commit IR files to `Resources/IRs/` until the specific source and redistribution licence have been re-verified in Stage 4.

## Selected Strategy

- Self-capture real spaces with sine sweep and/or balloon pop.
- Use free-library candidates where redistribution terms allow it.
- Candidate libraries: Voxengo and Samplicity Bricasti M7.

## Rejected

- Paid commercial libraries: rejected for cost at v1.
- Purely synthetic IRs: rejected as the core strategy because they lose authenticity.

## Stage 4 Licence Check

Before any IR enters the repo, record:

- Source URL
- Licence text or archived copy
- Redistribution allowance
- Required attribution
- Any processing performed
