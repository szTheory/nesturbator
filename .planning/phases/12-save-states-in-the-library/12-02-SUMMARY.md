---
phase: 12-save-states-in-the-library
plan: 02
subsystem: core-library
tags: [save-states, mapper, mmc3, mmc1, c17]
requires:
  - phase: 12-01
    provides: state.h visitor primitives and the MAPPER section with mapper.irq
provides:
  - mandatory state op in struct nesturbator__mapper_ops, filled by all six boards
  - MAPPER section = mapper.irq followed by the board registers in struct field order
affects: [12-03 README, 12-04 fork matrix, 12-06 rejection sweeps]
actuals:
  tokens: 14000
  tasks: 2
  commits: 2
plan_head_before: 7533ed413e67c3d9729c04d40f25db26b0ff6eb3
plan_head_after: 802cdc6a37a274477c8d067e2253416a1acae67d
key-files:
  created: []
  modified: [src/mapper.h, src/mapper_nrom.c, src/mapper_uxrom.c, src/mapper_cnrom.c, src/mapper_axrom.c, src/mapper_mmc1.c, src/mapper_mmc3.c, src/state.c, tests/mapper_test.h, tests/core/test_state.c]
key-decisions:
  - "Register round trip is whitebox: registers are written into the instance and the board rebuilt, not driven by a CPU program"
  - "state op is mandatory; state.c has no NULL check and the ops table test covers the six ids"
requirements-completed: [STATE-02, STATE-03]
status: complete
completed: 2026-10-12
---

# Phase 12 Plan 02: Board registers in the state Summary

**Every board (ids 0, 1, 2, 3, 4, 7) now has a `state` op, so a state carries the bank, shift and scanline-counter registers and a load rebuilds the same page tables.**

## Accomplishments

- `state` op added to `struct nesturbator__mapper_ops`; `mapper_io` in `src/state.c` calls it after `mapper.irq`. Bounds: MMC1 five-bit fields at most 0x1f and count at most 4; MMC3 reload, irq_enable and a12 at most 1; the u64 stamps accept any pattern.
- Mapper-section sizes in bytes (irq 1 plus registers): NROM 1, UxROM 2, CNROM 2, AxROM 2, MMC1 15, MMC3 25.
- `core.state` gained: ops table, an end-to-end round trip per board (registers equal, page and nametable offsets equal to the source, different from the power-on tables, size equal to the NROM twin plus the register bytes), and the MMC3 `map.a12` derivation from `ppu.bus_addr` for bit 12 set and clear.
- Verification: `cmake --workflow --preset ci` passed (489 tests), asan preset passed (484 tests).

## Deviations from Plan

None to behaviour. Task 2 was TDD-marked but the tracer in Task 1 already implemented every behaviour, so both tasks' cases were written together and passed once the test image shapes were corrected (NROM twin must use 32 KiB PRG and no PRG RAM, so PRG RAM bytes are counted in the expected size). Commits are split by kind: `0d6d8d5` feat (all source), `802cdc6` test (all cases).

## Known Stubs

None.

## Threat Flags

None. T-12-11: the bounds above are enforced in VALIDATE; bank registers wrap by the existing true modulo at rebuild. Plan 06 sweeps every bound.

## Self-Check: PASSED

- FOUND commits 0d6d8d5 and 802cdc6 in HEAD ancestry
- core.state, ci workflow and asan workflow green
