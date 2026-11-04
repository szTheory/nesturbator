---
phase: 27-pal-host-presentation-and-ntsc-preservation
reviewed: 2026-11-03T04:04:56Z
depth: standard
files_reviewed: 5
files_reviewed_list:
  - README.md
  - include/nesturbator.h
  - src/state.c
  - tests/accuracy/baseline-metadata.txt
  - tests/core/test_state_reject.c
findings:
  critical: 0
  warning: 1
  info: 0
  total: 1
status: issues_found
---

# Phase 27: Code Review Report

**Reviewed:** 2026-11-03T04:04:56Z  
**Depth:** standard  
**Files Reviewed:** 5  
**Status:** issues_found

## Summary

The resolver returned a degraded scope (`no-task-commits-since`) and used the
phase range after prior review commit `6eb0167`; no task commits were
unreachable. I read all five scoped files and checked the regional state
changes against the Phase 27.1 acceptance contract. The loader validation and
its continuation/refusal regressions showed no actionable defect. The refreshed
AccuracyCoin fingerprint is paired with the prior capture's source identity,
which weakens the source provenance recorded by the activation baseline.

## Narrative Findings (AI reviewer)

### WR-01: [WARNING] Fingerprint update is not paired with matching capture provenance

**File:** `tests/accuracy/baseline-metadata.txt:6-7,32,54,56`

**Issue:** `build_inputs_sha256` now records the updated 56-input digest
`e944fa…`, while the same metadata still identifies source revision
`ae2959…` and points `campaign_artifact` at the earlier Phase 27 capture. That
retained capture's own `metadata.txt` records the previous input digest
`40dca6…`. The Phase 27.1 acceptance names a later tested candidate and a fresh
campaign, but this checked-in record combines the new source fingerprint with
the older capture identity. The activation check validates the live fingerprint
and only checks that `source_revision` is syntactically valid, so it does not
detect this mismatch.

**Fix:** Retain the exact fresh campaign capture and refresh `source_revision`,
`source_dirty`/reason, `campaign_artifact`, and `build_inputs_sha256` together
from that capture. Alternatively, keep the old baseline record unchanged and
store the new candidate's capture and provenance separately, with the gate
checking that provenance alongside the report digest.

---

_Reviewed: 2026-11-03T04:04:56Z_  
_Reviewer: the agent (gsd-code-reviewer)_  
_Depth: standard_
