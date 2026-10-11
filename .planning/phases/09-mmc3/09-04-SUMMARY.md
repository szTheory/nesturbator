---
phase: 09-mmc3
plan: 04
subsystem: fuzz-corpus
tags: [mmc3, fuzz, corpus, reject-seeds]
requires: [09-02]
provides:
  - "nine D-16 identity reject seeds with manifest lines: mmc3-submapper-1/2/3/5/15, mmc3-four-screen, mmc3-four-screen-nes2, mapper-118, mapper-119"
affects: [09-07]
tech-stack:
  added: []
  patterns: ["each seed is the valid-mmc3 image with only the refused field changed; seed name equals the mmc3_rows label"]
key-files:
  created:
    - tests/fuzz/corpus/mmc3-submapper-1
    - tests/fuzz/corpus/mmc3-submapper-2
    - tests/fuzz/corpus/mmc3-submapper-3
    - tests/fuzz/corpus/mmc3-submapper-5
    - tests/fuzz/corpus/mmc3-submapper-15
    - tests/fuzz/corpus/mmc3-four-screen
    - tests/fuzz/corpus/mmc3-four-screen-nes2
    - tests/fuzz/corpus/mapper-118
    - tests/fuzz/corpus/mapper-119
  modified: [tests/roms/manifest.txt]
key-decisions:
  - "Seed manifest pin is 47fa6df2c9d8d84aa617ca20167e4de6329ec2f0 (HEAD at creation)"
requirements-completed: [MAP-03]
status: complete
duration: 6 min
completed: 2026-10-10
commits: 2
plan_head_before: 47fa6df2c9d8d84aa617ca20167e4de6329ec2f0
plan_head_after: 27546dc
actuals:
  tokens: 6000
  tasks: 2
  commits: 2
---

# Phase 9 Plan 04: MMC3 identity reject seeds Summary

Nine zero-filled reject seeds (MMC6 and other submappers, both four-screen forms, mappers 118 and 119) are committed with manifest lines; each is refused by `nesturbator-run` with status 1 and matches a BAD_ row in `mmc3_rows`.

## Accomplishments

- Tracer: `mmc3-submapper-1` end to end (f5da79a): seed, manifest line, `fuzz.regress`, `core.cartridge`, `manifest.sha256`, runner exit 1.
- The other eight seeds (27546dc), built with the scratchpad builder (not committed) from zeros, a header, a reset vector and a 3-byte loop.
- `cmake --workflow --preset ci` and `--preset hygiene` pass.

## Deviations from Plan

None - plan executed exactly as written.

## Known Stubs

None.

## Self-Check: PASSED

All nine seeds exist; commits f5da79a and 27546dc are on the branch; runner exits 1 on all nine; manifest has 9 matching lines.
