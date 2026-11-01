---
phase: 22-install-and-protect-progress
reviewed: 2026-10-31T12:45:41Z
depth: standard
files_reviewed: 10
files_reviewed_list:
  - README.md
  - docs/progress.md
  - docs/reproduction.md
  - docs/runner.md
  - docs/testing.md
  - tests/CMakeLists.txt
  - tests/accuracy/baseline-metadata.txt
  - tests/accuracy/measurements.md
  - tests/cmake/reproduction.cmake
  - tests/cmake/reproduction_contract.cmake
findings:
  critical: 1
  warning: 2
  info: 0
  total: 3
status: issues_found
---

# Phase 22: Code Review Report

**Reviewed:** 2026-10-31T12:45:41Z
**Depth:** standard
**Files Reviewed:** 10
**Status:** issues_found

## Summary

Evaluation-scope resolution was degraded (`phase-range/no-task-commits-since`): the Phase 22 summaries were unchanged, so this refresh covers the broader ten-file scope supplied for the stale review, including shared Phase 23 documentation and checks. Static review found one unsafe scratch-directory failure path and two gaps in the reproduction-document contract checker. No tests or CI workflows were run during this review.

The original findings remain preserved by reference in [22-REVIEW-INITIAL.md](22-REVIEW-INITIAL.md): WR-01, “Apple Silicon install command assumes RetroArch’s default directories”; WR-02, “Release guide names the wrong attestation subject”; and WR-03, “Status-table checker does not enforce the canonical API guide path.” [22-REVIEW-DISPOSITION-INITIAL.md](22-REVIEW-DISPOSITION-INITIAL.md) records all three as fixed; this refresh does not reopen them.

## Narrative Findings (AI reviewer)

### Critical Issues

#### CR-01: Failed scratch creation can redirect progress checks to shared paths (**BLOCKER**)

**File:** `docs/progress.md:101-109`
**Issue:** The POSIX walkthrough does not stop or validate after `mktemp` fails. An empty `scratch` makes the following paths `/config`, `/build`, and `.backup`; the commands continue because the block has no `set -e`. This can configure or write into a shared `/build` directory and can overwrite `.backup/newest-battery.sav` in the repository root instead of keeping generated files isolated.
**Fix:** Stop the recipe on failure and verify the created directory before deriving any paths:

```sh
(
  set -e
  scratch="$(mktemp -d "${TMPDIR:-/tmp}/nesturbator-progress.XXXXXX")" || exit 1
  test -n "$scratch" && test -d "$scratch" || exit 1
  backup="${scratch}.backup"
  config="$scratch/config"
  # Continue with the existing commands.
)
```

### Warnings

#### WR-01: Reproduction contract accepts a noncanonical document path (**WARNING**)

**File:** `tests/cmake/reproduction_contract.cmake:76-80`
**Issue:** `reproduction_check` only requires `doc_path` to exist. The current CTest registration points to `docs/reproduction.md`, but if it is later redirected to an old copy, the contract can keep validating that copy while the user-facing canonical guide has drifted. The README-link assertion does not tie `doc_path` to the linked target.
**Fix:** In normal check mode, normalize `doc_path` and require it to equal `${SOURCE_DIR}/docs/reproduction.md`. Give mutation checks an explicit test-only path allowance so their temporary documents can still be checked.

#### WR-02: Reproduction contract does not verify POSIX and PowerShell parity (**WARNING**)

**File:** `tests/cmake/reproduction_contract.cmake:329-362`
**Issue:** The checker combines the target lists from both shell examples and removes duplicates, so it validates only their union. It also extracts only the first `-R` test selector. If the PowerShell example loses a required target or test while the POSIX example retains it, the contract still passes even though the PowerShell recipe can fail or skip a required check.
**Fix:** Parse each shell example separately and require each target list and CTest selection to contain the required entries (and match the other example). Add a mutation that removes one target or test only from the PowerShell block and expects rejection.

---

_Reviewed: 2026-10-31T12:45:41Z_
_Reviewer: the agent (gsd-code-reviewer)_
_Depth: standard_
