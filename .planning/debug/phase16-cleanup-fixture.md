---
status: verifying
trigger: Phase 16 Plan 21 cleanup request materialization regression
created: 2026-10-16
updated: 2026-10-16
audit_acknowledged:
  milestone: v4
  at: 2026-10-24
  status: verifying
---

## Current Focus

bug_class: Bohrbug
hypothesis: An unguarded request redirection in cleanup_quit aborts EXIT cleanup under errexit before owned-host reap and lock release.
test: Run round-1 RED against the exact reviewed production helper with the corrected readiness fixture.
next_action: Parent runs the remaining formal Task2 FULL and Task3 independent review using round5 candidate38e14fb; no further local driver run is needed.
reasoning_checkpoint:
  hypothesis: Unguarded printf redirection aborts EXIT cleanup under errexit before host reap and lock release.
  confirming_evidence:
    - Round-3 actual reviewed helper reports READINESS_REPLY after the real readiness request is consumed and request.txt becomes a directory.
    - Primary result observes the owned host and exact owned endpoint lock still live before emergency cleanup; selector exits 1 with completed causal evidence.
  falsification_test: The same request-directory injection with only the write guard still leaves an owned host or lock, or loses original status.
  fix_rationale: Guard the failed write before transport launch and return normally to the remaining cleanup stages; record zero sends and preserve FIRST_FAILURE.
  blind_spots: Signal, writable request, duplicate QUIT and foreign-owner behavior still require GREEN.
  candidate_causes:
    - code: unguarded cleanup redirection under errexit.
    - environment: request-path obstruction deliberately injected after genuine readiness.
  and_gate: yes; request write must fail while an owned live host exists, and the unguarded cleanup path must encounter errexit.

## Symptoms

expected: Failed cleanup request writes preserve the original failure and complete bounded owned teardown.
actual: Review CR-01 identifies an unguarded cleanup printf redirection under set -e.
errors: Cleanup request path replaced with a directory after genuine readiness.
reproduction: Actual helper plus owned fake host and injected request directory.
started: Reviewed production helper at 772dbb0.

## Eliminated

## Evidence

- checked: Independently checked round5 continuation at 0a44a5b.
  found: One additional coherent pair authorized with the prior rounds retained and all formal gates still shared/unused.
- checked: Round5 causal RED after actual process wait.
  found: Exact reviewed driver, wait1, EXPECTED_CR01_FAILURE, outer same-clock elapsed3166ms, complete stream digests finalized after wait, no watchdog.
- checked: Round5 GREEN quick primary cases.
  found: EXIT1/READINESS_REPLY779ms, INT130/INTERRUPTED_INT969ms, TERM130/INTERRUPTED_TERM872ms, HUP130/INTERRUPTED_HUP841ms, writable1/READINESS_REPLY901ms, foreign endpoint1/ENDPOINT_BUSY55ms, replacement1/READINESS_REPLY869ms; each passed primary teardown assertions.
- checked: Round-4 actual GREEN and extracted saved-proof assertions.
  found: Harness exited 0 with eight-case banner, but proof rejects negative matrix duration, stale stdout digest and four cleanup reason fields. No accepted GREEN or Task1 commit exists.
- checked: Clock implementations on macOS.
  found: Perl CLOCK_MONOTONIC 527520.131849 and Python time.monotonic 376915.750594416 have different epochs; finalizer mixed them. Corrected finalizer uses the same Perl clock and rejects nonpositive/negative accounting.
- checked: Read-only saved-proof diagnostic with individual assertion reporting.
  found: Only timing, stdout final digest and four reason-field assertions reject; all other case identity, lifecycle, raw role, foreign fixture and exact send assertions pass. Diagnostic-only execution is not acceptance.
- checked: Production correction after round-4 ended.
  found: Cleanup journal reason is now CLEANUP_REQUEST_WRITE_FAILED while FIRST_FAILURE stays untouched; harness enforces exact zero-send diagnostic row before acceptance. Syntax and twelve embedded Python compilations pass; no further driver launched.
- checked: Round-3 RED actual wait and terminal log.
  found: Exit 1 and CR-01 reproduced banner with completed evidence; owned host and lock survive before emergency cleanup.
- checked: Round-2 finalizer.
  found: Missing driver/fixture/frame environment corrected before round-3; round-2 remains invalid retained evidence.
- checked: Round-1 actual harness wait.
  found: Exit 2, NameError foreign_path before primary-result serialization; no causal RED credited. Raw root retained.
- checked: Parent retained invalid RED dispositions.
  found: Earlier runs failed before injection from precreated quit and doubled request paths; neither proves CR-01.
- checked: Foreign fixture setup.
  found: Added actual INT/TERM/HUP traps before publishing fixture PID so the empty signal log is observable evidence.

## Resolution

root_cause: Unguarded cleanup printf redirection triggered errexit before owned host reap and endpoint release when request.txt became a directory.
fix: Locally guard request write after setting QUIT_SENT; emit and journal CLEANUP_REQUEST_WRITE_FAILED with sends0; restore LAST_REASON from preserved FIRST_FAILURE, then finish owned teardown.
verification: Round5 exact reviewed-helper RED wait1/3166ms and repaired eight-case GREEN wait0/105367ms; actual final stream hashes after wait; extracted exact committed-source saved proof PASS; seven reacquisitions PASS. Formal FULL/review remain pending.
files_changed: [tests/retroarch/state_session.sh, tests/retroarch/test_state_session.sh, README.md, include/nesturbator.h]
