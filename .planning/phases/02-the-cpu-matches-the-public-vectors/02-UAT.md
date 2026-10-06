---
status: partial
phase: 02-the-cpu-matches-the-public-vectors
source: [02-VERIFICATION.md]
started: 2026-10-03T16:45:00Z
updated: 2026-10-06T14:10:26.286Z
---

## Current Test

[testing complete]

## Tests

### 1. Decide on CR-01 before merge
expected: Either a fix commit with a test that the guard rejects a case-variant/symlinked DIR, or an explicit deferred disposition in 02-REVIEW-DISPOSITION.md
result: pass

### 2. Acknowledge the flagged prohibitions
expected: Owner accepts the verifier's directly observed compliance for the six test-tier prohibitions without a wired enforcing test and the judgment-tier clean-room rule (see the Prohibitions table in 02-VERIFICATION.md), or asks for enforcing tests
result: issue
reported: "goal is 0 human verificaiton/uat required. and remember this in our GSD .planning so that u do this by default going forward always whenever possible that will make our GSD workflow more automated which i like."
severity: major

### 3. Release after the owner merges PR #5
expected: `gh release view v0.1.1 --json tagName,isDraft,assets` shows v0.1.1 published (not draft) with the per-platform library, runner and libretro archives and SHA256SUMS; `gh run list --workflow release.yml --branch main --limit 2` shows success
result: blocked
blocked_by: other
reason: "PR #5 is still open as a draft; the release workflow runs after merge."

### 4. First scheduled nightly on main
expected: `gh run list --workflow nightly.yml --event schedule --limit 1` then `gh run view <id> --json jobs` shows vectors-full success with 258 tests passed, and the report job ran with issues: write
result: blocked
blocked_by: other
reason: "There is no scheduled main run yet; the nightly backstop runs after PR #5 merges."

## Summary

total: 4
passed: 1
issues: 1
pending: 0
skipped: 0
blocked: 2

## Gaps

- gap_id: G-02-2
  truth: "Machine-checkable Phase 2 prohibitions and post-merge outcomes are enforced or observed automatically, and the project keeps an automation-first verification default."
  status: failed
  reason: "Root cause diagnosed: six machine-checkable prohibitions had no direct policy-to-gate mapping, post-merge GitHub outcomes were not reconciled into verification, and the existing automation-first project rule was not operationalized at the UAT handoff."
  severity: major
  test: 2
  artifacts:
    - .planning/debug/DEBUG-automation-first-uat.md
  missing:
    - "Focused, dependency-free assertions for machine-verifiable prohibitions (vector parsing, API freeze for this phase, oversized vector JSON, complete 00-ff registration, nightly policy, and least-privilege workflow permissions)."
    - "Automated collection of release and main-push/nightly outcomes tied to the expected commit, with no manual polling; preserve owner authority over merge or publication consent."
    - "A durable GSD rule that command-verifiable criteria are run and recorded before conversational UAT; reserve owner prompts for identity/consent or a specific judgment with no useful automated oracle."
  debug_session: .planning/debug/DEBUG-automation-first-uat.md
