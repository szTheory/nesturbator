---
phase: 08-mmc1-and-battery-saves
plan: 01
subsystem: core
tags: [mmc1, battery-save, public-api, prg-ram, loader]
requires:
  - phase: 07-uxrom-cnrom-and-axrom
    provides: mapper seam, board switch, loader profiles
provides:
  - nesturbator_get_memory, nesturbator_save_generation, enum nesturbator_memory
  - PRG-RAM as one [V work][N NVRAM] allocation with the save span as a view
  - mapper 1 loader row (D-07, D-08) and the MMC1 board at its power-on mapping
  - core.save CTest
affects: [phase-08-plan-02, phase-08-plan-05, phase-08-plan-06, phase-08-plan-08]
tech-stack:
  added: []
  patterns: [zero-means-power-on register encoding, NULL-guarded span range check in the bus write path]
key-files:
  created:
    - src/mapper_mmc1.c
    - tests/core/test_save.c
  modified:
    - include/nesturbator.h
    - src/internal.h
    - src/mapper.h
    - src/cartridge.c
    - src/bus.c
    - src/instance.c
    - CMakeLists.txt
    - tests/CMakeLists.txt
    - tests/ines.h
    - tests/core/test_cartridge.c
    - tests/cmake/vector_api_policy.cmake
key-decisions:
  - "control_x stores Control XOR 0x0C and last_write stores stamp + 1, so a zeroed reg.mmc1 is the power-on state (D-16)."
  - "Span writes are recognised by a range check in nesturbator__bus_write, guarded by save_size != 0 inside the page != NULL branch."
  - "PHASE8_DECLARATIONS_SHA256 = 7ec4ca9a124042f9a64d090c4362af019c2b9773cb2aaf901683c0daade2d7f0"
requirements-completed: [SAVE-01, BOARD-02]
status: complete
commits: 2
plan_head_before: a24e9bf5ee5b350137e7c3d29e9e8babc3ed7067
plan_head_after: ca648e12b41e7567daa4e5afd288c842c40fd0ba
actuals:
  tokens: 40000
  tasks: 2
  commits: 2
completed: 2026-10-10
---

# Phase 8 Plan 01: Battery span, save API and MMC1 power-on board Summary

A mapper 1 image with battery RAM loads, its CPU writes to `$6000-$7FFF` land in the span `nesturbator_get_memory` returns, and `nesturbator_save_generation` counts exactly those writes; the span keeps its address across frames, reset and a refused load.

## Accomplishments

- Public API: `enum nesturbator_memory`, `nesturbator_get_memory(inst, kind, uint8_t **, size_t *)` and `nesturbator_save_generation`, with the D-11 and D-12 rules in the header comment. `NESTURBATOR_ABI_VERSION` and the behaviour revision (5) are unchanged.
- Loader: PRG-RAM is one allocation laid out `[V work][N NVRAM]`; iNES 1 and NES 2.0 sizing per D-07; mapper 1 row per D-08 (24 KiB total refused). Mappers 0, 2, 3 and 7 still refuse the battery bit and every RAM size. Trainer RAM is the same allocation.
- Board: `src/mapper_mmc1.c` rebuilds the power-on mapping from a zeroed `reg.mmc1` (PRG mode 3, last bank at `$C000`, one-screen lower, 8 KiB CHR, RAM at `$6000`); `nesturbator__map_prg_ram_8k` generalises the RAM page helper. A cartridge with no RAM reads `$6000` as open bus.
- Tests: `core.save` covers the tracer image, the no-RAM open bus edge, NULL arguments and an unknown kind (sentinels unchanged), no cartridge, a battery-less cartridge, host writes, a refused load, unload, and a reload that continues the count. `tests/ines.h` gained battery and RAM nibble fields and PRG `segments`. The board switch test expects `{0, 1, 2, 3, 7}`. The API policy hash is pinned to the new declarations.
- `cmake --workflow --preset ci`: 375 of 375 tests pass. `tests/runner/hashes.txt` and the scoreboard are byte-identical to origin/main.

## Task Commits

1. Task 1 (tracer): `0f97062`
2. Task 2: `ca648e1`

The tracer's `<verify>` was re-run end to end before Task 2 (both commands passed).

## Deviations from Plan

**1. [Rule 3 - Blocker] Precondition check used a stale commit id**
- **Found during:** Task 1 precondition
- **Issue:** `git merge-base --is-ancestor 214a118 origin/main` fails because PR #32 was squash-merged (`a24e9bf`), so 214a118 is not an ancestor even though Phase 7's content is on main.
- **Fix:** Verified the intent instead: the branch is not main, and `a24e9bf` ("feat: UxROM, CNROM and AxROM boards (#32)") is an ancestor of origin/main and of HEAD. Proceeded.
- **Files modified:** none

**2. [Rule 3 - Blocker] bus.unit source list**
- **Found during:** Task 1 build
- **Issue:** `bus.unit` links its own list of board sources, so `mapper_mmc1.c` was missing at link time.
- **Fix:** Added `src/mapper_mmc1.c` to that list in `tests/CMakeLists.txt` (already in the plan's file list).
- **Commit:** 0f97062

**Total deviations:** 2 auto-fixed (Rule 3). **Impact:** none on scope.

Task 2's cases passed on first run against Task 1's code, so no defect fix to `src/instance.c` or `src/cartridge.c` was needed (the tests were written alongside the tracer's implementation, not RED first).

## Known Stubs

`mmc1_cpu_write` in `src/mapper_mmc1.c` only records `last_write`; the serial port and banking registers are Plan 05's (stated in the plan). No stub reaches this plan's goal.

## Self-Check: PASSED

Created files exist (`src/mapper_mmc1.c`, `tests/core/test_save.c`), commits `0f97062` and `ca648e1` are ancestors of HEAD, `cmake --workflow --preset ci` passes, and the plan's acceptance greps match.
