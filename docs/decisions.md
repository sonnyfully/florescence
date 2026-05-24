# decisions.md

Append-only decision log. Resolved entries move here from `docs/open_questions.md` with date, rationale, and affected files.

## 2026-05-24 — Repository pre-flight scaffold

- Decision: initialize the repository in Stage 0 with documentation and directory scaffolding only.
- Rationale: Stage 0 explicitly says no code, and Q-NAME-1/Q-NAME-2 block bundle metadata and CMake/JUCE plumbing.
- Affected files: `README.md`, `.gitignore`, `docs/`, `Resources/`, `Source/`, `Tests/`, `notebooks/`, `.github/`.

## 2026-05-24 — Stage 0 identity decisions

Resolved from `docs/open_questions.md`: Q-NAME-1 and Q-NAME-2.

- Plugin name: Florescence
- Pronunciation: FLOR-ess-ence
- Vendor / shop name: Solace
- Bundle ID: `com.solace.florescence`
- AU manufacturer code: `Slce`
- AU subtype code: `flrs`
- Considered and rejected for plugin name: Penumbra, Catharsis, Apotheosis, Bloom, Empyrean, Efflorescence
- Considered and rejected for vendor name: Threshold, Parallax, Sidereal, Halcyon, Strata
- Rationale: Florescence keeps the bloom/light/decay association without the pronunciation cost of Efflorescence; Solace fits the nocturnal atmospheric palette while staying emotionally legible.
- Risks: mild confusion with "fluorescence"; SEO collision with Solace Corp, though it is enterprise software in a different industry.
- Affected files: future `CMakeLists.txt`, plugin binary metadata, AU/VST3 identifiers, marketing, docs.

Q-NAME-2 is locked when Stage 1 writes `CMakeLists.txt`.

## 2026-05-24 — Stage 0 IR sourcing strategy

Resolved from `docs/open_questions.md`: Q-IR-1.

- Decision: use a mix of self-captured IRs and free libraries.
- Self-capture sources: real spaces captured with sine sweep and/or balloon pop.
- Free-library candidates: Voxengo and Samplicity Bricasti M7.
- Considered and rejected: paid commercial libraries, due to cost; purely synthetic IRs, because they lose too much authenticity as the core library strategy.
- Rationale: the mixed strategy gives the product real-world texture while keeping Stage 4 practical and inexpensive.
- Risk: licence terms on "free" libraries can change or may not allow redistribution. Re-verify current licence terms in Stage 4 before any IR enters `Resources/IRs/`.
- Affected files: `Resources/IRs/`, `docs/research/ir-sourcing.md`, future reverb packaging/licensing notes.

## Unresolved Stage 0 Stop Points

These remain before Stage 0 is complete:

- Register plugin `.com` domain for Florescence.
- Pin 5 reference tracks in a playlist and write `docs/aesthetic/reference-tracks.md`.
- Write 5 named beta customers to local ignored `docs/customers.md`.
- Start Figma file and record the link here.
- Create Lemon Squeezy account.
