---
phase: 09-mmc3
plan: 08
subsystem: tests
tags: [mmc3, holymapperel, test-roms, hashes]
requires: [09-06, 09-07]
provides:
  - "Holy Mapperel M4_P128K_CR8K and M4_P256K_C256K committed byte-identical with manifest lines and notices"
  - "holymapperel.m4tnrom.* and holymapperel.m4txrom.* CTests: code 0000, PRG RAM MISSING"
  - "47-row tests/runner/hashes.txt"
affects: [09-09]
tech-stack:
  added: []
  patterns: ["ignored *.nes ROMs are added with git add -f, as .gitignore documents"]
key-files:
  created: [tests/roms/hm/M4_P128K_CR8K.nes, tests/roms/hm/M4_P256K_C256K.nes]
  modified: [tests/roms/manifest.txt, THIRD-PARTY-NOTICES.md, tests/holymapperel/roms.cmake, tests/runner/hashes.txt, README.md]
key-decisions:
  - "Both mapper 4 ROMs pinned at N = 600 (frame 600 equals frame 1200 on the first attempt)"
requirements-completed: [BOARD-03]
status: complete
duration: 15 min
completed: 2026-10-10
commits: 2
plan_head_before: 94e57c06b18511e793156bf5192e3353ee467f70
plan_head_after: 86f7a50
actuals:
  tokens: 14000
  tasks: 2
  commits: 2
---

# Phase 9 Plan 08: Holy Mapperel mapper 4 ROMs Summary

Holy Mapperel's TNROM-like and TxROM-like mapper 4 ROMs run to code 0000 with `PRG RAM MISSING`, and their result-screen hashes are pinned.

## Accomplishments

- Download check: `holy-mapperel-bin-0.02.7z` sha256 equals 70f85671...ce7a. Extracted with bsdtar in a scratchpad directory. Per-file sha256 matched D-01 (edb30163...4125 and 27804182...9b17) before copying.
- Committed `M4_P128K_CR8K.nes` (commit 502342d) and `M4_P256K_C256K.nes` (commit 86f7a50) with manifest lines, `NESTURBATOR_HOLYMAPPEREL_MANIFEST` lines and notices entries. `M4_P128K` and `M4_P128K_CR32K` are not committed.
- Entries `m4tnrom` and `m4txrom` added to `tests/roms/roms.cmake` list; the existing HM loop produced the dump, decode and prgram tests.
- Pinned N: m4tnrom 600, m4txrom 600. Frame 600 and frame 1200 hash alike for both, so no larger N was needed.
- `hashes.txt`: 45 rows unchanged plus two new rows (47).
- README names mapper 1, 2, 3, 4 and 7 ROMs, lists both mapper 4 ROMs and uses 47 keys.
- Behaviour revision stays 5; scoreboard unchanged.

## Verification

`cmake --workflow --preset ci` passed (430 tests) and `cmake --workflow --preset hygiene` passed.

## Deviations from Plan

**[Rule 3 - Blocking] ROM files are gitignored.** `*.nes` is ignored; the ROMs were added with `git add -f`, the practice `.gitignore` documents for manifest-listed ROMs. Commits: 502342d, 86f7a50.

**Total deviations:** 1 auto-fixed. **Impact:** none.

## Known Stubs

None.

## Next

Ready for 09-09.

## Self-Check: PASSED

Both ROM files exist, both commits are ancestors of HEAD, and the ci and hygiene workflows pass.
