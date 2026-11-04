---
phase: 20-nmi-line-and-vblank-flag
reviewed: 2026-10-30T12:12:42Z
depth: standard
files_reviewed: 49
files_reviewed_list:
  - .github/workflows/ci.yml
  - README.md
  - include/nesturbator.h
  - src/bus.c
  - src/cpu.c
  - src/internal.h
  - src/state.c
  - tests/CMakeLists.txt
  - tests/accuracy/baseline-metadata.txt
  - tests/accuracy/measurements.md
  - tests/accuracy/migration.tsv
  - tests/accuracy/scoreboard.txt
  - tests/cmake/accuracycoin_activation.cmake
  - tests/cmake/accuracycoin_baseline_fetch.cmake
  - tests/cmake/accuracycoin_regression_controls.cmake
  - tests/cmake/accuracycoin_v5_targets_control.cmake
  - tests/cmake/phase20_hosted_evidence.cmake
  - tests/cmake/phase20_hosted_evidence_controls.cmake
  - tests/cmake/phase21_hosted_evidence.cmake
  - tests/cmake/phase21_hosted_evidence_controls.cmake
  - tests/cmake/phase21_source_identity.cmake
  - tests/cmake/phase21_source_identity_controls.cmake
  - tests/cmake/skip_policy.cmake
  - tests/cmake/v5_targets.cmake
  - tests/cmake/vector_api_policy.cmake
  - tests/core/test_api.c
  - tests/core/test_apu.c
  - tests/core/test_controller.c
  - tests/core/test_interrupt_entry.c
  - tests/core/test_interrupt_state.c
  - tests/core/test_irq_poll.c
  - tests/core/test_nmi_state.c
  - tests/core/test_state_fork.c
  - tests/core/test_state_generation.c
  - tests/core/test_state_reject.c
  - tests/core/test_state_runahead.c
  - tests/cpu/test_cpu_unit.c
  - tests/cpu/vector_bus.c
  - tests/cpu/vector_bus.h
  - tests/fuzz/state_loader.c
  - tests/header/header_c.c
  - tests/header/header_cxx.cpp
  - tests/interrupt_fixture.h
  - tests/libretro/libretro_host.c
  - tests/retroarch/state_session.sh
  - tests/retroarch/test_state_session.sh
  - tests/runahead_model.h
  - tests/runner/hashes.txt
  - tests/state_test.h
findings:
  critical: 1
  warning: 3
  info: 0
  total: 4
status: issues_found
---

# Phase 20: Code Review Report

**Reviewed:** 2026-10-30T12:12:42Z
**Depth:** standard
**Files Reviewed:** 49
**Status:** issues_found

## Summary

The evaluation-scope resolver returned `degraded` (`phase-range`, `no-reachable-task-commits`) and widened from the previous review commit `a64c23f`. After standard exclusions this yielded 49 paths, including Phase 21 and interleaved changes. The existing Phase 21 review/history covers its earlier implementation; I used it as context and rechecked the current NMI/state paths against format 3 / behavior revision 9, the later shell/CMake deltas, and the current source-identity helper. I found no new NMI runtime defect. The Phase 20 collector's CR-01, WR-03 and WR-04 remain deferred. WR-01 (the captured-request API comment) and WR-02 (the changing sprite-status sample test) remain fixed per the prior disposition. The degraded scope also includes a distinct, still-present Phase 21 worktree-mode finding, recorded below as WR-05.

Historical acceptance remains tied to its tested source: the latest Phase 20 hosted bundle covers candidate `7848cc8`, tested merge `716e8d4`, and 418 effective inputs under format 2 / revision 8. The earlier `fe70d61` / 417-input bundle was superseded. Neither qualifies the current format 3 / revision 9 code. Phase 21's hosted bundle covers candidate `54c7db2` and tested merge `faf1b91`; later source changes mean it is not claimed as acceptance evidence for current HEAD `c9c090e`. No tests were run for this read-only review; the parent will run full CI.

## Narrative Findings (AI reviewer)

## Critical Issues

### CR-01: Hosted source validation ignores Git file modes

**Severity:** BLOCKER
**Carried:** Deferred from the prior Phase 20 review; same finding identity.
**File:** `tests/cmake/phase20_hosted_evidence.cmake:144-158, 272-281, 330-367`

**Issue:** `phase20_compute_effective_digest()` records each effective path and content digest, but not its Git mode. `phase20_check_remote_tree()` reads the remote `path`, `type` and `sha` fields and compares content without reading or comparing the entry's `mode`. A local script changed from `100644` to `100755` has the same blob hash, so the collector can accept hosted evidence for source with different executable behavior. The supplemental mode/blob comparison documented for the accepted run mitigates that bundle only; it does not close the recurring validator gap.

**Fix:** Include the Git index mode in the effective source identity and compare each remote tree entry's mode against `git ls-files --stage` for the corresponding local path. Keep symlink mode `120000` distinct and compare its stored target bytes without dereferencing it.

## Warnings

### WR-03: Scoreboard lookup recursively scans ignored build output

**Severity:** WARNING
**Carried:** Deferred from the prior Phase 20 review; same finding identity.
**File:** `tests/cmake/phase20_hosted_evidence.cmake:2274-2301, 2724-2725`

**Issue:** `phase20_copy_unique()` recursively globs all files under `source_root`, then matches a suffix. Its source-checkout call searches for `tests/accuracy/scoreboard.txt`, so an in-tree `build/` directory containing a fixture or prior evidence copy can create a second match. The collector then fails with an ambiguous match even when the tracked scoreboard is valid.

**Fix:** Copy `${source_dir}/tests/accuracy/scoreboard.txt` directly for the source scoreboard. Keep recursive suffix lookup scoped to extracted artifact directories, or prune ignored/generated directories before scanning.

### WR-04: Nightly validator is defined twice

**Severity:** WARNING
**Carried:** Deferred from the prior Phase 20 review; same finding identity.
**File:** `tests/cmake/phase20_hosted_evidence.cmake:1384-1465, 1598-1678`

**Issue:** `phase20_validate_nightly()` has two definitions with the same validation body. CMake uses the later definition, leaving the first one dead. A future fix applied to the shadowed definition would not affect collection or validation.

**Fix:** Remove one definition and keep a single `phase20_validate_nightly()` implementation.

### WR-05: Phase 21 source identity misses worktree file mode

**Severity:** WARNING
**File:** `tests/cmake/phase21_source_identity.cmake:83-108, 134-142`; `tests/cmake/phase21_source_identity_controls.cmake:99-102`

**Issue:** The identity manifest records committed and index modes plus the current worktree blob, but it does not record the mode of regular worktree files. A chmod-only change leaves both the blob and index entry unchanged, so `MODE=verify` accepts it. The existing mode control changes the index with `git update-index --chmod=+x`; it does not exercise an actual filesystem chmod with the index left unchanged.

**Fix:** Record and verify a normalized worktree mode for regular files, while retaining a separate symlink-type check. Add a POSIX control that changes only filesystem execute permission and requires verification to reject the mismatch.

---

_Reviewed: 2026-10-30T12:12:42Z_
_Reviewer: the agent (gsd-code-reviewer)_
_Depth: standard_
