---
status: testing
phase: 02-the-cpu-matches-the-public-vectors
source: [02-VERIFICATION.md]
started: 2026-10-06T17:46:43Z
updated: 2026-10-06T17:46:43Z
---

## Current Test

number: 1
name: Published release after owner-authorized merge
expected: |
  After the owner-authorized merge, `scripts/phase2_outcomes.sh <merge-sha> v0.1.1` reports a successful release run on the tag commit and exactly 18 platform archives plus nonempty SHA256SUMS.
awaiting: user response

## Tests

### 1. Published release after owner-authorized merge
expected: After the owner-authorized merge, `scripts/phase2_outcomes.sh <merge-sha> v0.1.1` confirms a successful non-draft release on the tag commit descended from the merge, with the exact 18 archives and SHA256SUMS.
result: [pending]

### 2. Main-push full-vector evidence
expected: The same command confirms the main-push run used the exact merge SHA, `vectors-full` succeeded, and the run-bound artifact validates the exact 258-test inventory and passing JUnit results.
result: [pending]

### 3. First scheduled nightly after merge
expected: After the first scheduled run, the same command confirms a descendant commit, all 258 vector checks passing, valid run-bound evidence, and a completed reporter job.
result: [pending]

### 4. Clean-room source provenance
expected: The owner confirms that no GPL or LGPL emulator source was consulted or copied while implementing the CPU.
result: [pending]

## Summary

total: 4
passed: 0
issues: 0
pending: 4
skipped: 0
blocked: 0

## Gaps

The hosted release and nightly checks remain pending until the owner-authorized merge and the corresponding GitHub runs. The clean-room source-opening history requires an owner judgment because repository automation cannot establish which external sources were consulted.
