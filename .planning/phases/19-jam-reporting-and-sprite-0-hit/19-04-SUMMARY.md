---
phase: 19-jam-reporting-and-sprite-0-hit
plan: "04"
subsystem: ppu
tags: [ppu, oamaddr, sprite-0, tests, accuracycoin]
requires: ["19-03"]
provides:
  - "owned ppu.sprites cases for OAMADDR during rendering and sprite-0 hit (D-10, with the 2026-10-25 amendment)"
  - "OAMADDR cleared on every dot 257-320 while rendering (D-04)"
affects: [19-05, 19-06]
key-files:
  modified:
    - tests/ppu/test_sprites.c
    - src/ppu.c
key-decisions:
  - "D-10 amendment (owner, 2026-10-25, recorded under D-10 in 19-CONTEXT.md): cases 2, 4, 8, 9 and the all-Y=$FF case are guards that pass on main by design; cases 1, 3, 5, 6, 7 and the $FD case are reproducers"
  - "Reproducers run through an expected-fail wrapper (expect_red) that requires at least one CHECK failure and restores check_failures; it is removed per case when the fix lands"
  - "test_oam_dma_copies_page_and_wraps_destination runs with rendering off (Pitfall 2) so the OAMADDR clear does not touch its wrap assertions"
requirements-completed: [SPR-01, SPR-02]
completed: 2026-10-25
status: complete
commits: 2
plan_head_before: f1c0bafe1c0362465873aa32623eb525c5e42526
plan_head_after: 89d119d0cca5b1793dbb5738aa224cbac020f9b6
actuals:
  tokens: 50000
  tasks: 2
  commits: 2
---

# Phase 19 Plan 04: Owned sprite-0 tests and OAMADDR reset Summary

Owned `ppu.sprites` cases pin OAMADDR during rendering and the sprite-0 hit edge before any PPU change, then `sprite_access` clears OAMADDR on dots 257-320, turning `Sprite 0 Hit behavior`, `Suddenly Resize Sprite` and `Misaligned OAM DMA` into `pass 0x01`.

## D-10 amendment (owner, 2026-10-25)

D-10's original text says each test fails on today's main. The amendment makes the guards below pass on main by design: they stop an over-broad fix and are not reproducers. All expected values are constants labelled "(derived ...)" from NESdev text or AccuracyCoin error codes, never read from this PPU.

## Cases: role and status per commit

Commit 3 = `7130723` (tests only, no `src/` change). Commit 4 = `89d119d` (OAMADDR clear). "wrapped" means run through `expect_red`.

| D-10 | Test | Role | After commit 3 (main) | After commit 4 |
|------|------|------|-----------------------|----------------|
| 1 | test_oamaddr_cleared_after_dot_320_with_rendering | reproducer | red (wrapped) | green (normal check) |
| 2 | test_oamaddr_kept_with_rendering_off | guard | green | green |
| 3 | test_oamaddr_write_at_dot_300_is_cleared | reproducer | red (wrapped) | green (normal check) |
| 4 | test_vblank_oamaddr_write_reset_before_line_0 | guard | green | green |
| 5 | test_oamaddr_80_makes_sprite_32_sprite_zero | reproducer | red (wrapped) | still red (wrapped; Plan 19-05) |
| 6 | test_misaligned_oamaddr_81_copies_bytes_in_order | reproducer | red (wrapped) | still red (wrapped; Plan 19-05) |
| 7 | test_first_read_out_of_range_gives_no_sprite_zero (OAMADDR $80, OAM[0] in range) | reproducer | red (wrapped) | still red (wrapped; Plan 19-05) |
| 8 | test_covered_sprite_zero_still_hits | guard | green | green |
| 9 | test_hit_on_dot_x254_not_x255 | guard | green | green |
| - | test_all_oam_y_ff_gives_no_hit | guard | green | green |
| - | test_oamaddr_fd_ends_at_end_of_oam ($FD end-of-OAM start) | reproducer (red on main) | red (wrapped) | still red (wrapped; Plan 19-05; asan check of the OAM index bound happens after 19-05) |

`ppu.sprites` passes after both commits. Cases 5, 6, 7 and $FD stay wrapped until Plan 19-05 changes where evaluation starts.

## Commits

- `7130723` `test(ppu): add owned sprite-0 and OAMADDR cases ahead of the fix` - `tests/ppu/test_sprites.c | 221 +`; no `src/` file (`git show --stat` lists only the test).
- `89d119d` `fix(ppu): clear OAMADDR during sprite tile loading` - `src/ppu.c | 5 +`, `tests/ppu/test_sprites.c | 8 ++--` (2 files, 11 insertions, 2 deletions).

## Page-19 observation (commit 4)

`--accuracycoin-observe 19 --frames 1924` with caps 32/5/1800:

| Row | Result |
|-----|--------|
| Sprite 0 Hit behavior | pass 0x01 |
| Suddenly Resize Sprite | pass 0x01 |
| Misaligned OAM DMA | pass 0x01 |
| Arbitrary Sprite zero | timeout 0x03 (Plan 19-05) |
| Misaligned OAM behavior | not-run 0x03 (Plan 19-05) |

This matches the research measurement for D-04 alone.

## Commit-4 campaign diff (raw bytes, scored columns key/status/raw/frames)

Full campaign (`-DNESTURBATOR_ACCURACYCOIN_FULL=ON`, `accuracycoin.campaign` passed) `build/ci/accuracycoin/full/report.tsv` against `tests/accuracy/scoreboard.txt` (not edited here; Plan 19-06 re-records):

```
Misaligned OAM DMA            fail 0x06 1800  ->  pass 0x01 1800
Sprite 0 Hit behavior         fail 0x06 1800  ->  pass 0x01 1800
Suddenly Resize Sprite        fail 0x06 1800  ->  pass 0x01 1800
$2007 read w/ rendering       fail 0x06 503   ->  pass 0x01 569
INC $4014                     fail 0x0a 503   ->  pass 0x01 569
Rendering Flag Behavior       fail 0x0a 503   ->  pass 0x01 569
$2004 Stress Test             fail 0x0a 503   ->  fail 0x0a 569   (frames only)
$2007 Stress Test             fail 0x0a 503   ->  fail 0x0a 569   (frames only)
Address $2004 behavior        fail 0x0e 503   ->  fail 0x0e 569   (frames only)
t Register Quirks             pass 0x01 503   ->  pass 0x01 569   (frames only)
```

Five `draw`-status rows (CPU RAM, CPU Registers, PPU RAM, PPU Reset Flag, Palette RAM) appear in the report and not in the scoreboard; the scoreboard lists scored rows only. `Arbitrary Sprite zero` and `Misaligned OAM behavior` are unchanged. Three rows beyond D-19's list moved to pass (`$2007 read w/ rendering`, `INC $4014`, `Rendering Flag Behavior`); they sit on page 18, where a stale OAMADDR no longer changes the guest's path. Plan 19-06 should add them to the re-recorded scoreboard and the measurements note.

## Driver case

`accuracycoin.driver`'s page-19 case still passes after commit 4, so it was not re-targeted here. Plan 19-05 re-measures it after commit 5.

## Verification

- `ctest --preset ci -E '^accuracycoin\.activation$'`: 669/669 passed (the campaign test was not registered in that run; it was run separately and passed, 1/1).
- `ctest --preset ci -R '^ppu\.'`: 6/6 passed.
- `accuracycoin.activation` was not run (expected red until Plan 19-06).
- Acceptance: `grep -c '(derived' tests/ppu/test_sprites.c` is 12 (needs >= 10); `test_all_oam_y_ff_gives_no_hit` appears 2 times; `oam_addr = 0u` is in `sprite_access`.

## Deviations from Plan

**1. [Docs] README and `nesturbator.h` comments not updated here.** Found during: Task 2. CLAUDE.md rule 6 asks for docs with every behaviour change; this plan's file list and Plan 19-06 (revision 6 to 7, README revision table, header comment) own that update, so it lands in the phase's docs commit rather than commit 4. Files modified: none. No fix applied.

**2. [Placement] The OAMADDR clear follows the local declarations in `sprite_access`.** The plan says "first statement"; it is the first statement after the declarations (C17 style used elsewhere in the file). No behaviour difference.

**Total deviations:** 2 (both minor). **Impact:** none on correctness.

## Known Stubs

None.

## Threat Flags

None.

## Self-Check: PASSED

- `tests/ppu/test_sprites.c` and `src/ppu.c` exist; commits `7130723` and `89d119d` are ancestors of HEAD.
