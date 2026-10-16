---
phase: 16-host-evidence-and-roadmap
plan: "12"
status: incomplete
evidence_status: capture-ready
observation_budget:
  ci: consumed-passing-observation
  ci_seconds: 4.18
  asan_allowance: 1
  asan_state: observed-once-inconclusive
  asan_remaining: 0
  asan_exit: 1
source_commit: 4bfa3a60957b772c3e62c2908eda0af125e2a31a
---

# Phase 16 Plan 12 Interruption Repair Record

This record is incomplete by design. Task 1 delivered diagnostic recorder coverage; it did not reproduce a driver defect, attribute the historical assertion failure, or accept a repair. The one remaining ASan observation is consumed before launch below. An exit 0 will be recorded as a passing, inconclusive observation and will stop Plan 12 incomplete.

## Preserved history and observation budget

Plan 11's `cmake --workflow --preset ci` observation passed in 4.18 seconds and is permanently consumed. Its separate interruption selector passed with TERM send status 0, driver wait status 130, grep status 0, one cleanup QUIT, a reaped host, and a released lock. It did not reproduce the post-wave failure and did not identify the historical assertion read. The prior process-tree query returned `/bin/ps: Operation not permitted`; that process tree remains unavailable.

The post-wave full CI failure remains tied to source HEAD `5f899ece5d20c5b8f260c52aee13cabd65054b10`: `retroarch.session.controls` failed at 230.07 seconds with `FAIL interruption first failure was not retained`; the full run exited 8 after 648/649 tests at 234.99 seconds, before CPack. Its retained log SHA-256 is `5a46aa38f4cc5e963d2d28f64e1beb46eabd19d2fb61183ecd5c66344a9a7098`. The copied eventual journal SHA-256 is `29c4bc65a2256dec40ad1083b970c1e34218614e684db22697701c90694b3f77`; it contains readiness and cleanup rows with `INTERRUPTED_TERM`. Those eventual bytes do not recover the historical selected file, grep status/stderr, or assertion-time process state. No cause is inferred.

One ASan invocation was available after the consumed CI pass. It is marked **consumed before launch**, with zero remaining, to prevent a retry if this command is interrupted or its evidence is unusable. Attempt, operation, and readiness budgets remain 30000 ms; cleanup remains 1000 ms; the outer bound remains 60 seconds. No full CI, repeat observation, or diagnostic retry is authorized by this record.

## Task 1 diagnostic evidence

The recorder test commit is `f61a29f` (`test(16-12): add interruption capture contract tests`). Its RED command exited 1 before implementation because `capture_interruption_assertion` was absent; output is retained at `build/phase16-capture-execute/task1-red.log`, SHA-256 `ca1e7e70e22c954b82fe452a0b8b8887d0ad9b8598260f6239fb8c963f415f91`. This is recorder-contract RED only, not a driver defect regression.

The implementation and docs commit is `4bfa3a60957b772c3e62c2908eda0af125e2a31a`. The exact synthetic selector exited 0 and reported seven cases: real grep status 0 and 1; controlled status 2 with binary stdout/stderr; recorder failures with both matching and non-matching inputs; pathname replacement while the original descriptor remained open; and isolated event ownership. The first GREEN log is `build/phase16-capture-execute/task1-green.log`, SHA-256 `770679f6613b1225c26cfdb1df2c3d3cbb2cd17be63a44b547ea3b2805aa4f98`. The post-commit tracer feedback rerun also exited 0 with the same seven-case result; its log is `build/phase16-capture-execute/task1-tracer-verified.log` with the same SHA-256. `sh -n tests/retroarch/test_state_session.sh tests/retroarch/state_session.sh` exited 0.

The synthetic selector parses the existing five arguments, then exits before `FRAME_INDEX`, fixture generation, or driver launch. It proves the recorder's status/byte/metadata/ownership contract. Pre/post snapshots copy bytes and obtain device/inode/size from the same open handle. They are nearby snapshots and do not establish which bytes genuine grep read. The private controlled command and replacement arguments are gated to selftests; the shared interruption path passes the fixed genuine `interruption_grep` function.

## Bound source, docs, build, and configuration

The current source/doc commit is `4bfa3a60957b772c3e62c2908eda0af125e2a31a`. These working-tree SHA-256 and Git blob identities bind the observation inputs:

| Path | Git blob at source commit | SHA-256 |
|---|---|---|
| `tests/retroarch/state_session.sh` | `52ed46b196068c32ce58472644672c832ee47caf` | `2fd51ead4519d53aaf1993ba56233a91e86d59620741822e7583d73d3b47ea7b` |
| `tests/retroarch/test_state_session.sh` | `8b595b1fd695609a96019960c8823fa3d12ad472` | `4282cdad12e31e74837e129db16ff1be478bed1145fa22fa0a4e1f93d7eed8f5` |
| `README.md` | `97789f64d4930bc47495488bf04736d0f081c720` | `3ab75824eec4e55891cf3b2b6167f34c086a4967250e590e1fa7a9421ce6ee23` |
| `include/nesturbator.h` | `a62fc2665f3b4fb416fa1e738bf8e5f07735dc8b` | `b917f0e33445597b1f8d4c1dde2e8c2a1a183f08346b252b0324813b400df199` |

The historical pre-Plan-12 source blobs recorded by Plan 11 were `state_session.sh=39c01e6e97a93e6349dc23d7f0dcdf886508d317`, `test_state_session.sh=015aa818c141f7f5788df93b13f77abf12f06fc1`, `frame_index.c=204d3ff64cc288176b0e6f47a1788c122a36815c`, `README.md=df027b039cc6ad93ff75da1327a5ecf469a68f65`, and `include/nesturbator.h=f920a754513105077507bb88013c11e76c7c5a82`. The current frame-index source remains read-only.

The ASan build is already configured with Ninja, Debug, `NESTURBATOR_SHORT_RUNS=ON`, and `-fsanitize=address,undefined -fno-sanitize-recover=all`. `build/asan/CMakeCache.txt` SHA-256 is `875a51bed2081f932dfa23ca0c07f1f3ed7effe8a934c557f8db2e53012cd304`; `build/asan/tests/retroarch/frame_index` is `1a137b9ac1f782285ec9ea1362f9cd747bebe0e71362cf3930c7e795ef42b917`; `build/asan/runner/nesturbator-run` is `51b6323a21de9d95ef2d240f83c491272d865e049e0d005ee38ddec5fbbdff6e`. No build was required. The passed Task 1 command used `build/ci/tests/retroarch/frame_index`, SHA-256 `10fba3ddd9bdbe01d02f16b7d2938263938533a49d8619aa960f4f44534270f9`.

`.planning/config.json` remains byte-identical to the protected baseline `0517388f5d422ae97afdc5dc1ae11bc43ebd481a757424f152ee8af61559ff5e` and unstaged. The observation command is exactly:

```sh
gsd_run run-with-timeout 60 -- cmake -E env STATE_SESSION_CONTROL=interruption sh tests/retroarch/test_state_session.sh tests/retroarch/state_session.sh build/asan/tests/retroarch/frame_index build/phase16-capture-execute/asan-observation cmake tests/retroarch/run_retroarch.cmake
```

## Discriminating question and limits

The question is: if the unchanged `grep -F INTERRUPTED_TERM` assertion fails on this current source, do the owned records distinguish a no-match result, a selected-file/path identity condition, or grep execution failure? The run makes one genuine assertion call. Status 0 means grep found a line; status 1 means grep reported no match; status 2 or another nonzero status, together with captured stderr and command status, indicates execution failure or another command outcome. Missing, malformed, or out-of-order recorder/driver events make the observation unusable and stop this continuation. A changed pre/post handle identity establishes that the pathname named different file identities around the call, not which identity grep read. Stable nearby snapshots still do not prove grep's read view. A bare status, plausible interleaving, or matching eventual journal is insufficient to infer the historical cause or authorize a correction.

The harness retains every expanded attempt candidate and the original first selected path, launch PID, TERM-send status, driver wait status, grep status/output bytes, pre/post handle metadata/bytes, monotonic event samples, driver handler-entry/journal-close/handler-exit trace, direct owned-process liveness checks where observable, QUIT count, and lock state. It does not query or reconstruct an unrelated process tree. Any identity or liveness not established through these owned handles/APIs is recorded unavailable; shell `$$` identifies only the harness or driver process where directly sampled, never a command-substitution process.

## Observation outcome

The one permitted ASan interruption command was launched once under the 60-second wrapper and exited 1 after 6.26 seconds. Its stdout and stderr are retained at `build/phase16-capture-execute/asan-observation.stdout` (SHA-256 `87525798c2f494d18f53fd6d5a825773b2b65f4ddf986a9b2dcc68c5be5ec5e0`) and `build/phase16-capture-execute/asan-observation.stderr` (SHA-256 `90dd6b4c766ba59ce32773e7e917e5a94010b0b6c5be83a05c5af3040f484331`). The exit record is `build/phase16-capture-execute/asan-observation.exit` (SHA-256 `4355a46b19d348dc2f57c046f8ef63d4538ebb936000f3c9ee954a27460dd865`), containing `1`. The run repeated the seven synthetic capture checks, generated and checked all 44 progression controls, and then ended with `FAIL interruption recorder unusable: driver-trace-missing-or-invalid`.

The recorder retained one expanded candidate and selected `build/phase16-capture-execute/asan-observation/session-97127-interrupt/ra/attempts/attempt-451162955-97610`. It recorded TERM send status 0, driver wait status 130, assertion decision status 0, one QUIT send, and lock absent after wait. The harness's post-wait host `kill -0` result was nonzero, which its event explicitly says cannot distinguish process exit from permission denial. Driver PID 97610 and host PID 97743 were observed; the driver trace also recorded child PID 97762 at handler entry. Later trace fields mark unavailable identities/liveness as unavailable.

The assertion snapshots were both 377 bytes, read 377 bytes from device 16777230/inode 615355542, and remained identical around the assertion. Both snapshots and the selected journal have SHA-256 `9f8814ab643f1145b83023ea17b90b5660093f09ad48b49803570f91b5ecd268`. The captured assertion stdout and stderr files are each empty. The three driver trace rows are present in `session-97127-interrupt/ra/interruption-events.tsv`, in the expected handler-entry, journal-close-success, handler-exit order. At handler exit, however, the trace writer recorded `host_alive=unavailable` after its owned host PID was no longer available in the handler state. The trace validator only accepts a numeric value in that field, so it rejected the explicit unavailable marker and made the observation unusable. This is a recorder/validator compatibility defect in the current instrumentation, not evidence explaining the historical first-failure assertion: the current recorded assertion status is 0, and no compatible historical cause was demonstrated.

The one ASan allowance remains exhausted. This observation is therefore terminal for Plan 12: status `INCOMPLETE`, no cause-specific assertion repair or acceptance, no additional CI/ASan observation, no Task 3, no `16-12-SUMMARY.md`, and no Plan 07 handoff. The diagnostic capture coverage and this truthful repair record are retained for replanning.
