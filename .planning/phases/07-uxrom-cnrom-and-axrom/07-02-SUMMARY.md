---
phase: 07-uxrom-cnrom-and-axrom
plan: 02
subsystem: mapper
tags: [cnrom, axrom, bus-conflict, board-01]
requires:
  - phase: 07-uxrom-cnrom-and-axrom
    provides: probed board switch, shared page helpers, UxROM board (plan 01)
provides:
  - CNROM board (mapper 3) with D-15 AND rule and dropped CHR-ROM writes
  - AxROM board (mapper 7) with single-screen CIRAM page and bank-0 power-on PC
affects: [07-03, 07-04]
tech-stack:
  added: []
  patterns: [one board per task with its profile row, switch case, cases and wording]
key-files:
  created: [src/mapper_cnrom.c, src/mapper_axrom.c]
  modified: [src/mapper.h, src/cartridge.c, CMakeLists.txt, tests/CMakeLists.txt, tests/core/test_mapper_discrete.c, include/nesturbator.h, runner/main.c, README.md]
key-decisions:
  - "board_profile_ok checks CHR geometry per board instead of one 8 KiB rule up front."
  - "AxROM ANDs on submapper 2 only; CNROM on submappers 0 and 2 (submapper-0 AND labelled a project default)."
requirements-completed: []
status: complete
commits: 2
plan_head_before: 842b4ca03e562d5d953aec6965e2596457ae4758
plan_head_after: 7456be095e73a239b55d7b379f7165b7677ed7ab
actuals:
  tokens: 28000
  tasks: 2
  commits: 2
completed: 2026-10-10
---

# Phase 7 Plan 02: CNROM and AxROM boards Summary

Mappers 3 (CNROM) and 7 (AxROM) now load on the board seam, each with its bus-conflict rule and synthetic proof, and the runner, public header and README name exactly the mappers that load at each commit.

BOARD-01 is left open in REQUIREMENTS.md; plans 03 and 04 still contribute and the orchestrator closes it.

## Accomplishments

- CNROM: fixed PRG as NROM, CHR bank from the written byte (true modulo, 8/16/32 KiB CHR), AND conflict on submappers 0 and 2, CHR-ROM writes dropped because `chr_w` is NULL.
- AxROM: 32 KiB PRG bank from bits 0-2, bits 4 selects one CIRAM page for all four nametables, header mirroring ignored, reset vector read from bank 0, AND only on submapper 2.
- `core.mapper_discrete` gains 15 cases covering D-11, D-15, D-16 and the edges the plan lists.
- `chr_ram_write_lands_control` and step (a) of `axrom_single_screen_ciram` both passed, which shows the `ppu_poke` path is live for the CHR-ROM drop and the page-selection cases.
- Wording: commit 1 names mappers 0, 2 and 3; commit 2 names 0, 2, 3 and 7.

## Task Commits

1. Task 1 (tracer, CNROM): `42d06bd`
2. Task 2 (AxROM): `7456be0`

## Deviations from Plan

None. The tracer feedback gate was met by re-running the plan's `<verify>` subset, which passed before AxROM began. `NESTURBATOR_BEHAVIOUR_REVISION` stays 5; `hashes.txt` and `scoreboard.txt` equal origin/main. `clang-format` is not installed locally, so formatting is by hand and left to CI.

## Verification

`cmake --workflow --preset ci`: 362 of 362 tests pass. `scripts/hygiene.sh` exits 0.

## Known Stubs

None.

## Threat Flags

None.

## Self-Check: PASSED

`src/mapper_cnrom.c` and `src/mapper_axrom.c` exist; commits `42d06bd` and `7456be0` are ancestors of HEAD.
