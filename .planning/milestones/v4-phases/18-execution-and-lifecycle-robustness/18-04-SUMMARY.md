---
phase: 18-execution-and-lifecycle-robustness
plan: "04"
subsystem: testing
tags: [C, CMake, libFuzzer, ASan, UBSan, GitHub-Actions, state, libretro]
requires:
  - phase: 18-02
    provides: six-family execution recipes and measured replay instrumentation
  - phase: 18-03
    provides: loop/JAM, state lifecycle and adapter presentation contracts
provides:
  - required six-family replay and campaign-control tests across normal CI
  - copied-corpus bounded Linux sanitizer campaigns with source-bound evidence
  - exact-host correction for state-load presentation-buffer lifetime
  - current six-platform, sanitizer, parity, protected, performance and RetroArch evidence
affects: [phase-18-verification, ci, fuzzing, save-state, libretro]
actuals:
  tokens: 35021
  tasks: 3
  commits: 12
plan_head_before: 2a7fde878fddf2468bb701c1f177e50824d1a440
plan_head_after: 23ef1a05e8dfbd1ffc7958c1ec60509597e26891
tech-stack:
  added: []
  patterns:
    - copied source corpus with before/after manifest verification
    - independent deterministic target caps and measured process/fuzzer limits
    - synthetic failure controls exercise actual evidence collection
key-files:
  created:
    - tests/cmake/execution_campaign.cmake
    - tests/fuzz/execution-budgets.txt
    - tests/fuzz/execution-measurements.md
  modified:
    - tests/fuzz/execution.c
    - tests/cmake/execution_measure.cmake
    - tests/cmake/fuzz_registration.cmake
    - tests/cmake/nightly_workflow_policy.cmake
    - tests/CMakeLists.txt
    - .github/workflows/nightly.yml
    - .github/workflows/ci.yml
    - libretro/libretro.c
    - tests/libretro/libretro_host.c
    - README.md
    - include/nesturbator.h
key-decisions:
  - Use the repository's existing Linux Clang libFuzzer lane because local Apple Clang has no libFuzzer runtime.
  - Derive target budgets from a hosted Linux pilot and retain separate fuzzer, replay, memory-sampling and outer-process limits.
  - On state restore, invalidate frame validity while preserving the XRGB storage already handed to the frontend.
  - Extend measured CI/Nightly timeout bounds only after exact-host runs demonstrated that prior limits ended before evidence or test collection finished.
patterns-established:
  - Campaign mutation uses a writable copy; original descriptors and source manifest remain available for diagnosis.
  - A finite clean campaign records actual work and stop causes and does not imply universal reliability.
requirements: [ROB-01, ROB-02, ROB-03, ROB-04]
requirements-completed: []
duration: 3h20m
completed: 2026-10-24
status: complete
---

# Phase 18 Plan 04: Bounded Execution Campaigns Summary

**Measured copied-corpus sanitizer campaigns and corrected a reproduced libretro state-load presentation failure.**

## Performance

- **Duration:** about 3h20m from the first task commit; initial plan preparation preceded that commit.
- **Started:** 2026-10-24 16:14 EDT (first task commit).
- **Completed:** 2026-10-24.
- **Tasks:** 3/3.
- **Files modified:** 20 plan source/build/workflow/documentation files.
- **Plan source commits:** 12, measured from the persisted plan ledger; metadata commit is separate.

## Accomplishments

- Added a required execution replay corpus with eight recipes covering mappers 0, 1, 2, 3, 4 and 7, plus loop and JAM outcomes. Normal CTest and the ordinary sanitizer replay require those seeds, registrations and evidence controls.
- Added finite campaign accounting that copies each source corpus into writable build storage, hashes source inputs before and after mutation, preserves originals, validates measured budgets and records exact run limits, toolchain, seed, stopping cause, elapsed time, RSS and coverage.
- Selected final bounds from a real Linux Clang 18.1.3 ASan/UBSan/libFuzzer pilot. The final per-target bounds are 100,000 runs/10 seconds/768 MiB sampled RSS for ROM loading, 1,000 runs/15 seconds/512 MiB for state loading, and 2,000 runs/15 seconds/256 MiB for execution. All use 2-second per-input timeouts, 163,856-byte maximum input, seed 1804 and a separate 30-second process timeout. RSS is libFuzzer's sampled limit, not an instantaneous allocation quota.
- Tested deliberate crash, per-input timeout, resource-failure, collector-failure, cancellation and minimization controls in disposable processes. Exact crash/timeout inputs were retained; the resource fixture's lack of an exact input is recorded; canceled work is classified incomplete; originals survive minimization.
- The exact-head Nightly campaign on PR head `23ef1a0` completed all targets with zero reproducers. Its before/after source corpus tables match and carry manifest SHA-256 `f524ddcbf03dea6faa115938df004cbfe08eae8aaac9921c3b4b39ec9bb95ac1`.
- A separate hosted RetroArch progression run reproduced a real presentation-buffer lifetime failure after runahead and state-slot load. The fix preserves XRGB bytes already passed to the frontend, invalidates only frame validity, adds a callback-buffer regression, retains black output on the next JAM callback, and updates README/header comments.
- Final exact-head CI passed all 15 jobs: six platform builds, Linux 668/668 tests and three packages, ASan, both no-FP lanes, perf, RetroArch E2E, six-platform hash equality and the required rollup. The three deliverables remain available.

## Final Campaign Evidence

The Linux pilot was [run 40129666207](https://github.com/szTheory/nesturbator/actions/runs/40129666207) on source `694455e`. It measured useful mutation throughput and sampled RSS before final budgets were selected.

The exact-head campaign was [Nightly run 40140031327](https://github.com/szTheory/nesturbator/actions/runs/40140031327), testing PR head `23ef1a05e8dfbd1ffc7958c1ec60509597e26891` through merge revision `428d71e9b533555e095cb2605c98d6611ba5aff5`. It used Ubuntu 24.04 x86_64, Clang 18.1.3, Release `-O1`, ASan/UBSan/libFuzzer and the unchanged source corpus manifest. All three targets completed with `failure_class=none` and zero reproducers:

| Target | Bound | Actual work / stop | Outer elapsed | Peak RSS | Edges / features |
| --- | --- | --- | ---: | ---: | ---: |
| ROM loader | 100,000 runs, 10 s, 768 MiB | 100,000; run-count cap | 2.12 s | 499,429,376 B | 294 / 319 |
| State loader | 1,000 runs, 15 s, 512 MiB | 778; max-total-time | 16.26 s | 374,779,904 B | 1,332 / 2,381 |
| Execution | 2,000 runs, 15 s, 256 MiB | 2,000; run-count cap | 10.14 s | 126,509,056 B | 1,383 / 1,987 |

The final [execution campaign artifact 12578900226](https://github.com/szTheory/nesturbator/actions/runs/40140031327/artifacts/12578900226) is 286,139 bytes with digest `sha256:5cb3ee0043ae62bec1dd68b83f581ca136bdf7988bf1aba73feb50d67ffcbe44`. It contains source-corpus manifests, copied working corpora, exact descriptors and hashes, run records, logs and deliberate evidence-control fixtures. The campaign reports only the work observed under these finite limits; it makes no universal reliability claim.

## Exact-Head CI and Host Evidence

- [CI run 40140031333](https://github.com/szTheory/nesturbator/actions/runs/40140031333) passed all 15 jobs on head `23ef1a0`. Its Linux x64 lane passed 668/668 tests in 610.95 seconds and generated library, runner and libretro packages. The protected observation, protected PASS, no-lost-pass and regression-control tests passed.
- All six downloaded platform hash manifests are byte-identical; `hashes.txt` SHA-256 is `718946884e27fd589ba243a6d22296d1ac8d8e3c416d14541dcd34ebb5eb0fa1`.
- The base-to-head perf comparison passed: nesteroids 23,743,914→23,743,918 Ir/frame; runner 24,651,894→24,651,898; M4_P256K_C256K 23,319,733→23,319,737. Each delta is +4 Ir/frame (+0.00%). The exact-host setup showed the prior 10-minute job cap could expire during Valgrind installation, so it was raised to 15 minutes; the subsequent exact run passed.
- Hosted RetroArch E2E passed on the same head, including pinned exact progression, game frame, normal and MMC3 battery round trips, and deterministic auto-state lanes. [Marker progression artifact 12578218505](https://github.com/szTheory/nesturbator/actions/runs/40140031333/artifacts/12578218505) and [frame artifact 12578797936](https://github.com/szTheory/nesturbator/actions/runs/40140031333/artifacts/12578797936) belong to this run.
- The plan's focused local protected suite passed 8/8 with `ctest --preset ci -R '^(accuracycoin\\.(protected|protected_observation|regression_controls|baseline_fetch)|accuracy\\.scoreboard.*)$' --no-tests=error`.
- The first exact-head Nightly run reached the old 40-minute suite-flake job cap while its third slow `retroarch.session.controls` repeat was still in progress. The first two repeats took 450.35 and 451.35 seconds; no test failure was reported. The timeout was raised to 50 minutes, and the final Nightly suite completed successfully in 43m32s with all three randomized repeats.

## Task Commits

1. **Task 1: Implement finite campaign accounting and evidence controls** — `6758e85`, `27985c2`.
2. **Task 2: Measure Linux mutation, set budgets and integrate CI/Nightly replay** — `fbab2c3`, `13a7168`, `694455e`, `f1ba984`, `8363ae4`, `282e67e`, `f4bf381`, `23ef1a0`.
3. **Task 3: Resolve the reproduced adapter finding and record acceptance evidence** — `adf6457`, `5a49662`.

The plan ledger measures 12 commits from `2a7fde878fddf2468bb701c1f177e50824d1a440` through source head `23ef1a05e8dfbd1ffc7958c1ec60509597e26891`. The state/summary metadata commit is separate.

## Files Created and Modified

- `tests/cmake/execution_campaign.cmake` — bounded campaign, replay and evidence-control driver.
- `tests/cmake/execution_measure.cmake`, `tests/fuzz/execution.c` — matched measurement and independently capped execution target.
- `tests/fuzz/execution-budgets.txt`, `tests/fuzz/execution-measurements.md` — measured pilot/final limits and durable campaign records.
- `.github/workflows/nightly.yml`, `.github/workflows/ci.yml` — Linux sanitizer lane, failure evidence, and measured job timeouts.
- `tests/CMakeLists.txt`, `tests/cmake/fuzz_registration.cmake`, `tests/cmake/nightly_workflow_policy.cmake` — required seed, registration, controls and workflow guards.
- `tests/fuzz/rom_loader.c`, `tests/fuzz/state_loader.c`, `tests/fuzz/test_execution.c`, `tests/runner/test_movie.c` — replay/fuzz test support and formatting.
- `libretro/libretro.c`, `tests/libretro/libretro_host.c` — state-load presentation invalidation and regression.
- `README.md`, `include/nesturbator.h`, `.planning/preparation/ENGINEERING.md`, `.planning/preparation/CONFORMANCE.md` — paired behavior, budget and process documentation.
- `tests/accuracy/baseline-metadata.txt` — refreshed provenance from the real AccuracyCoin output while preserving the canonical report bytes and counts.

## Decisions Made

- The local Apple Clang toolchain has no libFuzzer runtime, so actual mutation and budget selection use the already-provisioned Linux Clang workflow path.
- Campaign caps are separate from target-enforced input/allocation/frame/operation limits, fixed-replay timeout, fuzzer time/run/input/RSS limits and the outer process timeout. RSS is sampled, not a hard memory quota.
- The front-end-visible XRGB storage remains intact through successful state restoration; cache validity is invalidated and normal complete-frame/JAM behavior remains tested.
- Nightly suite-flake budget uses a 50-minute job limit after the exact 40-minute run demonstrated the prior bound was too short; the final three-repeat suite completed within the new limit.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Preserve frontend-visible video storage during successful state load**

- **Found during:** Task 3, exact-host RetroArch progression.
- **Issue:** After runahead toggle and state load, the frame marker failed at boundary 16. The successful state-load path cleared shared XRGB storage while the frontend could still observe the buffer passed by the previous video callback.
- **Fix:** Invalidate frame validity without zeroing the shared storage. Add a regression for buffer bytes remaining unchanged on load and retain the next-JAM black-output assertion.
- **Files:** `libretro/libretro.c`, `tests/libretro/libretro_host.c`, `README.md`, `include/nesturbator.h`, plus refreshed AccuracyCoin provenance metadata.
- **Verification:** Focused host test, full local CI 668/668, and final hosted RetroArch E2E passed.
- **Commit:** `5a49662`.

**2. [Rule 3 - Blocking issue] Include measured setup and artifact time in the perf job budget**

- **Found during:** Task 3, exact hosted CI.
- **Issue:** A 10-minute perf job expired while the Valgrind installation was still running (7m44s in the recorded attempt), before measurement/evidence could complete.
- **Fix:** Raise the measured job cap to 15 minutes. The exact-head perf lane then completed in 3m25s with all comparisons passing.
- **Files:** `.github/workflows/ci.yml`.
- **Verification:** `actionlint .github/workflows/ci.yml`, diff checks and exact hosted CI perf gate.
- **Commit:** `f4bf381`.

**3. [Rule 3 - Blocking issue] Reserve enough time for the randomized full-suite lane**

- **Found during:** Final exact-head Nightly verification.
- **Issue:** Nightly run `40136996662` reached its 40-minute job cap during the third `retroarch.session.controls` repeat; the two completed repeats each took about 450 seconds and no test failure was reported.
- **Fix:** Increase `suite-flake` to a measured 50-minute bound with remaining-suite and cleanup headroom. The complete three-repeat randomized suite passed in 43m32s on the final head.
- **Files:** `.github/workflows/nightly.yml`.
- **Verification:** Nightly workflow policy and actionlint syntax checks, followed by successful full Nightly run `40140031327`.
- **Commit:** `23ef1a0`.

**Total deviations:** 3 auto-fixed (one behavior bug, two measured workflow budgets). All were required to complete accepted tests/evidence; no architecture or product scope was added.

## Issues Encountered and Limits

- Exact-host RetroArch found one real adapter contract issue; it now has a behavioral regression and correction. The finite fuzz campaigns themselves produced zero reproducers and no input was invented or attributed to a real failure.
- The resource-limit fixture did not retain an exact triggering input; the artifact records this limitation instead of claiming it did. The sampled RSS threshold is not an instantaneous allocation quota.
- Default `actionlint` reported pre-existing ShellCheck warnings SC2016/SC2129 in unchanged workflow scripts. The changed Nightly YAML passed actionlint syntax/semantic validation with ShellCheck disabled, and the workflow's dedicated policy test passed. The unrelated script warnings were not modified.
- Local libFuzzer execution is unavailable; all actual mutation runs use the recorded hosted Linux toolchain.
- No stubs, skipped tests or unrun final plan verifications remain. The initial 40-minute randomized suite attempt is retained as canceled evidence; its corrected final run passed.

## User Setup Required

None.

## Next Phase Readiness

Plan 18-04 is complete and PR #55 remains a draft, open and unmerged. Requirements ROB-01 through ROB-04 remain pending independent Phase 18 goal verification; this plan does not mark them complete. The phase-level verifier owns requirement status and phase completion.

---
*Phase: 18-execution-and-lifecycle-robustness*
*Completed: 2026-10-24*

## Self-Check: PASSED

- Summary file exists, contains exact CI/Nightly and artifact references, and has no whitespace errors.
- All 12 source task commits listed above are ancestors of `23ef1a05e8dfbd1ffc7958c1ec60509597e26891`.
- `.planning/config.json` remains the only pre-existing user edit and is excluded from this plan's files and commits.

## Review Findings Follow-up (2026-10-25)

After the original plan close-out, source review confirmed five warnings in the campaign and
state-loader evidence controls. The source fixes are committed in `91ca836`, `25ee82a`, `9b04024`
and `78bb8d8`. `18-REVIEW.md` remains unchanged; the exact titles, RED/GREEN evidence and limits
are recorded in [`18-REVIEW-FIX.md`](18-REVIEW-FIX.md), with current dispositions in
[`18-REVIEW-DISPOSITION.md`](18-REVIEW-DISPOSITION.md).

- **Minimization and collection:** Exact-source Nightly run
  [40148240864](https://github.com/szTheory/nesturbator/actions/runs/40148240864) completed
  successfully, including three randomized suite passes, Linux state/ROM fuzz, the bounded campaign,
  target-specific failure reproductions, negative target/class/operation/acceptance controls, and
  collector-copy controls. Synthetic minimization results were ROM 131,088→131,088 bytes (no
  smaller input found), state 3→2, and execution 23→17; no real campaign finding was invented.
  Its campaign artifact is ID `12580614764` with SHA-256
  `d7ff6f7a93c3b33143c62c85315cde76cdf2ddf309c190d63896abd85debaf7b`. Campaign limits and
  observed runs remain as recorded in the review-fix artifact.
- **JAM and tick accounting:** The positive JAM oracle accepts equivalent canonical post-JAM states
  despite poisoned, differing stale output buffers; its negative control rejects deliberately
  divergent canonical post-JAM states with identical poison buffers. The corrected state-loader
  matched replay records 17,155,728 ticks for 24 successful frames and zero output ticks for 12 JAM
  calls. The earlier 25,733,664 observation remains in the historical table but is marked invalid
  as guest work. The 24-function coverage count and 20,728,248 execution ticks were refreshed with
  the corrected source.
- **Exact-source CI:** PR CI
  [40148240830](https://github.com/szTheory/nesturbator/actions/runs/40148240830) passed all 15
  jobs on code head `78bb8d8` (merge revision `c0518a4`): six platform builds, ASan, both no-FP
  lanes, Cachegrind, RetroArch E2E, hygiene, hash equality and required CI gates. Six platform
  `hashes.txt` manifests compared byte-for-byte; each has SHA-256
  `718946884e27fd589ba243a6d22296d1ac8d8e3c416d14541dcd34ebb5eb0fa1`. The hosted RetroArch tests
  passed exact progression and negative controls, frame parity, mapper-1/MMC3 battery round trips,
  and deterministic state save/load lanes.
- **AccuracyCoin provenance:** The exact-source full campaign preserved all 22 pages and 146 scored
  rows: 101 pass, 42 fail, 2 not-run, 1 timeout, and 5 inventory-only draw rows. The canonical
  report SHA-256 remains
  `6d0a811960f6964e7fc66802c6e4ade0c4ffb8772c40d88d50cf79e1f8463652`; the protected scoreboard
  SHA-256 remains
  `9e96015ca205f0ed2a351c21d7b6b0a718b26e8e6da87b84f2542adadd04279a`. The tracked
  `tests/accuracy/baseline-metadata.txt` now records the actual clean-source campaign revision,
  policy and elapsed time. Its activation test passed after restoring the required clean-source
  reason field.
- **Final local verification:** After that metadata correction, `cmake --workflow --preset ci`
  completed with 669/669 tests and generated the library, runner and libretro ZIP packages. The
  first post-refresh run exposed only the omitted required `source_dirty_reason`; that key was
  restored from the clean campaign provenance, the focused activation test passed, and the full
  workflow then passed.

These follow-up fixes close only WR-01 through WR-05. Requirements ROB-01 through ROB-04 remain
pending independent Phase 18 verification. PR #55 remains draft, open and unmerged.

## Follow-up Self-Check: PASSED

- The disposition lists all five original warning IDs and `18-REVIEW-FIX.md` retains their exact
  titles and evidence; the original review file has no changes.
- Exact-source hosted CI and Nightly runs concluded successfully, and the corrected local workflow
  passed all 669 tests and generated all three packages.
- `.planning/config.json` remains the pre-existing user edit and is excluded from the follow-up.
