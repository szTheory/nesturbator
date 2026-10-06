---
status: partial
phase: 02-the-cpu-matches-the-public-vectors
source: [02-VERIFICATION.md]
started: 2026-10-06T17:46:43Z
updated: 2026-10-06T20:59:30Z
---

## Current Test

[testing complete]

## Tests

### 1. Published release after owner-authorized merge
expected: After the owner-authorized merge, v0.1.1 is published and non-draft, with exactly 18 platform archives plus nonempty SHA256SUMS; release CI, archive attestations, the tamper check and publication pass.
result: pass
source: automated
evidence: "scripts/phase2_outcomes.sh ea55b1f74b5e60ef088a4048bd15fcb2568e44fc v0.1.1 (2026-10-06): exact release inventory and tag ancestry pass; run 37513567481 passed CI, archive attestation, tamper check and publish. Its obsolete final metadata assertion alone failed; PR #10 corrected it and live metadata was verified."

### 2. Main-push full-vector evidence
expected: The same command confirms the main-push run used the exact merge SHA, `vectors-full` succeeded, and the run-bound artifact validates the exact 258-test inventory and passing JUnit results.
result: pass
source: automated
evidence: "scripts/phase2_outcomes.sh ea55b1f74b5e60ef088a4048bd15fcb2568e44fc v0.1.1 (2026-10-06): PASS, run https://github.com/szTheory/nesturbator/actions/runs/37509491346."

### 3. First scheduled nightly after merge
expected: After the first scheduled run, the same command confirms a descendant commit, all 258 vector checks passing, valid run-bound evidence, and a completed reporter job.
result: blocked
blocked_by: third-party
reason: "The first post-merge scheduled run has not started yet (checked 2026-10-06T20:59Z); scheduled for 2026-10-07 04:17 UTC. The read-only collector confirms release and exact-merge main-push evidence pass, while the scheduled run remains pending. Recheck automatically with scripts/phase2_outcomes.sh after it runs; no owner action is needed."

### 4. Clean-room source provenance
expected: Confirm that no GPL or LGPL emulator source was consulted or copied while implementing the CPU. Repository scans cannot establish which external sources a person opened.
result: pass
source: owner
evidence: "Owner confirmed in conversation on 2026-10-06: pass."

## Summary

total: 4
passed: 3
issues: 0
pending: 0
skipped: 0
blocked: 1

## Gaps

Release publication and exact-merge full-vector evidence passed automated checks. The first scheduled nightly remains blocked only until the scheduled GitHub run exists; it is not an owner hand-off. Clean-room provenance remains the one identity-bound owner judgment.
