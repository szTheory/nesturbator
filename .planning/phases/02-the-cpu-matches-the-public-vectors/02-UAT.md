---
status: complete
phase: 02-the-cpu-matches-the-public-vectors
source: [02-VERIFICATION.md]
started: 2026-10-06T17:46:43Z
updated: 2026-10-06T21:12:09.122Z
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

### 3. Post-merge full-vector backstop (manual dispatch substituted for schedule)
expected: A full-vector run on main after the Phase 02 merge confirms all 258 tests pass with valid run-bound inventory and JUnit evidence. The reporter path is verified separately by the exact-merge main-push run; the owner requested a manual dispatch to avoid waiting for the first cron event.
result: pass
source: automated
evidence: "Owner requested manual dispatch on 2026-10-06. `workflow_dispatch` run 37531643137 on main commit c3e96382ae991d18f69f88096611d42e6a076e0e, a descendant of Phase 02 merge ea55b1f74b5e60ef088a4048bd15fcb2568e44fc, completed `vectors-full` successfully. Downloaded `vectors-full-evidence-37531643137`; `cmake -DINVENTORY=registered-tests.json -DJUNIT=vectors-full.junit.xml -P tests/cmake/vector_result_policy.cmake` exited 0, validating the exact 258-test inventory and completed passing results. The `report` job is intentionally skipped for manual dispatch; exact-merge main-push run 37509491346 completed both `vectors-full` and `report` successfully. The scheduled trigger and report condition remain covered by workflow policy tests."

### 4. Clean-room source provenance
expected: Confirm that no GPL or LGPL emulator source was consulted or copied while implementing the CPU. Repository scans cannot establish which external sources a person opened.
result: pass
source: owner
evidence: "Owner confirmed in conversation on 2026-10-06: pass."

## Summary

total: 4
passed: 4
issues: 0
pending: 0
skipped: 0
blocked: 0

## Gaps

Release publication, exact-merge full-vector evidence, and the manually dispatched post-merge full-vector backstop all passed automated checks. The first cron event itself was not awaited because the owner requested a manual dispatch; the scheduled trigger remains wired and policy-checked, while its same-run reporter path passed on the exact-merge main-push run. Clean-room provenance passed with the owner's confirmation.
