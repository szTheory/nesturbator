---
phase: 09-mmc3
plan: 03
subsystem: ppu-mapper-seam
tags: [mmc3, a12, ppu, tests]
requires: [09-01]
provides:
  - "ppu_fixture_load_mmc3(submapper): NES 2.0 mapper 4 fixture, 32 KiB PRG, CHR-RAM"
  - "ppu.mmc3: 11 through-the-PPU counter clock cases"
affects: [09-04, 09-05, 09-06]
tech-stack:
  added: []
  patterns: ["frame cases run the real CPU and PPU; dot cases step the PPU and bump cpu_cycle every third dot"]
key-files:
  created: [tests/ppu/test_mmc3.c]
  modified: [tests/ppu/ppu_fixture.h, tests/CMakeLists.txt]
key-decisions:
  - "With BG at $1000 and sprites at $0000 a frame holds 242 clocks, not 241: the pre-render line clocks at dot 5 and dot 325"
requirements-completed: [BOARD-03]
status: complete
duration: 15 min
completed: 2026-10-10
commits: 1
plan_head_before: cc25866132803c6b56aa2be413ff08479de66b08
plan_head_after: 35589cc
actuals:
  tokens: 12000
  tasks: 2
  commits: 1
---

# Phase 9 Plan 03: MMC3 counter through the real PPU Summary

The real PPU's A12 rises, filtered by the board's M2 rule, clock the MMC3 counter once per rendered line (241 per frame) with no change to `src/ppu.c`.

## Accomplishments

- `ppu_fixture_load_mmc3` and registered `ppu.mmc3` (35589cc).
- Cases: BG $0000 / sprites $1000 gives 241; sprites in range 241; 8x16 empty slots (tile $FF) 241; both tables $0000 gives 0; BG $1000 gives 242 with the clock at dot 325 of a visible line; eight sprite fetches give one clock; unfiltered rises per line are at least 8 (filter negative control); `$2006` rise after 3 low CPU cycles clocks, after 2 does not; `$2007` step `$0FFF` to `$1000` clocks; NROM board with a counting hook receives zero `ppu_a12` calls.
- `cmake --workflow --preset ci` (419 tests) and `--preset hygiene` pass; `src/ppu.c` untouched.

## Deviations from Plan

**1. [Rule 1 - Plan expectation] BG $1000 frame count is 242** — Found during Task 2. The plan said one clock per line (241). The pre-render line has A12 low from the post-render lines, so the dot-5 rise clocks, and dot 325 clocks again. The test asserts 242 and the dot-325 placement on a visible line. No code change.

**2. Tasks committed together** — Task 1 and Task 2 share one file written in one pass; one commit (35589cc) covers both. No docs change: test-only, no behaviour change.

The 2C02 odd-frame double clock stays unmodelled, per PROH-09-03-02.

## Known Stubs

None.

## Self-Check: PASSED

tests/ppu/test_mmc3.c exists; commit 35589cc is on the branch; `git diff --stat origin/main -- src/ppu.c` is empty.
