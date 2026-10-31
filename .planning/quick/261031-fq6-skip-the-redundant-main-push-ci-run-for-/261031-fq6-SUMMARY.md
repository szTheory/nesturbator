---
phase: quick-261031-fq6
plan: "01"
subsystem: ci
tags: [github-actions, cmake, workflow-policy, release]
requires: []
provides:
  - Skips redundant direct CI runner jobs for generated Release Please main pushes.
  - Protects reusable Release validation, its publication gates, and main-push Nightly with offline policy checks.
affects: [ci, release, nightly, hygiene]
tech-stack:
  added: []
  patterns:
    - Caller-aware GitHub workflow conditions protect reusable CI from direct-run exclusions.
    - Static workflow policy tests combine live-file checks with bounded event classification and mutation controls.
key-files:
  created:
    - tests/cmake/ci_release_push_policy.cmake
  modified:
    - .github/workflows/ci.yml
    - tests/hygiene/CMakeLists.txt
    - README.md
    - docs/testing.md
    - docs/releases.md
key-decisions:
  - "Skip only direct CI pushes to main whose commit message starts with the Release Please release prefix."
  - "Use github.workflow to distinguish the direct CI run from Release's reusable caller while preserving push context."
  - "Keep main-push Nightly and all existing Release CI and publication checks enabled."
requirements-completed: []
duration: "about 17 min"
completed: 2026-10-31
commits: 3
plan_head_before: c1e9c239bd188f6538271dba8252687255437f15
plan_head_after: fe376dac1edb7ce0e78c1c54c3efdc60d260681e
status: complete
---

# Quick Task 261031-fq6: Skip redundant main-push CI Summary

**Generated Release Please merge pushes now avoid duplicate ordinary CI runner jobs while Release retains full reusable validation, artifacts, and its publication gate.**

## Performance

- **Duration:** About 17 minutes; estimated because the executor start timestamp was not captured.
- **Started:** Not recorded.
- **Completed:** 2026-10-31T15:33:06Z
- **Tasks:** 3
- **Files modified:** 6

## Accomplishments

- Added the canonical guard to every direct CI job. It skips only a `push` to `refs/heads/main` in workflow `CI` when the head commit message starts with `chore(main): release `.
- Added a live workflow policy and self-test covering all CI job guards, event controls, six-platform artifacts, the required-check roll-up, Release caller and publication wiring, and Nightly's main-push behavior.
- Documented that GitHub's reusable caller context identifies Release while retaining the push event, and clarified that local static checks do not establish hosted execution.

## Task Commits

1. **Task 1: Guard direct release pushes and prove the live workflow contract** — `00eedce` (`feat`)
2. **Task 2: Add event controls and policy mutation checks** — `3fd0d9f` (`test`)
3. **Task 3: Document the validation owner and finish focused local verification** — `fe376da` (`docs`)

Plan commit count is measured from `c1e9c239bd188f6538271dba8252687255437f15` through `fe376dac1edb7ce0e78c1c54c3efdc60d260681e`: **3 commits**. Per the handoff, planning artifacts and state bookkeeping are not committed here.

## Files Created/Modified

- `tests/cmake/ci_release_push_policy.cmake` — Checks the active workflow contract, classifies a bounded set of event inputs, and rejects routing, gate, artifact, and Nightly mutations.
- `.github/workflows/ci.yml` — Adds per-job caller-aware guards while preserving perf eligibility and `ci-required`'s `always()` behavior.
- `tests/hygiene/CMakeLists.txt` — Registers the live policy and its self-test in hygiene.
- `README.md`, `docs/testing.md`, `docs/releases.md` — Explain release CI ownership, the retained Nightly trigger, and limits of the local evidence.

## Verification

- The initial live policy run failed against the original unguarded workflow by naming the missing per-job guards.
- `cmake --preset hygiene` — passed.
- `ctest --preset hygiene --no-tests=error -R '^hygiene\.(ci_release_push_policy(\.selftest)?|ci_required_rollup)$'` — passed all 3 selected tests.
- The self-test covers direct CI release pushes, the Release reusable caller, ordinary main pushes, non-prefix titles, missing head metadata, PR/manual events, other refs, and squash messages with release notes. It also checks perf's separate eligibility and requires supported expression syntax.
- The same focused CTest command passed after all task commits. Comparison against base `c1e9c239bd188f6538271dba8252687255437f15` confirmed `.github/workflows/release.yml`, `.github/workflows/nightly.yml`, `include/nesturbator.h`, and `tests/accuracy/baseline-metadata.txt` are unchanged. `git diff --check` passed against that base.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Corrected policy parsing before the expected baseline failure**
- **Found during:** Task 1
- **Issue:** The first checker version had a malformed CMake regex, then its job scan also included event triggers and an assumed Windows matrix field not present in the workflow.
- **Fix:** Parse the jobs section separately, extract normalized job-level conditions, strip comments line by line, and match the existing Windows matrix rows as written.
- **Files modified:** `tests/cmake/ci_release_push_policy.cmake`
- **Verification:** The policy then failed for the intended missing guards on the original workflow and passed after the guards were added.
- **Committed in:** `00eedce`

## Issues Encountered

The sandbox denied direct writes to `.git/index.lock` and the prescribed plan ledger location under `.git`. The GSD commit helper succeeded after a narrow sandbox escalation for each authorized task commit. The plan's starting head was kept in a temporary project-local ledger, used to measure the three commits, and will be removed after this summary is checked.

## Authorization Boundary

Only local configure and static CTest checks were run. No `cmake --workflow --preset ci`, AccuracyCoin campaign, other campaign, remote dispatch, push, merge, or release action occurred. A fresh owner decision is still required before a push or full run starts another campaign.

No user setup is required for this local implementation. Remote validation remains intentionally stopped pending fresh authorization.

---
*Quick task: 261031-fq6*
*Completed: 2026-10-31*

## Self-Check: PASSED

The summary exists, all three task commits are ancestors of `HEAD`, and the
summary whitespace check passed.
