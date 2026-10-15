---
phase: 16-host-evidence-and-roadmap
fixed_at: 2026-10-15T02:01:54Z
review_path: .planning/phases/16-host-evidence-and-roadmap/16-REVIEW.md
iteration: 2
findings_in_scope: 5
fixed: 5
skipped: 0
status: all_fixed
---

# Phase 16: Code Review Fix Report

**Fixed at:** 2026-10-15T02:01:54Z
**Source review:** `.planning/phases/16-host-evidence-and-roadmap/16-REVIEW.md`
**Iteration:** 2

**Summary:**
- Findings in scope: 5
- Fixed: 5
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

### WR-04: Newline in an archive member name can impersonate multiple expected files

**Files modified:** `tests/cmake/check_archives.cmake`, `README.md`, `include/nesturbator.h`
**Commit:** `6ad25ce`
**Applied fix:** A bounded ZIP central-directory inventory check validates one unambiguous end record, bounds and record count, then rejects ASCII control bytes in raw member names before the line-oriented listing is used. The scan explicitly rejects unsupported multi-disk and ZIP64 layouts. A synthetic member named `LICENSE\nbin/nesturbator-run` proves the previous listing split is rejected by the production helper.

### WR-05: Duplicate allowlisted archive entries pass the exact-content gate

**Files modified:** `tests/cmake/check_archives.cmake`, `README.md`, `include/nesturbator.h`
**Commit:** `fea4f76`
**Applied fix:** The raw inventory rejects duplicate member-name bytes, including repeated directory entries; only component directories are accepted, which rejects noncanonical directory aliases. Every required file pattern must match exactly one entry. Synthetic controls cover repeated `LICENSE`, repeated `bin/`, and distinct runner filenames that both match the same required pattern.

## Verification

All verification ran in the main checkout on `gsd/phase-16-host-evidence-and-roadmap`; `.planning/config.json` has `workflow.use_worktrees=false`.

- `cmake --workflow --preset ci`: passed all 648 tests. The RetroArch session matrix passed in 293.08 seconds, and CPack generated the library, runner, and libretro archives.
- `cmake -DDIR=build/ci -P tests/cmake/check_archives.cmake`: passed; exactly the three current component archives were accepted.
- `libretro.info_claims.self_test` and `libretro.info_archives.self_test` passed in the full workflow. Direct support self-test passed all 13 support cases; direct archive self-test passed absent, stale, current, and semicolon-path controls.
- `scripts/hygiene.sh --tree`, `scripts/hygiene.sh --staged`, and `scripts/hygiene.sh --history HEAD~3..HEAD`: passed.
- `sh -n` passed for both modified RetroArch scripts; `git diff --check` passed.

Iteration 2 archive verification also ran in the main checkout on
`gsd/phase-16-host-evidence-and-roadmap`; `.planning/config.json` still has
`workflow.use_worktrees=false`.

- `cmake -DSELFTEST=ON -DWORK=build/archive-selftest -DSOURCE_ROOT=. -P tests/cmake/check_archives.cmake` passed absent, stale, current, semicolon,
  newline, duplicate-file, duplicate-directory, and multiple-pattern-match
  controls. `ctest --test-dir build/ci -R '^libretro.info_archives.self_test$'
  --output-on-failure` passed 1/1.
- `cmake -DDIR=build/ci -P tests/cmake/check_archives.cmake` accepted the
  current library, libretro, and runner CPack archives.
- `scripts/hygiene.sh --tree`, `--staged`, and `--history HEAD~2..HEAD` passed;
  the required clang-format dry-run for `include/nesturbator.h` and
  `git diff --check` passed.
- The full CI workflow was not rerun for iteration 2; the parent will run the
  final exact-head CI gate.

The README and public-header comments describe the setup-failure cleanup and exact
metadata/archive guards. The public core API and cartridge admission are unchanged;
no publication status is claimed.

---

_Fixed: 2026-10-15T02:01:54Z_
_Fixer: the agent (gsd-code-fixer)_  
_Iteration: 2_
