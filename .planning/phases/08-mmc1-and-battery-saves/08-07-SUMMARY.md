---
phase: 08-mmc1-and-battery-saves
plan: 07
subsystem: tests
tags: [holymapperel, mmc1, battery-save, sxrom, hashes]
requires:
  - phase: 08-mmc1-and-battery-saves
    provides: plan 03 (PRG RAM row decoder), plan 05 (MMC1 board), plan 06 (runner --save-dir)
provides:
  - five Holy Mapperel mapper 1 ROMs admitted with manifest lines and notices
  - holymapperel.m1sxrom two-run save chain showing + BATTERY on run 2
  - check_sav.cmake and clean_dir.cmake scripts
  - 45-row tests/runner/hashes.txt
affects: [phase-08-verification]
tech-stack:
  added: []
  patterns: [optional :save roms.cmake field selects a fixture-ordered two-run chain]
key-files:
  created:
    - tests/cmake/check_sav.cmake
    - tests/cmake/clean_dir.cmake
    - tests/roms/hm/M1_P128K_CR8K.nes
    - tests/roms/hm/M1_P128K_C32K_W8K.nes
    - tests/roms/hm/M1_P128K_C128K_S8K.nes
    - tests/roms/hm/M1_P512K_CR8K_S8K.nes
    - tests/roms/hm/M1_P512K_CR8K_S32K.nes
  modified:
    - tests/holymapperel/roms.cmake
    - tests/CMakeLists.txt
    - tests/cmake/write_hashes.cmake
    - tests/cmake/hash_inventory.cmake
    - tests/runner/hashes.txt
    - tests/roms/manifest.txt
    - THIRD-PARTY-NOTICES.md
    - README.md
key-decisions:
  - "Pinned N per ROM: m1sgrom 100, m1sjrom 100, m1skrom 100, m1surom 200, m1sxrom 400 (smallest round N whose frame N and 2N hash alike; the SXROM 32K RAM test finishes between frames 360 and 380)."
  - "Phase 7 WR-01/WR-02 not folded; hash_inventory.cmake only gains the .saved key and count."
requirements-completed: [BOARD-02, SAVE-02]
status: complete
commits: 4
plan_head_before: 882e6731d95228a3f4e8297bd900a5014a56c214
plan_head_after: 935b0cac269ab6c03daf7459161281c68b67ed5f
actuals:
  tokens: 30000
  tasks: 3
  commits: 4
completed: 2026-10-10
---

# Phase 8 Plan 07: MMC1 Holy Mapperel ROMs and the SXROM battery round trip Summary

All five Holy Mapperel mapper 1 ROMs report 0000 with their exact PRG RAM row, and the SXROM ROM's second `--save-dir` run reads `32K PRG RAM OK + BATTERY` from the save its first run wrote.

## Accomplishments

- Five ROMs byte-identical to the v0.02 asset (sha256 and asset checked before copying), each with a manifest line and a THIRD-PARTY-NOTICES entry; none of the twins or `.sav` files committed.
- PRG RAM rows: `PRG RAM MISSING` (m1sgrom), `8K PRG RAM OK` (m1sjrom, m1skrom, m1surom), `32K PRG RAM OK` run 1 and `32K PRG RAM OK + BATTERY` run 2 (m1sxrom).
- SXROM chain `holymapperel.m1sxrom.{clean, run1.dump/decode/prgram/sav, run2.dump/decode/prgram}` ordered by fixtures; `.sav` is 32,768 bytes with `5341564544415441` at 0x100.
- `write_hashes.cmake` runs `:save` entries twice on a fresh save directory, pins run 2 under `<key>.saved`, and fails if the two runs hash alike. `hash_inventory.cmake` counts and lists the `.saved` key.
- `hashes.txt`: 45 rows, 39 existing byte-identical, six added (diff against origin/main verified). Scoreboard unchanged, behaviour revision stays 5.
- README describes the ROMs, `.prgram` checks, two-run chain, 45 rows and the `:save` field.
- `cmake --workflow --preset ci`: 418 of 418 pass; hygiene preset passes.

## Task Commits

1. Task 1 (tracer): `d7cd3e2` (ROM admission), `22a7132` (chain and pins)
2. Task 2: `5c03c38`
3. Task 3: `935b0ca`

## Deviations from Plan

**1. [Rule 3 - Blocker] ROMs are gitignored**
- **Found during:** Task 1 commit
- **Issue:** `.gitignore` has `*.nes`; the repo's documented route for manifest-listed test ROMs is `git add -f`.
- **Fix:** staged the ROMs with `git add -f`.

**2. [Rule 3 - Added file] `tests/cmake/clean_dir.cmake`**
- The plan allowed a `-P`-free command list or fixtures; a two-line `-P` script keeps the clean step portable and follows the script policy. Not in the plan's file list.

**3. First-boot N.** The plan's 600 start was used for the first look; SXROM needed N=400 because its result screen appears only after frame 360.

**Total deviations:** 2 auto-fixed (Rule 3), 1 note. **Impact:** none on scope.

## Known Stubs

None.

## Threat Flags

None.

## Self-Check: PASSED
