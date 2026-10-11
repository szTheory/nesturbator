---
phase: 09-mmc3
plan: 05
subsystem: fuzz-corpus
tags: [mmc3, fuzz, corpus, reject-seeds]
requires: [09-02]
provides:
  - "nine D-16 reject seeds with manifest lines: mapper-206, mapper-249, mmc3-prg-8k, mmc3-prg-24k, mmc3-prg-1m, mmc3-prg-exp-huge, mmc3-chr-512k, mmc3-chr-24k, mmc3-chr-ram-16k"
affects: [09-07]
tech-stack:
  added: []
  patterns: ["oversize rejects are bare 16-byte headers; the mmc3_rows row builds the full image; seed name equals the row label"]
key-files:
  created:
    - tests/fuzz/corpus/mapper-206
    - tests/fuzz/corpus/mapper-249
    - tests/fuzz/corpus/mmc3-prg-8k
    - tests/fuzz/corpus/mmc3-prg-24k
    - tests/fuzz/corpus/mmc3-prg-1m
    - tests/fuzz/corpus/mmc3-prg-exp-huge
    - tests/fuzz/corpus/mmc3-chr-512k
    - tests/fuzz/corpus/mmc3-chr-24k
    - tests/fuzz/corpus/mmc3-chr-ram-16k
  modified: [tests/roms/manifest.txt]
key-decisions:
  - "Seed manifest pins: f88d7cfc068ac428c9f433f8a68372fcf600d828 (mmc3-prg-exp-huge) and e70b61106df977517b5c1549a1f32a489ab42f01 (the other eight; HEAD at creation)"
requirements-completed: [MAP-03]
status: complete
duration: 5 min
completed: 2026-10-10
commits: 2
plan_head_before: f88d7cfc068ac428c9f433f8a68372fcf600d828
plan_head_after: c4f3b20a099b49d0717ab1af7ec89a74604cb292
actuals:
  tokens: 3000
  tasks: 2
  commits: 2
---

# Phase 9 Plan 05: MMC3 mapper and size reject seeds Summary

Nine reject seeds (mappers 206 and 249, PRG 8k/24k/1m/exponent-huge, CHR 512k/24k, CHR-RAM 16k) are committed with manifest lines; each is refused by `nesturbator-run` with status 1 and matches a BAD_ row in `mmc3_rows`.

## Accomplishments

- Tracer: `mmc3-prg-exp-huge` end to end (e70b611): bare 16-byte header, manifest line, `core.cartridge`, `fuzz.regress`, `manifest.sha256`, runner exit 1.
- The other eight seeds (c4f3b20): bare headers for `mmc3-prg-1m` and `mmc3-chr-512k`, byte 9 exponent form for `mmc3-prg-24k` and `mmc3-chr-24k`, full images otherwise. Built with an uncommitted scratchpad builder from zeros, a header, a reset vector and a 3-byte loop.
- `cmake --workflow --preset ci` and `--preset hygiene` pass.

## Deviations from Plan

None - plan executed exactly as written.

## Known Stubs

None.

## Self-Check: PASSED

All nine seeds exist; commits e70b611 and c4f3b20 are on the branch; runner exits 1 on all nine; manifest has 9 matching lines.
