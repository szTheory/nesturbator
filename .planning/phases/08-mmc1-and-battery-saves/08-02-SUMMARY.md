---
phase: 08-mmc1-and-battery-saves
plan: 02
subsystem: core
tags: [mmc1, loader, battery-save, fuzz-seeds, docs]
requires:
  - phase: 08-mmc1-and-battery-saves
    provides: plan 01 loader, save API and MMC1 power-on board
provides:
  - D-07 sizing and D-08 mapper 1 accept and refuse rows as tests
  - seeds valid-mmc1, valid-mmc1-sorom and mapper-155 with manifest lines
  - README save API, size table and mapper 1 shapes; matching header comments
affects: [phase-08-plan-04, phase-08-plan-05]
tech-stack:
  added: []
  patterns: [row tables named after fuzz seeds, assumption-delta invariant checked on every accepted row]
key-files:
  created:
    - tests/fuzz/corpus/valid-mmc1
    - tests/fuzz/corpus/valid-mmc1-sorom
    - tests/fuzz/corpus/mapper-155
  modified:
    - tests/core/test_cartridge.c
    - tests/core/test_save.c
    - tests/core/test_api.c
    - tests/header/header_c.c
    - tests/header/header_cxx.cpp
    - tests/roms/manifest.txt
    - README.md
    - include/nesturbator.h
    - .planning/research/ARCHITECTURE.md
key-decisions:
  - "Seed pins: valid-mmc1 and valid-mmc1-sorom pin 87afe8dfa420f957ea0d9cdace9a5c3b8aa9cc0e; mapper-155 pins ac72d0559c2f48f74132131fadbc7a974c0d53c0."
requirements-completed: [SAVE-01, BOARD-02]
status: complete
commits: 3
plan_head_before: 87afe8dfa420f957ea0d9cdace9a5c3b8aa9cc0e
plan_head_after: 6241000af9bbd5bcffe3a211526fbf80c8059239
actuals:
  tokens: 30000
  tasks: 3
  commits: 3
completed: 2026-10-10
---

# Phase 8 Plan 02: Loader rows, seeds and save API docs Summary

Plan 01's mapper 1 loader is now pinned row by row to the D-07 size table and D-08 shapes, three seeds replay in `fuzz.regress`, and the README and public header describe the save API and the accepted shapes.

## Accomplishments

- SOROM (NES 2.0, 8 KiB work + 8 KiB NVRAM) exposes `save == prg_ram + 8192`, `save_size == 8192`, also through `nesturbator_get_memory`.
- `test_cartridge.c`: a mapper 1 row table with 16 accept and 22 refuse rows (refuse labels equal the D-09 seed names, `mapper-1` renamed `mmc1-prg-16k` in the iNES 1 form), the `save == prg_ram + V`, `save_size == N` invariant on every accepted row, and iNES 1 size cases (8 KiB, 32 KiB, battery or not, non-zero byte 8 refused).
- `test_save.c`: trainer on mapper 1 with no declared RAM maps 8 KiB of work RAM with the trainer at `$7000` and no span.
- Seeds `valid-mmc1`, `valid-mmc1-sorom` (runner exits 0) and `mapper-155` (runner exits 1), built from a header, zeros, a reset vector and `4C 00 80`; generator kept in the scratchpad.
- `core.api`, `header.c` and `header.cxx` reference `NESTURBATOR_MEMORY_SAVE_RAM`, the typedef and both functions.
- README "Battery saves" section (D-11 rules, D-12 counting, size table, `[V work][N NVRAM]`, SOROM known gap, copy-in-after-load rule) and mapper 1 shapes in the runner text; header comments aligned. ARCHITECTURE.md corrected to `[ work | nvram ]` and `uint8_t **`.
- `cmake --workflow --preset ci`: 375 of 375 pass; hygiene preset passes. `tests/runner/hashes.txt` and the scoreboard match origin/main; behaviour revision stays 5; `PHASE8_DECLARATIONS_SHA256` did not move.

## Task Commits

1. Task 1 (tracer): `ac72d05`
2. Task 2: `86cc755`
3. Task 3: `6241000`

The tracer's `<verify>` was re-run before Task 2 (core.save, fuzz.regress, manifest.sha256 pass; both valid seeds run; hygiene passes).

## Deviations from Plan

**1. [Rule 3 - Blocker] clang-format 18 violations from Plan 01**
- **Found during:** Task 1 hygiene
- **Issue:** `hygiene.format` failed on `src/cartridge.c`, `src/internal.h` and `tests/ines.h` (Plan 01 leftovers).
- **Fix:** Ran clang-format 18 on those files; whitespace only.
- **Commit:** ac72d05

**2. Task 2 needed no loader fix.** Every new row passed against Plan 01's code on first run, so `src/cartridge.c` has no behaviour change in this plan. The rows were written against D-07/D-08, not copied from the code.

**Total deviations:** 1 auto-fixed (Rule 3). **Impact:** none on scope.

## Known Stubs

None added. (`mmc1_cpu_write` stays Plan 05's.)

## Threat Flags

None.

## Self-Check: PASSED

Seeds, test files and commits `ac72d05`, `86cc755`, `6241000` exist and are ancestors of HEAD; `cmake --workflow --preset ci` and the hygiene preset pass; the plan's greps match.
