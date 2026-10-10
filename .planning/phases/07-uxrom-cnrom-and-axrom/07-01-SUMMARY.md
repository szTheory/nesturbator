---
phase: 07-uxrom-cnrom-and-axrom
plan: 01
subsystem: mapper
tags: [uxrom, loader, bus-conflict, board-01]
requires:
  - phase: 06-mapper-seam-and-ppu-fetch-pipeline
    provides: mapper seam, page tables, watch bits
provides:
  - nesturbator__mapper_ops_for probe (single switch on the board id)
  - PC read through power-on pages
  - shared page helpers in internal.h
  - UxROM board (mapper 2) with D-15 bus-conflict rule
affects: [07-02, 07-03]
tech-stack:
  added: []
  patterns: [probe board before allocation, per-board profile row, shared static inline page helpers]
key-files:
  created: [src/mapper_uxrom.c, tests/core/test_mapper_discrete.c]
  modified: [src/mapper.h, src/internal.h, src/mapper_nrom.c, src/cartridge.c, tests/ines.h, tests/core/test_cartridge.c, include/nesturbator.h, libretro/libretro.c, CMakeLists.txt, tests/CMakeLists.txt, runner/main.c, README.md]
key-decisions:
  - "Board lookup moved into nesturbator__mapper_ops_for, probed in validate_image before any allocation (D-10)."
  - "Submapper-0 AND on mapper 2 is a project default, labelled as such."
patterns-established:
  - "Each board adds one case in nesturbator__mapper_ops_for and one row in board_profile_ok."
requirements-completed: [BOARD-01]
status: complete
commits: 2
plan_head_before: c902dc88807ac6235bc6dfcc0836d4c243beb274
plan_head_after: 5584dbc9ce7679cb389aa3c0c01e66469461db0b
actuals:
  tokens: 30000
  tasks: 2
  commits: 2
completed: 2026-10-10
---

# Phase 7 Plan 01: UxROM board and probed board switch Summary

Every load now picks its board through one switch probed before allocation, the power-on PC comes from the board's pages, and mapper 2 (UxROM) loads with its submapper-dependent bus-conflict rule, proven by synthetic CTest cases.

## Precondition

Branch `gsd/phase-7-uxrom-cnrom-and-axrom`; origin/main was `c902dc88807ac6235bc6dfcc0836d4c243beb274` (contains Phase 6, PR #30) and is an ancestor of HEAD.

## Accomplishments

- `nesturbator__mapper_ops_for` is the only switch on the id; `validate_image` probes it with a scratch ops struct before allocating. This closes Phase 6 review finding IN-01 (an unknown id used to load as an empty cartridge). `test_unboarded_id_leaves_instance` shows cart bytes, PC and allocation count unchanged.
- Reset vector read via `nesturbator__map_cpu_read(0xfffc/0xfffd)` after `nesturbator__mapper_load`. Every pinned NROM hash is unchanged (`runner.write_hashes.content` passes; hashes.txt and scoreboard.txt equal origin/main).
- Shared helpers `nesturbator__map_header_mirroring`, `nesturbator__map_prg_ram`, `nesturbator__map_chr_8k`.
- UxROM: fixed last bank, bank register at `$8000-$FFFF`, wrap by true modulo, AND against the pre-write banking for submappers 0 and 2 (raw for submapper 1).
- `core.mapper_discrete` holds the ten UxROM cases; the CHR-RAM case writes through the `ppu_poke` helper (`$2006`/`$2007` on the CPU bus) after asserting `ppu.reset_flag == 0`.
- Runner help and error, public header comments and README name mappers 0 and 2 in the same commit as the board.

## Task Commits

1. Task 1 (tracer, loader path on NROM): `2ec363a`
2. Task 2 (UxROM board, tests, wording): `5584dbc`

## Deviations from Plan

None. The plan was executed as written. `NESTURBATOR_BEHAVIOUR_REVISION` stays 5. `clang-format` is not installed locally, so formatting was written by hand and is left to CI's `format_check`.

## Verification

`cmake --workflow --preset ci`: 362 of 362 tests pass.

## Known Stubs

None.

## Threat Flags

None.

## Self-Check: PASSED

Files `src/mapper_uxrom.c` and `tests/core/test_mapper_discrete.c` exist; commits `2ec363a` and `5584dbc` are ancestors of HEAD.
