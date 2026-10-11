---
phase: quick-261011-h5x
plan: 01
status: complete
requirements: [TUNE-06]
commits: 2
---

# Quick 261011-h5x: Reset retention tests

Pinned with tests that `nesturbator_reset` keeps AxROM, UxROM and CNROM bank registers and MMC3 PRG RAM bytes, and corrected the AxROM reset-vector wording (bank 0 at power-on only, selected bank after a soft reset).

## Commits
- 85d7216 test(core): AxROM soft reset reads the vector from the selected bank. This commit also holds the UxROM and CNROM tests, because they share tests/core/test_mapper_discrete.c with the AxROM test.
- d05fc7d test(core): MMC3 PRG RAM survives nesturbator_reset.

## Changes
- tests/core/test_mapper_discrete.c: test_axrom_reset_keeps_bank, test_uxrom_reset_keeps_bank, test_cnrom_reset_keeps_bank
- tests/core/test_mapper_mmc3.c: test_reset_keeps_prg_ram (enabled and write-protected)
- include/nesturbator.h, README.md: AxROM wording and mapper-registers-kept note on reset
- src/mapper_axrom.c: comment only

## Deviations
None. No retention test failed; the core needed no change. Submapper 1 was accepted for UxROM and CNROM.

## Verification
`cmake --workflow --preset ci`: 480/480 tests passed, including core.mapper_discrete and core.mapper_mmc3.

## Self-Check: PASSED
