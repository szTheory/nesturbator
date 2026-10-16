---
phase: 16-host-evidence-and-roadmap
plan: "14"
reviewed: 2026-10-16T03:17:41Z
reviewed_source_commit: c87857a484ba5b3710de4975f6a389e860ab1348
depth: deep
reviewer: gsd-code-reviewer, independent separate-context review
context_provenance: Fresh subagent context; reviewed the exact committed source blobs directly, read current Plan 14 acceptance and Plan 15 identity evidence as context, and did not reuse a prior review. No acceptance or test commands were run.
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
  warning: 1
  info: 0
  total: 2
status: issues_found
---

# Phase 16 Plan 14: Independent Code Review

**Reviewed source commit:** `c87857a484ba5b3710de4975f6a389e860ab1348`

The six reviewed paths match the exact source commit. Read-only context included `tests/retroarch/CMakeLists.txt`, `tests/retroarch/run_retroarch.cmake`, `CMakePresets.json`, the current Plan 14 source-acceptance record, and Plan 15's identity-repair evidence. The current record reports the focused CI and ASan controls passed and the full CI workflow passed 649/649 with all three packages emitted. The review did not run acceptance or test commands.

## Critical Issues

### CR-01: A startup signal can leave the shared endpoint lock stale

**Classification:** BLOCKER

**File:** `tests/retroarch/state_session.sh:30-35`

**Issue:** The driver creates the fixed endpoint lock at line 30 before defining `release_owned_lock`, installing the EXIT trap, or installing INT/TERM/HUP traps. If the process receives a terminating signal after `mkdir` succeeds but before cleanup is armed, it can exit while leaving `nesturbator-retroarch-55355.lock` behind. Later sessions then fail with `ENDPOINT_BUSY` until someone removes the stale directory. The interruption controls signal the driver only after host startup, so they do not cover this acquisition window.

**Fix:** Make lock acquisition signal-safe: establish signal handling before attempting acquisition, defer interruption across the lock-creation/ownership-registration transition, and release only a lock this invocation acquired. Add a focused control that delivers a signal at that transition and verifies the next session can acquire the endpoint.

### WR-01: README and public header report completed acceptance as pending

**Classification:** WARNING

**File:** `README.md:1614-1615`; `include/nesturbator.h:541-542`

**Issue:** Both passages say actual-driver compatibility and current-source acceptance remain pending Plan 14. The current `16-14-SOURCE-ACCEPTANCE.md` records the registered focused CI/ASan controls and the full 649-test CI/package workflow as passed at this exact commit. The text therefore gives readers an outdated status, although the historical assertion cause and later native/release gates remain unresolved.

**Fix:** Update both passages to state that Plan 14's current-source acceptance passed, while preserving the unknown historical assertion cause and identifying the independent review and later native/release gates as the remaining work.

---

_Reviewed: 2026-10-16T03:17:41Z_
_Reviewer: independent separate-context gsd-code-reviewer_
_Depth: deep_
