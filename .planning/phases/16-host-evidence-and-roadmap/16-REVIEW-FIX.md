---
phase: 16-host-evidence-and-roadmap
fixed_at: 2026-10-15T01:44:12Z
review_path: .planning/phases/16-host-evidence-and-roadmap/16-REVIEW.md
iteration: 1
findings_in_scope: 3
fixed: 3
skipped: 0
status: all_fixed
---

# Phase 16: Code Review Fix Report

**Fixed at:** 2026-10-15T01:44:12Z  
**Source review:** `.planning/phases/16-host-evidence-and-roadmap/16-REVIEW.md`  
**Iteration:** 1

**Summary:**
- Findings in scope: 3
- Fixed: 3
- Skipped: 0

## Fixed Issues

### WR-01: Setup failure can strand the RetroArch port lock

**Files modified:** `tests/retroarch/state_session.sh`, `tests/retroarch/test_state_session.sh`  
**Commit:** `e348e2c`  
**Applied fix:** Installed an immediate bootstrap EXIT and signal cleanup after successful exclusive lock creation. The later full cleanup traps replace it after setup state is initialized. Added a setup-failure control that preserves the injected `mkdir` status 73, proves no host or transport starts, confirms the owned lock is removed, and verifies a subsequent session acquires it.

### WR-02: Semicolons in archive paths bypass the exact file allowlist

**Files modified:** `tests/cmake/check_archives.cmake`  
**Commit:** `85f1a51`  
**Applied fix:** The archive contents check rejects a literal semicolon in the raw listing before newline-to-list conversion. Metadata entry counting now matches exact newline-delimited entries without converting the listing to a CMake list. Added a synthetic ZIP with the combined `LICENSE;THIRD-PARTY-NOTICES.md` path and valid current metadata; its source-equality and support checks pass before the exact semicolon-path allowlist error is asserted.

### WR-03: Documentation guard ignores later contradictory support claims

**Files modified:** `tests/cmake/info_claims.cmake`  
**Commit:** `94a29c2`  
**Applied fix:** The localized support checker rejects multiple `Supported NTSC ` clause prefixes. Independent later-contradiction controls cover metadata, README, and the public header while retaining the existing factual mutants, duplicate-key control, and harmless-prose control.

## Verification

All verification ran in the main checkout on `gsd/phase-16-host-evidence-and-roadmap`; `.planning/config.json` has `workflow.use_worktrees=false`.

- `cmake --workflow --preset ci`: passed all 648 tests. The RetroArch session matrix passed in 293.08 seconds, and CPack generated the library, runner, and libretro archives.
- `cmake -DDIR=build/ci -P tests/cmake/check_archives.cmake`: passed; exactly the three current component archives were accepted.
- `libretro.info_claims.self_test` and `libretro.info_archives.self_test` passed in the full workflow. Direct support self-test passed all 13 support cases; direct archive self-test passed absent, stale, current, and semicolon-path controls.
- `scripts/hygiene.sh --tree`, `scripts/hygiene.sh --staged`, and `scripts/hygiene.sh --history HEAD~3..HEAD`: passed.
- `sh -n` passed for both modified RetroArch scripts; `git diff --check` passed.

The README and public-header comments describe the setup-failure cleanup and exact
metadata/archive guards. The public core API and cartridge admission are unchanged;
no publication status is claimed.

---

_Fixed: 2026-10-15T01:44:12Z_  
_Fixer: the agent (gsd-code-fixer)_  
_Iteration: 1_
