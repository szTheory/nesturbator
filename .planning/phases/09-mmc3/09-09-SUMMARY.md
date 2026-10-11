---
phase: 09-mmc3
plan: 09
subsystem: tests
tags: [mmc3, holymapperel, derived-roms, save, hashes]
requires: [09-08]
provides:
  - "holymapperel-derive and hm_derive.cmake: sha256-checked, build-time header-patched ROM copies"
  - "holymapperel.m4w8k.* and holymapperel.m4tkrom.* CTests, holymapperel.derive.refuse"
  - "roms.cmake derived/ entries, save span size and extra .sav byte check fields"
  - "50-row tests/runner/hashes.txt"
affects: []
tech-stack:
  added: []
  patterns: ["derived ROMs built by an ALL custom target, never committed, no manifest line"]
key-files:
  created: [tests/holymapperel/derive.c, tests/cmake/hm_derive.cmake]
  modified: [tests/holymapperel/roms.cmake, tests/CMakeLists.txt, tests/cmake/write_hashes.cmake, tests/cmake/hash_inventory.cmake, tests/runner/hashes.txt, README.md]
key-decisions:
  - "Both derived ROMs pinned at N = 600 (frame 600 equals frame 1200 on the first attempt)"
  - "Measured .sav byte 0 after m4tkrom run 1 is 0xB6 (not 0x6B), so write protect works; pinned 0=b6"
requirements-completed: [BOARD-03]
status: complete
duration: 25 min
completed: 2026-10-10
commits: 2
plan_head_before: 2a942fb73b1c74ba670a0283ae593f4e383e0a31
actuals:
  tokens: 12000
  tasks: 2
  commits: 2
---

# Phase 9 Plan 09: Derived MMC3 PRG RAM ROMs Summary

Two build-time header-patched copies of M4_P128K_CR8K (8 KiB work RAM; battery) report 0000 with `8K PRG RAM OK`, prove `$A001` enable and write protect, and are pinned in a 50-row `hashes.txt`.

## Accomplishments

- `holymapperel-derive` patches the iNES header (`set@OFF=HH`, `or@OFF=HH`); `hm_derive.cmake` requires the base sha256 to equal its manifest line and the output size to equal the base size. `holymapperel-derived` (ALL) builds the copies so CI's bare `write_hashes.cmake` finds them. `holymapperel.derive.refuse` proves a wrong base is refused.
- m4w8k: 0000 and exactly `8K PRG RAM OK`.
- m4tkrom: two-run chain; run 1 `.sav` is 8192 bytes, SAVEDATA at 0x100, byte 0 = 0xB6 (measured; 0x6B would mean write protect was ignored); run 2 shows `+ BATTERY`. Eight m4tkrom tests.
- Entry fields 6 (span size) and 7 (`offset=hex` extra check) added; `hash_inventory.cmake` now matches `:save` followed by `:` or end.
- Pinned N: m4w8k 600, m4tkrom 600 (run 2 also 600). `hashes.txt`: 47 rows unchanged plus 3 (50). Run 1 of m4tkrom hashes alike to m4w8k (same screen); the saved run differs.
- README describes derived copies, the new fields and 50 keys. Revision stays 5, scoreboard unchanged.

## Deviations from Plan

**[Rule 3 - Blocking] `hash_inventory.cmake` matched `:save$`.** The m4tkrom entry has trailing fields, so the saved-row count would have missed it; changed to `:save(:|$)`. Included in Task 1 commit.

Otherwise none; no mapper fix was needed.

## Verification

`cmake --workflow --preset ci` passed (442 tests) and `cmake --workflow --preset hygiene` passed. No derived file is tracked.

## Known Stubs

None.

## Self-Check: PASSED
