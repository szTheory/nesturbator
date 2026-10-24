---
phase: 17-complete-pinned-conformance-baseline
reviewed: 2026-10-24T07:32:36Z
depth: standard
files_reviewed: 37
files_reviewed_list:
  - .github/workflows/ci.yml
  - README.md
  - include/nesturbator.h
  - runner/CMakeLists.txt
  - runner/accuracycoin.c
  - runner/accuracycoin.h
  - runner/main.c
  - scripts/hygiene.sh
  - tests/CMakeLists.txt
  - tests/accuracy/baseline-metadata.txt
  - tests/accuracy/budgets.txt
  - tests/accuracy/measurements.md
  - tests/accuracy/migration.tsv
  - tests/accuracy/old-673ef-metadata.txt
  - tests/accuracy/old-673ef-scoreboard.txt
  - tests/accuracy/scoreboard-main-metadata.txt
  - tests/accuracy/scoreboard-main.txt
  - tests/accuracy/scoreboard.txt
  - tests/accuracy/test_conformance.c
  - tests/accuracy/test_scoreboard.c
  - tests/cmake/accuracycoin_activation.cmake
  - tests/cmake/accuracycoin_baseline_fetch.cmake
  - tests/cmake/accuracycoin_campaign.cmake
  - tests/cmake/accuracycoin_driver.cmake
  - tests/cmake/accuracycoin_input_fingerprint.cmake
  - tests/cmake/accuracycoin_partial_failure.cmake
  - tests/cmake/accuracycoin_protected.cmake
  - tests/cmake/accuracycoin_regression_controls.cmake
  - tests/cmake/accuracycoin_tracer.cmake
  - tests/cmake/hash_keys.cmake
  - tests/cmake/prepare_scoreboard_baseline.cmake
  - tests/cmake/verify_scoreboard_regression.cmake
  - tests/hygiene/scan_selftest.sh
  - tests/retroarch/test_state_session.sh
  - tests/roms/accuracycoin-673ef.nes
  - tests/roms/accuracycoin.nes
  - tests/roms/manifest.txt
findings:
  critical: 1
  warning: 0
  info: 0
  total: 1
status: issues_found
---

# Phase 17: Code Review Report

**Reviewed:** 2026-10-24T07:32:36Z  
**Depth:** standard  
**Files Reviewed:** 37  
**Status:** issues_found

## Summary

Reviewed the phase 17 source, tests, CI wiring, and pinned conformance evidence. The protected same-pin verification path has a gap that can accept a report whose raw evidence omits an inventory row, weakening the regression gate.

## Critical Issues

### CR-01: Duplicate raw samples can conceal a missing protected row

**Classification:** BLOCKER  
**File:** `tests/accuracy/test_scoreboard.c:390-430`  
**Issue:** `load_final_raw()` skips malformed lines and all rows whose stage is not `final` (lines 390-391), then validates each remaining row independently and checks only that the number of accepted final rows equals `report_count`. It does not track which inventory/report keys have already appeared or require every expected key exactly once. Replacing one final row for key B with a duplicate of key A therefore preserves the count: A maps to its report row twice, B has no raw sample, and the report's full page coverage still passes. `load_page_observation()` relies on this check before the protected same-pin verifier trusts the page report, so a report can claim a PASS for B without corresponding final raw evidence.

**Fix:** Reject malformed final-stage records instead of skipping them, track a seen bit for each expected report/inventory row, reject duplicate keys, and require every expected key exactly once before accepting the page. For example:

```c
if (seen[report_index])
    return set_error(error, error_size, "duplicate final raw evidence key");
seen[report_index] = 1u;
...
for (size_t i = 0u; i < report_count; i++)
    if (!seen[i])
        return set_error(error, error_size, "final raw evidence is incomplete");
```

---

_Reviewed: 2026-10-24T07:32:36Z_  
_Reviewer: the agent (gsd-code-reviewer)_  
_Depth: standard_
