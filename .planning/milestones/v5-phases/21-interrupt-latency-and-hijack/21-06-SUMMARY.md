---
phase: 21-interrupt-latency-and-hijack
plan: "06"
subsystem: testing
tags: [native-state, interrupt-entry, audio-history, state-fork]
requires:
  - phase: 21-04
    provides: Shared resumable BRK/IRQ/NMI production entry and native state fields
  - phase: 21-05
    provides: Source-reachable entry-state matrix and reset/rejection coverage
provides:
  - Every-stage native replay for BRK, IRQ, and NMI with exact production bus traces
  - Nonzero synthesis-history, future frame/audio, and interleaved-instance equality checks
  - Entry-stage continuation in the retained multi-board state-fork test executable
affects: [native-state, INT-02, KEEP-01, libretro, hosted-acceptance]
actuals:
  tokens: 11478
  tasks: 2
  commits: 2
commits: 2
plan_head_before: 3d38eafd4e8ddd49d14e026a2f7286b3f84d2167
plan_head_after: 6260537b1581936f6f7758eeea15186b9b265dd3
tech-stack:
  added: []
  patterns:
    - Observe and delegate production bus actions in test-only wrappers
    - Produce legal entry cutpoints with production stages and restore only through public state APIs
key-files:
  created:
    - tests/core/test_interrupt_state.c
  modified:
    - tests/interrupt_fixture.h
    - tests/core/test_state_fork.c
    - tests/CMakeLists.txt
    - tests/accuracy/baseline-metadata.txt
key-decisions:
  - "Keep constructed-origin cutpoint evidence separate from ordinary cpu_step initiation evidence."
  - "Leave runtime code unchanged because Plan 21-04 already supplies the exercised entry behavior."
requirements-completed: []
coverage:
  - id: D1
    description: "BRK, IRQ, and NMI entry cutpoints restore through native state APIs and resume with exact production bus traces."
    requirement: INT-02
    verification:
      - kind: unit
        ref: "core.interrupt_state, core.interrupt_entry"
        status: pass
      - kind: integration
        ref: "cmake --workflow --preset ci (693/693 tests)"
        status: pass
    human_judgment: true
    rationale: "Phase-wide INT-02 acceptance still includes hosted and all-lane checks scheduled for later plans."
  - id: D2
    description: "A retained state-fork entry case loads a genuine IRQ C6 cut and matches future frame, audio, marker, and state output."
    requirement: KEEP-01
    verification:
      - kind: integration
        ref: "core.state_fork.entry and retained 14-test state-fork/runahead matrix"
        status: pass
      - kind: integration
        ref: "cmake --workflow --preset ci (693/693 tests)"
        status: pass
    human_judgment: true
    rationale: "KEEP-01 remains pending confirmation across hosted CI lanes."
duration: 29m
completed: 2026-10-29
status: complete
---

# Phase 21 Plan 06: Native interrupt-entry replay summary

**Every resumable BRK, IRQ, and NMI entry stage now has exact native save/load replay evidence, including nonzero APU history and future frame/audio equality.**

## Performance

- **Duration:** About 29 minutes.
- **Started:** 2026-10-29T12:29:11Z (the prior STATE.md session timestamp).
- **Completed:** 2026-10-29.
- **Tasks:** 2/2.
- **Files modified:** 5 task deliverables.

## Accomplishments

- Reached each resumable next-action cut through production entry actions: BRK C2–C7, IRQ C1–C7, and NMI C1–C7. BRK opcode fetches are observed production bus reads; test-only wrappers delegate every recorded access to the production bus implementation.
- Kept explicitly constructed cutpoint labels separate from ordinary `cpu_step` initiation proofs for BRK, an APU frame IRQ, and NMI. Restored instances use the public native loader and ordinary `cpu_step` continuation.
- Compared remaining bus kind/address/value and cycle timestamps, PC/P/S/stack, interrupt consumption history, canonical save bytes, guest markers, native pixels, valid PCM, frame metadata, and APU transition history.
- Exercised pre-selection, post-selection, and between-vector-read NMI edges. Omitted-history, skipped-load, and wrong-origin controls diverged at their expected bus address while the clean sibling passed.
- Verified nonzero synthesis/filter/sample history and interleaved IRQ/NMI instances with distinct entry and audio histories. Retained all board-family state forks and added the IRQ C6 entry fork.
- Updated only the measured AccuracyCoin build-input digest. Campaign counts remain 119 pass / 27 fail / 5 draw over 146 terminal rows; its report digest and tracked reports are unchanged.

## Verification

- Focused entry tests (`core.interrupt_state`, `core.state_fork.entry`, `core.interrupt_entry`): **3/3 passed**.
- Retained state-fork/runahead matrix (`core.state_fork.*`, `core.nmi_state`, `core.state_runahead`): **14/14 passed**.
- AccuracyCoin activation and its campaign fixture: **2/2 passed**.
- `cmake --workflow --preset ci`: **693/693 tests passed**; library, runner, and libretro packages were generated.
- `git diff --check` passed. Commit hooks passed for both task commits.

## Task Commits

1. **Task 1: Replay every production entry cutpoint through native APIs** — `955e3ec` (`test(21-06): cover native interrupt entry cutpoints`).
2. **Task 2: Extend cutpoint equality through synthesis, instances, and frames** — `6260537` (`test(21-06): register entry replay in state-fork suite`).

## Files Created/Modified

- `tests/core/test_interrupt_state.c` — production-action cutpoint sweep, ordinary initiation proof, exact continuation traces, audio/frame comparisons, interleaving, and restore controls.
- `tests/interrupt_fixture.h` — reusable owned NROM, reachable audio warm-up, labeled entry constructor, production-action advancement, and public-save cutpoint helper.
- `tests/core/test_state_fork.c` — IRQ C6 public-load continuation in the existing state-fork runner.
- `tests/CMakeLists.txt` — registers the native entry and state-fork entry tests without removing board-family cases.
- `tests/accuracy/baseline-metadata.txt` — refreshed `build_inputs_sha256` only.

## Decisions Made

- Constructed origins label the source history for exhaustive internal cutpoints; they do not replace the separate normal `cpu_step` initiation evidence.
- No runtime change was needed: the tested behavior was already implemented in Plan 21-04, and no production defect reproduced.

## Deviations from Plan

None. The work remained within native replay tests, the shared interrupt fixture, and CMake registration.

## Issues Encountered

An early test setup had an incorrect expected BRK return PC and did not leave the pulse channel active. The fixture expectations and warm-up were corrected; subsequent focused, matrix, activation, and full-workflow checks passed without runtime edits.

## User Setup Required

None.

## Next Phase Readiness

Plan 21-06 is complete. INT-02 and KEEP-01 remain pending hosted and all-lane acceptance; this plan ran local CI only.

## Self-Check: PASSED

- `21-06-SUMMARY.md` exists in the phase directory.
- Task commits `955e3ec` and `6260537` are ancestors of HEAD.
- The persisted plan ledger measures two task commits from `3d38eafd4e8ddd49d14e026a2f7286b3f84d2167` through `6260537b1581936f6f7758eeea15186b9b265dd3`.
- Full CI passed 693/693; `git diff --check` passed.
- The pre-existing `.planning/config.json` modification remains unstaged.

---
*Phase: 21-interrupt-latency-and-hijack*
*Completed: 2026-10-29*
