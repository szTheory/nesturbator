---
phase: 09-mmc3
plan: 02
subsystem: loader
tags: [mmc3, mapper-4, cartridge, fuzz-seeds, save-span]
requires: [09-01]
provides:
  - "mmc3_rows: 15 accept and 28 refuse rows named after D-16 seeds"
  - "core.save mapper 4 span sizing cases"
  - "five D-16 valid seeds with manifest lines"
affects: [09-04, 09-05, 09-06]
tech-stack:
  added: []
  patterns: ["refuse rows check bytes, size, prg_ram_size, mapper id and reset vector unchanged"]
key-files:
  created: [tests/fuzz/corpus/valid-mmc3, tests/fuzz/corpus/valid-mmc3-ines1, tests/fuzz/corpus/valid-mmc3-tgrom, tests/fuzz/corpus/valid-mmc3-sub4, tests/fuzz/corpus/valid-mmc3-prg16k]
  modified: [tests/core/test_cartridge.c, tests/core/test_save.c, tests/roms/manifest.txt, tests/core/test_mapper_mmc3.c]
key-decisions:
  - "Seed pin SHA 0d332633e56ed8d0c8d88e88483389c01044742e (HEAD after the rows commit)"
requirements-completed: [MAP-03, BOARD-03]
status: complete
duration: 20 min
completed: 2026-10-10
commits: 3
plan_head_before: d1b9a53c4373a443c43175712ce39976ed07c8fe
plan_head_after: 308b261d5cd3c6fb7fd0451250cdec78be4c59ad
actuals:
  tokens: 14000
  tasks: 3
  commits: 3
---

# Phase 9 Plan 02: Mapper 4 loader rows and valid seeds Summary

Named accept and refuse rows pin every D-12 mapper 4 shape, mapper 4 save-span sizing is tested, and the five D-16 valid seeds are committed and played by the runner.

## Accomplishments

- `mmc3_rows` and `test_mmc3_rows` (0d33263): the five valid seeds, limit rows, and a refuse row for every D-16 reject name (submappers 1/2/3/5/15, four-screen in iNES 1 and NES 2.0, mappers 118/119/206/249, PRG 8k/24k/1m/exponent-huge, CHR 512k/24k/RAM 16k/none/NVRAM, RAM 1k/2k/16k, battery and NVRAM mismatches). Each refusal asserts the previously loaded instance is unchanged.
- `test_mmc3_spans` in `core.save`: iNES 1 battery span 8192, iNES 1 work RAM without span, NES 2.0 (0,8K) and (8K,0), (0,0) with no RAM, trainer with 8 KiB work RAM.
- Seeds (c557c46): five files built from zeros, a header, reset vector `$8000` and `4C 00 80`; runner exits 0 on each for one frame; five manifest lines, pin `0d332633e56ed8d0c8d88e88483389c01044742e`.
- All rows passed against Plan 01's loader on first run; no `src/cartridge.c` change was needed.

## Re-tag scan

Scan of every `tests/fuzz/corpus/*` with the loader's NES 2.0 bit rule found mapper 4 only in: valid-mmc3, valid-mmc3-ines1, valid-mmc3-prg16k, valid-mmc3-sub4, valid-mmc3-tgrom. No existing seed re-tagged; mapper-155 and mapper-bits unchanged.

## Deviations from Plan

**1. [Rule 3 - Blocking] clang-format violations failed hygiene.format** — one in this plan's new struct comment, and four in `tests/core/test_mapper_mmc3.c` left by Plan 01. Fixed with clang-format 18 (formatting only), commit 308b261.

Rows and `core.save` cases were committed together in one commit (Task 1 and 2 test content), then the seeds in a second, rather than three separate task commits; content matches the plan.

## Known Stubs

None.

## Verification

- `cmake --workflow --preset ci`: 418 of 418 passed. `cmake --workflow --preset hygiene`: 8 of 8 passed.
- `tests/runner/hashes.txt` and `tests/accuracy/scoreboard.txt` identical to origin/main; `NESTURBATOR_BEHAVIOUR_REVISION` 5.

## Next

Ready for 09-03.

## Self-Check: PASSED
