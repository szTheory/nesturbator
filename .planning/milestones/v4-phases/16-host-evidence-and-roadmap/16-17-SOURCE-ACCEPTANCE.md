---
phase: 16
plan: 17
record: source-acceptance
status: terminal_stop
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

README and the public header now describe the tested ownership guarantee and bind the prior CI/ASan/full-workflow result to its historical candidate and its separate five-open-finding review. They identify this file as the authoritative corrected-source candidate/result register. Task 1 behavior commit `483bdceb69f8b33d8c9ff3add86f2dbf665be791` and the post-commit source identities are included in the candidate record below.

## Corrected candidate acceptance

### Candidate and registered inputs before finite acceptance

The committed five-path corrected source/doc candidate is `16f7804205afb49459b46b936509e41e8d5566c6` on branch `gsd/phase-16-host-evidence-and-roadmap`. The Task 2 candidate has the same five file blobs in the live checkout. The source-only review will include these five paths and read-only `frame_index.c`:

| Path | Git blob | SHA-256 |
| --- | --- | --- |
| `tests/retroarch/state_session.sh` | `435a0f5fd1c4444e3b45ca668bd5af0e1a87d7e0` | `56dabb21e19d4501bd920dc55413e069e3570f8fdd440deb08d5b84920063af0` |
| `tests/retroarch/interruption_trace.sh` | `2de1af0dce0fb7d1a512b0ca2398f83566070eaa` | `ab11f15c1e77e7d5a9655f37c7a76992222226cf3b550fb8ef805df2f388198a` |
| `tests/retroarch/test_state_session.sh` | `073f78a061f9bd5d2ca62cc84d8c26e7335ac6a2` | `5d075ae02fd4a266e5b0e028384fae52db6ccd191bbcfd12692ef7e47840c299` |
| `README.md` | `461b8e8efeab3c30a6b22bfa49ff9d97b4ff8bd0` | `62bad86cd7e1470a9280f7d1d7b92fad2e3ecd68154b74f1c4d526660cd1ac3d` |
| `include/nesturbator.h` | `a514ec862372f908db60298c4dab010fc22e0312` | `75b9f70dc091c32b59852f0f2c64a151dc194e72002be58a1eed0a0bc394d091` |
| `tests/retroarch/frame_index.c` (read-only) | `204d3ff64cc288176b0e6f47a1788c122a36815c` | `ecab2d5fbf294596572617759ae55c8494ec9bd95705b70bf21ea90767ea5d8b` |

Read-only registered inputs are bound here too:

| Input | Git blob | SHA-256 |
| --- | --- | --- |
| `tests/retroarch/CMakeLists.txt` | `443cbc4b35286f56c9a5c6b29c89e37c14aaa4bf` | `84bbaf464d812c1142e85851203f56e41f04511fee8bc16cf09bcc898400ec0e` |
| `tests/retroarch/run_retroarch.cmake` | `cfe2e4e7942bc6728a0a41b56d1cf9eee09f1e45` | `6a387dc0ac1c2be498f5375d597e4d84ae69763a471e73e80c6a5400972e9581` |
| `CMakePresets.json` | `10a19b58652adb634cc0357de5f5cc91cd848409` | `a97957fcd87052b863be85a26609c0c985402b8eac645748907cec6ea237a024` |
| `cmake/packaging.cmake` | `7afbd7cc89a153ff9397a1bdcc6a913dafd4af96` | `0797be3f2ba69a96076d342174ca4393274719d34d9207ce0acb45cf4a9261e4` |

The CI cache SHA-256 is `af03303ad33e142dde959b50627ec4720d62635c88eeb032de79aacbf1a910d4` and its `frame_index` executable SHA-256 is `10fba3ddd9bdbe01d02f16b7d2938263938533a49d8619aa960f4f44534270f9`. The ASan cache SHA-256 is `875a51bed2081f932dfa23ca0c07f1f3ed7effe8a934c557f8db2e53012cd304` and its `frame_index` executable SHA-256 is `1a137b9ac1f782285ec9ea1362f9cd747bebe0e71362cf3930c7e795ef42b917`. Both configured caches point at this checkout; the existing caches and binaries are present. This correction changes shell controls and documentation only, and the read-only C source is unchanged, so no preliminary build or selector observation was needed. The subsequent normal workflow performs its prescribed configure/build/package steps. Host is Darwin 25.6.0 arm64 with CMake/CTest 4.4.3; CI and ASan test presets each specify four jobs. Config SHA-256 remains the preserved baseline `0517388f5d422ae97afdc5dc1ae11bc43ebd481a757424f152ee8af61559ff5e`.

The current plan-scope resume query resolved the seven commits produced by this active 16-17 executor and no matching async-job manifest exists. The plan is continuing in this executor; no second executor is being dispatched. The parent’s pre-dispatch scope was empty. The recorded source commits are, newest first: `16f7804`, `5f50bbc`, `18ecd15`, `4d1d193`, `483bdce`, `e47917f`, and `b25fd8f`.

The one-time acceptance commands will run on the unchanged five-path candidate, in this order:

1. `gsd_run run-with-timeout 60 -- ctest --preset ci -R '^retroarch.(session.(readiness|controls)|marker[.]|progression[.])' --no-tests=error --output-on-failure` — expected 10/10.
2. `gsd_run run-with-timeout 60 -- ctest --preset asan -R '^retroarch.session.(readiness|controls)$' --no-tests=error --verbose --output-on-failure` — expected 2/2 with no sanitizer diagnostics.
3. `gsd_run run-with-timeout 900 -- cmake --workflow --preset ci` — expected 649/649 at four jobs and three component ZIPs: `nesturbator-0.1.10-library-macos-arm64.zip`, `nesturbator-0.1.10-runner-macos-arm64.zip`, and `nesturbator-0.1.10-libretro-macos-arm64.zip`.

Raw commands, status, complete output, CTest logs/reports and all three archive bytes, SHA-256 digests and inventories are to be retained under `build/phase16-review-corrections-execute/evidence/`. No result from the historical candidates is reused.

### Terminal stop: registered CI selection

The exact first CI selection ran once on candidate `16f7804205afb49459b46b936509e41e8d5566c6` and hit the executor-applied 60-second outer bound. `gsd_run run-with-timeout` returned exit `124`; CTest had completed 9/10 tests, with `retroarch.session.controls` still running. The nine completed tests passed: `retroarch.progression.tracer`, `retroarch.marker.generate`, `retroarch.progression.controls`, `retroarch.progression.complete_required`, `retroarch.progression.prefix`, `retroarch.marker.fixtures`, `retroarch.marker.controls`, `retroarch.marker.qualify`, and `retroarch.session.readiness`. The complete raw stdout, empty stderr, exit code, CTest checkpoint and copied temporary logs are retained in `build/phase16-review-corrections-execute/evidence/ci-selection/`.

Evidence digests: stdout SHA-256 `281053d49795d5562228009466d44140dcfcc22c49d844cf79e5fcb0ac85212f`; empty stderr SHA-256 `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855`; exit-code file SHA-256 `ca2ebdf97d7469496b1f4b78958f9dc8447efdcb623953fee7b6996b762f6fff`. The copied `LastTest.log` predates this selected run and contains a full-workflow result; it is not treated as current evidence. The current checkpoint lists the nine completed test IDs and the copied `LastTest.log.tmp*` files preserve their per-test output. No completed 10/10 report or JUnit was produced.

This is a terminal acceptance failure under Task 3. The unchanged source candidate was not retried. ASan, the normal full workflow, ZIP production and the fresh independent six-path review were not run. No `16-17-SUMMARY.md`, state/roadmap advancement, phase completion or provisional 07 handoff is claimed. The historical assertion and native timing causes remain UNKNOWN; native exact-head acceptance/consent and publication remain separate Plans 07/08.


## Parent reconciliation: outer command guard mismatch

The 60-second outer wrapper was an orchestration mistake, not a demonstrated emulator or session timing defect. Task 1/2 explicitly use 60 seconds for the offline selector. Task 3 lists the registered CTest selectors without that wrapper, while its prose says to preserve existing “60sfocused/900sfull guards.” The executor applied 60 seconds to both registered suite selections. That interpretation cannot accommodate known successful durations: the immutable Plan 16 record reports CI selection 247.44 seconds and ASan selection 297.33 seconds, with full normal workflow tests 245.69 seconds. Registration already gives session.controls TIMEOUT 900, session.readiness TIMEOUT 60, and marker.qualify TIMEOUT 600. None was changed. The raw current selector shows nine successful completions and no final controls result before exit124; it does not demonstrate whether controls would pass or fail.

No unchanged-candidate retry or follow-on gate was launched after the terminal stop. ASan, full CI/packages, fresh independent review/disposition and parent post-wave gates remain unperformed for this candidate. No 17SUMMARY/07 handoff/phase completion is permitted. All nine partial execution commits through aac195c and raw evidence remain retained; empty-scope planning dispatch evidence is historical, and normal safe_resume_gate now blocks another unchanged executor because matching commits exist without SUMMARY.

Recommended next command: `$gsd-plan-phase 16 --gaps`, for a narrow acceptance-only continuation. Reconcile the actual partial commits and five-path candidate16f7804205afb49459b46b936509e41e8d5566c6 rather than reimplementing the completed corrections or fabricating completion credit. Explicitly distinguish offline60s, proposed registered-suite outer1200s (allowing existing per-test900s plus runner overhead), and normal full-workflow900s. This is a proposed orchestration amendment, not executed or approved plan scope here. Preserve every registered timeout, all session operation/prefix/cleanup limits, exact selectors/counts/jobs4, retained timeout history, finite attempts and terminal-stop behavior. Reauthorize the finite acceptance sequence only under a checked amendment; do not treat a budget correction as evidence of current runtime success. Independent exact-source six-path clean review/open0 and every mandatory later parent/native/consent/publication gate remain required. No new standalone diagnostic observation is allocated.

Parent audit: five source/doc candidate blobs remain unchanged through the execution stop; frame_index, registration, wrapper, presets and all older evidence/plans/reviews/verification/requirements/PRODUCT-ROADMAP are byte-identical to dispatch. Config remains exact and unstaged. No automatic runtime retry, source repair, remote mutation or lifecycle command was performed during this reconciliation.
