---
phase: 09-mmc3
plan: 06
subsystem: fuzz-corpus
tags: [mmc3, fuzz, corpus, reject-seeds]
requires: [09-02, 09-05]
provides:
  - "eight D-16 reject seeds with manifest lines: mmc3-chr-none, mmc3-chr-nvram, mmc3-ram-1k, mmc3-ram-2k, mmc3-ram-16k, mmc3-battery-no-nvram, mmc3-nvram-no-battery, mmc3-work-and-nv"
  - "all 31 D-16 seeds in the corpus (5 valid, 26 reject)"
affects: [09-07]
tech-stack:
  added: []
  patterns: ["RAM and CHR shape rejects are full 128 KiB images; seed name equals the mmc3_rows label"]
key-files:
  created:
    - tests/fuzz/corpus/mmc3-chr-none
    - tests/fuzz/corpus/mmc3-chr-nvram
    - tests/fuzz/corpus/mmc3-ram-1k
    - tests/fuzz/corpus/mmc3-ram-2k
    - tests/fuzz/corpus/mmc3-ram-16k
    - tests/fuzz/corpus/mmc3-battery-no-nvram
    - tests/fuzz/corpus/mmc3-nvram-no-battery
    - tests/fuzz/corpus/mmc3-work-and-nv
  modified: [tests/roms/manifest.txt]
key-decisions:
  - "Seed manifest pins: ceead30a878e8f08ff82131127d20efb03a7cd34 (HEAD at creation of all eight seeds, and of mmc3-work-and-nv before its commit)"
requirements-completed: [MAP-03]
status: complete
duration: 5 min
completed: 2026-10-10
commits: 2
plan_head_before: ceead30a878e8f08ff82131127d20efb03a7cd34
plan_head_after: ebaf961e7bca9a7579bb48f98c295b52fa908bc1
actuals:
  tokens: 3000
  tasks: 2
  commits: 2
---

# Phase 9 Plan 06: MMC3 RAM and CHR shape reject seeds Summary

Eight reject seeds for the CHR-RAM, CHR-NVRAM and PRG-RAM shape rules are committed with manifest lines; each is refused by `nesturbator-run` with status 1 and matches a BAD_ row in `mmc3_rows`, completing the 31 D-16 seeds.

## Accomplishments

- Tracer: `mmc3-work-and-nv` end to end (353ccef): NES 2.0 header with 8 KiB work plus 8 KiB NV RAM, manifest line, `core.cartridge`, `fuzz.regress`, `manifest.sha256`, runner exit 1.
- The other seven seeds (ebaf961), built with an uncommitted scratchpad builder from zeros, a header, a reset vector and a 3-byte loop.
- Full count checked: 31 D-16 seed files and 31 manifest lines.
- `cmake --workflow --preset ci` and `--preset hygiene` pass; `hashes.txt` and `scoreboard.txt` unchanged against origin/main; behaviour revision still 5.

## Deviations from Plan

None - plan executed exactly as written.

## Known Stubs

None.

## Threat Flags

None.

## Self-Check: PASSED

All eight seeds exist; commits 353ccef and ebaf961 are on the branch; runner exits 1 on all eight; manifest has 31 D-16 lines.
