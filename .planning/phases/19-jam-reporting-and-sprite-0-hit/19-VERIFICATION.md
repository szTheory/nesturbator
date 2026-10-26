---
phase: 19-jam-reporting-and-sprite-0-hit
verified: 2026-10-25T00:00:00Z
status: passed
score: 4/4 must-haves verified
covered_files:
  - ".planning/phases/19-jam-reporting-and-sprite-0-hit/19-01-PLAN.md"
  - ".planning/phases/19-jam-reporting-and-sprite-0-hit/19-01-SUMMARY.md"
  - ".planning/phases/19-jam-reporting-and-sprite-0-hit/19-02-PLAN.md"
  - ".planning/phases/19-jam-reporting-and-sprite-0-hit/19-02-SUMMARY.md"
  - ".planning/phases/19-jam-reporting-and-sprite-0-hit/19-03-PLAN.md"
  - ".planning/phases/19-jam-reporting-and-sprite-0-hit/19-03-SUMMARY.md"
  - ".planning/phases/19-jam-reporting-and-sprite-0-hit/19-04-PLAN.md"
  - ".planning/phases/19-jam-reporting-and-sprite-0-hit/19-04-SUMMARY.md"
  - ".planning/phases/19-jam-reporting-and-sprite-0-hit/19-05-PLAN.md"
  - ".planning/phases/19-jam-reporting-and-sprite-0-hit/19-05-SUMMARY.md"
  - ".planning/phases/19-jam-reporting-and-sprite-0-hit/19-06-PLAN.md"
  - ".planning/phases/19-jam-reporting-and-sprite-0-hit/19-06-SUMMARY.md"
  - "libretro/libretro.c"
  - "runner/accuracycoin.c"
  - "src/ppu.c"
  - "tests/accuracy/scoreboard.txt"
  - "tests/cmake/accuracycoin_jam.cmake"
covered_digest: "v3:sha256:6f28e9f2447c8d8ac5e167ae849e90712795edccf90aec45367d72a3cea5fee4"
behavior_unverified: 0
overrides_applied: 0
---

# Phase 19: JAM reporting and sprite-0 hit Verification Report

**Phase Goal:** A developer reading an AccuracyCoin run sees a guest JAM named as a JAM, and a game's sprite-0 status-bar split lands on the console's line.
**Status:** passed. **Re-verification:** No.

## Observable Truths (ROADMAP success criteria)

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | AccuracyCoin run stopping on a guest JAM reports `guest-jam`; a control dropping attribution fails | VERIFIED | `runner/accuracycoin.c` has `observe_note_jam`, `ACCURACYCOIN_ACTIVE_JAM`, `jam_frame` metadata. Independently ran `accuracycoin.jam`, `accuracycoin.classifier`, `accuracycoin.report` (fixture controls `jam-cause-dropped`, `jam-row-dropped`, `jam-frame-missing` rejected): all pass. |
| 2 | `libretro.runahead` shows run-ahead across a JAM gives same picture and silence | VERIFIED | Adapter tags its cached frame with frame number and timeline in `libretro/libretro.c`; ran `libretro.runahead` and `libretro.host`: pass. |
| 3 | `Sprite 0 Hit behavior` and `Arbitrary Sprite zero` pass within the 1800 cap; owned `ppu.sprites` tests show hit dot and sprite-0 at any OAM index | VERIFIED | `tests/accuracy/scoreboard.txt`: both `pass 0x01` at 521 frames. `src/ppu.c` changed (sprite evaluation from OAMADDR, OAMADDR cleared on dots 257-320). `ppu.sprites` passes, `accuracy.scoreboard` passes. |
| 4 | No lost passes; moved hashes re-pinned with reason | VERIFIED | `accuracy.scoreboard.no_lost_pass` passes. Summary 19-05 and 19-06 record the re-pin (12 state rows, revision 7) and the 108/38 baseline. The orchestrator's `cmake --workflow --preset ci` result (670 of 670) is consistent with these targeted runs. |

**Score:** 4/4 truths verified, 0 behavior-unverified.

## Requirements Coverage

| Requirement | Plan | Status | Evidence |
|-------------|------|--------|----------|
| JAM-01 | 19-01, 19-02 | SATISFIED | Execution, boot and strict-mode JAM naming; `accuracycoin.jam` legs; collector controls |
| JAM-02 | 19-03 | SATISFIED | `libretro.runahead` JAM matrix at depths 1-3 with black-oracle control |
| SPR-01 | 19-04, 19-05 | SATISFIED | Scoreboard row `Sprite 0 Hit behavior` pass; `ppu.sprites` |
| SPR-02 | 19-05 | SATISFIED | Scoreboard row `Arbitrary Sprite zero` pass at 521 frames; `ppu.sprites` |

All four IDs are marked Complete in REQUIREMENTS.md and there are no orphaned requirements.

## Anti-Patterns

None blocking. 19-REVIEW.md records 0 critical, 2 warning, 1 info, all advisory and recorded open in 19-REVIEW-DISPOSITION.md. No TBD/FIXME/XXX gate was run beyond this review. Navigation- and start-stage JAMs share the boot branch shape and are not separately constructed (a flagged assumption in plan 19-02).

## Human Verification

None. Every truth is shown by an automated command.

## Gaps Summary

No gaps. Uncommitted `.planning/config.json` edit is unrelated to this phase.

_Verifier: Claude (gsd-verifier)_
