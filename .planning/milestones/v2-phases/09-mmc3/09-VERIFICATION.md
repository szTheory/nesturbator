---
phase: 09-mmc3
verified: 2026-10-10T12:00:00Z
status: passed
score: 4/4 must-haves verified
covered_files:
  - .planning/phases/09-mmc3/09-01-PLAN.md
  - .planning/phases/09-mmc3/09-01-SUMMARY.md
  - .planning/phases/09-mmc3/09-02-PLAN.md
  - .planning/phases/09-mmc3/09-02-SUMMARY.md
  - .planning/phases/09-mmc3/09-03-PLAN.md
  - .planning/phases/09-mmc3/09-03-SUMMARY.md
  - .planning/phases/09-mmc3/09-04-PLAN.md
  - .planning/phases/09-mmc3/09-04-SUMMARY.md
  - .planning/phases/09-mmc3/09-05-PLAN.md
  - .planning/phases/09-mmc3/09-05-SUMMARY.md
  - .planning/phases/09-mmc3/09-06-PLAN.md
  - .planning/phases/09-mmc3/09-06-SUMMARY.md
  - .planning/phases/09-mmc3/09-07-PLAN.md
  - .planning/phases/09-mmc3/09-07-SUMMARY.md
  - .planning/phases/09-mmc3/09-08-PLAN.md
  - .planning/phases/09-mmc3/09-08-SUMMARY.md
  - .planning/phases/09-mmc3/09-09-PLAN.md
  - .planning/phases/09-mmc3/09-09-SUMMARY.md
  - .planning/phases/09-mmc3/09-10-PLAN.md
  - .planning/phases/09-mmc3/09-10-SUMMARY.md
  - .planning/phases/09-mmc3/09-11-PLAN.md
  - .planning/phases/09-mmc3/09-11-SUMMARY.md
  - .planning/phases/09-mmc3/09-12-PLAN.md
  - .planning/phases/09-mmc3/09-12-SUMMARY.md
  - include/nesturbator.h
  - runner/main.c
  - src/cartridge.c
  - src/mapper.h
  - src/mapper_mmc3.c
  - src/ppu.c
covered_digest: "v3:sha256:139539744efb2d8c07d3e37e01f928d38c199afc08d15706bd66ba69819655b7"
behavior_unverified: 0
---

# Phase 9: MMC3 Verification Report

**Phase Goal:** MMC3 games run with the scanline IRQ clocked by real PPU A12 rises, and the loader accepts exactly the six v2 mappers.
**Status:** passed. **Re-verification:** No.

## Observable Truths

| # | Truth (ROADMAP SC) | Status | Evidence |
|---|---|---|---|
| 1 | Synthetic tests: A12 rise clocks only after 3 M2 falls low; sprite fetches clock even for empty slots; NEC on submapper 4 | VERIFIED | `src/mapper_mmc3.c` `mmc3_ppu_a12` filters on `cpu_cycle - low_cycle >= 3`; `clock_counter` has the NEC `old != 0 \|\| reload_seen` rule. `core.mapper_mmc3` (#107) and `ppu.mmc3` (#115, empty-slot 8x8 and 8x16, negative control) pass in a fresh `ci` run. |
| 2 | Holy Mapperel mapper 4 ROMs report 0000, including `$A001` enable and write protect | VERIFIED | `holymapperel.m4tnrom/m4txrom` plus derived `m4w8k` and `m4tkrom` run1/run2/.sav/.sav_byte tests all pass. The derived copies make the `$A001` path non-vacuous (row text `8K PRG RAM OK`, `.sav` byte 0 = 0xB6). Two ROMs are in `tests/roms/manifest.txt`. |
| 3 | Nightly fetches blargg ROMs at the pin, checks SHA-256, passes Sharp tests; NEC tests recorded unsupported | VERIFIED | I ran `cmake --workflow --preset mmc3-oracle` here: 13/13 pass (fetch plus 12 rows). `oracle.txt` marks `6-MMC3_alt` (0x02) and `5.MMC3_rev_A` (0x03) unsupported, and the other ten `pass`. `pins.txt` holds the commit and 12 hashes. The `mmc3-oracle` job exists in `.github/workflows/nightly.yml` (contents: read, pinned SHAs, evidence upload). |
| 4 | Loader accepts 0,1,2,3,4,7; runner rejects others, MMC6 and four-screen with a message and non-zero status; fuzz corpus and cartridge tests updated | VERIFIED | `runner.reject.{four_screen,mmc6,mapper,generic,truncated}` pass. The corpus has the 5 valid `valid-mmc3*` and about 25 reject seeds. The README text matches the runner's messages. The public header documents the mapper 4 profile. |

**Score:** 4/4. No truth is behaviour-unverified: the state and ordering invariants are covered by named passing tests (`core.mapper_mmc3`, `ppu.mmc3`, oracle rows).

## Requirements Coverage

| Requirement | Plans | Status |
|---|---|---|
| BOARD-03 | 09-01, 02, 03, 08, 09, 10, 11, 12 | SATISFIED (SC 1-3) |
| MAP-03 | 09-01, 02, 04, 05, 06, 07 | SATISFIED (SC 4) |

REQUIREMENTS.md maps only these two IDs to Phase 9, and both are claimed by plans. There are no orphans.

## Commands run

- `cmake --workflow --preset ci`: 446/446 pass.
- `ctest --test-dir build/ci -j8`: 100% of 446.
- `cmake --workflow --preset hygiene`: pass.
- `cmake --workflow --preset mmc3-oracle`: 13/13 pass (network).
- `tests/runner/hashes.txt` diff against main is additions only (5 m4 rows). No existing hash moved. `tests/accuracy/scoreboard.txt` is unchanged. `NESTURBATOR_BEHAVIOUR_REVISION` is still 5.
- No TODO/FIXME/XXX/TBD in `src/mapper_mmc3.c`, `runner/main.c` or `tests/mmc3/*.c`.

## Executor deviations weighed

- **09-03, 242 clocks per frame for BG at `$1000`:** accepted. The pre-render line clocks twice, so the plan's 241 was an arithmetic slip. The test pins observed behaviour that the oracle's `4-scanline_timing` also confirms.
- **09-07, empty file gets "cannot read cartridge":** accepted. It is a distinct, non-zero-exit, named failure, and the spec's "message and non-zero exit" holds. The generic line applies to header-readable refusals.
- **09-11, `src/ppu.c` pre-render dot 0 now shows `v`:** accepted. The new test is in `tests/ppu/test_fetch.c`, the README describes it, and the code comment cites blargg `4-scanline_timing` tests 8 and 9. The public header needs no comment, since this is internal timing and is not part of the API contract. Hashes are provably unchanged: only additive hash rows, the accuracy scoreboard identical and the revision still 5. Because no rendering-on frame hash moved, leaving the revision at 5 is correct.

## Code review ruling, WR-01 (header mirroring bit ignored for MMC3)

D-07 defines `$A000` bit 0 as the sole mirroring source, and D-08 locks a zeroed `reg.mmc3` as power-on, which is vertical mirroring. Real MMC3 hardware has no header, so ignoring the iNES flag 6 bit 0 **matches the locked decisions** and is correct. It is not a defect. The remaining gap is documentation only: `include/nesturbator.h` says "ignoring the header mirroring bit" for other boards but not for mapper 4, and no loader-level test pins a horizontal-header image starting vertical.

Classification: advisory, non-blocking. It does not break any success criterion. Suggested follow-up (a `/gsd-quick` item): add one sentence to the mapper 4 header comment and a one-case test. IN-01 and IN-02 are style or maintainability items and are non-blocking. The disposition file still lists all three as open.

## Anti-patterns

None blocking. Hygiene is green (no ROM bytes beyond the manifest, blargg ROMs fetched and never committed).

## Human verification

None. Every check is automated, as the project requires.

---
_Verified: 2026-10-10 / Verifier: Claude (gsd-verifier)_
