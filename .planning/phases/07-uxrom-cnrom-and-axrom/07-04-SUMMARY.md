---
phase: 07-uxrom-cnrom-and-axrom
plan: 04
subsystem: tests
tags: [holymapperel, test-roms, frame-decoder, hash-inventory, board-01]
requires:
  - phase: 07-uxrom-cnrom-and-axrom
    provides: UxROM, CNROM and AxROM boards, loader rows, fuzz seeds (plans 01-03)
provides:
  - Holy Mapperel M2, M3 and M7 v0.02 ROMs with manifest lines and notices
  - holymapperel-decode, a test-only reader of the result code from the runner's frame
  - holymapperel.* CTests and one shared ROM list (tests/holymapperel/roms.cmake)
  - three new rows in tests/runner/hashes.txt (39 in total)
affects: [phase-08, phase-09]
tech-stack:
  added: []
  patterns: [shared CMake ROM list included by tests, write_hashes and hash_inventory; frame N / 2N stability check at hash-write time]
key-files:
  created:
    - tests/roms/hm/M2_P128K_CR8K_V.nes
    - tests/roms/hm/M3_P32K_C32K_H.nes
    - tests/roms/hm/M7_P128K_CR8K.nes
    - tests/holymapperel/hm_decode.h
    - tests/holymapperel/decode.c
    - tests/holymapperel/test_decode.c
    - tests/holymapperel/roms.cmake
  modified:
    - tests/roms/manifest.txt
    - THIRD-PARTY-NOTICES.md
    - tests/CMakeLists.txt
    - tests/cmake/write_hashes.cmake
    - tests/cmake/hash_inventory.cmake
    - tests/runner/hashes.txt
    - README.md
    - src/mapper_axrom.c
    - src/mapper_cnrom.c
    - src/mapper_uxrom.c
key-decisions:
  - "N = 100 for all three ROMs: the result screen is up by frame 100 (absent at frame 60 for M2) and frame 100 hashes like frame 200."
  - "Wrong-format and unreadable input exit 2, with the message holymapperel: not a 256x240 P6 image."
requirements-completed: []
status: complete
commits: 4
plan_head_before: 74a944b6d9db86b5601cf42b1f44a277d4faef07
plan_head_after: 214a118e9132380709f6cbce47b2a8178ed35556
actuals:
  tokens: 30000
  tasks: 3
  commits: 4
completed: 2026-10-10
---

# Phase 7 Plan 04: Holy Mapperel oracle for mapper 2, 3 and 7 Summary

Holy Mapperel's mapper 2, 3 and 7 ROMs run through `nesturbator-run`, a test-only decoder reads the dumped frame, all three report `0000` on the first boot, and one frame hash per ROM is pinned from one shared ROM list; the phase branch's CI run is green on all jobs.

BOARD-01 is left open in REQUIREMENTS.md; the orchestrator closes it through `phase.complete`.

## Accomplishments

- Asset sha256 `70f85671...4ce7a` (17,964 bytes) and all three ROM sha256 values matched D-01 before commit; headers checked (byte 10 is 0). The twins are not committed. Manifest lines, a "Holy Mapperel" notices section with the verbatim zlib LICENSE and the intro mention are in the same commit as the ROMs. The ROMs needed `git add -f` (`*.nes` is ignored by design).
- `holymapperel-decode` prints `holymapperel: code 0000 (WRAM 0, PRG 0, IRQ 0, CHR 0)` only on the exit-0 path; exits 1 with digit meanings (C0DE as never finished) and 2 for no result screen, unreadable cell or bad image.
- Confirmed screen layout on first boot, with no correction needed: anchor D and E at x 16 and 24, y 64; digits at x 192, 200, 208, 216, y 64. Research assumptions A1-A4 held: the CHR-RAM ROMs (M2, M7) show the same glyphs, M2/M3/M7 all read 0000 with no board-code change, and the submapper-0 AND was not an issue.
- Pinned N: M2 = 100, M3 = 100, M7 = 100. Frame 60 of M2 has no result screen yet (exit 2); 100, 150, 200, 300 and 600 all read 0000, and frame N and 2N hash the same for all three.
- 13 `holymapperel.*` CTests: 3 dump, 3 decode, early dump, testcard, early, badsize, unit, glyphs. The unit test paints frames for every branch and colour independence; the glyph test equals tiles $30-$39 and $01-$06 of the committed M3 ROM.
- `write_hashes.cmake` and `hash_inventory.cmake` (real check and fixture) include `tests/holymapperel/roms.cmake`; the total is `36 + list length`. `write_hashes.cmake` fails with "the result screen is not static" if frame N and 2N differ. `hashes.txt` has 39 rows; the diff against `origin/main` adds exactly three `holymapperel/` rows and changes none.
- README describes the ROMs, the tests, the exit statuses, the 39-key inventory and the one-line way to add a board ROM.

## Task Commits

1. Task 1 (tracer, ROMs, notices, decoder, M2 chain): `8f5f86e`
2. Task 2 (M3, M7, unit, glyph and exit-2 tests): `0338200`
3. Deviation fix (float scan): `30f5b66`
4. Task 3 (shared-list hashes, README): `214a118`

## Deviations from Plan

**1. [Rule 1 - Bug] `abi.float_scan` failed in the `nofp` lane on the board comments**
- **Found during:** Task 3 phase gate (`cmake --workflow --preset nofp`)
- **Issue:** The comments in `src/mapper_uxrom.c`, `mapper_cnrom.c` and `mapper_axrom.c` (plans 01 and 02) quote the NESdev page "NES 2.0 submappers"; the float scan reads comments and flags `2.0`. This would have failed the Linux `nofp` CI legs.
- **Fix:** Reworded to "NES 2 submappers". Comment text only.
- **Files modified:** src/mapper_axrom.c, src/mapper_cnrom.c, src/mapper_uxrom.c
- **Verification:** `abi.float_scan` passes; `ci` workflow 374 of 374; CI `nofp` legs green.
- **Commit:** `30f5b66`

**Total deviations:** 1 auto-fixed (1 bug). **Impact:** none on behaviour.

## Verification

- `cmake --workflow --preset ci`: 374 of 374 pass. `asan`: 374 of 374 pass. `hygiene` workflow and `scripts/hygiene.sh` exit 0.
- Local `nofp` (macOS): only `abi.undefined_symbols` fails (the known pre-existing `___stack_chk_*` failure); `abi.float_scan` passes after the fix.
- `hashes.txt` gate: 39 lines, exactly 3 added `holymapperel/` rows, no removed rows, scoreboard unchanged, behaviour revision 5.
- Pull request: https://github.com/szTheory/nesturbator/pull/32 (draft, title `feat: UxROM, CNROM and AxROM boards`).
- CI run `38082253437` for HEAD `214a118`, https://github.com/szTheory/nesturbator/actions/runs/38082253437: conclusion success. Jobs, all success: build on ubuntu-24.04, ubuntu-24.04-arm, macos-15, macos-15-intel, windows-2025, windows-11-arm; nofp (ubuntu-24.04, ubuntu-24.04-arm); asan; hygiene; title; retroarch-e2e; hash-equality; CI required.

## Next step for the owner

Review PR 32 and merge it when ready (it stays in draft); run `/gsd-verify-work 7` first.

## Known Stubs

None.

## Threat Flags

None.

## Self-Check: PASSED

The three ROMs, `hm_decode.h`, `decode.c`, `test_decode.c` and `roms.cmake` exist; commits `8f5f86e`, `0338200`, `30f5b66` and `214a118` are ancestors of HEAD.
