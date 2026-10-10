---
phase: 07-uxrom-cnrom-and-axrom
verified: 2026-10-10T00:00:00Z
status: passed
score: 3/3 must-haves verified
covered_files:
  - .planning/phases/07-uxrom-cnrom-and-axrom/07-01-PLAN.md
  - .planning/phases/07-uxrom-cnrom-and-axrom/07-01-SUMMARY.md
  - .planning/phases/07-uxrom-cnrom-and-axrom/07-02-PLAN.md
  - .planning/phases/07-uxrom-cnrom-and-axrom/07-02-SUMMARY.md
  - .planning/phases/07-uxrom-cnrom-and-axrom/07-03-PLAN.md
  - .planning/phases/07-uxrom-cnrom-and-axrom/07-03-SUMMARY.md
  - .planning/phases/07-uxrom-cnrom-and-axrom/07-04-PLAN.md
  - .planning/phases/07-uxrom-cnrom-and-axrom/07-04-SUMMARY.md
  - src/mapper_axrom.c
  - src/mapper_cnrom.c
  - src/mapper_uxrom.c
  - tests/core/test_mapper_discrete.c
  - tests/holymapperel/roms.cmake
  - tests/runner/hashes.txt
covered_digest: "v3:sha256:c3e5d319ace0f50c32d3a1b367dd50e2ccaf8f0923f1a741e31ed55a552bf760"
behavior_unverified: 0
---

# Phase 7: UxROM, CNROM and AxROM Verification Report

**Goal:** Games on UxROM, CNROM and AxROM run, with bus conflicts where the board has them.
**Status:** passed. **Re-verification:** No.

| # | Truth (ROADMAP SC) | Status | Evidence |
|---|---|---|---|
| 1 | Holy Mapperel m2/m3/m7 report 0000 via CTest; frame hashes pinned | VERIFIED | Ran `ctest --preset ci -R holymapperel.m[237].decode`: "code 0000" for each, tests pass. Hashes for m2, m3 and m7 at frame 100 are in tests/runner/hashes.txt (lines 23-25). |
| 2 | Submapper-0 defaults: AND on mappers 2 and 3, none on mapper 7 | VERIFIED | tests/core/test_mapper_discrete.c: test_uxrom_and_submapper0, test_cnrom_and_submapper0_and_2, test_axrom_no_and_submapper0_and_1. Passing as core.mapper_discrete. src/mapper_axrom.c sets the bus-conflict watch for submapper 2 only. |
| 3 | Submapper 1 and 2 overrides; CNROM ignores CHR-ROM writes | VERIFIED | Tests: uxrom submapper 1 and 2, cnrom_no_and_submapper1, axrom_and_submapper2, test_cnrom_chr_rom_write_dropped (chr_w NULL, byte unchanged). |

Score: 3/3. Requirement BOARD-01 appears in all four plans and is SATISFIED. No orphaned Phase 7 requirements (REQUIREMENTS.md maps only BOARD-01).

Full run: `cmake --workflow --preset ci` ran locally here and 100% of the 374 tests passed. Per the orchestrator, CI run 38082253437 passed on all platforms.

## Advisory findings (not blocking)

- WR-01 (documentation accuracy): include/nesturbator.h:251, README.md:669 and src/mapper_axrom.c:8-10 say the AxROM reset vector "is read from bank 0". That holds only at power-on. A soft reset keeps the bank register, as on hardware, and no test pins it. Fix by adding "at power-on" to those three places, plus a nesturbator_reset test. Project rule 6 asks for accurate docs, so fix this soon. It does not affect the phase goal.
- WR-02, IN-01, IN-02 and IN-03: the README status paragraph is stale (IN-01), the hash_inventory key comparison is weak (WR-02), and the board-size limits are magic numbers (IN-03). None affects the goal. All five rows in 07-REVIEW-DISPOSITION.md are still `open`.
