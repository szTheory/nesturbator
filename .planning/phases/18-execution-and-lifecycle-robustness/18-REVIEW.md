---
phase: 18-execution-and-lifecycle-robustness
reviewed: 2026-10-24T23:51:28Z
depth: standard
files_reviewed: 29
files_reviewed_list:
  - .github/workflows/ci.yml
  - .github/workflows/nightly.yml
  - README.md
  - include/nesturbator.h
  - libretro/libretro.c
  - tests/CMakeLists.txt
  - tests/accuracy/baseline-metadata.txt
  - tests/cmake/execution_campaign.cmake
  - tests/cmake/execution_measure.cmake
  - tests/cmake/fuzz_registration.cmake
  - tests/cmake/nightly_workflow_policy.cmake
  - tests/core/test_frame.c
  - tests/fuzz/corpus_execution/axrom-deep
  - tests/fuzz/corpus_execution/cnrom-deep
  - tests/fuzz/corpus_execution/jam
  - tests/fuzz/corpus_execution/loop
  - tests/fuzz/corpus_execution/mmc1-deep
  - tests/fuzz/corpus_execution/mmc3-deep
  - tests/fuzz/corpus_execution/nrom-deep
  - tests/fuzz/corpus_execution/uxrom-deep
  - tests/fuzz/execution-budgets.txt
  - tests/fuzz/execution-measurements.md
  - tests/fuzz/execution.c
  - tests/fuzz/replay_main.h
  - tests/fuzz/rom_loader.c
  - tests/fuzz/state_loader.c
  - tests/fuzz/test_execution.c
  - tests/libretro/libretro_host.c
  - tests/runner/test_movie.c
findings:
  critical: 0
  warning: 5
  info: 0
  total: 5
status: issues_found
---

# Phase 18: Code Review Report

**Reviewed:** 2026-10-24T23:51:28Z  
**Depth:** standard  
**Files Reviewed:** 29  
**Status:** issues_found

## Summary

The execution harness, mapper recipes, lifecycle controls, adapter cache policy, and CI wiring were reviewed against the Phase 18 contracts. The libretro cache preserves frontend-visible frame storage across state loads and the new execution oracle avoids partial JAM output. The campaign minimization and artifact-control paths do not establish the failure evidence they claim, and the existing state-loader comparison violates the now-documented JAM output-validity contract. Its added guest-tick metric also sums absolute timestamps rather than executed work.

## Narrative Findings (AI reviewer)

### Warnings

### WR-01 [WARNING]: Loader-target minimization validates inputs as execution recipes

**File:** `tests/cmake/execution_campaign.cmake:357-390`
**Issue:** Minimization mode explicitly accepts `FUZZ_TARGET=rom-loader` and `state-loader` at line 362, but `validate_minimized` always calls `measure_execution` at lines 144-149. That helper runs `fuzz.execution --measure`, which only accepts NEXE descriptors and requires an accepted generated execution image. ROM-loader and state-loader reproducers therefore fail validation even when their own libFuzzer minimizer preserves them.
**Fix:** Dispatch validation by `FUZZ_TARGET`. Replay ROM-loader and state-loader inputs through their corresponding target-specific acceptance and failure checks; reserve the NEXE execution validator for `FUZZ_TARGET=execution`.

### WR-02 [WARNING]: Execution minimization cannot verify a reproduced failure

**File:** `tests/cmake/execution_campaign.cmake:124-160, 378-390`; `tests/fuzz/execution.c:609-615, 1332-1338`
**Issue:** `validate_minimized` only compares the caller-supplied class strings, then requires both inputs to complete a successful `--measure` run. A minimized input that still triggers an `nt_exec_fail` abort or sanitizer crash returns nonzero and is rejected, while the validator never runs the fuzz binary to derive and compare the actual failure class. The claimed operation is also only `operation_count - 1`; `nt_exec_operation` records sequential indices, not which API operation failed. Thus the flow cannot accept and prove an execution reproducer with the same failure class at the intended operation.
**Fix:** Keep accepted-load verification separate from failure reproduction. Run original and reduced inputs in disposable processes using the same instrumented target, derive and compare the observed failure class, and record named operation identities at the operation sites so both inputs can be checked against the intended failing operation.

### WR-03 [WARNING]: Evidence self-test writes directly to the expected artifact paths

**File:** `tests/cmake/execution_campaign.cmake:7-17, 238-275, 639-650`
**Issue:** The crash and timeout child controls write the synthetic descriptor directly to `CHILD_ARTIFACT`, which is the final path that `require_artifact` checks. They do not create a libFuzzer artifact under `artifact_dir` or exercise the campaign's artifact discovery and copy loop at lines 639-650. The controls can pass even if real crash/timeout inputs are no longer collected or copied for upload.
**Fix:** Exercise the same collection path used by campaign runs: trigger crash and timeout artifacts from a disposable fuzz target, or factor artifact discovery/copying into a helper and feed it inputs in the actual `artifact_dir`; assert that the resulting uploaded reproducer matches the triggering input.

### WR-04 [WARNING]: State-loader oracle hashes frame outputs after JAM

**File:** `tests/fuzz/state_loader.c:237-253, 348-351`
**Issue:** The public header states that `STOP_JAM` leaves frame output fields unspecified, but `nt_run` hashes `video`, `audio_count`, `frame_number`, and `ticks` unconditionally after accepting either `OK` or `STOP_JAM`. The video/audio arrays are static and reused. If both same-state instances JAM, the second run can hash the first instance's stale video, and the oracle compares only those hashes rather than the two post-run canonical states. This can allow a divergent JAM continuation to pass based on shared stale buffer contents.
**Fix:** On `STOP_JAM`, compare only stop status and canonical saved state for each instance; do not read frame metadata or video/audio outputs. Compare the two post-run state snapshots in the accepted-state continuation path.

### WR-05 [WARNING]: State-loader guest-tick metric sums absolute timestamps

**File:** `tests/fuzz/state_loader.c:237-242`
**Issue:** `frame.ticks` is documented in `include/nesturbator.h` as cumulative emulated time since power-on and is restored from state. Adding it for every successful frame counts the preexisting state origin repeatedly instead of measuring work performed by those frame calls. Mutated accepted states with larger tick origins therefore inflate the recorded `guest_ticks` metric and distort the matched work comparison.
**Fix:** Accumulate per-call tick deltas from a tracked baseline for each run, including the restored origin, or remove the guest-work metric if an exact baseline cannot be obtained.

---

_Reviewed: 2026-10-24T23:51:28Z_  
_Reviewer: the agent (gsd-code-reviewer)_  
_Depth: standard_
