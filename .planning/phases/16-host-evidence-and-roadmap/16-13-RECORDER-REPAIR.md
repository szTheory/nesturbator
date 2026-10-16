---
phase: 16
plan: "13"
status: observation-consumed-pending
evidence_status: pending
source_commit: 6454457c37632d6eb90bd463154cc96f743f6b52
observation_budget:
  prior_ci_seconds: 4.18
  prior_asan_exit: 1
  prior_asan_seconds: 6.26
  prior_asan_state: consumed-unusable
  new_asan_allowance: 1
  new_asan_state: consumed-before-launch
  new_asan_remaining: 0
  new_asan_exit: pending
---

# Phase 16 Plan 13 Recorder Repair Record

## Scope and current status

This record preserves one bounded current-source recorder compatibility question. The new ASan allowance is consumed before launch and will not be restored if the selector fails, times out, or yields unusable evidence. At this point the selector has not yet launched and no outcome is claimed. A compatible pass can establish only that the corrected actual writer and validator interoperate on this source/build identity. It cannot explain the historical `FAIL interruption first failure was not retained` assertion. Plan 14 still owns current source controls, normal CI and package acceptance, independent review, and its inherited cause-specific conditions; Plans 07 and 08 retain their native/release obligations.

## Preserved history and exhausted observations

Plan 11's bounded CI interruption selector passed once in 4.18 seconds and remains consumed. It recorded TERM status 0, driver wait 130, grep status 0, one cleanup QUIT, an unavailable post-wait liveness distinction, and a released lock. It did not reproduce or explain the historical full-workflow failure. Plan 12's sole ASan allowance was then consumed before launch and produced an unusable exit 1 after 6.26 seconds because the retained handler-exit trace used `unavailable` for field 8 while the validator required a numeric PID. The old observation's original assertion status was 0, driver wait 130, TERM status 0, one QUIT, lock absent, 242 bytes of assertion stdout, and empty assertion stderr. These records remain separate from this new compatibility observation.

The unchanged [Plan 12 recorder repair record](16-12-INTERRUPTION-REPAIR.md) retains its complete old observation and source-identity register; the unchanged [Plan 11 interruption repair record](16-11-INTERRUPTION-REPAIR.md) retains the prior CI result and post-wave failure. The Plan 12 raw outputs remain:

| Evidence | SHA-256 |
|---|---|
| Prior ASan selector stdout | `87525798c2f494d18f53fd6d5a825773b2b65f4ddf986a9b2dcc68c5be5ec5e0` |
| Prior ASan selector stderr | `90dd6b4c766ba59ce32773e7e917e5a94010b0b6c5be83a05c5af3040f484331` |
| Prior ASan selector exit record (`1`) | `4355a46b19d348dc2f57c046f8ef63d4538ebb936000f3c9ee954a27460dd865` |
| Prior selected journal and both assertion snapshots | `9f8814ab643f1145b83023ea17b90b5660093f09ad48b49803570f91b5ecd268` |
| Prior driver trace | `49c89073fe12db72fc468f7e75964c2ea5f8c86ea92e58b5663d3d293954ebb7` |
| Prior captured assertion stdout (242 bytes) | `4d8a7fcbbe0f1cd2bf88d271e0e0dfcc57e64e2a34273fc1ed0124b074b1d101` |
| Prior captured assertion stderr (empty) | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |
| Prior Plan 11 post-wave CI log | `5a46aa38f4cc5e963d2d28f64e1beb46eabd19d2fb61183ecd5c66344a9a7098` |
| Prior Plan 11 eventual journal | `29c4bc65a2256dec40ad1083b970c1e34218614e684db22697701c90694b3f77` |

The old Plan 12 source commit remains `4bfa3a60957b772c3e62c2908eda0af125e2a31a`. Its `state_session.sh` blob/SHA-256 were `52ed46b196068c32ce58472644672c832ee47caf` / `2fd51ead4519d53aaf1993ba56233a91e86d59620741822e7583d73d3b47ea7b`; `test_state_session.sh` were `8b595b1fd695609a96019960c8823fa3d12ad472` / `4282cdad12e31e74837e129db16ff1be478bed1145fa22fa0a4e1f93d7eed8f5`; `README.md` were `97789f64d4930bc47495488bf04736d0f081c720` / `3ab75824eec4e55891cf3b2b6167f34c086a4967250e590e1fa7a9421ce6ee23`; and `include/nesturbator.h` were `a62fc2665f3b4fb416fa1e738bf8e5f07735dc8b` / `b917f0e33445597b1f8d4c1dde2e8c2a1a183f08346b252b0324813b400df199`. The historical Plan 11 pre-Plan-12 blobs remain in the linked record: `state_session.sh=39c01e6e97a93e6349dc23d7f0dcdf886508d317`, `test_state_session.sh=015aa818c141f7f5788df93b13f77abf12f06fc1`, `frame_index.c=204d3ff64cc288176b0e6f47a1788c122a36815c`, `README.md=df027b039cc6ad93ff75da1327a5ecf469a68f65`, and `include/nesturbator.h=f920a754513105077507bb88013c11e76c7c5a82`.

## Task 1 recorder RED and GREEN

The extraction commit is `69d396d` (`refactor(16-13): extract interruption trace helpers`). It moves the actual writer and legacy validator once into the shared source-only include and leaves their behavior unchanged. The defect-specific RED commit is `4c7d814` (`test(16-13): add failing test for cleanup trace admission`). The named `cleanup-host-pid admission` test called the actual writer through handler entry, successful journal close, and cleanup; it confirmed a complete 16-field, three-row trace with numeric PID at entry and `unavailable` for fields 8 and 10 after cleanup. The unchanged validator rejected that trace, after all seven inherited capture checks passed and before fixture, transport, or driver launch. Its complete TAP report is `build/phase16-recorder-execute/cleanup-host-pid-red.tap` (SHA-256 `2408a4b5cc11a64a7a1f9eec63d93d680155ee746c7d15f55b60c1ba5e927dfa`); the classifier record is `build/phase16-recorder-execute/cleanup-host-pid-red-evidence.json` (SHA-256 `e620b33d6316b351f2b87c50727a6004f609540c79a2b2a008a2f525382fea19`). `gsd_run check tdd-red-evidence` returned `RED_EVIDENCE_OK`, matching the named target and intended cleanup-PID assertion.

The GREEN commit is `6454457` (`feat(16-13): admit cleanup interruption trace lifecycle`). It admits positive-decimal or exact `unavailable` host/child identities, keeps liveness as `1` or `unavailable`, requires 16 fields and the ordered three events, validates exact invocation and the separately observed launch PID, and reports the declared reason token for the first violated condition. The existing real caller now binds the invocation to its unique owned session directory and passes its observed background-driver PID. The seven original capture checks and 36 trace checks pass. The final captured offline tracer output is `build/phase16-recorder-execute/task1-green.log` (SHA-256 `5247df30e9ccd45e69caa798139876e91eeb842a5f66aee875a08eb8c970ad5c`); its exit record is `build/phase16-recorder-execute/task1-green.exit` (SHA-256 `9a271f2a916b0b6ee6cecb2426f0b3206ef074578be55d9bc94f6f3fe3ab86aa`). The post-commit tracer rerun also passed all seven plus 36 cases, shell syntax checks passed, and `git diff --check 4bfa3a60957b772c3e62c2908eda0af125e2a31a HEAD` passed. No Task 1 interruption selector or registered real-driver control was executed.

## Bound inputs and one-use allowance

The corrected source identity before launch is commit `6454457c37632d6eb90bd463154cc96f743f6b52`:

| Path | Git blob | SHA-256 |
|---|---|---|
| `tests/retroarch/interruption_trace.sh` | `c4c91cfc88bd0917faf2d1d8ad592a463c12213b` | `43821bd32c822983d1229bef812b4d5d12e7dece9024b6aae0620265df40eac2` |
| `tests/retroarch/state_session.sh` | `36c2e937f5bd23b07e859d203a54bd49ae8a0fac` | `bd8a3830133f54a7b1530b5f4ff216dbdf0db9b16e897eca9b6b9eb7a3aa230c` |
| `tests/retroarch/test_state_session.sh` | `9912d1d33c74e9d31610ce6908be3f2f87785970` | `01fe65340543155a7d56d27492659d5ba25c7b3f9f306d420ab45144cacf83d4` |
| `README.md` | `90e5b35b5561e6d64ff929ac2b6abd80ac3bcb18` | `6093fe8fc91fc584516d59e73fd20a17f55bf0e51618057da75cc821bb3233f1` |
| `include/nesturbator.h` | `0385b92412e1c16badb686333352c1239dd0352e` | `eb25aeeaf9f24afe541084da71626cad3aa4f8313d426990a53650b0d5109eda` |

Build/config inputs are unchanged from the inherited record: `build/asan/CMakeCache.txt` SHA-256 `875a51bed2081f932dfa23ca0c07f1f3ed7effe8a934c557f8db2e53012cd304`; ASan `frame_index` SHA-256 `1a137b9ac1f782285ec9ea1362f9cd747bebe0e71362cf3930c7e795ef42b917`; ASan runner SHA-256 `51b6323a21de9d95ef2d240f83c491272d865e049e0d005ee38ddec5fbbdff6e`; CI `frame_index` SHA-256 `10fba3ddd9bdbe01d02f16b7d2938263938533a49d8619aa960f4f44534270f9`; protected `.planning/config.json` SHA-256 `0517388f5d422ae97afdc5dc1ae11bc43ebd481a757424f152ee8af61559ff5e`. The ASan binary already exists; no build or test workflow is needed to prepare it.

One new ASan interruption-selector allowance is recorded **consumed before launch**, with zero remaining. The old CI and ASan observations remain consumed. The exact authorized command is:

```sh
gsd_run run-with-timeout 60 -- cmake -E env STATE_SESSION_CONTROL=interruption sh tests/retroarch/test_state_session.sh tests/retroarch/state_session.sh build/asan/tests/retroarch/frame_index build/phase16-recorder-execute/compatibility-asan cmake tests/retroarch/run_retroarch.cmake
```

The actual driver runs with its existing owned fake host and transport and the real ASan `frame_index`; it does not launch the installed GUI RetroArch. The harness sets attempt/operation/readiness bounds to 30000 ms, cleanup to 1000 ms, publication to 5000 ms and settle to 0, preserving its current gaps. The outer wrapper is 60 seconds. The newly created ignored evidence directory is `build/phase16-recorder-execute/compatibility-asan/`. Its `allowance-consumed-before-launch.md` and `launch-state.txt` record the consumed budget and initial `not-started` state before invoking the command; `launch-state.txt` is changed to `started` immediately before invocation. The command may launch only once; on interruption, inspect retained process/output state and do not start it again.

## Bounded question and pass conditions

This run asks whether the corrected actual writer is accepted by the real interruption call path through handler entry, journal close, cleanup and handler exit. A usable pass requires unchanged original assertion status and bytes; recorder status usable; exactly three owned ordered trace events with the expected unique invocation and observed driver PID; handler exit 130 with the retained first reason; one QUIT; the existing reap/lock evidence; and complete byte/status records. Any nonzero original assertion, unusable recorder, timeout or missing state is a terminal incomplete result with zero remaining. There is no retry, second real observation, full CI/workflow, Plan 14 dispatch or success summary after such a result.

Even a pass demonstrates only recorder lifecycle compatibility for the exact identities above. It does not explain or repair the historical first-failure assertion. Full current-source acceptance and independent review remain assigned to Plan 14; native hosted verification and owner consent remain assigned to Plan 07; actual publication and release evidence remain assigned to Plan 08.
