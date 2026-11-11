---
phase: 28-qualified-nes-gameplay-package
plan: "26"
subsystem: testing
tags: [cmake, accuracycoin, provider-package, local-acceptance, source-identity]
requires:
  - phase: 28-25
    provides:
      - Historical and current input identities remain separately validated.
      - The bounded private D-44 driver and receipt controls are available.
provides:
  - One frozen D-44 source identity with a fresh 435-path provider manifest.
  - One successful full local ci workflow and saved handoff-time publication absence observation.
  - An explicit separation between historical measurement identity, current inputs, and local acceptance.
affects: [28-27 candidate CI, provider publication]
actuals:
  tokens: 25211
  tasks: 2
  commits: 0
commits: 0
plan_head_before: ce80b9e20b55d2eea65b2ed82952416b9619296e
plan_head_after: ce80b9e20b55d2eea65b2ed82952416b9619296e
tech-stack:
  added: []
  patterns:
    - Freeze the reviewed source and complete current manifest before the sole local workflow.
    - Record the historical dirty-source map separately from the current checker identity.
key-files:
  created:
    - .planning/workstreams/libfuturo-nes-gameplay-handoff/phases/28-qualified-nes-gameplay-package/28-D44-FREEZE.json
    - .planning/workstreams/libfuturo-nes-gameplay-handoff/phases/28-qualified-nes-gameplay-package/28-D44-IDENTITY.json
    - .planning/workstreams/libfuturo-nes-gameplay-handoff/phases/28-qualified-nes-gameplay-package/28-D44-ACCEPTANCE.md
    - .planning/workstreams/libfuturo-nes-gameplay-handoff/phases/28-qualified-nes-gameplay-package/28-26-SUMMARY.md
  modified:
    - .planning/workstreams/libfuturo-nes-gameplay-handoff/phases/28-qualified-nes-gameplay-package/28-D44-REVIEW.md
key-decisions:
  - "Local CI success qualifies only this frozen source for the next hosted step; candidate CI, main CI, and publication remain separate pending gates."
  - "The reconstructed historical map preserves the recorded dirty-source identity but does not authenticate that historical dirty tree or hardware correctness."
patterns-established:
  - "Preflight, freeze, local acceptance, and saved handoff inspection use one source-bound route with distinct historical and current fingerprints."
requirements-completed: []
coverage:
  - id: D1
    description: "The frozen D-44 source passed exactly one full local ci workflow with all 700 discovered test results passing."
    verification:
      - kind: integration
        ref: "build/phase28-d43/plan26-local-independent.json and 28-D44-IDENTITY.json"
        status: pass
    human_judgment: false
  - id: D2
    description: "Saved handoff evidence records the successful local source and the expected missing-publication refusal."
    verification:
      - kind: other
        ref: "build/phase28-d43/handoff-once/result.json and build/phase28-d43/handoff-control.json"
        status: pass
    human_judgment: false
duration: 36m
completed: 2026-11-10
status: complete
---

# Phase 28 Plan 26: D-44 Local Candidate Acceptance Summary

**The reviewed D-44 source passed its sole full local ci workflow; hosted CI and publication remain pending.**

## Performance

- **Duration:** 36m from the first retained configure-only preflight to this summary.
- **Started:** 2026-11-10T20:31:16Z (first retained preflight command).
- **Completed:** 2026-11-10T21:07:16Z.
- **Tasks:** 2.
- **Files changed:** Four D-44 evidence documents plus this summary; no product source file changed.
- **Actuals:** 25211 chars/4 across the new evidence documents, review addition, and summary; machine-generated ignored inventories and logs are excluded.

## Accomplishments

- Froze reviewed commit ce80b9e20b55d2eea65b2ed82952416b9619296e and its 435-path provider manifest, then ran the full ci workflow exactly once with AccuracyCoin FULL enabled and four workers. All configure, build, test, and package stages passed.
- Confirmed all 700 unique CTest results passed, including 662 ordinary pass markers and 38 valid required-regex pass-reason results. The full CTest discovery and fixture closure were captured before acceptance; no tests were run during preflight.
- Preserved the historical 56-input fingerprint separately from the current 56-input fingerprint. The historical source remains declared dirty and is not authenticated by the reconstructed map.
- Recorded the three local package archives, runtime profile check, gameplay report and campaign identity in the immutable local identity. The separate bounded handoff observation passed and retained the expected named refusal because publication is absent.

## Task Commits

No Plan26 product or task commit was required. The parent evidence-only closeout commit is separate from this plan's task commits.

## Files Created or Modified

- [28-D44-FREEZE.json](28-D44-FREEZE.json) — source, historical/current fingerprints, provider manifest, checker and driver identities, and the single freeze result.
- [28-D44-IDENTITY.json](28-D44-IDENTITY.json) — local source/build/package identity and full workflow evidence.
- [28-D44-ACCEPTANCE.md](28-D44-ACCEPTANCE.md) — local success, timing, and still-pending release gates.
- [28-D44-REVIEW.md](28-D44-REVIEW.md) — prefreeze review plus post-freeze evidence and timing disposition.
- [plan26-local-independent.json](../../../../../build/phase28-d43/plan26-local-independent.json) — retained independent count, manifest, archive and gameplay result comparison.
- [plan26-preflight.json](../../../../../build/phase28-d43/plan26-preflight/plan26-preflight.json) — configured-only CTest inventory, fixture closure, fingerprints and source identities.
- [LastTest.log](../../../../../build/phase28-d43/local/LastTest.log) — full local workflow test output.
- [handoff-once result](../../../../../build/phase28-d43/handoff-once/result.json) and [missing-publication control](../../../../../build/phase28-d43/handoff-control.json) — saved one-shot handoff result and separate expected rejection.

## Verification

- cmake --preset ci -DNESTURBATOR_ACCURACYCOIN_FULL=ON configured the pinned checkout successfully; no build or test ran during preflight.
- ctest --preset ci --show-only=json-v1 inventoried 700 tests and 39 fixtures, with the approved campaign, protected-result, install, profile, and receipt controls in the required closure.
- The one frozen local operation passed in 443.692 seconds including route orchestration; its inner cmake --workflow --preset ci took 434 seconds. The test log, three archives, installed gameplay report, and current inputs match the frozen identity.
- MODE=handoff passed once in 0.270 seconds. Its saved synthetic missing-publication control exited 1 for the expected missing receipt. python3 build/phase28-d43/run.py inspect --stage=handoff passed read-only; it launched no test, checker, workflow, campaign, network request, or publication action.

## Issues Encountered

- The initial preflight selector evidence wrapper read stdout, while CMake emitted its selector records on stderr. The successful saved stderr was parsed and independently checked; the selector was not rerun.
- An audit-only result counter initially accepted only the ordinary Test Passed form. The saved CTest log showed 38 valid Test Pass Reason entries; the count was corrected to include them. The actual workflow exited zero, every one of 700 unique results passed, and no live gate was retried.

## Deviations from Plan

None. The local workflow, saved handoff observation, and read-only inspection followed the approved bounds.

## User Setup Required

None.

## Next Phase Readiness

Plan 28-27 is the next eligible plan after this evidence and summary are committed. The exact-candidate CI, resulting main CI, PKG-01, and published-receipt gates remain pending; this plan claims local qualification only.

## Self-Check: PASSED

The freeze, identity, acceptance, review, and summary files exist. The route records one passed local attempt, current source and driver hashes match the frozen identity, and the retained local evidence reports all 700 results passing. Plan26 has zero product/task commits; its plan head remains ce80b9e20b55d2eea65b2ed82952416b9619296e. The parent evidence-only closeout is tracked separately.

---
*Phase: 28-qualified-nes-gameplay-package*
*Completed: 2026-11-10*
