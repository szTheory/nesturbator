---
phase: 08-mmc1-and-battery-saves
plan: 05
subsystem: mapper
tags: [mmc1, mapper-seam, serial-port, write-stamp, sxrom]
requires:
  - phase: 08-mmc1-and-battery-saves
    provides: plans 01-03 (reg.mmc1 block, loader RAM layout, banked PRG-RAM helper, save counter)
provides:
  - zero-based hook write stamp (nes->cpu_cycle - 1), Phase 6 WR-01, WR-03 and IN-02 fixed
  - complete MMC1B board in src/mapper_mmc1.c (serial port, PRG/CHR/RAM banking, mirroring, SNROM/SOROM/SUROM/SXROM from sizes)
  - core.mapper_mmc1 test with hook-level and CPU-level cases
affects: [phase-08-plan-06, phase-08-plan-07, phase-09]
tech-stack:
  added: []
  patterns: [last_write stores stamp+1 with zero meaning never written, RAM variant chosen by allocation size, disabled RAM as NULL pages]
key-files:
  created:
    - tests/core/test_mapper_mmc1.c
  modified:
    - src/bus.c
    - src/internal.h
    - src/mapper.h
    - src/mapper_mmc1.c
    - tests/core/test_mapper.c
    - tests/CMakeLists.txt
    - .planning/phases/06-mapper-seam-and-ppu-fetch-pipeline/06-REVIEW-DISPOSITION.md
    - include/nesturbator.h
    - README.md
key-decisions:
  - "Stamp fix first: the hook receives nes->cpu_cycle - 1, the zero-based index of the write's own cycle; no pinned hash or scoreboard row moved and the behaviour revision stays 5."
  - "RAM bank is chosen by the PRG-RAM allocation size (<=8 KiB none, 16 KiB CHR0 bit 3, 32 KiB CHR0 bits 3-2); SOROM therefore ignores bit 2."
  - "PRG outer bit (CHR0 bit 4) applies only above 256 KiB PRG and moves both windows including the fixed bank."
requirements-completed: [BOARD-02]
status: complete
commits: 3
plan_head_before: 5b1ab6db7db38c808e7d054cd3607f0eeff284ee
plan_head_after: 893bcf88c23991010dfdc5ceb21f629f73537d14
actuals:
  tokens: 10800
  tasks: 3
  commits: 3
duration: 25 min
completed: 2026-10-10
---

# Phase 8 Plan 05: MMC1B board Summary

Mapper 1 is now a complete MMC1B board on the seam: a serial port that ignores the second write of a read-modify-write unless it sets the reset bit, size-derived SNROM/SOROM/SUROM/SXROM banking, and a corrected write stamp that the port depends on.

## Accomplishments

- **Stamp fix (D-14).** `nesturbator__bus_write` passes `nes->cpu_cycle - 1u`; comments in `bus.c`, `internal.h` and `mapper.h` now state one rule. `nesturbator__map_cpu_read` returns open bus below `$4020`; `nesturbator__map_chr_4k` added. `core.mapper` has `check_stamp_is_cycle_index` and `check_map_cpu_read_below_4020`. WR-01, WR-03 and IN-02 are `fixed` (open count 2).
- **Serial port (D-15, D-16).** Adjacency is `last_write != 0 && stamp == last_write`, `last_write = stamp + 1` on every hook call; bit 7 resets and is never ignored; the fifth write's address bits 14-13 select the register. No opcode or CPU state is read.
- **Banking (D-05, D-10).** PRG modes 0-3 with the 256 KiB outer half on both windows, CHR 8 KiB/4 KiB modes, four mirroring maps, RAM disable by `$E000` bit 4, SNROM disable by CHR0 bit 4 only on CHR-RAM boards with PRG <= 256 KiB and RAM <= 8 KiB, SOROM bit 3 only, SXROM bits 3-2.
- **Tests.** `core.mapper_mmc1` (18 cases): tracer (five CPU writes switch the bank, `$0010 == 0x42`), hook adjacency and first-write cases, `$6000`-then-`$8000` drop, register select and bit order, reset bit, `INC $8000` on `$02`/`$FF`/`$7F`, `INC $6000` raising the generation by exactly 2, state kept across two resets, mirroring, CHR modes, RAM enable, SNROM, SOROM, SXROM and SUROM/SXROM PRG halves. Mutating the adjacency check makes it fail.
- **Docs.** `include/nesturbator.h` comment and README describe the MMC1B board.

## Task Commits

1. Task 1 (tracer): `8e544df` feat(08-05): zero-based write stamp and MMC1 serial port tracer
2. Task 2: `c16037b` test(08-05): mmc1 write rules at the hook and through the CPU
3. Task 3: `893bcf8` feat(08-05): mmc1 chr modes, mirroring, ram enable and sxrom variants

## Deviations from Plan

None - plan executed exactly as written. Task 2's cases passed on first run because Task 1's tracer already carried the full serial port; the adjacency check was mutated to confirm they are not vacuous. Task 3 followed red then green.

## Verification

- `cmake --workflow --preset ci`: passed. `cmake --workflow --preset asan`: 379/379 passed. `cmake --workflow --preset hygiene`: 8/8 passed.
- `tests/runner/hashes.txt` and `tests/accuracy/scoreboard.txt` identical to `origin/main`; behaviour revision 5.
- Acceptance greps: 3 fixed rows and `open: 2` in the Phase 6 ledger; `SOROM implements only this bit` in `src/mapper_mmc1.c`; INC cases target `$8000`.

## Known Stubs

None.

## Threat Flags

None. T-08-04 (bank math) is covered by the true-modulo helpers and asan; T-08-05 (disabled RAM) by NULL pages tested in `test_ram_enable`.

## Self-Check: PASSED

Files found: `src/mapper_mmc1.c`, `tests/core/test_mapper_mmc1.c`. Commits `8e544df`, `c16037b`, `893bcf8` are ancestors of HEAD.
