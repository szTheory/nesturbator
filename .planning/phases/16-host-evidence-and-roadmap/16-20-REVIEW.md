---
phase: 16-host-evidence-and-roadmap
reviewed: 2026-10-16T19:23:52Z
depth: standard
files_reviewed: 6
files_reviewed_list:
  - tests/retroarch/state_session.sh
  - tests/retroarch/interruption_trace.sh
  - tests/retroarch/test_state_session.sh
  - README.md
  - include/nesturbator.h
  - tests/retroarch/frame_index.c
findings:
  critical: 1
  warning: 0
  info: 0
  total: 1
status: issues_found
reviewed_source_commit: 772dbb0fdaedb3d8af52beeeab854943c867441b
reviewer_provenance: "Fresh independent separate-context review; model gpt-6-luna, effort xhigh; configured depth standard."
source_blobs:
  tests/retroarch/state_session.sh: 435a0f5fd1c4444e3b45ca668bd5af0e1a87d7e0
  tests/retroarch/interruption_trace.sh: 2de1af0dce0fb7d1a512b0ca2398f83566070eaa
  tests/retroarch/test_state_session.sh: 073f78a061f9bd5d2ca62cc84d8c26e7335ac6a2
  README.md: 461b8e8efeab3c30a6b22bfa49ff9d97b4ff8bd0
  include/nesturbator.h: a514ec862372f908db60298c4dab010fc22e0312
  tests/retroarch/frame_index.c: 204d3ff64cc288176b0e6f47a1788c122a36815c
---

# Phase 16: Code Review Report

**Reviewed:** 2026-10-16T19:23:52Z  
**Depth:** standard  
**Files Reviewed:** 6  
**Status:** issues_found  
**Reviewed source commit:** `772dbb0fdaedb3d8af52beeeab854943c867441b`

## Summary

Reviewed the six requested source paths at the verified commit. The interruption-trace lifecycle checks, strict 21-row real-control validation, and public scope claims address the previously recorded findings. One cleanup failure path can leave the RetroArch host running and retain its endpoint lock when the cleanup QUIT request cannot be written.

## Narrative Findings (AI reviewer)

### CR-01: Failed cleanup request write skips host reap and lock release

**Severity:** BLOCKER (critical)

**File:** `tests/retroarch/state_session.sh:298`

**Issue:** `cleanup_quit` sets `QUIT_SENT=1` and then performs an unguarded `printf` under `set -e`. If writing `REQUEST` fails (for example, the attempt filesystem becomes read-only or the request path is removed), the function exits before `cleanup` reaches the unconditional host reap at line 313 and `release_owned_lock` at line 323. This function runs from both the EXIT trap and the signal handler, so a cleanup I/O failure can leave a live RetroArch process bound to the endpoint and make later sessions fail as busy.

**Fix:** Make QUIT materialization best-effort and ensure host reaping and lock release run regardless of its result. For example, handle the write failure locally, record the cleanup-send failure if possible, and structure `cleanup` so it always attempts `reap_one "$HOST_PID"` and `release_owned_lock` after `cleanup_quit` returns.

---

_Reviewed: 2026-10-16T19:23:52Z_  
_Reviewer: the agent (gsd-code-reviewer)_  
_Depth: standard_
