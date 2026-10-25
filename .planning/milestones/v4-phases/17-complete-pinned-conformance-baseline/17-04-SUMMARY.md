---
phase: 17-complete-pinned-conformance-baseline
plan: "04"
subsystem: testing
tags: [accuracycoin, CMake, conformance, GitHub-Actions]

requires:
  - phase: 17-complete-pinned-conformance-baseline
    provides: Measured campaign evidence and protected-main migration authority from Plans 17-02 and 17-03
provides:
  - Atomic canonical 6bee919 campaign fixture, measured baseline, migration, and retained 673ef550 comparison
  - Authenticated same-revision canonical protected-main scoreboard handoff and dynamically selected same-invocation PASS protection
  - One measured full AccuracyCoin campaign on the primary Linux x64 PR lane with evidence artifacts
affects: [Phase 17 verification, AccuracyCoin CI]

actuals:
  tokens: 85870.5
  tasks: 2
  commits: 13
  plan_head_before: 393cb34b445d2c5295226d1597706b250e04ee6e
  plan_head_after: dcdb7b39bb0aec6ff74b192bfa239f6c0dc9243b

tech-stack:
  added: []
  patterns:
    - Authenticate the canonical scoreboard bytes and ROM manifest from one immutable protected-main commit before projecting measured PASS rows.
    - Derive protected pages from the exact baseline used by the current invocation and require fresh same-pin PASS/raw evidence.
    - Run the complete measured campaign once on primary Linux x64 PR CI while retaining unconditional protected checks on each platform lane.

key-files:
  created:
    - tests/accuracy/baseline-metadata.txt
    - tests/accuracy/scoreboard.txt
    - tests/cmake/accuracycoin_activation.cmake
    - tests/cmake/accuracycoin_input_fingerprint.cmake
    - tests/roms/accuracycoin-673ef.nes
  modified:
    - tests/accuracy/test_scoreboard.c
    - tests/cmake/prepare_scoreboard_baseline.cmake
    - tests/cmake/accuracycoin_protected.cmake
    - tests/cmake/accuracycoin_regression_controls.cmake
    - tests/CMakeLists.txt
    - .github/workflows/ci.yml
    - README.md
    - include/nesturbator.h

key-decisions:
  - "The canonical new-pin baseline becomes authority only after its bytes, metadata, fixture manifest and protected-main revision agree; only its measured PASS rows are projected into the normal-lane verifier."
  - "Same-pin protection follows the current protected baseline dynamically and requires a fresh PASS for every protected key; the original 17 old-pin migration controls remain separate."
  - "The actual campaign fits one primary Linux x64 PR lane; keep its 15-second page bound and the existing workflow/job timeouts."

patterns-established:
  - "A next-main regression control builds an adjacent local protected-main commit and exercises the production fetch, page planner, cold observer and verdict path."
  - "A unique scratch directory per RetroArch state-session control avoids stale PID-derived fixtures after process-ID reuse."

requirements-completed: []

duration: 7h22m
completed: 2026-10-24
status: complete
---

# Phase 17 Plan 04: Complete Pinned Conformance Baseline Summary

**The canonical 6bee919 baseline is activated with its retained old-pin comparison, and every normal CI lane now freshly protects the PASS set selected from its exact protected-main revision.**

## Performance

- **Duration:** 7h22m, from the 17-03 session close to hosted verification completion.
- **Started:** 2026-10-24T00:01:04Z
- **Completed:** 2026-10-24T07:22:37Z
- **Tasks:** 2
- **Files modified:** 38 in the measured plan range, including the synchronized released-main history.

## Accomplishments

- Activated `tests/roms/accuracycoin.nes` at pin `6bee919529289c555803b612b8c9e4f7eaaeeabb` and SHA-256 `8ac8904a3cf26fbdfa6be501e414d45886ff4478d30dee8e58fa436cf5fd5adc`. The unchanged old fixture is retained exactly once as `accuracycoin-673ef.nes`; the temporary `accuracycoin-6bee.nes` name is removed. Manifest and activation checks bind both ROMs, their pins and inventory identities.
- Committed the actual compact 146-row new-pin baseline and the 17-row migration beside the retained old strict scoreboard. The inventories remain distinct: old pin 149 entries (144 scored + 5 DRAW); new pin 151 entries (146 scored + 5 DRAW). The measured new-pin counts are 101 pass, 42 fail, 0 skip, 2 not-run and 1 timeout; all five DRAW entries remain outside the scored denominator. All 146 scored rows have observed raw bytes, with 143 terminal classifications.
- Kept the 17 old-pin protected PASS measurements and their new-pin observations. The four source-reviewed changed meanings are Frame Counter IRQ, Frame Counter 4-step, Frame Counter 5-step and PPU Register Open Bus. The cross-pin dispositions do not grant same-pin failures an exemption.
- Closed the next-main activation gap end to end. The pre-configuration fetcher accepts a canonical schema-2 scoreboard plus matching metadata only from one protected-main commit, verifies its canonical byte hash and manifest identity, and projects only its PASS rows. The normal protected verifier derives current-invocation pages from that selected baseline. The primary-Linux full-lane positive control freshly observes and accepts all 101 same-pin PASS keys; adjacent negative controls reject a key outside the old pages and a formerly changed-meaning Frame Counter IRQ failure. Old-baseline and cross-pin controls remain.
- Selected a single full campaign on the primary Linux x64 PR lane. Normal CI still runs focused inventory/decoder/activation/migration checks and actual protected observations on each platform. The campaign retains the 15-second page-child deadline, 22-page + collection envelope of 340 seconds, and 370-second CTest bound; protected observation uses 30-second page children with a finite 760-second setup enclosure. Workflow/job timeouts and the six-platform, no-FP, ASan, determinism/state, perf, hash and hygiene gates were left intact.
- Updated the README, public-header test contract and preparation notes to distinguish historical and current pins, preserve the five-way result protocol, describe the dynamic protection cost, and bound the NTSC claims.

## Campaign identity and placement

The final measured campaign was captured before commit at source revision `47601b1f9dab` with `source_dirty=1`. That dirty state truthfully included the implementation, tests, documentation, and the pre-existing `.planning/config.json` edit. The 56-file build-input fingerprint excludes generated reports and that planning config, and binds SHA-256 `65fea2e1a75d83e23c3d9fc9d730794ce50a1af634c76ef6b50c0fd59385a70e`. The campaign report SHA-256 is `6d0a811960f6964e7fc66802c6e4ade0c4ffb8772c40d88d50cf79e1f8463652`; the canonical scoreboard SHA-256 is `9e96015ca205f0ed2a351c21d7b6b0a718b26e8e6da87b84f2542adadd04279a`.

The 22 page children summed to 5.758805 seconds locally; every child exited zero. The selected hosted primary-lane campaign CTest completed in 16.68 seconds. Hosted PR CI run [40070974145](https://github.com/szTheory/nesturbator/actions/runs/40070974145) tested source head `dcdb7b39bb0aec6ff74b192bfa239f6c0dc9243b`, merged for the PR test at `5089682ede09741d73819246373f77f7acf3718a` over base `e47feb3afc34baf1d611ef44153af199424f41a2`.

The hosted Linux x64 `cmake --workflow --preset ci` step completed **663/663 tests, 0 failed**, in 499.36 seconds. The campaign, protected observation, activation, baseline fetch and full-enabled regression controls passed in 16.68s, 2.73s, 0.12s, 1.03s and 22.25s respectively. The regression-control work directory is under `build/ci/accuracycoin/regression-controls/matrix`, included in the uploaded Linux evidence artifact.

All 15 jobs in PR CI run 40070974145 concluded success: Linux x64 and arm64, macOS arm64 and Intel x64, Windows MSVC x64 and arm64, ASan, both no-FP lanes, RetroArch e2e, hash equality, perf, hygiene, title and required rollup. The hash-equality job confirms all six platform artifacts contain the same complete 114-key inventory. The real base-to-head perf gate compared base `e47feb3…` with candidate merge head `5089682…`; `nesteroids`, `runner` and `M4_P256K_C256K` each measured +4 guest instructions/frame (+0.00%), all `ok`.

AccuracyCoin artifacts from that exact source head:

- Linux x64: `accuracycoin-evidence-linux-x64-40070974145-1`, ID `12551072567`, 95,164 bytes, SHA-256 `e553c777dab857b730852d3781a530c729d23435a9205b8de2237ad1f906a2d0`.
- Linux arm64: ID `12551820218`; macOS arm64: ID `12551695888`; macOS Intel x64: ID `12552085184`; Windows x64: ID `12550969062`; Windows arm64: ID `12550718491`.
- ASan diagnostics: ID `12551257220`. The same run also retained six platform hash artifacts, six archive artifacts, and RetroArch frame/marker evidence (IDs `12551985757` and `12551647433`). All listed artifacts identify source head `dcdb7b39bb0aec6ff74b192bfa239f6c0dc9243b`.

## Task Commits

### Post-execution source-review follow-up

Independent review found CR-01 in protected final-raw coverage. Commit `c23d0ac`
adds per-key uniqueness/completeness and malformed-record controls; independent
re-review is clean. This later measurement supersedes the earlier build-input
fingerprint above for current source: 56 inputs now hash to
`5aedbb3b04eca54a04d42e4e0ec7851e2e694cd72fd032d2ab1c29dfcbf2afe0`, while the
campaign report hash and all result counts remain unchanged. Exact-source
[PR CI 40075421001](https://github.com/szTheory/nesturbator/actions/runs/40075421001)
passed all 15 jobs and primary Linux 663/663 tests. See
[the fix evidence](17-REVIEW-FIX.md), [the clean review](17-REVIEW.md), and
[the regression gate](17-REGRESSION.md) for the latest attribution and artifact hash.

The persisted plan ledger records 13 commits between `plan_head_before` and the tested production head. The plan implementation commits are:

1. **Task 1: Activate the fixture, measured baseline and protected migration** — `584ce9f`, with correctness/provenance follow-ups `cc48e64`, `91fb93b`, `51c9ce8` and `dcdb7b3`.
2. **Task 2: Place recurring observation from measured CI cost** — `6f533c6`, with documentation/evidence/budget updates `8429599`, `7b026fb`, `15b17f0` and `47601b1`.
3. Branch synchronization: `ada8cb3`; the plan-range ledger also includes upstream commits `b39a010` and `e47feb3` brought in during phase-branch maintenance.

`dcdb7b3` closes the final same-pin next-main activation gap and is the exact production source head verified by hosted CI. Plan metadata is committed locally after this summary and does not alter that tested source identity.

## Files Created/Modified

- `tests/accuracy/scoreboard.txt`, `baseline-metadata.txt`, `migration.tsv`, and the retained old scoreboard/metadata provide the compact durable baseline and reviewed comparison.
- `tests/roms/accuracycoin.nes`, `accuracycoin-673ef.nes`, and `manifest.txt` bind the canonical and comparison fixtures.
- `runner/accuracycoin.c`, `runner/accuracycoin.h`, `tests/accuracy/test_conformance.c`, `tests/accuracy/test_scoreboard.c`, and the `tests/cmake/accuracycoin_*.cmake` checks implement and verify collection, activation, protected observations and campaign controls.
- `tests/CMakeLists.txt`, `.github/workflows/ci.yml`, `README.md`, `include/nesturbator.h`, and `.planning/preparation/{CONFORMANCE,LIBRETRO-AND-RUNNER}.md` register the chosen lane and document contracts and evidence.
- `tests/retroarch/test_state_session.sh` uses a validated unique scratch root for each run; its README and public-header test contract notes were updated in the same behavior commit.

## Decisions Made

- Treat canonical baseline metadata and the canonical scoreboard bytes as one authenticated authority from the same exact protected-main commit. Legacy manifest fallback remains available for prior-format protected main snapshots.
- Derive protected pages on the current invocation from every PASS key in the selected baseline. Preserve the old 17-key migration controls separately; do not transfer evidence between lanes or runs.
- Place one full campaign on primary Linux x64 PR CI based on measured cost. Keep every platform's actual protected observations and all pre-existing gates; retain existing workflow/job timeouts.
- Keep the campaign input identity tied to selected source/build inputs, not generated output. State the pre-commit revision and dirty explanation accurately.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Authenticate the canonical protected-main campaign scoreboard**
- **Found during:** Task 1 activation controls.
- **Issue:** The pre-configuration baseline fetcher accepted the earlier dedicated/local snapshot formats but could not select the newly canonical campaign scoreboard plus matching same-commit metadata.
- **Fix:** Added canonical schema/hash/count/pin/manifest validation and a PASS-only projection; added a positive local-Git protected-main control and a reason-specific scoreboard-byte hash-mismatch negative.
- **Verification:** `accuracycoin.baseline_fetch` passed in hosted full CI and the Linux x64 artifact retains `build/ci/accuracycoin/**` evidence.
- **Committed in:** `dcdb7b3`.

**2. [Rule 2 - Missing correctness] Protect every PASS key when main adopts the new pin**
- **Found during:** Final activation contract review.
- **Issue:** Static old pages 2/14/17 would stop proving same-pin results for the complete 101-PASS baseline after it became protected main.
- **Fix:** The selected current baseline now drives bounded page planning and fresh same-invocation raw/status verification; the full-lane production-path control covers all 101 PASS rows and rejects outside-page and formerly changed-meaning failures.
- **Verification:** `accuracycoin.regression_controls` passed on Linux x64 PR CI in 22.25s; full CI passed 663/663.
- **Committed in:** `dcdb7b3`.

**3. [Rule 1 - Portability] Bound decoder keys, preserve CRLF reports and isolate session fixtures**
- **Found during:** Cross-platform compiler and hosted-run validation.
- **Issue:** GCC found an unbounded key-formatting site, Windows report text translation required byte-exact CRLF handling, and a reused PID-derived scratch path collided with stale fixture data.
- **Fix:** Reject overlong keys explicitly, accept platform-correct line endings while retaining malformed-line negatives, create/validate a unique scratch root, and document the related contracts.
- **Verification:** Exact clang-format 18.1.8 check passed (`133 files formatted`); hosted six-platform workflow passed, including both Windows lanes; `retroarch.session.controls` passed in hosted CI in 274.13s.
- **Committed in:** `91fb93b`, `51c9ce8` and `dcdb7b3`.

**Total deviations:** 3 correctness/portability fixes. **Impact:** Necessary to make canonical next-main activation, Windows parsing and repeatable fixtures correct; no dependency or core hardware change was added.

## Issues Encountered

- Two local full-workflow attempts each finished CTest with 658/663 passing and five timeouts. The first unshielded run timed out in tests 211, 221, 301, 647 and 648. The subsequent command-scoped `caffeinate -i cmake --workflow --preset ci` run timed out in tests 219, 220, 270, 647 and 648. Its CTest total was 2481.02 seconds. Power logs show a 1003-second “Maintenance Sleep” from 01:47:15 to 02:03:58 EDT during that run even while the owned caffeinate process held `PreventUserIdleSystemSleep`; this is recorded as overlapping host sleep evidence, not proven resource causation. Workflow/test timeouts were not changed to cover local suspension.
- Both local failure logs were preserved outside the repository with SHA-256 values: first run `c6b15bbcf2f0e79b0d440bbb7a0527c77225f1f637fe13a471d75c747abc91f2` (CTest log) and `ed865c9b8aa735c1b884051f8cff0fa05bc2b96b660e16ae649dfbe46ddf73d2` (failure list); second run `8a04cffaa91a3048c882e63bcfb0baaa3f272228db6cc1cf34c64aea12de4fd2` and `bb9d4e21f97fe4ba868fc155c6dd21cf7fcf687795d6d63abe48d31148e0522f` respectively. The exact-source hosted workflow completed successfully, including the same long session controls.
- Separate Nightly run `40070974273` passed `mmc3-oracle`, `state-fuzz` and `rom-loader-fuzz`; `vectors-full` passed. `suite-flake` remained in progress at closeout. This separate run is not the selected Plan 17-04 CI gate and no conclusion is claimed for its unfinished job.
- `scripts/ci_monitor.cjs`, referenced by the local GitHub-workflows skill, is absent from this checkout. Hosted run, job and artifact status was therefore verified with the connected GitHub Actions read tools. No workflow syntax was changed in the final activation-gap commit.
- The pre-existing `.planning/config.json` edit remains byte-for-byte unchanged and unstaged (SHA-256 `0517388f5d422ae97afdc5dc1ae11bc43ebd481a757424f152ee8af61559ff5e`).

## User Setup Required

None.

## Next Phase Readiness

Plan 17-04 implementation, activation controls and measured CI placement are ready for the parent phase review. PR #54 remains open and draft, mergeable, and unmerged. Keep CONF-01 through CONF-04 pending until parent phase verification. Do not start Phase 18 or a milestone lifecycle step from this plan.

---
*Phase: 17-complete-pinned-conformance-baseline*
*Completed: 2026-10-24*

## Self-Check: PASSED

- Summary file exists at the required phase path.
- Tested production commit `dcdb7b39bb0aec6ff74b192bfa239f6c0dc9243b` is reachable from HEAD.
- Persisted ledger count is 13 from base `393cb34b445d2c5295226d1597706b250e04ee6e`; the production HEAD remains `dcdb7b39bb0aec6ff74b192bfa239f6c0dc9243b` before the metadata commit.
- Hosted run, artifact, local failure and pending-requirements claims are recorded with their exact source identity and outcomes.
