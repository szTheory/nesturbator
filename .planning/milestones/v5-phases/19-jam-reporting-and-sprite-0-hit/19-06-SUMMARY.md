---
phase: 19-jam-reporting-and-sprite-0-hit
plan: "06"
subsystem: accuracy
tags: [accuracycoin, scoreboard, baseline, activation, docs]
requires: ["19-05"]
provides:
  - "re-recorded 146-row canonical scoreboard (108 pass, 38 fail, 0 timeout, 0 not-run)"
  - "D-11 page-19 guard in accuracycoin.activation"
  - "corrected research record (D-03)"
affects: []
key-files:
  modified:
    - tests/accuracy/scoreboard.txt
    - tests/accuracy/baseline-metadata.txt
    - tests/cmake/accuracycoin_activation.cmake
    - tests/cmake/accuracycoin_baseline_fetch.cmake
    - tests/cmake/accuracycoin_regression_controls.cmake
    - tests/accuracy/measurements.md
    - tests/accuracy/budgets.txt
    - README.md
    - .planning/ROADMAP.md
    - .planning/research/SUMMARY.md
key-decisions:
  - "Counts and provenance hashes come from the measured full campaign; the research prediction (108/38/0/0/146) matched exactly"
  - "Neighbour rows that started passing (INC $4014, Rendering Flag Behavior, $2007 read w/ rendering) are recorded with their cause, not chased"
requirements-completed: [SPR-01, SPR-02, JAM-01, JAM-02]
completed: 2026-10-25
status: complete
commits: 3
plan_head_before: fe62f00
plan_head_after: 58c7381
actuals:
  tokens: 30000
  tasks: 3
  commits: 3
---

# Phase 19 Plan 06: Re-record the AccuracyCoin baseline Summary

Measured full-campaign scoreboard (108 pass, 38 fail, no timeout or not-run) with a page-19 guard in `accuracycoin.activation`, and the research record corrected to the stale-OAMADDR cause.

## Measured versus predicted

Measured by `-DNESTURBATOR_ACCURACYCOIN_FULL=ON` campaign (22 of 22 pages, `trustworthy_observation=1`, `infrastructure_verdict=complete`): pass 108, fail 38, skip 0, not_run 0, timeout 0, terminal 146. Research prediction: 108/38/0/0/146. Identical. The scoreboard was generated from `report.tsv` by a pipeline (test rows, C-locale sort), not by hand; a second campaign run after the activation edit reproduced it byte for byte.

## Scoreboard diff (moved rows)

| Row | Before | After |
|---|---|---|
| Sprite 0 Hit behavior | fail 0x06 | pass 0x01 |
| Arbitrary Sprite zero | timeout 0x03 | pass 0x01 |
| Suddenly Resize Sprite | fail 0x06 | pass 0x01 |
| Misaligned OAM DMA | fail 0x06 | pass 0x01 |
| Misaligned OAM behavior | not-run 0x03 | fail 0x16 |
| OAM Corruption | not-run 0x00 | fail 0x0a |
| INC $4014 | fail 0x0a | pass 0x01 |
| Rendering Flag Behavior | fail 0x0a | pass 0x01 |
| $2007 read w/ rendering | fail 0x06 | pass 0x01 |

Other rows changed only frame counts (page 18: 503 to 569; page 19: 1800 to 521). Same cause for all: OAMADDR is now cleared during tile loading and evaluation starts at OAMADDR. No previously passing row was lost (`accuracy.scoreboard.no_lost_pass` and `accuracycoin.protected` green).

## Hash outcome

No frame hash in `tests/runner/hashes.txt` moved in this plan. The 12 `state/*` rows were re-pinned in Plan 19-05 only for the behaviour revision byte (now 7); `runner.write_hashes` and `.content` are green.

## Commits (D-20 map: commits 1 = 19-01/02, 2 = 19-03, 3-4 = 19-04, 5 = 19-05, 6 = this plan)

- `431688f` test(accuracy): re-record the AccuracyCoin baseline after the sprite-0 fix (commit 6; scoreboard, baseline-metadata, activation; 3 files, 77 insertions, 34 deletions)
- `1cab0cf` test(accuracy): follow the 108-pass baseline in the fetch and control checks (see deviation)
- `58c7381` docs(19): record the sprite-0 cause and the new AccuracyCoin baseline (README, measurements, budgets, ROADMAP, research SUMMARY)

## Deviations from Plan

**1. [Rule 3 - Blocking] Hard-coded 101-row PASS set in two check scripts**
- **Found during:** Task 3 (`cmake --workflow --preset ci`: `accuracycoin.baseline_fetch` failed, "exported 108 PASS rows, expected 101")
- **Fix:** `accuracycoin_baseline_fetch.cmake` (4 constants) and `accuracycoin_regression_controls.cmake` (3 constants and a comment) now say 108. Neither file is a fingerprint input, so activation stays green. The regression controls also ran green under the full campaign.
- **Commit:** `1cab0cf`

**2. Baseline provenance is a local, dirty-tree run.** `source_revision=fe62f00ffa68`, `source_dirty=1` with a stated reason in `source_dirty_reason`, instead of the Phase 18 exact-source hosted clean run. Inputs and report are still pinned by `build_inputs_sha256` and `campaign_report_sha256`.

**Total deviations:** 1 auto-fixed (Rule 3), 1 provenance note.

## Gates

- `cmake --workflow --preset ci` on the final commit: 100% tests passed out of 670, 399.55 s, exit 0 (FULL flag reset to OFF).
- `cmake --workflow --preset asan`: 100% tests passed out of 665, 436.36 s, exit 0, no sanitizer report (run after the check-script edits, before the docs-only commit).
- `scripts/hygiene.sh`: exit 0.
- With the full campaign ON: `accuracycoin.*` 17 of 17 passed, including activation with the new page-19 checks.

## Known Stubs

None.

## Self-Check: PASSED

Scoreboard, metadata, activation and docs files exist; commits 431688f, 1cab0cf, 58c7381 are ancestors of HEAD.
