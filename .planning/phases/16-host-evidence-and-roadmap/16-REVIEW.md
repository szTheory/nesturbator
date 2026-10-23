---
phase: 16-host-evidence-and-roadmap
reviewed: 2026-10-23T18:35:39Z
depth: standard
files_reviewed: 6
files_reviewed_list:
  - README.md
  - include/nesturbator.h
  - tests/retroarch/CMakeLists.txt
  - tests/retroarch/interruption_trace.sh
  - tests/retroarch/state_session.sh
  - tests/retroarch/test_state_session.sh
findings:
  critical: 0
  warning: 1
  info: 0
  total: 1
status: issues_found
---

# Phase 16: Code Review Report

**Reviewed:** 2026-10-23T18:35:39Z

**Depth:** standard

**Files Reviewed:** 6

**Reviewed HEAD:** `d5cce3f86239ac15cedad14b27131d84a2a304b3`

**Diff base:** `dfc4f66e9b92aa4395096916b5aefaaff20176c3`
**Status:** issues_found

## Summary

Reviewed the six scoped source and documentation paths at current HEAD. The Plan 16-56 exact-source review and its disposition remain separate historical evidence; this report checks the current tree, including the later Plan 16-07 changes. The current CTest registrations do not execute the new source-root preflight matrix, leaving its copied-input refusal and finalizer working-directory coverage outside the normal CI workflow.

## Warnings

### WR-07: Source-root preflight is not run by the registered CI tests

**Severity:** WARNING

**File:** `tests/retroarch/CMakeLists.txt:71-88`; `tests/retroarch/test_state_session.sh:3770-3807`
**Issue:** CMake registers `retroarch.session.controls` and `retroarch.session.readiness`, but neither sets `STATE_SESSION_CONTROL=source-root-preflight`. The controls test uses the default `CONTROL=all`; that branch runs the interruption, lock PID and lock release checks but never calls `source_root_preflight`. The only call is behind the standalone selector at lines 3770-3772. Consequently, the advertised three-working-directory matrix, its copied-driver/include refusal check, and its twelve CWD-specific finalizer cases are not exercised by `cmake --workflow --preset ci`. A regression that permits a copied or mismatched input can pass the registered session tests.
**Fix:** Register a `retroarch.session.source_root` CTest entry that runs the harness with `STATE_SESSION_CONTROL=source-root-preflight`, and give it `RUN_SERIAL TRUE` with a bounded timeout. Alternatively, call `source_root_preflight` from the already serial `all` control.

## Narrative Findings (AI reviewer)

The finding above is the only actionable issue identified in the reviewed scope. No source behavior blocker was found in the current session driver or trace helper.

---

_Reviewed: 2026-10-23T18:35:39Z_
_Reviewer: the agent (gsd-code-reviewer)_
_Depth: standard_
