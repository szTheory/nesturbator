---
phase: 20-nmi-line-and-vblank-flag
reviewed: 2026-10-28T15:04:19Z
depth: standard
files_reviewed: 29
files_reviewed_list:
  - .github/workflows/ci.yml
  - .github/workflows/nightly.yml
  - README.md
  - include/nesturbator.h
  - src/apu.c
  - src/bus.c
  - src/internal.h
  - src/ppu.c
  - src/state.c
  - src/synth.c
  - tests/CMakeLists.txt
  - tests/accuracy/baseline-metadata.txt
  - tests/accuracy/measurements.md
  - tests/cmake/fetch_vectors.cmake
  - tests/cmake/hash_inventory.cmake
  - tests/cmake/phase20_hosted_evidence.cmake
  - tests/cmake/phase20_hosted_evidence_controls.cmake
  - tests/cmake/vectors_transport.cmake
  - tests/core/test_apu.c
  - tests/core/test_nmi_state.c
  - tests/core/test_state.c
  - tests/core/test_state_reject.c
  - tests/fuzz/state_loader.c
  - tests/ppu/test_mmc3.c
  - tests/ppu/test_nmi_races.c
  - tests/retroarch/test_state_session.sh
  - tests/runner/hashes.txt
  - tests/runner/phase20_fake_gh.c
  - tests/state_test.h
findings:
  critical: 1
  warning: 2
  info: 0
  total: 3
status: issues_found
---

# Phase 20: Code Review Report

**Reviewed:** 2026-10-28T15:04:19Z
**Depth:** standard
**Files Reviewed:** 29
**Status:** issues_found

## Summary

Reviewed all 29 files in the resolver's explicit Phase 20 scope and traced the interrupt, state-restore, and hosted-evidence validation paths. The resolver status was degraded (`no-reachable-task-commits`), so this uses its truthful wider since-review scope. The emulator changes and their state continuation paths were internally consistent in this pass. The hosted source identity check has a blocker: it compares paths and blob contents but omits Git file modes, allowing a different executable tree to pass. I also found an in-tree collector robustness defect and a duplicated Nightly validator definition.

## Narrative Findings (AI reviewer)

## Critical Issues

### CR-01: Hosted source validation ignores Git file modes

**Severity:** BLOCKER
**File:** `tests/cmake/phase20_hosted_evidence.cmake:243-252`
**Issue:** `phase20_check_remote_tree()` reads each hosted entry's `path`, `type`, and `sha`, but never reads or compares its `mode`. The effective-input manifest also records only path and content digest. A clean checkout whose file bytes match while its Git mode differs (for example, a `100644` script versus a `100755` script) therefore passes the remote-tree comparison. Executable bits and symlink modes change how source files run, so the collector can certify hosted results against a source tree different from the local tree whose digest it records.
**Fix:** Include Git mode in the effective source identity and compare each remote tree entry's mode with the local Git index mode (`git ls-files --stage`), rather than relying on the checked-out filesystem mode. Using the index preserves correct comparisons for Windows checkouts that materialize mode-`120000` symlinks as regular files.

## Warnings

### WR-03: Scoreboard lookup recursively scans ignored build output

**Severity:** WARNING
**File:** `tests/cmake/phase20_hosted_evidence.cmake:2215-2241`
**Issue:** `phase20_copy_unique()` recursively globs every file below `source_root` before suffix matching. When called for `tests/accuracy/scoreboard.txt`, this also scans ignored `build/` fixtures and prior evidence. A populated in-tree build can produce additional matching paths or make relative-path handling fail, so the documented `SOURCE_REF=.` collection can fail even when the tracked source is clean. The collector required an absolute clean equivalent checkout to avoid this case.
**Fix:** For the source scoreboard, copy the known `${source_root}/tests/accuracy/scoreboard.txt` path directly. Keep recursive suffix matching scoped to extracted artifact directories; if source-root scanning remains, exclude build and other ignored/generated directories.

### WR-04: Nightly validator is defined twice

**Severity:** WARNING
**File:** `tests/cmake/phase20_hosted_evidence.cmake:1547-1627`
**Issue:** `phase20_validate_nightly()` is defined once at lines 1333-1415 and again here with an identical normalized body. CMake uses the later definition, leaving the earlier full validator dead. This duplication makes future edits easy to apply to the shadowed copy while the called validator remains unchanged.
**Fix:** Remove one definition and keep a single `phase20_validate_nightly()` implementation.

---

_Reviewed: 2026-10-28T15:04:19Z_
_Reviewer: the agent (gsd-code-reviewer)_
_Depth: standard_
