---
phase: 16-host-evidence-and-roadmap
plan: "14"
status: incomplete-scope-amendment
source_candidate_commit: 0e20688badb715ef6d31a10148c7fa4e1431d4da
config_sha256: 0517388f5d422ae97afdc5dc1ae11bc43ebd481a757424f152ee8af61559ff5e
---

# Phase 16 Plan 14: Current-source acceptance record

This record keeps current registered acceptance, recorder compatibility, and historical assertion attribution separate. The only permitted source edits are conditional on a current assertion failure that establishes a compatible mechanism and a reason-specific RED first. No historical assertion repair is presumed.

## Admission and scope

- Started: 2026-10-16T02:57:36Z
- Task 1 controls completed: 2026-10-16T03:08:34Z
- Branch: gsd/phase-16-host-evidence-and-roadmap
- Task 1 source candidate: 58b4ad282992dba9b12c7202e9f490991c43de82.
- Pre-correction full-workflow and review candidate: c87857a484ba5b3710de4975f6a389e860ab1348.
- Corrected source candidate: 0e20688badb715ef6d31a10148c7fa4e1431d4da. It includes the cause-specific startup-lock correction and README/header status update; acceptance and review gates must run again on this identity.
- Safe-resume scope: resolved, no-matching-commits, commits empty; exit 0.
- Async-job manifests for Plan 16-14: none.
- Existing modified tracked paths at admission: .planning/STATE.md (parent state.begin-phase) and .planning/config.json (pre-existing user edit). Both are preserved; config remains unstaged and byte-identical to its supplied baseline.
- Plan 14 source paths were clean at admission. No Plan 14 production commit or SUMMARY existed at admission.
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

Host: Darwin 25.6.0 arm64; CMake/CTest 4.4.3. Both caches resolve to this checkout. The CI and ASan CTest presets specify four jobs. The commit identity passes the repository GitHub noreply convention. The actual configured presets, registered tests, shared source writer/validator, and current fixture/oracle paths were inspected before execution. At the pre-correction full-workflow candidate, the six reviewed source/doc paths had the same Git blobs and SHA-256 digests as at the Task 1 candidate above. The corrected candidate intentionally changes `state_session.sh`, `test_state_session.sh`, `README.md`, and `include/nesturbator.h`; their post-correction identities are recorded after revalidation.

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

Both commands were run once on the unchanged source candidate. The optional CTest JUnit output file was an evidence sink in the same CI invocation and did not change the selected tests. The overall normal workflow and package identities are recorded under Task 2. Parent-owned post-wave checks are a separate later gate and are not replaced by this record.

## Task 2: normal full workflow and package outputs

`cmake --workflow --preset ci` ran once at pre-correction candidate `c87857a484ba5b3710de4975f6a389e860ab1348`, with preset jobs=4. Configure completed, build reported no work, CTest passed 649/649 in 241.03 sec, and the workflow reached its CPack ZIP step (exit 0). Full stdout/stderr is `build/phase16-acceptance-execute/task2-full-ci.log`, SHA-256 `6d21fcf873412b22d86874aa8875dad673eb8452f3a9027392145cf13b9b2d2e`; copied CTest `LastTest.log` is `build/phase16-acceptance-execute/task2-full-ci-LastTest.log`, SHA-256 `9bb69d6b68da4d69fabeec3a0814ad554260aeba2735721d7b7796356b35b79c`. No standalone JUnit sink was configured for the normal workflow. CPack explicitly reported three packages for that pre-correction candidate; they do not satisfy corrected-candidate package acceptance:

| Package | SHA-256 |
|---|---|
| `build/ci/packages/nesturbator-0.1.10-library-macos-arm64.zip` | `637f0dfd93c19b874896834c8658a14075f8440390f3fd9afffaef5bd50fd280` |
| `build/ci/packages/nesturbator-0.1.10-libretro-macos-arm64.zip` | `72481e4dcdd07fc7d863ac7eb39cc206c6778326145e65cee1e32c7564cd4dbb` |
| `build/ci/packages/nesturbator-0.1.10-runner-macos-arm64.zip` | `513ef49190a76c6493ddffd7ca23ff147b70750bdb3e9c3de457ebe375a84a48` |

The emitted ZIP sizes were 99,520 bytes (library), 81,628 bytes (libretro), and 92,828 bytes (runner). Their contents respectively include the public header/static library/CMake and pkg-config metadata; the libretro dynamic library/info/notices; and the `nesturbator-run` executable. The required whitespace gate `git diff --check 3de23d4aed5d9696dc4020db3b8ef5e8205300fb HEAD` passed.

The pre-correction workflow identity was source commit `c87857a484ba5b3710de4975f6a389e860ab1348`; CI and ASan cache SHA-256 `af03303ad33e142dde959b50627ec4720d62635c88eeb032de79aacbf1a910d4` and `875a51bed2081f932dfa23ca0c07f1f3ed7effe8a934c557f8db2e53012cd304`; CI and ASan `frame_index` SHA-256 `10fba3ddd9bdbe01d02f16b7d2938263938533a49d8619aa960f4f44534270f9` and `1a137b9ac1f782285ec9ea1362f9cd747bebe0e71362cf3930c7e795ef42b917`; config SHA-256 `0517388f5d422ae97afdc5dc1ae11bc43ebd481a757424f152ee8af61559ff5e`, byte-identical to the supplied baseline. Corrected-candidate focused CI/ASan and full-workflow acceptance are pending. This local evidence does not establish native-host acceptance or EVID-04 publication.

## Independent review finding and correction trigger

Fresh review `16-14-REVIEW.md` at `c87857a484ba5b3710de4975f6a389e860ab1348` reported CR-01 (BLOCKER) in `state_session.sh` and WR-01 (WARNING) in `README.md` and `include/nesturbator.h`. CR-01 identified that TERM could arrive after fixed-path endpoint-lock `mkdir` succeeded but before the driver installed EXIT/INT/TERM/HUP cleanup. The existing interruption controls signal only after host startup, so they did not cover this acquisition transition. The planned wrapper trigger sent TERM immediately after the successful lock-creation primitive; the test then required status 143, no endpoint lock, and successful subsequent acquisition.

The cause-specific RED reproduced CR-01: test commit `5524fef` produced CTest exit 8 on `FAIL startup TERM left endpoint lock behind`. Raw log SHA-256: `8842a98e6db64f51774dd7aa0ceea034b0491483b7de53cef0632e3f67f2af6d`; JUnit SHA-256: `754af60a2c2bc5a9f03a6261f092cb9430b0db79fa47729f7f68b4cbca4b8a29`; evidence record `build/phase16-acceptance-execute/task1-lock-acquisition-red.json` returned `RED_EVIDENCE_OK`. First GREEN attempt log SHA-256 `26ce8f0df3c606ce3d8d01e7477fc09144e15a780f9d427eaa2b0794f9784629` exposed that normal cleanup's later EXIT trap still used directory-only `rmdir`; the corrected cleanup now calls the ownership-aware release routine. Second GREEN passed the existing readiness selector (24.06 sec, exit 0): log SHA-256 `ae90beb696f08ca9a384f14c0d8e7f8f9a74f93286aa38656f72446c542a3e2a`, JUnit SHA-256 `b9a3c5334d241cc5805cf2f30592b3fa0535214e30fdc0e7e758235f4b5cc7f2`. The correction is committed as `0e20688badb715ef6d31a10148c7fa4e1431d4da`: traps are established before acquisition, a unique token-bearing hard link identifies ownership atomically, and normal/signal cleanup removes only matching lock state. The regression checks that the next session can acquire the endpoint. WR-01 was resolved in the same behavior commit by updating README/header while preserving the unknown historical assertion cause and pending native/release gates. Focused CI/ASan and full-workflow acceptance for corrected candidate `0e20688badb715ef6d31a10148c7fa4e1431d4da` remain pending; the earlier `c87857a` pass is not reused.

## Results

Pre-correction focused CI/ASan and the full workflow passed, but the independent review found and the cause-specific RED reproduced a startup-lock cleanup issue not exercised by those controls. The corrected candidate passes the bounded regression GREEN; corrected-candidate focused CI/ASan, full workflow/packages, and fresh exact-source review/disposition remain pending. No original-assertion repair is claimed, and the historical assertion cause remains unknown. Parent post-wave checks remain pending. No native acceptance, release, EVID-04 completion, or Phase 16 completion is claimed.

## Terminal corrected-candidate failure and scope stop

The corrected candidate `0e20688badb715ef6d31a10148c7fa4e1431d4da` ran the prescribed CI selection once and exited **8**: **9/10 passed**, `retroarch.session.controls` failed after218.83seconds; total218.84seconds. The terminal reason was `FAIL interruption recorder unusable: driver-trace-TRACE_VALUE`. Corrected-candidate ASan, full CI/packages, refreshed clean review and parent post-wave acceptance were not run. No unchanged retry or new standalone observation occurred. No14 SUMMARY or07 handoff exists.

The failure is attributable to the new lock representation, not the historical assertion. The signal-safe correction publishes a token-bearing regular file with `ln`. The read-only trace writer still tests `[ -d "$LOCK_DIR" ]`; the retained driver trace therefore reports `absent` in rows1–2 while the lock exists, and its actual validator correctly rejects those values. The immediate genuine assertion returned0; record usability remained a separate failing gate. The fixture, seven capture cases,36trace cases and44progression cases passed before this terminal refusal. Passing the startup-signal regression does not accept this integration regression.

The independent review at `c87857a484ba5b3710de4975f6a389e860ab1348` remains `issues_found`, with CR-01 startup-lock signal cleanup and WR-01 status wording. Cause-specific RED `5524fef1692889687fa014f3cad34b24a62881bd` and bounded regression GREEN support a partial lock correction at0e20688; its failed integration means CR-01 is not accepted as closed. README/header now explicitly describe current acceptance as incomplete. No independent clean review or open0 disposition is claimed.

### Scope breach and reversal

Plan14 explicitly declares `tests/retroarch/interruption_trace.sh` read-only and requires a bounded amendment before editing outside its four source/doc paths. The executor incorrectly applied a Rule1 correction there in `d2d7969c1e7d4cdc3a6d470283b630a818feb92c`, changing exactly one predicate from directory-only to existing-object/symlink detection. That commit is retained in history, but the parent has reversed its sole source change to restore trace blob `c4c91cfc88bd0917faf2d1d8ad592a463c12213b`, SHA-256 `43821bd32c822983d1229bef812b4d5d12e7dece9024b6aae0620265df40eac2`. The parent commits this reversal together with the incomplete stop; no history was rewritten and no unrelated edits were reverted. The proposed patch is preserved under ignored `build/phase16-acceptance-execute/scope-stop/out-of-scope.patch` for amendment review. It has **no runtime acceptance evidence**.

The necessary next step is `$gsd-plan-phase 16 --gaps`: reconcile all partial14 commits without a success SUMMARY, explicitly decide the startup-lock/trace integration scope, retain the demonstrated cause and reason-specific regression, and carry forward corrected CI/ASan/full-workflow/packages/fresh independent review/required parent gates. Do not repeat unchanged14: installed DAG readiness is not resume permission once matching partial execution history exists. Native exact-head acceptance, owner merge consent and actual public bytes remain07/08 obligations; historical assertion cause remainsUNKNOWN and all older standalone allowances remain exhausted.

### Preserved terminal evidence

| Path | Bytes | SHA-256 |
|---|---:|---|
| `build/phase16-acceptance-execute/task1-ci-controls-corrected.log` | 12369 | `4654c240cc85a455ace3df55354559cb90ba65063603e81782f8c966d6d6199d` |
| `build/phase16-acceptance-execute/scope-stop/corrected-ci.junit.xml` | 17891 | `5f12726e21f05244fc634f606e48c8066305643e49da7c87f743231b9bfc3761` |
| `build/phase16-acceptance-execute/scope-stop/corrected-ci.LastTest.log` | 29686 | `5be834c15381aca00c871d45dd3a4028fabbbd49517d2cd08089661c30bef8aa` |
| `build/phase16-acceptance-execute/scope-stop/corrected-ci.LastTestsFailed.log` | 31 | `0197cde73d65a38f97f05bb38d6ef0d1cf8016755c9ca676472264acc2788ced` |
| `build/phase16-acceptance-execute/task1-lock-acquisition-red.log` | 735 | `8842a98e6db64f51774dd7aa0ceea034b0491483b7de53cef0632e3f67f2af6d` |
| `build/phase16-acceptance-execute/task1-lock-acquisition-red.json` | 2801 | `4559c73bef83a3c1acb683c1d09a988563902fa19f1d860b07f5d82c0901df1f` |
| `build/phase16-acceptance-execute/task1-lock-acquisition-green.log` | 917 | `26ce8f0df3c606ce3d8d01e7477fc09144e15a780f9d427eaa2b0794f9784629` |
| `build/phase16-acceptance-execute/task1-lock-acquisition-green2.log` | 233 | `ae90beb696f08ca9a384f14c0d8e7f8f9a74f93286aa38656f72446c542a3e2a` |
| `build/phase16-acceptance-execute/scope-stop/manifest.json` | 12530 | `927c1adc3131261b42371869cf4bd694922e913c3511ad954bfddea55195411b` |
| `build/phase16-acceptance-execute/scope-stop/out-of-scope.patch` | 1168 | `478d9d9b6197bb575245c2877bbb18ecd5d54b8ef28592419eebb9bd134bb48d` |

The manifest binds48 regular retained files, including the exact failed owned tree `session-87792-interrupt`, raw CTest/JUnit, proposed patch and three copied **pre-correction** ZIPs. These packages remain bound to c87857a and are not accepted for0e20688 or the current tree. Fresh parent verification checks the manifest hashes and failed10-case/one-failure JUnit, shell syntax, source reversal, config preservation and metadata consistency. These are stop-integrity checks, not runtime acceptance. The required parent post-wave workflow applies after a successful serial plan completion; this plan stopped incomplete at its failure/scope gate, so no post-wave test launch substitutes for the missing amendment. No phase verification/completion, native refresh, PR mutation, push, merge, publication, later phase or product-roadmap upkeep occurred.

### Source identities at the bounded stop

The two runtime shell paths remain byte-identical to failed candidate0e20688; the trace include is restored byte-identically to its0e20688/58b4ad2 blob. README/header differ only in status comments. No later current-source pass or clean review exists.

| Path | Worktree Git blob | SHA-256 |
|---|---|---|
| `tests/retroarch/state_session.sh` | `d04fef7bb29ea60bcd63f3552240fae973ba87d3` | `7f72aac3b6ce6c06acc84252264ebb9e132543074223d24049a446afba0872db` |
| `tests/retroarch/test_state_session.sh` | `2e30c11ebeee3db010ea78b36038abecc6c8ddfe` | `1e4a7cc36df75b6bca31c7e2baf5383d252e8409703d1715cf0098dbd62dc1d1` |
| `tests/retroarch/interruption_trace.sh` | `c4c91cfc88bd0917faf2d1d8ad592a463c12213b` | `43821bd32c822983d1229bef812b4d5d12e7dece9024b6aae0620265df40eac2` |
| `tests/retroarch/frame_index.c` | `204d3ff64cc288176b0e6f47a1788c122a36815c` | `ecab2d5fbf294596572617759ae55c8494ec9bd95705b70bf21ea90767ea5d8b` |
| `README.md` | `befe39a548d6f4d669d5cfff217ed997734319ae` | `0fb15c041f94ee9a83832dcb1001071882ca36b1cabb0d5cbaab687abdf8e3e3` |
| `include/nesturbator.h` | `bc084c20239931e62ff62f1df9b07bc61622cc6f` | `ab2a2c67ce76f1ab60d8678cd190e8b5b4e4c90085d0fdff818f7cf01f70ccea` |
