---
plan: 16
phase: 16-host-evidence-and-roadmap
reviewed: 2026-10-16T12:57:22Z
depth: deep
reviewed_source_commit: 7a6b1aaa87c9f8eb244aa1e901becc32352ecf4e
reviewer_provenance: "Fresh independent separate-context review; resolved reviewer model gpt-6-luna, effort xhigh."
files_reviewed: 6
files_reviewed_list:
  - tests/retroarch/interruption_trace.sh
  - tests/retroarch/state_session.sh
  - tests/retroarch/test_state_session.sh
  - tests/retroarch/frame_index.c
  - README.md
  - include/nesturbator.h
findings:
  critical: 1
  warning: 4
  info: 0
  total: 5
status: issues_found
---

# Phase 16: Independent Code Review

**Reviewed:** 2026-10-16T12:57:22Z  
**Depth:** deep  
**Source commit:** `7a6b1aaa87c9f8eb244aa1e901becc32352ecf4e`  
**Provenance:** Fresh independent, separate-context review of the exact committed candidate. Reviewer model resolved to `gpt-6-luna`, effort `xhigh`.

## Summary

All six requested files were reviewed alongside the Plan 16 Task 3/acceptance context and the registration, wrapper, and preset files. Five actionable findings remain: one blocker in owner-sidecar cleanup and four warnings in trace validation, harness-ledger validation, and acceptance documentation. No historical assertion-failure cause is inferred.

## Critical Issues

### CR-01: Cleanup deletes a regular sidecar it failed to create

**Classification:** BLOCKER  
**File:** `tests/retroarch/state_session.sh:44`  
**Issue:** The EXIT trap is installed before the owner sidecar is created. If the noclobber create at line 52 fails because the generated `LOCK_OWNER_FILE` already contains a regular file, the script exits, then `release_owned_lock` unconditionally removes that regular path. A concurrent or pre-existing occupant can therefore be deleted even though this invocation never created it; the path's token-shaped name alone does not establish ownership.
**Fix:** Make owner-sidecar cleanup conditional on proof that this invocation created that exact file, and never unlink it on the failed-create path. Add a test that pre-populates the would-be owner path and verifies refusal preserves the foreign file.

## Warnings

### WR-01: Strict trace validation accepts impossible PID/liveness lifecycle values

**Classification:** WARNING  
**File:** `tests/retroarch/interruption_trace.sh:133`  
**Issue:** The validator checks each PID and liveness field independently, but does not bind them to the event lifecycle. It accepts `alive=1` with PID `unavailable`, changing host/child PIDs between `handler-entry` and `journal-close-success`, or positive host/child PIDs on `handler-exit`. In the writer, the first two rows occur before cleanup mutates those PIDs, and cleanup reaps and clears both before the final row.
**Fix:** Validate PID/liveness consistency and lifecycle: retain the same PID observations through the first two rows (or unavailable throughout when absent), disallow live-without-PID, and require unavailable PID/liveness after cleanup. Add one-field mutants for each rejected state.

### WR-02: Strict trace validation allows QUIT before journal-close success

**Classification:** WARNING  
**File:** `tests/retroarch/interruption_trace.sh:152`  
**Issue:** Row 1 requires `QUIT_SENT=0` and row 3 requires `QUIT_SENT=1`, but row 2 only validates that the field is a bit. `state_session.sh` writes `journal-close-success` before invoking cleanup, and cleanup is where `QUIT_SENT` becomes 1. The strict validator therefore accepts a trace claiming QUIT was sent before journal close completed.
**Fix:** Require `QUIT_SENT=0` on `journal-close-success` and add a validator mutant that changes only that field to 1.

### WR-03: Harness-ledger verifier does not verify the actual event sequence

**Classification:** WARNING  
**File:** `tests/retroarch/test_state_session.sh:1343`  
**Issue:** `verify_interruption_event_file` checks generic row identity, role, sequence numbers, numeric timestamps/status/PID, monotonic time, and only a minimum of eight rows. It never checks field count, event names/order, or the expected lifecycle coverage. The real accepted interruption ledger has 21 rows, so a truncated or reordered ledger can pass this final recorder gate while omitting events such as the signal/wait, assertion decision, QUIT count, host reap, or lock release.
**Fix:** Add a real-control-specific check for the expected 21-row event sequence and required field shape/values, while retaining the looser identity-mixing helper cases as separate tests. Add omission and reorder negatives for the real ledger gate.

### WR-04: Public acceptance status still says completed gates are pending

**Classification:** WARNING  
**Files:** `README.md:1620`, `README.md:1635`, `include/nesturbator.h:544`  
**Issue:** The README says the exact registered CI, ASan, and full-workflow selections remain required and that current source acceptance is incomplete until those gates and independent review pass; the public header repeats that those gates remain required. The acceptance record documents those exact gates passing for the unchanged reviewed source on commit `0268d3450f64392911a187ce5c040847fda5f8f8`, with the reviewed commit being documentation-only. Readers are therefore told that completed acceptance gates are still pending.
**Fix:** Update both public documents to say the exact CI, ASan, and full-workflow/package gates passed for the recorded source candidate, with independent review/disposition still pending at this point. Preserve the recorded UNKNOWN historical assertion cause and pending native/publication status.

---

_Reviewed: 2026-10-16T12:57:22Z_  
_Reviewer: fresh independent separate-context reviewer (`gpt-6-luna`, xhigh)_  
_Depth: deep_
