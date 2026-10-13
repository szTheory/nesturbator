---
phase: 12-save-states-in-the-library
plan: 05
subsystem: core-library
tags: [save-states, tests, fork-matrix, stress]
requires:
  - phase: 12-04
    provides: fork_image driver and fork matrix
provides:
  - st_stress_image (NROM and MMC3 DMA and interrupt stress programs, built in memory)
  - core.state_fork --stress mode with its own fork list and straddle assertions
affects: [12-10 pins]
actuals:
  tokens: 30000
  tasks: 2
  commits: 2
plan_head_before: 880e52b884113381cef5db40d2284acf1601edd9
plan_head_after: 4f0126afdc051bffefded1cc6f43fde891e5054a
key-files:
  created: []
  modified: [tests/state_test.h, tests/core/test_state_fork.c, tests/CMakeLists.txt, README.md]
key-decisions:
  - "Stress straddles are made by timing, not by luck: the NMI handler waits a counted delay (Y=1, X=220) then restarts the DMC and writes $4014 5 loop-pairs apart, so the drifting frame end passes each instant in turn"
  - "The DMC sample ends with its IRQ enabled and is restarted every frame instead of using the loop bit, so the IRQ line is high for most of each frame and poll_latch straddles reliably"
  - "core.state_fork TIMEOUT stays 300 s (ci run 138 s, asan 66 s); the save-every-frame fallback was not needed"
requirements-completed: []
status: complete
completed: 2026-10-12
---

# Phase 12 Plan 05: Save-state stress fork Summary

**NROM and MMC3 DMA and interrupt stress programs are forked at every frame 0 to 120 through the same driver as the committed ROMs, with OAM DMA, DMC DMA and the IRQ poll latch shown pending at frame boundaries.**

## Accomplishments

- `st_stress_image(out, cap, mapper)` builds a 32 KiB image with `tests/ines.h` for mapper 0 or 4. The NMI handler does a timed DMC restart and a timed `$4014` write; the IRQ handler acknowledges the frame IRQ only every 64th entry; MMC3 adds the scanline IRQ reload.
- `core.state_fork --stress` runs both images after the file ROMs: 121 fork points over 150 frames in ci; {0, 1, 2, 3, 30} over 60 frames with `--short`. It prints `forks stress-nrom 121` and `forks stress-mmc3 121`, and fails unless the straddle flags were each seen.
- Straddle boundaries seen (ci, 151 boundaries): stress-nrom oam_dma_pending 4 (first at frame 44), dmc.dma_pending 6 (first 18), poll_latch 16 (first 1); stress-mmc3 oam 2 (first 36), dmc 2 (first 19), poll 16 (first 1). All three appear within the first 60 frames on both, so the `--short` run asserts them too.
- Every stress fork loaded with OK after the poisoned pre-fill and continued identically; save-every-frame matched.
- README "Save states" states the fork evidence.
- Measured: ci `core.state_fork` 138 s (was 77 s), asan 66 s; TIMEOUT kept at 300 s. `cmake --workflow --preset ci` (491 tests) and `--preset asan` (486 tests) pass.
- STATE-03 stays pending: its last clause (no pinned hash changes with saving every frame) is Plan 10.

## Visitor fixes

None. Every stress fork matched on the first run; `src/state.c` unchanged.

## Deviations from Plan

**1. [Rule 1 - Bug in plan assumption] Main-loop STA $4014 cannot straddle.** Found during Task 1. A frame ends at the first instruction end at or after a fixed tick count, and the OAM DMA runs inside the instruction after `STA $4014`, so a loop-written `$4014` is pending at the boundary only 4/530 of the time and never in a scan of 250 loop paddings. The program was changed to a timed NMI handler (frame end drifts against the PPU by instruction overshoot; the handler's instants lie where the drift passes). Constants chosen by measurement; neighbouring values also hit.

**2. [Rule 1] DMC restart instead of the loop bit.** The plan says the sample loops. A loop-mode sample refills at once and is almost never pending at a boundary. The sample is 17 bytes, ends with IRQ enabled (`$4010 = $8F`) and is restarted by the handler each frame (`$4015` off then on), which keeps a DMA pending and the IRQ line high. Files: tests/state_test.h.

Total deviations: 2 (both in the test program design). Impact: none on production code.

## Known Stubs

None.

## Threat Flags

None.

## Self-Check: PASSED

- FOUND tests/state_test.h, tests/core/test_state_fork.c; commits 100c99d and 4f0126a in HEAD ancestry
- Acceptance greps and output lines `forks stress-nrom 121`, `forks stress-mmc3 121` confirmed; ci and asan workflows green
