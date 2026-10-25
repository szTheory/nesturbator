---
phase: 17-complete-pinned-conformance-baseline
plan: "02"
subsystem: testing
tags: [accuracycoin, conformance, bounded-campaign, ctest]
requires:
  - phase: 17-complete-pinned-conformance-baseline
    provides: pinned comparison ROM, authenticated inventory and page-3 tracer
provides:
  - 22 independently bounded cold-page attempts and production campaign collection
  - separated inventory, attempts, raw observations, classifications and outer outcomes
  - measured caps, finite wall deadlines and retained final campaign evidence
affects: [17-03, 17-04, accuracycoin, ci]
actuals:
  tokens: 41468
  tasks: 3
  commits: 3
plan_head_before: eb69849fe38873bab1ced1f95819775a71c0404e
plan_head_after: dbcc64190d9a6269eb1ac1a437709c51ff37fdab
tech-stack:
  added: []
  patterns: [independent bounded CMake child attempts, provenance-checked TSV collection]
key-files:
  created:
    - tests/cmake/accuracycoin_campaign.cmake
    - tests/cmake/accuracycoin_driver.cmake
    - tests/cmake/accuracycoin_partial_failure.cmake
    - tests/accuracy/budgets.txt
    - tests/accuracy/measurements.md
  modified:
    - runner/main.c
    - runner/accuracycoin.c
    - tests/accuracy/test_conformance.c
    - tests/CMakeLists.txt
    - README.md
    - include/nesturbator.h
key-decisions:
  - "Use caps of 32 boot, 5 frames per page transition, 2 attributed start/release frames, and 1800 execution frames."
  - "Keep the 5-frame navigation policy distinct from the 6-frame probe because the additional neutral frame changes page-19 guest timing and active row."
  - "Use one full campaign in a single required PR lane as the 17-04 placement recommendation; hosted runtime still needs measurement."
patterns-established:
  - "Keep declared inventory, attempts, raw snapshots, final classifications and outer process outcomes as separate evidence layers."
  - "A timeout requires final cursor and automation evidence for the currently active source row; preserve noncurrent $03 bytes as not-run evidence."
requirements-completed: []
coverage:
  - id: D1
    description: "Run and collect all 22 independent page attempts against the authenticated comparison ROM."
    verification:
      - kind: integration
        ref: "ctest --preset ci -R '^accuracycoin.campaign$' --no-tests=error"
        status: pass
    human_judgment: false
  - id: D2
    description: "Reject malformed evidence and preserve partial, duplicate, outer-timeout and conflicting observations honestly."
    verification:
      - kind: unit
        ref: "accuracycoin.report, accuracycoin.driver, accuracycoin.outer_failure"
        status: pass
    human_judgment: false
  - id: D3
    description: "Measure guest-frame bounds, wall deadlines, actual counts and campaign cost."
    verification:
      - kind: other
        ref: "tests/accuracy/measurements.md and build/ci/accuracycoin/full/"
        status: pass
    human_judgment: false
duration: 67min
completed: 2026-10-23
status: complete
---

# Phase 17 Plan 02: Bounded AccuracyCoin Campaign Summary

**A source- and ROM-bound 22-page campaign now preserves raw evidence and classifies every scored row under measured guest and outer deadlines.**

## Performance

- **Duration:** 67 min
- **Started:** 2026-10-23T22:02:17Z
- **Completed:** 2026-10-23T23:09:46Z
- **Tasks:** 3
- **Files modified:** 12

## Accomplishments

- Added independent cold-loaded page processes, attribution checks for menu readiness, selected page, A press/release and source-defined running/completion state, plus a production collection/verdict mode.
- The production collector distinguishes inventory, attempts, stage snapshots, classified rows and launcher outcomes. It retains missing bytes as absent, preserves conflicting syntactically valid observations in `observations.tsv`, and fails infrastructure trust for duplicate or invalid evidence.
- Added focused controls for cap-1/cap/cap+1, controller-sensitive input, DRAW-only page 16, source-qualified page-19 active identity, wrong provenance, missing raw, conflicting bytes, real CMake outer timeout and partial child failure with later-page continuation.
- Executed the actual final campaign at 32/5/1800 guest caps. All 22 attempts launched, started and completed at the infrastructure layer; all 146 scored raw bytes were observed. The score was 101 pass, 42 fail, 0 skip, 2 not-run and 1 guest timeout, with five DRAW rows outside the denominator.
- Recorded all page measurements, cap arithmetic, navigation timing sensitivity, wall bounds, CI placement input and the retained artifact path in [budgets.txt](../../../tests/accuracy/budgets.txt) and [measurements.md](../../../tests/accuracy/measurements.md).
- The final full CI workflow passed 657/657 tests and packaged library, runner and libretro ZIPs. The registered full-campaign test remains optional/off in the normal build pending Plan 17-04 placement.

## Task Commits

1. **Task 1: Run independently bounded pages with attributable startup and checked collection** — `13afa18` (`feat`)
2. **Task 2: Discriminate false starts, held controls, timeouts and incomplete evidence** — `2b8fe10` (`fix`)
3. **Task 3: Execute the actual full bounded campaign and measure accepted caps and cost** — `dbcc641` (`docs`)

The plan metadata commit is created after this summary's self-check and state updates.

## Files Created/Modified

- `runner/main.c`, `runner/accuracycoin.c`, `runner/accuracycoin.h` — bounded stage options, per-page observation and production collection.
- `tests/cmake/accuracycoin_campaign.cmake` — sequential page deadlines, outer outcome capture and honest continuation.
- `tests/cmake/accuracycoin_driver.cmake`, `tests/cmake/accuracycoin_partial_failure.cmake`, `tests/accuracy/test_conformance.c` — production-driver and adversarial controls.
- `tests/CMakeLists.txt` — focused tests always registered; complete campaign registered only when explicitly enabled.
- `tests/accuracy/budgets.txt`, `tests/accuracy/measurements.md` — accepted measured caps, complete outcomes, durations and placement recommendation.
- `README.md`, `include/nesturbator.h` — paired runner and evidence-contract documentation.

## Decisions Made

- Accepted boot 32, navigation 5 per page change, start/release 2 and execution 1800 guest frames. The aggregate declared budget is 41,503 guest frames; the run consumed 7,153.
- Five navigation frames form the fixed measured input unit. A separate one-page six-frame probe made `Misaligned OAM behavior` the active page-19 row, while the final five-frame campaign timed out on `Arbitrary Sprite zero`. The runs are separate evidence, not interchangeable classifications.
- No row-A fallback was needed: every scored row has a cold-attempt raw byte. The current timeout, noncurrent `$03`, and untouched `$00` remain explicitly classified without inventing missing data or changing guest context.
- Recommend one full campaign in one required PR lane, not six matrix repetitions. The local campaign measured 5.97 seconds; Plan 17-04 should use hosted samples before changing any job timeout or enabling placement.
- CONF-02 and CONF-03 remain phase-level pending requirements until Phase 17 verification completes.

## Deviations from Plan

None — the plan executed as written. The collector's raw-output ordering was refined so a syntactically valid conflicting sample is recorded before the duplicate/order check rejects campaign trust; this satisfies the plan's conflict-retention requirement and has a dedicated control.

## Issues Encountered

- The first full workflow caught that the README option audit requires a separate bullet per `--help` switch. The cap descriptions were split into separate bullets; the full workflow then passed.
- After committing Task 2, the campaign test initially reused a runner binary compiled with the prior revision. The runner and conformance targets were rebuilt, and the final campaign was rerun with `source_revision=2b8fe1023608`.
- The campaign metadata reports `source_dirty=1` because the preexisting `.planning/config.json` edit was preserved. No implementation file was dirty at final capture.

## Verification

- `cmake --workflow --preset ci` — passed, 657/657 tests; all four workflow steps completed, including package generation.
- `ctest --preset ci -R '^accuracycoin.(driver|report|outer_failure|classifier|tracer)$' --no-tests=error --output-on-failure` — passed, 5/5.
- After the final documentation update, the focused AccuracyCoin set plus `runner.options.readme` passed, 6/6.
- `ctest --preset ci -R '^accuracycoin.campaign$' --no-tests=error --output-on-failure` — final committed-source campaign passed, 1/1 in 5.97 seconds.
- Final output at `build/ci/accuracycoin/full/` reports `trustworthy_observation=1`, `infrastructure_verdict=complete`, 22/22 launches, and the exact result counts above. The build was restored to `NESTURBATOR_ACCURACYCOIN_FULL=OFF` afterward.

## User Setup Required

None — no external service or manual configuration is required.

## Next Phase Readiness

Plan 17-03 can use the new comparison inventory and production evidence model while the old pin remains canonical. Plan 17-04 should perform the measured single-lane CI placement and activation decisions only after the protected-main regression work and whole-phase verification. The actual report, raw samples, diagnostics, and child logs remain under `build/ci/accuracycoin/full/` in this workspace.

## Self-Check: PASSED

The summary, budgets, measurements and campaign output exist; all three task commits are ancestors of HEAD; the commit ledger measures three task commits and 12 unique files. `git diff --check` is clean. The preexisting `.planning/config.json` remains unstaged.

---
*Phase: 17-complete-pinned-conformance-baseline*
*Completed: 2026-10-23*
