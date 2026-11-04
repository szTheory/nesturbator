---
phase: 21-interrupt-latency-and-hijack
plan: "05"
subsystem: testing
tags: [native-state, interrupt-entry, reset, generation]
requires:
  - phase: 21-04
    provides: Reachable serialized BRK/IRQ/NMI entry continuation and pre-APPLY validation
provides:
  - CRC-correct legal and invalid entry-state controls with mutation and future-run checks
  - Production-reachable active-entry reset coverage and six-family generation controls
  - Source-established serialized-entry invariant and transition matrix
affects: [native-state, phase-21-follow-up, hosted-acceptance]
actuals:
  tokens: 8772
  tasks: 2
  commits: 2
commits: 2
plan_head_before: 81a0f98c3d8b6785753eaaa93f001927f407617d
plan_head_after: d84477c96daef040125529a8ceac08b4ccac0a29
tech-stack:
  added: []
  patterns:
    - Build legal state siblings through production entry actions before repairing CRCs on invalid controls
key-files:
  created:
    - tests/state_test.h
    - .planning/phases/21-interrupt-latency-and-hijack/21-STATE-MATRIX.md
  modified:
    - tests/core/test_state_reject.c
    - tests/core/test_state_generation.c
key-decisions:
  - "Use only source-reachable entry histories as legal siblings for serialized-state rejection controls."
  - "Keep INT-02 and KEEP-01 pending until their hosted and all-lane phase acceptance is complete."
patterns-established:
  - "Create resumable-state fixtures through production transitions and compare the next run after round-trip."
requirements-completed: []
coverage:
  - id: D1
    description: "Serialized interrupt-entry reachability, refusal, reset, and generation behavior is covered without runtime changes."
    verification:
      - kind: unit
        ref: "core.state_reject, core.state_generation"
        status: pass
      - kind: integration
        ref: "cmake --workflow --preset ci (691/691 tests)"
        status: pass
    human_judgment: false
metrics:
  duration: 25m
  completed: 2026-10-29
  status: complete
---

# Phase 21 Plan 05: Reachable state rejection and reset lifecycle summary

**CRC-correct entry-state rejection controls and production-reachable reset cutpoints establish the state contract without changing runtime behavior.**

## Performance

- **Duration:** About 25 minutes.
- **Started:** 2026-10-29T12:03:38Z (the previous session timestamp).
- **Completed:** 2026-10-29.
- **Tasks:** 2/2.
- **Files modified:** 4 task deliverables.

## Accomplishments

- Added legal production-generated siblings and CRC-repaired invalid controls for entry origin, stage, vector, partial-byte, NMI history, and cross-field invariants. Refusals assert the full native instance, cartridge RAM, save generation, and next production run remain unchanged.
- Preserved the historical NMI latch/live-line disagreement cases and the CRC-correct format 1/2 error-precedence controls.
- Added reset checks for every active BRK, IRQ, and NMI stage, including selected IRQ/NMI vector pairs. Reset checks cover seven cycles, reset-vector execution, stack behavior, CPU markers, and the following ordinary instruction.
- Extended same-state and refused-load generation checks across mappers 0, 1, 2, 3, 4, and 7, without reducing existing battery-span coverage.
- Documented legal state transitions and matching invalid controls in `21-STATE-MATRIX.md`. No runtime source, public API, README, shared fixture, or CMake changes were needed.

## Verification

- Task 1 focused tests: `core.state_reject`, `core.state_generation`, `core.state`, and `state.format_pin` — **4/4 passed**.
- Task 2 focused tests: `core.state_generation`, `core.state_reject`, `fuzz.state`, `fuzz.state.regress`, and `core.nmi_state` — **5/5 passed**.
- `cmake --workflow --preset ci` — **691/691 tests passed**; library, runner, and libretro packages were generated.
- `scripts/hygiene.sh` and `git diff --check` passed.

## Task Commits

1. **Task 1: Exercise reachable and CRC-correct invalid entry combinations** — `9d161b1` (`test(21-05): exercise reachable state rejection matrix`).
2. **Task 2: Check partial-entry reset and compatibility lifecycle** — `d84477c` (`test(21-05): cover entry reset and generation lifecycle`).

## Decisions Made

- Use production transitions for legal serialized-state histories; do not use fabricated partial pushes or seeded unreachable histories as proof of acceptance.
- Keep INT-02 and KEEP-01 open until hosted and all-lane phase acceptance is complete.
- Treat this plan as test and documentation coverage for the Plan 21-04 contract; no public behavior change was intended.

## Deviations from Plan

None. Test-fixture setup issues were corrected within the planned test files; no runtime defect was found.

## Issues Encountered

The test setup initially used an invalid CNROM image without CHR ROM and attempted NMI transitions while reset left the PPU pin path inactive. The image builder now creates a valid CNROM image, and NMI history uses pin transitions sampled through production bus actions. The focused tests and full workflow pass with these source-reachable fixtures.

## User Setup Required

None.

## Next Phase Readiness

Plan 21-05 is complete. INT-02 and KEEP-01 remain pending for the later hosted and all-lane acceptance work.

## Self-Check: PASSED

- `21-05-SUMMARY.md` exists in the phase directory.
- Task commits `9d161b1` and `d84477c` are ancestors of HEAD.
- The persisted plan ledger records two task commits from `81a0f98c3d8b6785753eaaa93f001927f407617d` through `d84477c96daef040125529a8ceac08b4ccac0a29`.
- `git diff --check` passed; the pre-existing `.planning/config.json` modification remains unstaged.

---
*Phase: 21-interrupt-latency-and-hijack*
*Completed: 2026-10-29*
