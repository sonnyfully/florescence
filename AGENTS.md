# AGENTS.md

Instructions for AI coding agents working in this repo. Read this first, every session.

## What this project is

A stereo character processor plugin (AU + VST3, Mac) aimed at the Kissland / After Hours sonic palette. See `docs/CONTEXT.md` for the full briefing before doing any work.

## Read order for a fresh session

1. This file (`AGENTS.md`) — conventions
2. `docs/CONTEXT.md` — product thesis, aesthetic, scope, what's been ruled out
3. `docs/ARCHITECTURE.md` — module contract, signal flow, repo layout
4. `docs/roadmap.md` — current stage, goal of that stage, end-state checklist, what's in scope and not
5. **`docs/open_questions.md` — STOP POINTS. The agent must check this every session and before starting any new module.**
6. `docs/decisions.md` — what's been decided and why (check before proposing alternatives)
7. Any `docs/research/<module>.md` relevant to the current branch
8. Current branch name and open PR — that's where work is happening

## The stop-point protocol — NON-NEGOTIABLE

`docs/open_questions.md` lists points where the agent **must not guess**. When the agent's work approaches a stop point:

1. **Stop.** Do not commit a placeholder, do not pick a "reasonable default" silently.
2. Surface the question in the PR description (or current message) with full context: which question (by Q-ID), why it's blocking, what the options are, what the agent's tentative lean is.
3. **Wait** for an explicit human decision.
4. Once decided, move the resolved entry from `open_questions.md` to `decisions.md` with date and rationale. Update the affected code to reference the decision.

If a stop point is reached **before** its expected week, surface it anyway. The schedule is wrong, not the question.

If the agent thinks a *new* ambiguity has appeared mid-work that meets the bar for human input, it adds an entry to `open_questions.md` in the same format and surfaces it. The bar: "if I pick wrong here, the customer notices" or "this is taste, not engineering."

## Git workflow — non-negotiable

- `main` is always working. Never commit directly to main.
- One feature branch per sub-problem. Prefixes: `feature/`, `fix/`, `research/`, `docs/`, `refactor/`. Examples: `feature/saturation-module`, `fix/chorus-stereo-phase`, `research/ir-sourcing`.
- Write the PR description **before** starting work on the branch. If you can't describe the PR in three sentences, the scope isn't clear enough yet — stop and clarify.
- One concern per PR. If you're touching DSP and licensing in the same branch, split it.
- PR description includes: what changed, why, what was considered and rejected, how it was tested, any follow-ups created (with Q-IDs if applicable).
- Squash-merge to main. Keep history clean.
- Commit messages: imperative mood, present tense. `add saturation module` not `added saturation module`.

## Working conventions

- **Ask before scope creep.** If a task reveals a deeper problem, surface as a separate issue or PR. Don't silently expand.
- **Plot DSP in notebooks.** Every DSP function gets a plot against a known input (sine, sweep, impulse, real audio) before it's trusted. Notebooks live in `notebooks/`.
- **No magic numbers.** Sample rates, FFT sizes, smoothing times, default values — all in a config header with comments on *why*.
- **Decisions go in `docs/decisions.md`** when they're non-obvious. Append-only, dated.
- **Open questions go in `docs/open_questions.md`** when they require human judgment. See protocol above.
- **Research synthesis goes in `docs/research/`** as one file per module or topic.
- **Don't pick new dependencies without raising it.** The stack is JUCE + chowdsp_utils + Catch2. Anything else needs a `decisions.md` entry first.

## Code style

- C++20. Type-safe, RAII, no naked `new`/`delete`.
- Audio thread: zero allocations, zero locks, zero file I/O, zero `std::cout`. Use `juce::SmoothedValue` for parameter changes, lock-free FIFOs for thread crossings.
- Each DSP module implements `FXModule` interface (see `docs/ARCHITECTURE.md`).
- One class per file. Header in `.h`, implementation in `.cpp`. Both same module.
- `clang-format` config in repo. Run before commit.
- Tests for any DSP function with deterministic output. Catch2, in `Tests/`.
- Notebooks (Python) for exploration only. Once a pattern stabilises it moves into a `Source/DSP/` module. Notebooks import from `Source/` via pybind11 or via plotting C++ output, never the other way.

## What I want from you, the agent

- **Push back when I'm wrong.** If a request contradicts `decisions.md` or seems technically off, say so. Don't just execute.
- **Surface tradeoffs explicitly.** "I'm going to do X" should usually be "I'm going to do X because Y, alternative was Z, rejected because W."
- **Flag when you're guessing.** If you don't know the right oversampling factor, IR length, smoothing time, or aesthetic call, say "I'm guessing here — this should probably go in `open_questions.md`" rather than committing a guess.
- **Don't vibe-code DSP.** This project's whole point is real work with real backing. If you're about to write a feature extractor, saturator, or filter without a reference paper, library function, or documented technique, stop and find one first.
- **Hit stop points proactively.** Don't wait to be asked. If you see a Q-ID in upcoming work, surface it in the current PR before starting.

## What not to do

- No commits directly to main.
- No `pip install` / `brew install` / new CMake dependencies without flagging.
- No expanding PR scope mid-branch.
- No deleting things from `decisions.md` or `open_questions.md` — only append, move, or strike through.
- No dumping unsynthesized research into the repo.
- No "I'll fix it later" TODOs without a corresponding issue or Q-ID in `open_questions.md`.
- No silent defaults on stop points. Every Q-ID hit must surface explicitly.
- No audio-thread violations (allocations, locks, I/O). Use the proper JUCE patterns.
