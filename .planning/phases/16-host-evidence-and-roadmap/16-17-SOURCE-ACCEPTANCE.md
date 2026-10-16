---
phase: 16
plan: 17
record: source-acceptance
status: in_progress
---

# Phase 16 Plan 17 Source Acceptance

This record tracks the corrected Plan 17 candidate only. Historical results below remain bound to their original source revisions and do not accept this correction.

## Baseline and scope

- Baseline commit: `023f42041f753d474f155d694d4bff6f0248cf6f`
- Branch: `gsd/phase-16-host-evidence-and-roadmap`
- Project root pin: dispatched repository root (guard passed before the first write)
- Config SHA-256: `0517388f5d422ae97afdc5dc1ae11bc43ebd481a757424f152ee8af61559ff5e` (preserved unstaged)
- The prior Plan 16 review candidate `7a6b1aaa87c9f8eb244aa1e901becc32352ecf4e` has five open findings and remains immutable.
- The prior completed-gates candidate `0268d3450f64392911a187ce5c040847fda5f8f8` passed focused CI 10/10, ASan 2/2 and normal CI 649/649 with three ZIPs. Those facts remain historical and do not accept Plan 17.
- The six-path review scope is `tests/retroarch/state_session.sh`, `tests/retroarch/interruption_trace.sh`, `tests/retroarch/test_state_session.sh`, `README.md`, `include/nesturbator.h`, and read-only `tests/retroarch/frame_index.c`.

## Baseline source identities

Git blob IDs captured before production edits at baseline `023f42041f753d474f155d694d4bff6f0248cf6f`:

| Path | Blob |
| --- | --- |
| `tests/retroarch/state_session.sh` | `d04fef7bb29ea60bcd63f3552240fae973ba87d3` |
| `tests/retroarch/interruption_trace.sh` | `e1020eba10d88dc4815db53114d986f2514fde62` |
| `tests/retroarch/test_state_session.sh` | `582c6d47820c1ed54ee86c096780aa9865452a12` |
| `README.md` | `69b43eac4d04b76820df29f6926b28753642caf6` |
| `include/nesturbator.h` | `834591b23eba163460c351d542adbda5061df026` |
| `tests/retroarch/frame_index.c` (read-only) | `204d3ff64cc288176b0e6f47a1788c122a36815c` |

## Task 2 pre-edit source identities

Captured at committed candidate `483bdce` before Task 2 production edits:

| Path | Blob |
| --- | --- |
| `tests/retroarch/state_session.sh` | `435a0f5fd1c4444e3b45ca668bd5af0e1a87d7e0` |
| `tests/retroarch/interruption_trace.sh` | `e1020eba10d88dc4815db53114d986f2514fde62` |
| `tests/retroarch/test_state_session.sh` | `d596346ce30e235a528b6daacb28353b45a38eca` |
| `README.md` | `50ca87504b85ba8abe30692b7c30c2e826a9249e` |
| `include/nesturbator.h` | `7cc60adc256f91f25b3109a3db518eef7d66fb32` |
| `tests/retroarch/frame_index.c` (read-only) | `204d3ff64cc288176b0e6f47a1788c122a36815c` |

## Task 2 lifecycle and QUIT causal REDs

The existing actual-writer trace controls were extended with single-field mutants. Each starts from the actual three-row writer output, including the regular-file hard-link lifecycle. The test verifies the unchanged bytes and all other fields before checking the production validator's reason. The following actual-helper regressions all returned `RED_EVIDENCE_OK` with the named target as the sole TAP failure:

| Finding | Target | Evidence |
| --- | --- | --- |
| WR-01 | `host-live-without-pid` | `build/phase16-review-corrections-execute/evidence/trace-host-live-without-pid/` |
| WR-01 | `child-live-without-pid` | `build/phase16-review-corrections-execute/evidence/trace-child-live-without-pid/` |
| WR-01 | `host-pid-changed-before-cleanup` | `build/phase16-review-corrections-execute/evidence/trace-host-pid-changed-before-cleanup/` |
| WR-01 | `child-pid-changed-before-cleanup` | `build/phase16-review-corrections-execute/evidence/trace-child-pid-changed-before-cleanup/` |
| WR-01 | `host-pid-retained-after-cleanup` | `build/phase16-review-corrections-execute/evidence/trace-host-pid-retained-after-cleanup/` |
| WR-01 | `child-pid-retained-after-cleanup` | `build/phase16-review-corrections-execute/evidence/trace-child-pid-retained-after-cleanup/` |
| WR-01 | `host-live-after-cleanup` | `build/phase16-review-corrections-execute/evidence/trace-host-live-after-cleanup/` |
| WR-01 | `child-live-after-cleanup` | `build/phase16-review-corrections-execute/evidence/trace-child-live-after-cleanup/` |
| WR-02 | `row2-quit-before-journal-close` | `build/phase16-review-corrections-execute/evidence/trace-row2-quit-before-journal-close/` |

Positive actual-writer controls also passed before correction: both first-two PID observations absent (`absent-host-child-observations`) and a numeric PID with unavailable kill-0 liveness (`numeric-pid-with-unavailable-liveness`). The mutant helper proves each negative changes only its target field; it does not rewrite report output.

The validator now returns `TRACE_VALUE` for every named lifecycle/QUIT mutant while preserving both positive cases. Each targeted GREEN report is TAP 1/1, exit 0, under `build/phase16-review-corrections-execute/evidence/trace-green-<target>/`. The correction requires stable first-two PID observations, rejects live-without-PID and absent-PID-with-live-state, permits numeric-PID/unavailable-liveness, requires all final host/child PID and liveness fields unavailable, and requires row-two QUIT=0 while retaining the existing row-one 0 / row-three 1 contract.

## Task 2 real-control ledger causal REDs

The new `verify_interruption_real_control_events` entry is first wired as a behavior-neutral delegate to `verify_interruption_event_file`, and `run_interruption_control` passes it the driver PID, host PID and harness PID observed outside the ledger, before `interruption_recorder_gate`. A valid offline 21-row positive was produced by the actual `record_interruption_event` writer and shared `capture_interruption_boundary`; an independent event/value/PID oracle verified all 21 ordered eight-field rows, final newline and monotonic timestamps before validation. The positive is TAP 1/1, exit 0 at `build/phase16-review-corrections-execute/evidence/real-event-baseline-positive/`.

The actual newly wired helper then accepted each of these independently checked target mutations. Every named TAP failure was machine-validated as `RED_EVIDENCE_OK`:

| Finding | Target | Evidence |
| --- | --- | --- |
| WR-03 | `event-omission-resequenced` | `build/phase16-review-corrections-execute/evidence/event-omission-resequenced/` |
| WR-03 | `event-reorder-valid-sequence` | `build/phase16-review-corrections-execute/evidence/event-reorder-valid-sequence/` |
| WR-03 | `event-short-row` | `build/phase16-review-corrections-execute/evidence/event-short-row/` |
| WR-03 | `event-extra-row` | `build/phase16-review-corrections-execute/evidence/event-extra-row/` |
| WR-03 | `event-missing-field` | `build/phase16-review-corrections-execute/evidence/event-missing-field/` |
| WR-03 | `event-extra-field` | `build/phase16-review-corrections-execute/evidence/event-extra-field/` |
| WR-03 | `event-wrong-pid` | `build/phase16-review-corrections-execute/evidence/event-wrong-pid/` |
| WR-03 | `event-bad-status` | `build/phase16-review-corrections-execute/evidence/event-bad-status/` |
| WR-03 | `event-bad-detail` | `build/phase16-review-corrections-execute/evidence/event-bad-detail/` |

These failures used no fixture, frame-index or transport work. The generic verifier already rejects some malformed identity/time values, so those are tested for their precise strict-gate reason after implementation but are not claimed as causal REDs.

The strict helper now returns `EVENTS_OK` for the valid positive and a specific `EVENT_FIELDS`, `EVENT_SEQUENCE`, `EVENT_IDENTITY`, `EVENT_VALUE` or `EVENT_TIME` reason for each malformed 21-row ledger. All 13 targeted GREEN controls passed TAP 1/1, exit 0, under `build/phase16-review-corrections-execute/evidence/real-event-green-<target>/`; this includes wrong role and invocation ID, wrong role-specific PID, decreasing time and the nine causal mutants. The real control call site uses the observed PID arguments and binds these reason codes into the recorder error ledger before its final recorder gate.

The complete Plan 17 offline selector passed on the corrected Task 2 source: exit 0; raw stdout/stderr and exit code are retained in `build/phase16-review-corrections-execute/evidence/task2-offline-green-rerun/`. It exercised both owner-sidecar controls, all seven capture cases, two positive identity ledgers/four ownership negatives/invalid-ID behavior, 13 real-control ledger cases, and all 51 trace cases. An initial selector attempt is retained at `evidence/task2-offline-green/`; it exited 1 after the added row-two QUIT case used the existing lock-field comparison helper. The test was corrected to use the exact one-field comparator and rerun with an independent workspace; no production correction or acceptance retry was involved.

## Owner-sidecar transition audit

At baseline, the driver computes `ATTEMPT_ID` from the monotonic start time and its PID, derives `LOCK_OWNER_FILE`, installs EXIT and simple INT/TERM/HUP traps, then creates the sidecar with a noclobber redirection in a subshell. A successful create is hard-linked to the endpoint lock and verified by token contents. Cleanup removes a token-matching endpoint link and then removes any regular non-symlink `LOCK_OWNER_FILE`, regardless of whether this invocation created that file. Consequently a failed noclobber create over a foreign regular file reaches EXIT cleanup and deletes the foreign object. The same path-based cleanup also has no exact-object check if the owner pathname is replaced.

The correction must establish ownership from the successful atomic create and retain an exact object reference through acquisition, hard-link publication, verification, normal/interrupted/setup-failure cleanup and repeated EXIT cleanup. Failed creation or a replaced foreign pathname must not grant unlink authority. The existing startup TERM 143, release and later reacquisition control is retained.

## Causal RED and GREEN evidence

The actual `state_session.sh` startup driver was exercised through the existing TAP control. A test-owned Perl clock wrapper created the exact monotonic-clock/PID-derived owner path before returning the clock value to the driver; the driver then attempted the real noclobber create and ran its own EXIT cleanup.

| Finding | RED evidence | Result |
| --- | --- | --- |
| Failed create must preserve foreign object | `build/phase16-review-corrections-execute/evidence/owner-red-captured/evidence.json` | `RED_EVIDENCE_OK`, exit 1, TAP 1 pass / 1 named failure; `LOCK_OWNER_UNAVAILABLE`, no host/transport marker, foreign inode/bytes lost by EXIT cleanup. Test-only commit `b25fd8f`. |
| Replaced owner pathname must survive cleanup | `build/phase16-review-corrections-execute/evidence/owner-replacement-red/evidence.json` | `RED_EVIDENCE_OK`, exit 1, TAP 1 pass / 2 named failures; actual owner path and hard-link identity were recorded, TERM returned 143, owned endpoint link was released, replacement inode/bytes were lost. Test-only commit `e47917f`. |

The minimal correction keeps a private descriptor open across the noclobber create, writes the token through that descriptor, records its device/inode with Perl `fstat`, and closes it before host/transport launch. Startup signal traps defer exit only through that bounded ownership registration. Cleanup now requires the recorded device/inode to match the current owner or hard-link path in addition to the existing token check. A failed create has no recorded authority; a pathname replacement does not match the captured identity.

Targeted actual-driver GREEN report: `build/phase16-review-corrections-execute/evidence/owner-green/report.tap`, exit 0, TAP 3/3. The original regular-hard-link lifecycle check, exact collision preservation, and replacement-preservation case all passed.

The complete Plan 17 offline selector passed after the correction:

- Command: `gsd_run run-with-timeout 60 -- cmake -E env STATE_SESSION_CONTROL=interruption-capture-selftest sh tests/retroarch/test_state_session.sh tests/retroarch/state_session.sh build/ci/tests/retroarch/frame_index build/phase16-review-corrections-execute/evidence/owner-offline-green/work cmake tests/retroarch/run_retroarch.cmake`
- Exit: `0`; raw stdout/stderr: `build/phase16-review-corrections-execute/evidence/owner-offline-green/`
- Ownership controls: failed-create bytes and identity preserved; exact owned hard link released while replacement bytes and identity survived TERM.
- Existing controls retained: 7 capture cases; 40 trace cases (36 original plus 4 lock cases); two positive identity ledgers; four identity negatives; invalid-ID behavior.
- The selector returned before fixture/frame-index/transport work; the two owner regressions also assert their host and transport markers remain absent.

README and the public header now describe the tested ownership guarantee and bind the prior CI/ASan/full-workflow result to its historical candidate and its separate five-open-finding review. They identify this file as the authoritative corrected-source candidate/result register. The Task 1 behavior commit and post-commit source identities will be added after commit.

## Corrected candidate acceptance

Pending. Record each causal RED, machine validation, GREEN, exact source identities, registered CI and ASan selector evidence, normal `cmake --workflow --preset ci` output, package artifacts, and the independent six-path review here before declaring acceptance. No result for the corrected candidate is implied by the historical rows above.
