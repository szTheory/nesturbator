---
phase: 16-host-evidence-and-roadmap
plan: "14"
status: task1-accepted
source_candidate_commit: 58b4ad282992dba9b12c7202e9f490991c43de82
config_sha256: 0517388f5d422ae97afdc5dc1ae11bc43ebd481a757424f152ee8af61559ff5e
---

# Phase 16 Plan 14: Current-source acceptance record

This record keeps current registered acceptance, recorder compatibility, and historical assertion attribution separate. The only permitted source edits are conditional on a current assertion failure that establishes a compatible mechanism and a reason-specific RED first. No historical assertion repair is presumed.

## Admission and scope

- Started: 2026-10-16T02:57:36Z
- Task 1 controls completed: 2026-10-16T03:08:34Z
- Branch: gsd/phase-16-host-evidence-and-roadmap
- Candidate source commit: 58b4ad282992dba9b12c7202e9f490991c43de82
- Safe-resume scope: resolved, no-matching-commits, commits empty; exit 0.
- Async-job manifests for Plan 16-14: none.
- Existing modified tracked paths at admission: .planning/STATE.md (parent state.begin-phase) and .planning/config.json (pre-existing user edit). Both are preserved; config remains unstaged and byte-identical to its supplied baseline.
- Plan 14 source paths were clean at admission. No Plan 14 production commit or SUMMARY exists.
- Bootstrap source: build/phase16-recorder-execute/gsd-env.sh. Source acceptance and review records are kept in this directory; raw logs and generated files stay under ignored build/phase16-acceptance-execute/.

## Candidate identity before acceptance

Git blob IDs and SHA-256 digests bind the source and read-only dependencies at the candidate commit. CMake cache and executable digests bind the pre-existing configured builds; the later workflow section records their post-run identities too.

| Path | Git blob | SHA-256 |
|---|---|---|
| tests/retroarch/interruption_trace.sh | c4c91cfc88bd0917faf2d1d8ad592a463c12213b | 43821bd32c822983d1229bef812b4d5d12e7dece9024b6aae0620265df40eac2 |
| tests/retroarch/state_session.sh | 36c2e937f5bd23b07e859d203a54bd49ae8a0fac | bd8a3830133f54a7b1530b5f4ff216dbdf0db9b16e897eca9b6b9eb7a3aa230c |
| tests/retroarch/test_state_session.sh | 09fdecaca9f1457d05c853018ce3bd1de17c9177 | 20e6480e3b8ba2332ad907aaf8aefba6ac8e29ebf2fce802bcaaecb295876484 |
| tests/retroarch/frame_index.c | 204d3ff64cc288176b0e6f47a1788c122a36815c | ecab2d5fbf294596572617759ae55c8494ec9bd95705b70bf21ea90767ea5d8b |
| tests/retroarch/CMakeLists.txt | 443cbc4b35286f56c9a5c6b29c89e37c14aaa4bf | 84bbaf464d812c1142e85851203f56e41f04511fee8bc16cf09bcc898400ec0e |
| tests/retroarch/run_retroarch.cmake | cfe2e4e7942bc6728a0a41b56d1cf9eee09f1e45 | 6a387dc0ac1c2be498f5375d597e4d84ae69763a471e73e80c6a5400972e9581 |
| README.md | 6513831795a75e529cda7cb7459afa7b047607e8 | c4b60a778f7ec775956f50dc07d11b648513a139602632106a2d102c35a2b570 |
| include/nesturbator.h | 4e0aa679658db55fea97d99e69a85acb7af0c518 | 5bfcda9e4df5ddab9287feb73f4124f4029bfe4242e813a7deb905248da786e8 |
| CMakePresets.json | 10a19b58652adb634cc0357de5f5cc91cd848409 | a97957fcd87052b863be85a26609c0c985402b8eac645748907cec6ea237a024 |

| Build input | SHA-256 before acceptance |
|---|---|
| build/ci/CMakeCache.txt | af03303ad33e142dde959b50627ec4720d62635c88eeb032de79aacbf1a910d4 |
| build/asan/CMakeCache.txt | 875a51bed2081f932dfa23ca0c07f1f3ed7effe8a934c557f8db2e53012cd304 |
| build/ci/tests/retroarch/frame_index | 10fba3ddd9bdbe01d02f16b7d2938263938533a49d8619aa960f4f44534270f9 |
| build/asan/tests/retroarch/frame_index | 1a137b9ac1f782285ec9ea1362f9cd747bebe0e71362cf3930c7e795ef42b917 |

Host: Darwin 25.6.0 arm64; CMake/CTest 4.4.3. Both caches resolve to this checkout. The CI and ASan CTest presets specify four jobs. The commit identity passes the repository GitHub noreply convention. The actual configured presets, registered tests, shared source writer/validator, and current fixture/oracle paths were inspected before execution.

## Immutable history and exhausted observation budget

The original post-wave candidate 5f899ece5d20c5b8f260c52aee13cabd65054b10 failed the normal workflow at test 634/649, retroarch.session.controls, with “FAIL interruption first failure was not retained”; CPack was not reached. The revoked completion was removed by 11adf3d268800682e4bef8ec96600b1fd8c6bb2c. The original workflow log SHA-256 is 5a46aa38f4cc5e963d2d28f64e1beb46eabd19d2fb61183ecd5c66344a9a7098; LastTest.log is 9a26b048d5334c06627825c7e5bf4535d07284d21d21e488e20420db0dc9beeb; the eventual journal is 29c4bc65a2256dec40ad1083b970c1e34218614e684db22697701c90694b3f77. The captured assertion stdout hash is 2de18c5b8f2b0585af0f6f2c9474c63cfa6645b55302ff39412790b6449c6f75. These eventual artifacts do not identify what the failed assertion read; the historical cause remains unknown.

The retained timing/prefix history includes 0b60c88, 00f61ce, 262f580, c1db852, 1b1589a, 2ca8485, and 10694f7. Native run 38764197994 tested head 3de23d4aed5d9696dc4020db3b8ef5e8205300fb against base 9f9386c661f6fb8f6063bf756655713aedadae77 with preview 81c0a2132250892b324ec4c9be2a26fe8ea91056. Intel readiness-response-stall measured 25,946 ms and ASan readiness-delayed 26,269 ms against strict 25,000 ms limits. Run 38740783575 is a separate earlier stall; 38764179312 was automatically canceled after a body update.

Standalone allowances remain exhausted: Plan 11 CI 4.18 s, consumed; Plan 12 ASan 6.26 s, exit 1 and unusable; Plan 13 ASan 6,555 ms, exit 1 and unusable. Plan 13's 21-row event ledger mixed 13 invocation-owned rows with eight rows named capture (ledger SHA-256 93c48175fc482e9f5c7fbf37862dd6dd858e7bb0faf8a1a889bfc930fd169079); its accepted three-row driver trace SHA-256 is 9e60257ad47743754c7513a96a21e567d6e26f9a681df5c8936e10c6b8ab3196 and recorder-error file SHA-256 is 1b9ed77bffd3ba2dfe824ce12565a635511bf4af4915d9c603482eef86e98a37. Plan 15's source-bound RED/GREEN corrected the offline shared-boundary identity contract at bed875eb7a3a8fc4a738e92a00942d431b3b9509, with RED commit 44d1630184f1243ea1599ca3f5c5e74dfc693f25; it ran no driver, standalone observation, full workflow, or ASan test. Its offline proof is not actual-driver compatibility or historical-cause attribution.

No diagnostic replay, standalone selector, or replacement observation is allocated. The commands below are the previously unfulfilled registered current-source acceptance contexts. A terminal failure will be retained without an unchanged rerun. A source correction, if justified by a current failure, requires compatible mechanism evidence and a reason-specific RED before editing, then fresh affected ci/asan acceptance.

## Task 1: registered current-source controls

The selected CTest names bind actual production shell-driver controls and existing marker/progression controls. The interruption harness uses its local fake host/transport and production state-session driver, then the real shared event writer and final validator. It does not launch GUI RetroArch. The required single-session identity is the caller-owned invocation ID across before-assertion, driver handler-entry/journal-close/handler-exit, and after-wait capture rows; the final check rejects mixed or reused IDs. The original grep status, bytes and error stream remain separate from recorder status.

| Acceptance command | Result | Captured output |
|---|---|---|
| `ctest --preset ci -R '^retroarch.(session.(readiness|controls)|marker[.]|progression[.])' --no-tests=error --output-on-failure` | exit 0; 10/10 passed; 233.81 sec | `build/phase16-acceptance-execute/task1-ci-controls.log` SHA-256 `d87d4dfd7ffb195409482d34989d3792dec3ec2ef70ddde1c8800d94d1088b67`; JUnit `build/ci/build/phase16-acceptance-execute/task1-ci-controls.junit.xml` SHA-256 `26c50d25242c8ff96fea187c4ec84f33881c9edc9baafab78754a80eb686448d`, 10 cases, 0 failures; CTest `LastTest.log` copy SHA-256 `870808e1bba1bf85b8501940fb46658e8a6e0e77c9559414fc7e24247c3ac696` |
| `ctest --preset asan -R '^retroarch.session.(readiness|controls)$' --no-tests=error --verbose --output-on-failure` | exit 0; 2/2 passed; 290.48 sec | CTest `LastTest.log` copy `build/phase16-acceptance-execute/task1-asan-controls-LastTest.log` SHA-256 `a265732c9e1bae27938f0cf5563a6c52b626bffd276e5bdc5b904750e5855167` |

The CI selector registered exactly ten tests: marker controls/fixtures/generate/qualify, progression tracer/prefix/complete-required/controls, and session controls/readiness. The ASan selector registered both session controls/readiness. The complete machine output is retained in the CTest logs above; the ASan and normal-workflow invocations did not request separate JUnit files. Neither run emitted an AddressSanitizer diagnostic or changed source. The readiness delayed and response-stall operation measurements were respectively 1604/5086 ms in CI and 1613/5066 ms in ASan; their driver+cleanup measurements were 9258/12593 ms and 8661/12366 ms, all within the declared per-case windows and strict 25000 ms driver bound. The controls exercised the actual driver and existing shared writer/validator.

The CI interruption case used `session-60414-interrupt`. Its harness event ledger has 21 contiguous rows under that ID (SHA-256 `7fd5066f50ae308672966f8b23f0d420f4af024f66b9cac1efac9bab61ed3134`); the driver trace has three contiguous rows under the same ID (SHA-256 `7a7739bd7344dc3418d936abd25c098bb9fc86e59d6f7ed41eabd223d644bb68`). Immediate assertion stdout SHA-256 is `08846660e4014a672f99ed4dd42ea47db01bb34d4ce3ac140a8953d033ff5f02`, stderr is empty (SHA-256 `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855`), and both journal entries retain `INTERRUPTED_TERM` (journal SHA-256 `e92aab789760f12d855cdccd7cf5a62e435d4a772c8c45490c040db314e5f8fa`). Pre/post snapshot metadata kept the same device/inode/size and read all 375 bytes (SHA-256 `540729acd28774794c968f9c3816d4ec7dfe698e8ccf8fcd00704acc7e0c78be`). The final event rows record one QUIT send, dead host after wait, and released lock. This is a current passing observation only; it does not recover the old failed assertion's cause.

Both commands are planned once per unchanged candidate. The optional CTest JUnit output file, if emitted in the same invocation, is only an evidence sink and does not change the selected tests. The overall normal workflow and package identities are recorded under Task 2. Parent-owned post-wave checks are a separate later gate and are not replaced by this record.

## Results

Task 1's unchanged-candidate CI and ASan registered controls passed with no reproduced current assertion failure. No source correction or original-assertion repair is claimed; the historical cause remains unknown. Task 2 normal full workflow, package digests, independent exact-source review, review disposition, and required parent post-wave checks remain pending. No native acceptance, release, EVID-04 completion, or Phase 16 completion is claimed.
