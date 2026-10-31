---
phase: 24-02-oam-dmc-overlap
reviewed: 2026-10-31T20:30:38Z
depth: standard
files_reviewed: 12
files_reviewed_list:
  - src/bus.c
  - tests/cpu/test_bus.c
  - tests/core/test_state_fork.c
  - include/nesturbator.h
  - README.md
  - docs/api.md
  - docs/reproduction.md
  - tests/cmake/phase21_hosted_evidence_controls.cmake
  - tests/cmake/reproduction_contract.cmake
  - tests/cmake/vector_api_policy.cmake
  - tests/core/test_api.c
  - tests/runner/hashes.txt
findings:
  critical: 0
  warning: 0
  info: 0
  total: 0
status: passed
---

# Phase 24-02: Code Review Report

**Reviewed:** 2026-10-31T20:30:38Z  
**Depth:** standard  
**Files Reviewed:** 12  
**Status:** passed

## Summary

The OAM/DMC arbitration implementation appears consistent with the cited overlap model: OAM advances on GET/PUT, the DMC GET occupies a GET slot, and the source byte is retried. No production blocker was found. The bounded bus test should pin its intended interior and tail outcomes independently of the observed event timing. The checkpoint's intentionally failing `conflict-latches` diagnostic concerns the separate, unapproved split-latch correction and is not an OAM finding.

## Narrative Findings (AI reviewer)

### WR-01: OAM tests derive expected duration from the observed trace

**Severity:** WARNING  
**File:** `tests/cpu/test_bus.c:952`

**Issue:** `run_oam_probe` computes `extra` from the observed halt cycle, the derived DMC GET cycle, and the baseline OAM end, then checks that elapsed cycles equal that value. It never asserts that `requested_offset=128` produces a GET within OAM and an exact two-cycle extension, or that the 254/255 probes hit the expected one- and three-cycle tail cases. A shift that moves one of those probes into a different timing class can be accepted as the newly computed expectation; the requested offset and detected collision offset are only printed.

**Fix:** Pin each matrix row to an independent expectation. Require the interior probe's DMC GET to overlap OAM with `extra == 2`, and assert the expected `extra` and GET/OAM relation for each final-window phase/load combination (`1` or `3` as appropriate). Also assert that the observed collision offset matches the configured window.

### WR-02: Save-state guide still describes format 3 as current

**Severity:** WARNING  
**File:** `docs/api.md:250`

**Issue:** The current guide identifies format 4 above, but this paragraph still says “Format 3 rejects every other format version, including versions 1 and 2.” This is a carried documentation defect from the prior slice, not an OAM behavior regression.

**Fix:** State that format 4 rejects other versions, including formats 1, 2 and 3.

---

_Reviewed: 2026-10-31T20:30:38Z_  
_Reviewer: the agent (gsd-code-reviewer)_  
_Depth: standard_

## Resolution and scope

WR-01 resolved: every interior/final phase/load row now pins its expected collision index, +2/+1/+3 extra clocks, GET/OAM-end relation, exact resumed CPU address/data/phase and source-resume index. The trace-derived relation remains an additional check only. Fresh default bus tests pass; independently compiling both production and observed copies from pre-OAM source rejects the fixed matrix, missing GET and wrong-phase controls.

WR-02 withdrawn after direct parent inspection: docs/api.md already said format 4 rejects formats 1/2/3 in the committed first-plan correction, and still does. The review quote was stale context, not the current file. Both doc contract/mutation checks pass with current revision-11 text. No active OAM finding remains; this does not approve or review a future split-latch implementation.
