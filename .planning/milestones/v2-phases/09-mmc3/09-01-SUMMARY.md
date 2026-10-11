---
phase: 09-mmc3
plan: 01
subsystem: mapper
tags: [mmc3, mapper-4, a12, irq, loader]
requires: []
provides:
  - "src/mapper_mmc3.c: MMC3 board (banking, mirroring, $A001 RAM gating, M2-filtered A12 counter, Sharp and NEC IRQ)"
  - "mapper 4 loader row and iNES 1 RAM branch"
  - "core.mapper_mmc3 test"
affects: [09-02, 09-03, 09-04]
tech-stack:
  added: []
  patterns: ["a001 stored XOR 0x80 so a zeroed block is power-on", "board-owned A12 level and fall record in reg.mmc3"]
key-files:
  created: [src/mapper_mmc3.c, tests/core/test_mapper_mmc3.c]
  modified: [src/mapper.h, src/cartridge.c, CMakeLists.txt, tests/CMakeLists.txt, tests/core/test_cartridge.c, README.md, include/nesturbator.h]
key-decisions:
  - "Clock iff nes->cpu_cycle - low_cycle >= 3 (D-05 as amended in D-09); boundary pinned 2 filtered, 3 and 4 clock"
  - "$A001 stored XOR 0x80 (D-08); NULL pages only, no bus branch"
  - "Hook rule in mapper.h amended: ppu_a12 may read nes->cpu_cycle"
requirements-completed: [BOARD-03, MAP-03]
status: complete
duration: 40 min
completed: 2026-10-11
commits: 3
plan_head_before: 0b46f84f21da4c55311b5739b714bd684d16932f
plan_head_after: c0dbdec9c880fdd1a64c996034faf173769b5fd3
actuals:
  tokens: 12000
  tasks: 3
  commits: 3
---

# Phase 9 Plan 01: MMC3 board Summary

MMC3 (mapper 4) board with the D-12 loader row, D-07 registers, `$A001` NULL-page RAM gating, and an A12 counter filtered on `cpu_cycle` M2 falls with Sharp and NEC IRQ rules.

## Accomplishments

- Task 1 (tracer, 63b104f): loader row, board banking, CPU-visible bank switch (`$8000=6`, `$8001=3` reads bank 3), `core.cartridge` row renamed `mapper-4-ines1-16k` and boarded count 6.
- Task 2 (516e88f): `$A001` gating, power-on and reset tests, README and public-header text (six mappers, shapes, RAM rows, refused shapes, Low G Man gap).
- Task 3 (c0dbdec): `ppu_a12` filter, one `clock_counter` routine, `$C000`-`$E001`, hook-rule comment; boundary, restart, long-low, repeated-low, duplicate-rise, first-rise, Sharp, NEC, iNES 1 and APU-IRQ cases.

## Deviations from Plan

None - plan executed exactly as written. A mutation check (filter threshold 3 to 2) failed `core.mapper_mmc3` as intended.

Note: acceptance grep `grep -c "mapper_mmc3.c" tests/CMakeLists.txt` returns 2, because the `core.mapper_mmc3` registration line also matches; the `bus.unit` source entry is present once.

## Known Stubs

None.

## Verification

- `cmake --workflow --preset ci`: 418 of 418 passed. `cmake --workflow --preset asan`: 418 of 418 passed.
- `tests/runner/hashes.txt` and `tests/accuracy/scoreboard.txt` identical to origin/main; `NESTURBATOR_BEHAVIOUR_REVISION` is 5.

## Next

Ready for 09-02.

## Self-Check: PASSED
