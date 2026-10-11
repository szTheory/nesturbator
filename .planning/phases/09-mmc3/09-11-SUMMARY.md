---
phase: 09-mmc3
plan: 11
subsystem: tests
tags: [mmc3, blargg, oracle, ppu, a12]
requires: [09-10]
provides:
  - "tests/mmc3/oracle.txt: 12 rows, 10 pass, 2 unsupported (the NEC-revision ROMs)"
  - "mmc3.pins, mmc3.oracle.inventory, mmc3.oracle.unit offline in ci"
  - "PPU fix: pre-render dot 0 shows v, so BG at $1000 first clocks the MMC3 at dot 5"
affects: [09-12]
key-files:
  created: [tests/cmake/mmc3_oracle_inventory.cmake, tests/mmc3/test_oracle.c]
  modified: [tests/mmc3/oracle.txt, tests/CMakeLists.txt, src/ppu.c, tests/ppu/test_fetch.c, README.md]
key-decisions:
  - "Open question 1 (D-06): no IRQ delay added. Only mmc3_test_2/4-scanline_timing failed (code 0x08); mmc3_irq_tests/4.Scanline_timing passed, so the one-cycle rule did not fire. The failure was a PPU defect, fixed in src/ppu.c."
  - "Budgets: 120 frames for every row except 4-scanline_timing 480 (passes at frame 328) and irq 4.Scanline_timing 240 (60 fails, 120 passes)."
requirements-completed: [BOARD-03]
status: complete
duration: 40 min
completed: 2026-10-10
commits: 3
plan_head_before: 0d782d85d113e058821828676a462ed8e1d3b3da
plan_head_after: 06de06db25f1285bfb1891cd1edf174ea4491185
actuals:
  tokens: 14000
  tasks: 3
  commits: 3
---

# Phase 9 Plan 11: Blargg MMC3 Results and Offline Checks Summary

All 12 blargg MMC3 ROMs are measured on the finished board: 10 pass and the two NEC-revision ROMs are recorded unsupported, after a one-line PPU fix for the pre-render idle dot.

## Measurements

| Row | Result | Frames to result / budget |
|---|---|---|
| mmc3_test_2/1-clocking | pass | 27 / 120 |
| mmc3_test_2/2-details | pass | 32 / 120 |
| mmc3_test_2/3-A12_clocking | pass | 28 / 120 |
| mmc3_test_2/4-scanline_timing | pass (first run: 0x08) | 328 / 480 |
| mmc3_test_2/5-MMC3 | pass | 27 / 120 |
| mmc3_test_2/6-MMC3_alt | unsupported, 0x02 | 31 / 120 |
| mmc3_irq_tests/1.Clocking, 2.Details, 3.A12_clocking, 6.MMC3_rev_B | pass | passes at budget 60 / 120 |
| mmc3_irq_tests/4.Scanline_timing | pass | fails at 60, passes at 120 / 240 |
| mmc3_irq_tests/5.MMC3_rev_A | unsupported, 0x03 | 120 |

A5 confirmed once: `--dump-frame` of `mmc3_irq_tests/1.Clocking` shows PASSED while the oracle reads `$F8 == 1`. A7 held: the 16 KiB ROMs boot and pass with the true-modulo aliasing.

## Open question 1 decision

Scanline-timing codes: `mmc3_test_2/4-scanline_timing` first reported 0x08 ("Scanline 0 IRQ should occur later when $2000=$10"); `mmc3_irq_tests/4.Scanline_timing` passed. The rule needs both to fail, so no assertion delay was added. A12 and IRQ tracing showed the cause: at pre-render dot 0 the PPU drove the pending tile's pattern address, so with PPUCTRL bit 4 set A12 rose there after the long low of vblank and clocked the counter at dot 0, earlier than the console. With dot 0 of the pre-render line showing `v`, the first clock is at dot 5; test 8 passed and test 9 ("sooner") also passes, bracketing the position. A first experiment that removed the BG rise on the whole pre-render line failed test 9, which is how the window was found.

## Deviations from Plan

**1. [Rule 1 - Bug] PPU pre-render dot 0 drove A12**
- **Found during:** Task 2 (4-scanline_timing exit 0x08, a Sharp-revision row)
- **Fix:** `src/ppu.c` applies the dot-0 pattern address only when the new line is not 261; `test_pre_render_dot_zero_shows_v` in `tests/ppu/test_fetch.c` pins no rise at dot 0 and a rise at dot 5; README states the rule.
- **Files modified:** src/ppu.c, tests/ppu/test_fetch.c, README.md
- **Commit:** cae155c
- All hashes and the scoreboard are unchanged (`ci`, 446 tests, passes). The fix sits in the PPU, not the board, so the `core.mapper_mmc3` case the plan names was not needed; `ppu.mmc3` frame counts (242 clocks with BG at $1000) still hold.

**2. Task 1 tracer** was run as a one-row lane pass, then folded into the all-rows commit (362f244) because the oracle table is one file; `mmc3.oracle.mmc3_irq_tests/1.Clocking` passed alone first.

`oracle.c` was not touched; no oracle defect appeared.

## Known Stubs

None.

## Self-Check: PASSED

`cmake --workflow --preset ci` (446 tests), `--preset hygiene` and `--preset mmc3-oracle` (13 tests) pass; `tests/accuracy/scoreboard.txt` is unchanged; commits cae155c, 362f244 and 06de06d exist.
