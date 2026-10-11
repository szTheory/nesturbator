---
phase: 07-uxrom-cnrom-and-axrom
plan: 03
subsystem: loader
tags: [loader, fuzz-seeds, board-01]
requires:
  - phase: 07-uxrom-cnrom-and-axrom
    provides: UxROM, CNROM and AxROM boards and per-board profiles (plans 01, 02)
provides:
  - test_board_profiles with every D-09/D-13 accept and reject row
  - test_board_switch_matches_profiles (ids 0-4095, boards exactly 0, 2, 3, 7)
  - six D-13 fuzz seeds with manifest lines
affects: [07-04]
tech-stack:
  added: []
  patterns: [table-driven loader rows with printed labels]
key-files:
  created: [tests/fuzz/corpus/valid-uxrom, tests/fuzz/corpus/valid-cnrom, tests/fuzz/corpus/valid-axrom, tests/fuzz/corpus/cnrom-chr-ram, tests/fuzz/corpus/axrom-odd-prg, tests/fuzz/corpus/submapper-3]
  modified: [tests/core/test_cartridge.c, tests/roms/manifest.txt]
key-decisions:
  - "Seed manifest lines pin dc01ae863a7793e2c3bed93c11f7c92e09defe4d, the HEAD when the seeds were built."
requirements-completed: []
status: complete
commits: 3
plan_head_before: c896f785d8b55d5269b13f9f85e75a67a62ff7c3
plan_head_after: b3532f8e9d04ffa5cdd9408348478e1a54cd7821
actuals:
  tokens: 14000
  tasks: 2
  commits: 3
completed: 2026-10-10
---

# Phase 7 Plan 03: Loader rows and fuzz seeds Summary

The loader's accept/reject contract for mappers 0, 2, 3 and 7 is pinned by a 54-row table against the real loader, the board switch and the profiles are proven to agree on exactly {0, 2, 3, 7}, and six synthetic seeds are replayed by `fuzz.regress`.

BOARD-01 is left open in REQUIREMENTS.md; plan 04 still contributes and the orchestrator closes it.

## Accomplishments

- `test_board_profiles`: 19 accept rows and 35 reject rows (labels as in the plan's edge table, including `uxrom-4mib`, `uxrom-4mib-plus-16k`, `nes2-mapper-256`, `nes2-mapper-4095`, `diskdude`, `prg-ram`). Every row matched the loader on the first run; no loader change was needed.
- `test_board_switch_matches_profiles`: walks 0-4095, expects a board only for 0, 2, 3, 7 with non-NULL `init` and `rebuild`.
- Seeds `valid-uxrom`, `valid-cnrom`, `valid-axrom` load (runner exit 0); `cnrom-chr-ram`, `axrom-odd-prg`, `submapper-3` are refused (runner exit 1). All built from zeros, a reset vector and a 3-byte loop by a script kept in the scratchpad.
- Seed pin SHA: `dc01ae863a7793e2c3bed93c11f7c92e09defe4d`.

## Task Commits

1. Task 1 (tracer, rows and id walk): `dc01ae8`
2. Task 2 (six seeds and manifest lines): `e449f79`
3. Formatting fix (below): `b3532f8`

## Deviations from Plan

**1. [Rule 3 - Blocking] clang-format violations failed `hygiene.format`**
- **Found during:** Task 2 verify (`cmake --workflow --preset hygiene`)
- **Issue:** Files from plans 01 and 02 plus two lines of my new test were not clang-formatted, so the hygiene workflow failed. Plan 02 had noted clang-format was missing locally; it is at `/opt/homebrew/opt/llvm@18/bin/clang-format`.
- **Fix:** Ran clang-format over the six flagged files; whitespace and line breaks only.
- **Files modified:** include/nesturbator.h, runner/main.c, src/cartridge.c, src/mapper_uxrom.c, tests/core/test_cartridge.c, tests/core/test_mapper_discrete.c
- **Commit:** `b3532f8`

**Total deviations:** 1 auto-fixed (1 blocking). **Impact:** none on behaviour.

## Verification

`cmake --workflow --preset ci`: 362 of 362 tests pass. `cmake --workflow --preset hygiene` passes and `scripts/hygiene.sh` exits 0. `manifest.sha256` and `fuzz.regress` pass.

## Known Stubs

None.

## Threat Flags

None.

## Self-Check: PASSED

The six seeds exist under `tests/fuzz/corpus`; commits `dc01ae8`, `e449f79` and `b3532f8` are ancestors of HEAD.
