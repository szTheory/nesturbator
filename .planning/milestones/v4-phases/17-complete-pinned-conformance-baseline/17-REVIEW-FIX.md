---
phase: 17-complete-pinned-conformance-baseline
fixed_at: 2026-10-24T07:44:45Z
review_path: .planning/phases/17-complete-pinned-conformance-baseline/17-REVIEW.md
iteration: 1
findings_in_scope: 1
fixed: 1
skipped: 0
status: all_fixed
---

# Phase 17: Code Review Fix Report

**Fixed at:** 2026-10-24T07:44:45Z  
**Source review:** `.planning/phases/17-complete-pinned-conformance-baseline/17-REVIEW.md`  
**Iteration:** 1

**Summary:**
- Findings in scope: 1
- Fixed: 1
- Skipped: 0

## Fixed Issues

### CR-01: Duplicate raw samples can conceal a missing protected row

**Files modified:** `tests/accuracy/test_scoreboard.c`, `tests/cmake/accuracycoin_regression_controls.cmake`, `README.md`, `include/nesturbator.h`, `tests/accuracy/baseline-metadata.txt`, `tests/accuracy/measurements.md`  
**Commit:** `c23d0ac`  
**Status:** fixed  
**Independent verification:** The source reviewer rechecked the original 37-file scope after `c23d0ac` and confirmed CR-01 resolved with no new findings. The production-path RED/GREEN controls and independent review satisfy the project's automated verification contract; no human checkpoint is required. Exact-source hosted CI is tracked separately before phase completion.
**Applied fix:** The protected verifier now rejects malformed final-stage records, rejects duplicate final keys, and requires one final raw sample for each page report row. It also accepts LF and CRLF raw headers and always closes the evidence file after checking read errors. Production-path controls cover a missing final row, identical key-A substitution for missing key B on both retained pins, malformed final data, and CRLF evidence while retaining non-final snapshots. README and public-header test-contract prose now documents this behavior. The canonical 22-page campaign was rerun; counts and report hash stayed the same, while the source fingerprint and measured timings were refreshed.

**Verification:** Ran in the main checkout (`workflow.use_worktrees=false`). The RED control failed before the C change because the production verdict accepted a duplicated key-A sample in place of key B. After the fix:

- `cmake --build build/ci --target nesturbator-run accuracy.scoreboard -j 4` passed.
- Focused CTest fixture and controls passed: 4/4 (`accuracycoin.protected_observation`, `accuracycoin.protected`, `accuracy.scoreboard`, and `accuracycoin.regression_controls`).
- Full-enabled future-main regression control passed with 101 protected PASS rows observed across 19 current-pin pages plus 3 old-pin pages.
- `accuracycoin.campaign` passed: 22/22 pages, 146 scored rows (101 pass, 42 fail, 2 not-run, 1 guest timeout), and 5 DRAW rows. Campaign report SHA-256 remains `6d0a811960f6964e7fc66802c6e4ade0c4ffb8772c40d88d50cf79e1f8463652`; build-input fingerprint refreshed to `5aedbb3b04eca54a04d42e4e0ec7851e2e694cd72fd032d2ab1c29dfcbf2afe0`.
- AccuracyCoin activation passed against the refreshed baseline metadata.
- `clang-format` 18.1.8 dry-run passed for `tests/accuracy/test_scoreboard.c`.

---

_Fixed: 2026-10-24T07:44:45Z_  
_Fixer: the agent (gsd-code-fixer)_  
_Iteration: 1_
