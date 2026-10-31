---
phase: 23-reproduce-and-report-a-scene
reviewed: 2026-10-31T03:52:36Z
depth: standard
files_reviewed: 8
files_reviewed_list:
  - README.md
  - docs/progress.md
  - docs/reproduction.md
  - docs/runner.md
  - docs/testing.md
  - tests/CMakeLists.txt
  - tests/cmake/reproduction.cmake
  - tests/cmake/reproduction_contract.cmake
findings:
  critical: 1
  warning: 2
  info: 0
  total: 3
status: issues_found
---

# Phase 23: Code Review Report

**Reviewed:** 2026-10-31T03:52:36Z  
**Depth:** standard  
**Files Reviewed:** 8  
**Status:** issues_found

## Summary

The replay comparator checks complete ordered frame records and both audio hashes, and the documented output-layer limits are clear. I found one scratch-path safety issue and two preflight/test-contract defects that should be corrected before relying on these guarantees.

## Critical Issues

### CR-01: Scratch creation failure can redirect the POSIX walkthrough to `/build`

**Severity:** BLOCKER  
**File:** `docs/reproduction.md:82-86`  
**Issue:** The shell block continues if `mktemp` fails. In that case `scratch` is empty and `"$scratch/build"` expands to `/build`; the following configure/build commands can write into an unrelated existing writable directory instead of a fresh scratch root. This breaks the guide's isolation guarantee and can overwrite another build tree.  
**Fix:** Fail immediately and validate the created root before deriving paths, for example:

```sh
scratch="$(mktemp -d "${TMPDIR:-/tmp}/nesturbator-reproduction.XXXXXX")" || exit 1
test -n "$scratch" || exit 1
```

Also stop the block after failed configure/build commands, and make the PowerShell block stop when directory creation or a native command fails.

## Warnings

### WR-01: The documentation contract fails under the ASan preset

**Severity:** WARNING  
**File:** `tests/cmake/reproduction_contract.cmake:369-390`; `tests/CMakeLists.txt:1598-1604, 1677-1695`  
**Issue:** The reproduction contract requires every documented test name to exist in the current generated CTest metadata, including `runner.boot_to_play`. The ASan configure preset sets `NESTURBATOR_SHORT_RUNS=ON`, which intentionally omits that test from CTest, while `docs.reproduction` remains registered. Running the ASan test preset therefore fails the documentation contract even though the guide's documented command uses the normal `ci` preset.  
**Fix:** Make the contract aware of the short-run configuration or validate the documented CI selection against CI metadata so the intentional ASan omission does not fail the doc test.

### WR-02: Preflight success text asserts provenance the function does not establish

**Severity:** WARNING  
**File:** `tests/cmake/reproduction.cmake:79, 427-435`  
**Issue:** `reproduction_preflight` accepts the caller-supplied `runner-native-same-invocation` label plus matching file digests and then reports that the files match “a same-invocation native checkpoint.” The self-test demonstrates that the same function also accepts plain-text synthetic runner, ROM, and state files under that label. The guide correctly says the label does not authenticate arbitrary imported state, but the success message overstates what a passing preflight proves and can mislead someone reading the result alone.  
**Fix:** Report that the declared origin label and supplied digests match, and state that provenance is caller-supplied; reserve the same-invocation claim for the positive fixture path that actually creates the state with the runner.

---

_Reviewed: 2026-10-31T03:52:36Z_  
_Reviewer: the agent (gsd-code-reviewer)_  
_Depth: standard_
