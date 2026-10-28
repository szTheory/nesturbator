---
phase: 20-nmi-line-and-vblank-flag
reviewed: 2026-10-28T18:44:44Z
depth: standard
files_reviewed: 9
files_reviewed_list:
  - README.md
  - include/nesturbator.h
  - tests/accuracy/baseline-metadata.txt
  - tests/accuracy/measurements.md
  - tests/cmake/session_helpers.cmake
  - tests/retroarch/CMakeLists.txt
  - tests/retroarch/frame_index.c
  - tests/retroarch/state_session.sh
  - tests/retroarch/test_state_session.sh
findings:
  critical: 1
  warning: 2
  info: 0
  total: 3
status: issues_found
---

# Phase 20: Code Review Report

**Reviewed:** 2026-10-28T18:44:44Z
**Depth:** standard
**Files Reviewed:** 9
**Status:** issues_found

## Summary

Reviewed the nine files in the supplied incremental scope and traced the new session clock and hash calls through `runner/clock.c`, `progression_hash_file`, and `marker_write`. The canonical evaluation-scope result is degraded (`phase-range`, `no-reachable-task-commits`); historical short summary commit IDs are unreachable after branch reconstruction, so this review uses the supplied current-source diff and present files. I found no new reproducible defect in this nine-file scope. CR-01, WR-03, and WR-04 below are carried unresolved findings from the previous review and are not attributed to this incremental diff. WR-01 and WR-02 remain fixed per the existing disposition. The previous 417-input hosted acceptance is historical evidence only and does not establish acceptance of the current 418-input source.

## Narrative Findings (AI reviewer)

## Critical Issues

### CR-01: Hosted source validation ignores Git file modes

**Severity:** BLOCKER
**Carried:** Unresolved from the previous review; unchanged identity.
**File:** `tests/cmake/phase20_hosted_evidence.cmake:243-252`
**Issue:** `phase20_check_remote_tree()` reads each hosted entry's `path`, `type`, and `sha`, but never reads or compares its `mode`. The effective-input manifest also records only path and content digest. A clean checkout whose file bytes match while its Git mode differs (for example, a `100644` script versus a `100755` script) therefore passes the remote-tree comparison. Executable bits and symlink modes change how source files run, so the collector can certify hosted results against a source tree different from the local tree whose digest it records.
**Fix:** Include Git mode in the effective source identity and compare each remote tree entry's mode with the local Git index mode (`git ls-files --stage`), rather than relying on the checked-out filesystem mode. Using the index preserves correct comparisons for Windows checkouts that materialize mode-`120000` symlinks as regular files.

## Warnings

### WR-03: Scoreboard lookup recursively scans ignored build output

**Severity:** WARNING
**Carried:** Unresolved from the previous review; unchanged identity.
**File:** `tests/cmake/phase20_hosted_evidence.cmake:2215-2241`
**Issue:** `phase20_copy_unique()` recursively globs every file below `source_root` before suffix matching. When called for `tests/accuracy/scoreboard.txt`, this also scans ignored `build/` fixtures and prior evidence. A populated in-tree build can produce additional matching paths or make relative-path handling fail, so the documented `SOURCE_REF=.` collection can fail even when the tracked source is clean. The collector required an absolute clean equivalent checkout to avoid this case.
**Fix:** For the source scoreboard, copy the known `${source_root}/tests/accuracy/scoreboard.txt` path directly. Keep recursive suffix matching scoped to extracted artifact directories; if source-root scanning remains, exclude build and other ignored/generated directories.

### WR-04: Nightly validator is defined twice

**Severity:** WARNING
**Carried:** Unresolved from the previous review; unchanged identity.
**File:** `tests/cmake/phase20_hosted_evidence.cmake:1547-1627`
**Issue:** `phase20_validate_nightly()` is defined once at lines 1333-1415 and again here with an identical normalized body. CMake uses the later definition, leaving the earlier full validator dead. This duplication makes future edits easy to apply to the shadowed copy while the called validator remains unchanged.
**Fix:** Remove one definition and keep a single `phase20_validate_nightly()` implementation.

---

_Reviewed: 2026-10-28T18:44:44Z_
_Reviewer: the agent (gsd-code-reviewer)_
_Depth: standard_
