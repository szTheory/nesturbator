---
phase: 19-jam-reporting-and-sprite-0-hit
plan: "01"
subsystem: runner
tags: [accuracycoin, jam, observe, classifier, conformance]
requires: []
provides:
  - "execution-stage guest-jam verdict, stop cause and jam_frame metadata"
  - "three-state ACCURACYCOIN_ACTIVE_RUNNING/_BUDGET/_JAM classifier argument"
  - "accuracycoin.jam CTest (patched page-3 observe run)"
affects: [19-02]
tech-stack:
  added: []
  patterns: ["in-memory ROM patch inside the conformance program only"]
key-files:
  created:
    - tests/cmake/accuracycoin_jam.cmake
  modified:
    - runner/accuracycoin.c
    - runner/accuracycoin.h
    - tests/accuracy/test_conformance.c
    - tests/CMakeLists.txt
    - README.md
key-decisions:
  - "A JAM leaves the execution loop through a jam_stop flag and skips the cap chain, so it cannot misfire as active-row-not-current driver-failure"
  - "Attribution reads the controls again after the JAM (RAM stays readable) before naming the cursor row"
requirements-completed: []
duration: 35 min
completed: 2026-10-25
status: complete
commits: 1
plan_head_before: deb7349e30b84c8fb2442e256c99d0131e395aa1
plan_head_after: e66e6cc00853b42b6cec2a4a911abce98fc1f1d7
actuals:
  tokens: 18000
  tasks: 3
  commits: 1
---

# Phase 19 Plan 01: JAM attribution in the AccuracyCoin observe run Summary

An execution-stage guest JAM now stops the observe run as `guest-jam` (verdict and stop cause), records `jam_frame=N`, writes a report with exactly one `fail/0x03/guest-jam` row for the running test and exits 0; a real patched-ROM run of page 3 proves it in CI.

## What was built

- `frame_and_sample` returns `nesturbator_status`; `STOP_JAM` passes through, any other failure maps to `NESTURBATOR_ERR_ARGUMENT`. All five call sites compare against `NESTURBATOR_OK` (baseline of `!frame_and_sample(` was 5, now 0).
- One static helper `observe_note_jam` sets `stop_cause`/`verdict` to `guest-jam` and `jam_frame` to the completed total frame count (reusable by Plan 19-02 for earlier stages).
- Execution stage: STOP_JAM is tested before the generic failure branch; the loop breaks, the post-loop cap chain is skipped, final sampling and report writing still run. Attribution: automation active, cursor names a TEST row whose byte is `0x03`, else `active_key=-`.
- Classifier: `ACCURACYCOIN_ACTIVE_RUNNING 0`, `_BUDGET 1`, `_JAM 2`; the argument is renamed `active_state` (`classify`, `classify_row`, `classify_final_row`). Raw `0x03` with JAM is `FAIL`/`guest-jam`; terminal bytes are never overridden.
- Metadata: `jam_frame=<N>|-` after `verdict=` (report_schema stays 1). stderr `nesturbator-run: JAM stopped AccuracyCoin page %u at frame %s (active_key=...)`; stdout uses the existing `accuracycoin observe page P guest-jam (...)` summary format.
- Test: `accuracycoin.conformance jam NEW OLD OLD_SCOREBOARD WORK` parses the unpatched pin, picks the last TEST row of page 3 by rule, maps its `routine_address` through a bounds-checked local NROM offset helper, asserts the byte is not already `$02`, patches a copy in memory and runs `accuracycoin_run_observe_page` with budgets 32/5/1800. `accuracycoin.jam` (cmake -P script, TIMEOUT 60) asserts exit 0, stdout and stderr lines, metadata and the single report row. Old ROM and scoreboard arguments are accepted but unused until Plan 19-02's strict-mode control.
- README observe section documents the JAM verdict.

## Observed results

- Patched row: `accuracycoin/$1F   SLO absolute,X` (page 3 row 7), routine `0xa876`, file offset 10374.
- `jam_frame=49`, `execution_frames=11`, exit code 0, `active_key=accuracycoin/$1F   SLO absolute,X`, `active_raw=0x03`.
- report.tsv: rows 1-6 `pass 0x01 terminal-pass`, row 7 `fail 0x03 guest-jam`.
- Classifier cases: (0x03, JAM) FAIL guest-jam; (0x01, JAM) PASS; (0x06, JAM) FAIL terminal-fail; (0x00, JAM) NOT_RUN; (0x83, JAM) INVALID; (0x03, RUNNING) INVALID active-test; (0x03, BUDGET) TIMEOUT. RED was observed first by disabling the JAM branch (accuracycoin.classifier failed), then GREEN.

## Commit

- `e66e6cc` feat(runner): report a guest JAM in AccuracyCoin observe runs (Tasks 1-3 as one commit per the plan). `git show --stat`: README.md 12, runner/accuracycoin.c 121, runner/accuracycoin.h 16, tests/CMakeLists.txt 10, tests/accuracy/test_conformance.c 125, tests/cmake/accuracycoin_jam.cmake 85 (6 files, +338/-31).

## Deviations from Plan

None - plan executed exactly as written. README was updated in the same commit per CLAUDE.md rule 6 (plan listed no README file; this is a documentation obligation, not a scope change).

## Verification

- `ctest --preset ci -j8` (670 tests): 669 pass, 1 fail: `accuracycoin.activation`, the failure the plan expects until Plan 19-06 refreshes the build-input fingerprint (Pitfall 3). All other tests, including `accuracycoin.{tracer,driver,outer_failure,inventory,classifier,report,jam,protected}`, pass.
- Acceptance greps: `jam_frame=` 1, `JAM stopped AccuracyCoin page` 1, `!frame_and_sample(` 0, `NAME accuracycoin.jam` 1 line, `ACCURACYCOIN_ACTIVE_JAM` in header 2 and test 6, `git status --porcelain tests/roms` empty.
- Not run: `cmake --workflow --preset ci` as a whole (its activation test is red by design at this point); asan workflow left to Plan 19-06.

## Assumption recorded for the verifier

A JAM on the exact cap frame is not constructed separately; the JAM check precedes the cap chain so a JAM wins over a budget timeout.

## Known Stubs

None.

## Threat Flags

None. No ROM bytes are written anywhere; the patch exists only in memory in `accuracycoin.conformance`; `nesturbator-run` gained no flag.

## Self-Check: PASSED

- tests/cmake/accuracycoin_jam.cmake exists; commit e66e6cc is an ancestor of HEAD.
