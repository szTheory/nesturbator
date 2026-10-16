---
phase: 16-host-evidence-and-roadmap
plan: "15"
status: offline-complete
source_commit: bed875eb7a3a8fc4a738e92a00942d431b3b9509
red_commit: 44d1630184f1243ea1599ca3f5c5e74dfc693f25
observation_budget:
  prior_ci_seconds: 4.18
  prior_ci_state: consumed
  prior_asan_seconds: 6.26
  prior_asan_state: consumed-unusable
  plan13_asan_elapsed_ms: 6555
  plan13_asan_state: consumed-unusable
  plan15_driver_observations: 0
  plan15_full_workflows: 0
  plan15_remaining_diagnostic_allowance: 0
---

# Phase 16 Plan 15: Offline Capture Identity Repair

## Result and scope

Plan 16-15 proves and repairs the recorder's nested-capture identity defect using the existing offline selector. Before the fix, the named `nested-capture-session-identity` test reached the actual writer, genuine grep, snapshots and final validator, then failed because the eight rows from `capture_interruption_assertion` used the nested directory basename `capture` instead of the caller's owned session ID. After the fix, both independently identified sessions pass that same validator, and valid-shape wrong, mixed and reused identities fail.

This is only an offline harness identity result. No actual driver, standalone diagnostic or compatibility observation was run. The historical assertion cause remains unknown. Plan 14 still owns actual-driver compatibility through its existing registered acceptance, current `ci`/`asan` controls, normal full CI and packaging, independent review, and required later gates. EVID-04, the historical 21/22 verification, and the verifier fingerprint remain unchanged.

## RED and GREEN evidence

The production-boundary extraction was committed behavior-neutrally as `f01919af148dd418f41b79e4db5694e20f953e90`. RED then ran the existing early selector against that source with the new test content later committed unchanged as `44d1630184f1243ea1599ca3f5c5e74dfc693f25`. Its exact command was:

```sh
gsd_run run-with-timeout 60 -- cmake -E env STATE_SESSION_CONTROL=interruption-capture-selftest sh tests/retroarch/test_state_session.sh tests/retroarch/state_session.sh build/ci/tests/retroarch/frame_index build/phase16-identity-execute/offline cmake tests/retroarch/run_retroarch.cmake
```

The command exited 1. The complete TAP report is `build/phase16-identity-execute/task1-red.stdout` (161 bytes, SHA-256 `af56c50fe9d193b2667e6cbb5c05b3b21a98648d0417a5fc556666f2afe47c2b`); stderr was empty (SHA-256 `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855`). It reports only `nested-capture-session-identity` failing. The unchanged JSON evidence record is `build/phase16-identity-execute/task1-red.evidence.json` (SHA-256 `94d548b997c24805d8f579d3d57e22a901da0070d704a3c7dff3cc2ae00220b2`); its `semanticAssessment` binds the report to the actual event rows and successful assertion/snapshot prerequisites. `gsd_run check tdd-red-evidence build/phase16-identity-execute/task1-red.evidence.json --raw` exited 0 with `RED_EVIDENCE_OK`, target `nested-capture-session-identity`, one test, one failure, and no report errors. The retained classifier output is `build/phase16-identity-execute/task1-red-classifier.stdout` (SHA-256 `51e9373c8792832592f392ac8aad2e50a509434154de3d68dcc9c693d8c699f6`).

The RED ledger has ten ordered rows and a valid monotonic sequence. Rows 1 and 10 carry `session-A-owned`; rows 2–9 carry `capture`. Genuine grep returned 0, stdout matched the expected 29 bytes, stderr was empty, recorder status was 0, and both journal snapshots matched the 47-byte source with stable open-handle device/inode/size. The production `verify_interruption_event_file` rejected the ledger on ownership. The offline selector exited before the 36 trace cases and before fixture, frame-index, transport or driver work; the seven preexisting capture cases had already run without output.

GREEN was committed as `bed875eb7a3a8fc4a738e92a00942d431b3b9509`. The same selector exited 0 before the identity commit and again after it as the tracer feedback gate. Both complete outputs were 2,854 bytes and have SHA-256 `7ab188efefc55bfd6c67d3a58220db30a85ef26a37fe2fd4db77dadb3c5173d6`; the paths are `build/phase16-identity-execute/task1-green.stdout` and `build/phase16-identity-execute/task1-tracer.stdout`. Both stderr files are empty with SHA-256 `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855`.

The green output independently records:

- `session-A-owned` in `session-A/capture` and `session-B-owned` in `session-B/capture`; each had ten rows and passed the actual final validator.
- Invalid owned identity: recorder status became unusable with `session-id-invalid`, while the genuine assertion remained status 0 with its exact output bytes.
- Four actual-validator negatives after valid positive ledgers: wrong expected ID, one mixed internal row, one mixed outer row, and session-A ledger reuse under session B. The mixed-row copies differ from their source only in the identity field.
- The original seven capture cases and all 36 trace cases passed. Shell syntax checks passed for `test_state_session.sh`, `state_session.sh` and `interruption_trace.sh`.

The source identity register is:

| Commit | Path | Git blob | SHA-256 |
|---|---|---|---|
| `f01919af148dd418f41b79e4db5694e20f953e90` | `tests/retroarch/test_state_session.sh` | `88f3dedad2034424479536c28b3430e7efdfd161` | `335d7ad2eaa63ad6bb475e4d61eb8bb692fab2b2f1456e776b4b94e10eaceaa1` |
| `44d1630184f1243ea1599ca3f5c5e74dfc693f25` | `tests/retroarch/test_state_session.sh` | `2de8cb15b6e44c4b1ec4d0f208bc8206608b036c` | `5eed317f2d2b102feb50779944ad373844b34e9e7709d89dcd28a2071c07cca8` |
| `bed875eb7a3a8fc4a738e92a00942d431b3b9509` | `tests/retroarch/test_state_session.sh` | `09fdecaca9f1457d05c853018ce3bd1de17c9177` | `20e6480e3b8ba2332ad907aaf8aefba6ac8e29ebf2fce802bcaaecb295876484` |
| `bed875eb7a3a8fc4a738e92a00942d431b3b9509` | `README.md` | `6513831795a75e529cda7cb7459afa7b047607e8` | `c4b60a778f7ec775956f50dc07d11b648513a139602632106a2d102c35a2b570` |
| `bed875eb7a3a8fc4a738e92a00942d431b3b9509` | `include/nesturbator.h` | `4e0aa679658db55fea97d99e69a85acb7af0c518` | `5bfcda9e4df5ddab9287feb73f4124f4029bfe4242e813a7deb905248da786e8` |

## Immutable history and remaining acceptance

Plan 13 remains superseded and incomplete without a summary or completion credit. Its durable stop is `34d68516a15e31370a33da41ec96b9bdf404aa6b`; retain its commits `69d396dfd31d296372c9212c82499d80a3a2b133`, `4c7d8140118d3520a86adf01f72256f0149ab11e`, `6454457c37632d6eb90bd463154cc96f743f6b52`, `3dafb18f028a132732bbbf9b42b8a8827c8acf2a` and `1a481a645b0e95a9cc662c05cfdf0af10ab77926`. Its actual-driver ledger remains the immutable failed evidence: 21 rows, with 13 session-owned rows and eight rows named `capture`. The event ledger SHA-256 is `93c48175fc482e9f5c7fbf37862dd6dd858e7bb0faf8a1a889bfc930fd169079`; the accepted three-row driver trace SHA-256 is `9e60257ad47743754c7513a96a21e567d6e26f9a681df5c8936e10c6b8ab3196`; recorder errors SHA-256 is `1b9ed77bffd3ba2dfe824ce12565a635511bf4af4915d9c603482eef86e98a37`. Its actual assertion stdout was 242 bytes with SHA-256 `2de18c5b8f2b0585af0f6f2c9474c63cfa6645b55302ff39412790b6449c6f75`; stderr was empty with SHA-256 `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855`. The retained selector stdout/stderr hashes are `3ec231ebf725c25cabe1a2406a79482c63836c6534f49796664beeee432b75f4` and `14c08e3bca38490097038b221cc6ceb0129dea0b2dd192f5ac71d29f36571edd`. The Plan 13 ASan result exited 1 after 6,555 ms and remains consumed-unusable; no budget is restored by this offline correction.

Plans 11 and 12 and their records remain unchanged and incomplete. The prior Plan 11 CI observation took 4.18 seconds and is consumed. Plan 12's ASan observation took 6.26 seconds, exited 1 and is consumed-unusable. Its raw stdout/stderr hashes remain `87525798c2f494d18f53fd6d5a825773b2b65f4ddf986a9b2dcc68c5be5ec5e0` and `90dd6b4c766ba59ce32773e7e917e5a94010b0b6c5be83a05c5af3040f484331`; its driver trace did not satisfy its validator. These earlier statuses do not explain the historical assertion.

The Plan 10 timing/prefix lineage remains `0b60c8881328729706f9ff4dea8785838452460c`, `00f61cee1bc6525aa802b98d973d94747a887508`, `262f580303077d0c62b2efa5dd46d6bc9d3e2bbe`, `c1db85254d09f0e4e52fc0edc44ddd54e8daf333`, `1b1589a49f735195b9dbf89a238dc0518777335c`, `2ca8485dd53cd45e28570bc76ccaf58f8d634a85` and `10694f7da7baed83e3d61dca3f687213dbecb183`. Their immutable local results do not resolve the later native misses or replace current acceptance.

Native run [38764197994](https://github.com/szTheory/nesturbator/actions/runs/38764197994) tested PR head `3de23d4aed5d9696dc4020db3b8ef5e8205300fb`, base `9f9386c661f6fb8f6063bf756655713aedadae77` and preview merge `81c0a2132250892b324ec4c9be2a26fe8ea91056`. Intel job `116523895865` measured `readiness-response-stall` at 25,946 ms and ASan job `116523896064` measured `readiness-delayed` at 26,269 ms; both exceeded the strict 25,000 ms limit. Earlier run `38740783575` is a distinct stall failure whose old JSON mislabeled the case as delay. Run `38764179312` was automatically superseded/canceled after a body update, not manually rerun. Native timing subintervals and causes remain unknown. The old post-wave workflow failure is bound to source `5f899ece5d20c5b8f260c52aee13cabd65054b10`; its completion was revoked by `11adf3d268800682e4bef8ec96600b1fd8c6bb2c`. That workflow exited 8 after 648/649 tests, before CPack; its log hash is `5a46aa38f4cc5e963d2d28f64e1beb46eabd19d2fb61183ecd5c66344a9a7098`, and the eventual journal hash is `29c4bc65a2256dec40ad1083b970c1e34218614e684db22697701c90694b3f77`. It does not reveal the historical assertion-time read. The original assertion cause remains unknown.

Plan 14 must still use its existing registered controls for actual-driver compatibility, run current `ci` and `asan` acceptance once per unchanged candidate, then run `cmake --workflow --preset ci` with all registered tests and all three library/runner/libretro ZIPs. It also requires a fresh independent review of the exact final source and six source/document paths, zero open findings, and every required post-wave/post-merge gate. A failure remains a blocker; Plan 15 supplied no replacement probe and no test-workflow result.

Only completed Plan 14 acceptance permits Plan 07's fresh native exact-head PR run, including required CI, performance and pinned-host checks, followed by affirmative owner consent bound to that exact head and the normal merge/release path. Plan 08 then verifies the actual published release: all 18 archives, checksums and attestations, six platform metadata files and CI on the exact release commit. No consent, merge, tag or publication occurred here. The prior `v0.1.10` publication omitted MMC3; [16-RELEASE-DELIVERY.md](16-RELEASE-DELIVERY.md) retains the release boundary. EVID-04 remains open and Phase 16 remains incomplete.

The preexisting `.planning/config.json` remains byte-identical to `build/phase16-identity-execute/config-baseline.json` (SHA-256 `0517388f5d422ae97afdc5dc1ae11bc43ebd481a757424f152ee8af61559ff5e`). No CMake registration, preset, wrapper, fixture, oracle, driver, trace include, state-session implementation, network, full workflow or actual-driver observation was changed or run.
