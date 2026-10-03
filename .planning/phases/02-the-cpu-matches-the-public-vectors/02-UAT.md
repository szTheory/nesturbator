---
status: testing
phase: 02-the-cpu-matches-the-public-vectors
source: [02-VERIFICATION.md]
started: 2026-10-03T16:45:00Z
updated: 2026-10-03T16:45:00Z
---

## Current Test

number: 1
name: Decide on CR-01 before merge
expected: |
  Either a fix commit to tests/cmake/fetch_vectors.cmake (resolve the path, case-fold on macOS/Windows, delete only a marked directory) with a test that the guard rejects a case-variant or symlinked DIR, or an explicit deferred disposition with a reason in 02-REVIEW-DISPOSITION.md
awaiting: user response

## Tests

### 1. Decide on CR-01 before merge
expected: Either a fix commit with a test that the guard rejects a case-variant/symlinked DIR, or an explicit deferred disposition in 02-REVIEW-DISPOSITION.md
result: [pending]

### 2. Acknowledge the flagged prohibitions
expected: Owner accepts the verifier's directly observed compliance for the five test-tier prohibitions without a wired enforcing test and the judgment-tier clean-room rule (see the Prohibitions table in 02-VERIFICATION.md), or asks for enforcing tests
result: [pending]

### 3. Release after the owner merges PR #5
expected: `gh release view v0.1.1 --json tagName,isDraft,assets` shows v0.1.1 published (not draft) with the per-platform library, runner and libretro archives and SHA256SUMS; `gh run list --workflow release.yml --branch main --limit 2` shows success
result: [pending]

### 4. First scheduled nightly on main
expected: `gh run list --workflow nightly.yml --event schedule --limit 1` then `gh run view <id> --json jobs` shows vectors-full success with 258 tests passed, and the report job ran with issues: write
result: [pending]

## Summary

total: 4
passed: 0
issues: 0
pending: 4
skipped: 0
blocked: 0

## Gaps
