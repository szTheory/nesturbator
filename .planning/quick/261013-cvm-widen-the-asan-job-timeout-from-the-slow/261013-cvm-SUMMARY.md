---
phase: quick-261013-cvm
plan: 01
subsystem: ci
tags: [github-actions, timeouts, WR-04]
status: complete
commits: 2
plan_head_before: 7cd1b63
plan_head_after: 461402c
key-files:
  modified:
    - .github/workflows/ci.yml
    - .github/workflows/nightly.yml
    - .github/workflows/release.yml
    - .planning/preparation/ENGINEERING.md
    - tests/cmake/timeout_comments.cmake
    - .planning/phases/15-speed-and-close-out/15-REVIEW-FIX.md
    - .planning/phases/15-speed-and-close-out/15-REVIEW-DISPOSITION.md
metrics:
  completed: 2026-10-13
---

# Quick 261013-cvm: Job timeouts from sampled run history

Every workflow timeout is now twice the slowest sampled successful run of its job, with run ID and sample count in the comment. Short network-bound jobs have a 5-minute floor. The asan timeout went from 12 to 26 minutes. WR-04 is recorded as fixed.

## Commits
- bd10137 `ci: set job timeouts from sampled run history` (3 workflows, ENGINEERING section 5, lint wording)
- 461402c `docs(15): record WR-04 fix`

## Verification
- `hygiene.timeout_comments`: 17 timeout-minutes lines carry a run ID.
- `cmake --workflow --preset ci`: 633 of 633 tests passed.
- `scripts/hygiene.sh`: exit 0 with the untracked quick PLAN.md set aside.
- Pushed HEAD 461402c. The single `gh pr checks 46` right after the push still listed the previous run (38398904814, asan failed); the new run had not registered. The orchestrator watches CI.
- release.yml and nightly.yml do not run on a pull request; their new values are checked only by the lint.

## Deviations from Plan
None to the work. The tracer task 1 and task 2 were committed together as one `ci:` commit, as the plan's commit step describes.

## Issues
`scripts/hygiene.sh` flags `261013-cvm-PLAN.md` for an email address: the plan text quotes the Co-Authored-By trailer (the noreply address), which the file scan does not allow (only commit messages allow it). The file is untracked, so the commit hooks did not trip. The orchestrator's docs commit of that PLAN.md will fail hygiene unless the address is reworded in the file.

## Self-Check: PASSED
