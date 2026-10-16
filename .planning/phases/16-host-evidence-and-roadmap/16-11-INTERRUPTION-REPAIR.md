---
phase: 16
plan: "11"
status: incomplete
evidence_status: undiagnosed
dispatch_head: 89638fd0490a136d67dde4493438f0fdb990bfc0
post_wave_failure_head: 5f899ece5d20c5b8f260c52aee13cabd65054b10
ci_probe_harness_blob: 1f72eb4a3cb445a9e90617ca76462873f7c8b550
---

# Plan 16-11: Interruption Repair — Undiagnosed Stop

## Result

Plan 16-11 is incomplete. One bounded CI interruption observation passed, but it did not reproduce or explain the historical `FAIL interruption first failure was not retained` assertion. No defect-specific regression, correction, README/header behavior update, ASan control, full workflow, independent source review, success summary, or delivery handoff is claimed. The cause remains unproven; stop before expanding work or granting Plan 07 completion credit.

## Reconciled Plan 10 history

Plan 10 remains superseded without completion credit. Its source and evidence commits are ancestors of dispatch HEAD `89638fd0490a136d67dde4493438f0fdb990bfc0`:

| Commit | Full SHA | Change |
| --- | --- | --- |
| `0b60c88` | `0b60c8881328729706f9ff4dea8785838452460c` | Timing-report RED test |
| `00f61ce` | `00f61cee1bc6525aa802b98d973d94747a887508` | Timing reporter implementation and docs |
| `262f580` | `262f580303077d0c62b2efa5dd46d6bc9d3e2bbe` | Frame-index RED test |
| `c1db852` | `c1db85254d09f0e4e52fc0edc44ddd54e8daf333` | Conditional target>anchor replay correction and docs |
| `1b1589a` | `1b1589a49f735195b9dbf89a238dc0518777335c` | Registered timing reporter regression |
| `2ca8485` | `2ca8485dd53cd45e28570bc76ccaf58f8d634a85` | Historical native timing/review record |
| `10694f7` | `10694f7da7baed83e3d61dca3f687213dbecb183` | Retained RED evidence records |

The attempted completion `5f899ece5d20c5b8f260c52aee13cabd65054b10` was revoked by `11adf3d268800682e4bef8ec96600b1fd8c6bb2c`, which removed `16-10-SUMMARY.md` after the required post-wave failure. All five protected source/document blobs are identical at the failed post-wave HEAD and dispatch HEAD:

| Path | Blob SHA-1 |
| --- | --- |
| `tests/retroarch/state_session.sh` | `39c01e6e97a93e6349dc23d7f0dcdf886508d317` |
| `tests/retroarch/test_state_session.sh` | `015aa818c141f7f5788df93b13f77abf12f06fc1` |
| `tests/retroarch/frame_index.c` | `204d3ff64cc288176b0e6f47a1788c122a36815c` |
| `README.md` | `df027b039cc6ad93ff75da1327a5ecf469a68f65` |
| `include/nesturbator.h` | `f920a754513105077507bb88013c11e76c7c5a82` |

The historical target>anchor change is retained unchanged. The earlier clean review and 649/649/package result at `1b1589a49f735195b9dbf89a238dc0518777335c` remain historical evidence only; the source-bound review and normal workflow after a justified current correction have not run.

## Preserved native timing evidence

Run `38764197994` on PR #50 tested head `3de23d4aed5d9696dc4020db3b8ef5e8205300fb`, base `9f9386c661f6fb8f6063bf756655713aedadae77`, and preview merge `81c0a2132250892b324ec4c9be2a26fe8ea91056`. Intel job `116523895865` failed `readiness-response-stall` at 25,946 ms; ASan job `116523896064` failed `readiness-delayed` at 26,269 ms, both against strict `<25,000 ms`. Earlier run `38740783575` is a distinct historical stall failure. Run `38764179312` was automatically canceled after a body update, not manually rerun. Native subintervals and causes remain unknown. See [16-NATIVE-TIMING-REPAIR.md](16-NATIVE-TIMING-REPAIR.md) and its unchanged [review](16-NATIVE-REVIEW.md) and [disposition](16-NATIVE-REVIEW-DISPOSITION.md).

## Reconciled post-wave interruption failure

At source HEAD `5f899ece5d20c5b8f260c52aee13cabd65054b10`, `cmake --workflow --preset ci` exited 8 after 648/649 tests in 234.99 seconds. Test #634, `retroarch.session.controls`, failed at 230.07 seconds with `FAIL interruption first failure was not retained`; both readiness cases passed their unchanged bounds, and CPack was not reached. The raw log SHA-256 is `5a46aa38f4cc5e963d2d28f64e1beb46eabd19d2fb61183ecd5c66344a9a7098`; `post-wave-LastTest.log` SHA-256 is `9a26b048d5334c06627825c7e5bf4535d07284d21d21e488e20420db0dc9beeb`.

The later copied journal at `build/phase16-native-execute/post-wave-interrupt/ra/attempts/attempt-428185203-20183/journal.tsv` has SHA-256 `29c4bc65a2256dec40ad1083b970c1e34218614e684db22697701c90694b3f77`. It contains readiness and cleanup-quit rows with `INTERRUPTED_TERM`; the copied send log contains only cleanup `QUIT`, and the copied driver output is empty. The isolated replay retained both rows immediately after its wait. These are eventual observations and do not establish what path or bytes the failed assertion read. No signal/journal race or other cause is inferred from them.

## Bounded diagnostic observation 1 — CI

The recorded hypothesis was that the harness's `set -- "$interrupt_ra"/attempts/*; interrupt_attempt=$1` could select an ambiguous or wrong attempt because it did not require exactly one current attempt. The pre-run record is `build/phase16-interruption-execute/ci-probe.md`.

The instrumented harness factored the same interruption driver path into `run_interruption_control`, made it reachable through the private `STATE_SESSION_CONTROL=interruption` selector and captured the selected path/count, immediate grep result, journal bytes/identity, send/lock/liveness facts, and monotonic event samples. The temporary candidate harness blob was `1f72eb4a3cb445a9e90617ca76462873f7c8b550`; its preserved patch SHA-256 is `4926c4c95e3ded9cedc55a7672d3089eb87bf0410ea48053b34a14c1044c6568`. After the observation, this instrumentation was reverted from the tracked harness because it did not diagnose the historical failure. The current tracked harness blob remains `015aa818c141f7f5788df93b13f77abf12f06fc1`.

Command:

```text
gsd_run run-with-timeout 60 -- cmake -E env STATE_SESSION_CONTROL=interruption sh tests/retroarch/test_state_session.sh tests/retroarch/state_session.sh build/ci/tests/retroarch/frame_index build/phase16-interruption-execute/ci-control cmake tests/retroarch/run_retroarch.cmake
```

The command exited 0 in 4.18 seconds with the unchanged attempt/cleanup budgets of 30000/1000 ms. The private selector ran the existing 44 progression fixture controls, then the shared real-driver interruption path and exited before the rest of the session matrix. It selected exactly one attempt, `session-46380-interrupt/ra/attempts/attempt-433266814-46752`. TERM succeeded; wait returned 130; immediate grep returned 0; the host was alive before TERM and dead after wait; the journal had two rows, readiness and cleanup-quit, both `INTERRUPTED_TERM`; sends contained one cleanup `QUIT`; and the lock was gone. The optional transport PID marker was absent, so this run did not establish a live transport-child identity at the signal boundary.

The assertion-boundary diagnostics attempted `ps -o pid=,ppid=,stat=,command= -p ...`, but the sandbox returned `/bin/ps: Operation not permitted`; the process-tree snapshots are unavailable. This limitation is retained in the raw diagnostic file. The monotonic samples were driver start `433266794`, host observed `433267575`, signal returned `433267590`, driver wait returned `433267688`, and grep returned `433267699`; these event samples are not additive intervals.

Raw outputs:

| Evidence | SHA-256 |
| --- | --- |
| `build/phase16-interruption-execute/ci-control.log` | `a4ba2d125805b4ac66b67447621d6413e502581c180dfade3a55a4c7b5d5eb8a` |
| `build/phase16-interruption-execute/ci-control/session-46380-interrupt/interruption-diagnostics.txt` | `a23e58a90f981c1470b418015351e81d4aa635f233141dfe4ec827be8547dc6b` |
| `build/phase16-interruption-execute/ci-control/session-46380-interrupt/journal-at-assertion.tsv` | `3a76b954773c659dc9b4019b8bb677e0b3e77188071ff6aacaf70b9242ce5d30` |

The hypothesis is unsupported by this run. It neither reproduced the assertion failure nor proves why the full workflow failed. No second observation was run: no distinct source-bound hypothesis was established that could explain the missing grep match from a test that had already returned 130. A further observation requires a new, discriminating hypothesis within the remaining one-ASan limit; it must not be an unchanged rerun seeking a green result.

## Stop and delivery state

No cause-specific RED evidence was produced, so the historical failed CTest is not treated as TDD RED and no GREEN correction is authorized. No behavior source changed; README and public-header behavior comments remain unchanged. The focused selector was the only current-source test observation. No ASan/full-suite/package workflow or independent review is claimed. There is no Plan 16-11 summary or completion credit.

Plan 07 remains blocked pending a demonstrated current-source repair, full CI with all three packages and a fresh independent review, followed by the existing exact-head native and owner-consent gates. Plan 08 remains blocked on Plan 07 and actual publication. EVID-04 remains open. No phase completion, merge consent, release, publication, or Phase 17 advancement is claimed.

At stop, branch is `gsd/phase-16-host-evidence-and-roadmap`, HEAD is `89638fd0490a136d67dde4493438f0fdb990bfc0`, and `.planning/config.json` is byte-identical to the saved baseline (SHA-256 `0517388f5d422ae97afdc5dc1ae11bc43ebd481a757424f152ee8af61559ff5e`). The read-only `frame_index.c` and all five failed-post-wave source/document blobs remain unchanged.
