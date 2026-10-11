---
phase: 08-mmc1-and-battery-saves
plan: 03
subsystem: tests
tags: [holymapperel, decoder, prg-ram, glyphs]
requires:
  - phase: 08-mmc1-and-battery-saves
    provides: plan 01 (shared tests/CMakeLists.txt edit order)
provides:
  - holymapperel-decode --prg-ram reading the PRG RAM text row
  - reference pixel (0,7) so the glyph M decodes
  - roms.cmake fourth field (PRG RAM text) and optional fifth field :save
  - holymapperel.m2/m3/m7.prgram tests
affects: [phase-08-plan-07]
tech-stack:
  added: []
  patterns: [decoder mode asserted by a quote-anchored regex]
key-files:
  created: []
  modified:
    - tests/holymapperel/hm_decode.h
    - tests/holymapperel/decode.c
    - tests/holymapperel/test_decode.c
    - tests/holymapperel/roms.cmake
    - tests/CMakeLists.txt
key-decisions:
  - "Text glyphs live in a second table (hm_text_glyph) beside the hex table hm_glyph; digits 1-9 and C, D, F are reused from hm_glyph. The font draws O and 0 alike, so a 0 in row text reads as O (no expected string has a 0)."
requirements-completed: [BOARD-02, SAVE-02]
status: complete
commits: 3
plan_head_before: 2dadc72c84bbface15ee252e22db49d7d6ba9cad
plan_head_after: 6f1bf61
actuals:
  tokens: 12000
  tasks: 2
  commits: 3
completed: 2026-10-10
---

# Phase 8 Plan 03: PRG RAM row decoder Summary

The Holy Mapperel frame decoder now reads the PRG RAM text row (`--prg-ram`), including the letter M via a (0,7) reference pixel, and the three Phase 7 ROMs assert `PRG RAM MISSING`.

## Accomplishments

- `hm_decode.h`: lit pixel means differs from the cell's pixel (0,7); `hm_text_glyph` (space, `+`, A B E G I K L M N O P R S T Y); `hm_text_cell`; `hm_read_text_row(rgb, y, out, cap)` reading from x = 16, up to 28 cells, `?` for unknown, trailing spaces trimmed.
- `decode.c`: `holymapperel-decode --prg-ram FILE` prints `holymapperel: prg ram "<text>"`, exit 0, or 2 on an unknown cell or bad input. Without the flag the output is unchanged.
- `roms.cmake` entries are `<key>:<file>:<N>:<prg ram text>[:save]`; m2, m3, m7 carry `PRG RAM MISSING`. `tests/CMakeLists.txt` adds `holymapperel.<key>.prgram` with `prg ram "<text>"` as the regex; the `:save` field is parsed and unused here.
- Unit cases: `32K PRG RAM OK + BATTERY`, `PRG RAM MISSING`, swapped colours, blank row, one stray pixel gives `?`, and a lone `M`. `holymapperel.glyphs` now also checks every text glyph against the M3 ROM CHR at tile ASCII & $3F.
- Confirmed row coordinates on the first boot dump: the PRG RAM line is on row 6, pixel y = 48, text starting at x = 16; M2, M3 and M7 at frame 100 all read `PRG RAM MISSING`.
- `cmake --workflow --preset ci`: 378 of 378 pass; hygiene preset passes. `tests/runner/hashes.txt` is identical to origin/main.

## Task Commits

1. Task 1 (tracer): `4ec3700`
2. Task 2: `2011e8e`
3. Format fix: `6f1bf61`

The tracer's `<verify>` was re-run (all holymapperel and write_hashes tests pass; hashes.txt unchanged) before Task 2.

## Deviations from Plan

**1. [Rule 3 - Blocker] clang-format 18 violation in the new test**
- **Found during:** final hygiene run
- **Fix:** `clang-format -i` on the test file; whitespace only.
- **Commit:** 6f1bf61

**2. Task 2 TDD order.** The decoder (Task 1) landed before its unit cases, as the plan orders the tasks (tracer first), so Task 2 has no separate failing-test commit; the cases passed on first run against the existing decoder.

**Total deviations:** 1 auto-fixed (Rule 3). **Impact:** none on scope.

## Known Stubs

None. The `:save` field is parsed and unused until Plan 07, as the plan states.

## Threat Flags

None.

## Self-Check: PASSED

Commits `4ec3700`, `2011e8e`, `6f1bf61` are ancestors of HEAD; the plan's greps match (`prg-ram` in decode.c, three `PRG RAM MISSING` entries, `BATTERY` and `MISSING` in test_decode.c); three `.prgram` tests listed and passing.
