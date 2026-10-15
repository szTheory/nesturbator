---
phase: 16-host-evidence-and-roadmap
plan: "10"
status: complete
final_source_commit: 1b1589a49f735195b9dbf89a238dc0518777335c
reviewed_source_commit: 1b1589a49f735195b9dbf89a238dc0518777335c
---

# Plan 16-10: Native Timing Repair Evidence

The bounded repair removes one overwritten 150-frame pre-load replay from each later-frame prefix verdict, retains all three independent native-prefix judgments, makes failing timing reports visible, and protects that reporting contract in the registered readiness test. Local checks and fresh independent source review pass; the two native timing failures remain unproven until a fresh exact-head hosted run.

## Native failure identities

The retained native REDs are separate cases from GitHub Actions run [38764197994](https://github.com/szTheory/nesturbator/actions/runs/38764197994), PR #50:

| Job | Case | Measured duration | Bound | Result |
| --- | --- | ---: | ---: | --- |
| Intel `116523895865` | `readiness-response-stall` | 25,946 ms | `<25,000 ms` | Failed, job 648/649 |
| ASan `116523896064` | `readiness-delayed` | 26,269 ms | `<25,000 ms` | Failed, job 643/644 |

Run mapping: PR head `3de23d4aed5d9696dc4020db3b8ef5e8205300fb`, base `9f9386c661f6fb8f6063bf756655713aedadae77`, preview merge `81c0a2132250892b324ec4c9be2a26fe8ea91056`. The older run `38740783575` is a distinct historical stall failure; its old JSON incorrectly labels the case as delay. Run `38764179312` was automatically superseded/canceled after a body update and was not a manual rerun. Native subintervals, platform details, and causes remain unknown.

Retained native inputs under ignored `build/phase16-release/`:

| File | SHA-256 |
| --- | --- |
| `failure-summary-38764197994.json` | `fd497d30e955665e51c1ce9ab12b41d01220d9bcd90ac26de7ed2063b64c2a74` |
| `macos-intel-repaired-failed.log` | `7da0d7e8ac69e7abdec4c6b17012a728f9e3f3bd23518e57c7c1aa3bf07faa6f` |
| `asan-job.log` | `82b27f0a5c9f76a8d79b472e82b91c640eea6c23e38afaaabedea7b8d226862e` |

The recorded evidence distinguishes both native failures and does not infer the cause from local timings.

## Bounded attribution and repair

The single baseline pair ran on Darwin 25.6.0 arm64 with Apple clang 21.0.1, CMake/CTest 4.4.3, Ninja, and preset `jobs=4`. `retroarch.session.readiness` passed 2/2 in 19.77 seconds for CI and 2/2 in 24.60 seconds for ASan. The phase measurements and raw outputs are in ignored `build/phase16-native-timing/comparison-before-repair.md`.

Each readiness attempt launches three independent prefix verdicts for one, two, then three boundaries. The baseline CI processes took 164/330/492 ms for delayed and 184/324/503 ms for stall; ASan took 357/649/962 ms and 377/649/968 ms. These nested subprocess times total about 0.99–1.01 seconds in CI and 1.97–1.99 seconds in ASan; they must not be added to the enclosing later-prefix or attempt wall intervals.

For an image at the native anchor, `frame_index` must render from reset because the framebuffer is not serialized. For a later target, that initial rendering is replaced by loading the validated native anchor before the continuation, so its earlier frames are discarded. The regression first observed 450 pre-load frames across targets 150, 150, and 151. An unconditional skip failed the native image-identity control because it removed the necessary anchor-frame renders. The final change, committed in `c1db85254d09f0e4e52fc0edc44ddd54e8daf333`, skips only the target-151 pre-load replay and requires exactly 300 retained pre-load frames. It preserves per-boundary artifact reading, hashing, native container/revision/origin/continuation validation, state loading, later emulation, image decoding, palette extraction, and all three independent prefix verdicts.

The native overrun margins were 946 ms and 1,269 ms. The measured repeated work is material but does not establish that it caused either hosted miss. In one candidate ASan trace, the third prefix process changed from 962 to 775 ms for delayed and from 968 to 741 ms for stall; the aggregate prefix totals were +24 and -97 ms, respectively. Candidate enclosing phase measurements varied and several were slower than baseline. Thus the candidate confirms the targeted process work disappeared but does not demonstrate an overall readiness improvement or account for the entire native overrun.

The timing reporter now prints available fixture/setup/readiness/later-prefix/attempt/full-duration fields and sanitized status/reason before the unchanged strict duration assertion. Its synthetic over-bound and early-error regression uses fixed data, not a long sleep. A fresh independent review initially found that this regression was opt-in and absent from registered CTest. Commit `1b1589a49f735195b9dbf89a238dc0518777335c` wires the checks into `STATE_SESSION_CONTROL=readiness`, then continues through the actual readiness prefixes; standalone `STATE_SESSION_REPORT_SELFTEST=1` still exits after its selftest. Existing CTest names, registration, timeouts, operation bounds, driver budgets, fake delays, and production behavior are unchanged.

Both task-level TDD RED records were independently accepted by `gsd_run check tdd-red-evidence` as `RED_EVIDENCE_OK` (exit 8 on the intended assertion). The reporter RED recorded that the synthetic 25,946 ms case emitted no phase report; its evidence is `build/ci/16-10-red-evidence.json` (SHA-256 `e443cbe80571dc4b5e6ca28118e3fa15cf0e7e73d8bb7488bc7bb84a0a96e7e7`) and raw JUnit is `build/ci/16-10-red-report.junit.xml` (`bc157b206a3f86ddb2207e598fe2a77d51b3fa11c4b99fe6ec79035eeedfceb7`). The frame-index RED ran a real three-boundary prefix to `PREFIX_OK` and measured 450 pre-load frames instead of the expected zero for the later target; its record is `build/ci/16-10-frame-index-red-evidence.json` (`d8fe6ef9b8d93581832ac07b565d9314de8a64e72bdef13b891342db1ee03256`) and raw JUnit is `build/phase16-native-timing/repair-red-progression-controls.junit.xml` (`e4a8d1907e84f2342628d653e807ca728688ed9ce97b4b61b9fbbd0100a32e50`). After the corrected frame-index candidate, `retroarch.progression.controls` passed with the two required anchor renders and the redundant later replay removed; raw JUnit is `repair-green-progression-controls.junit.xml` (`5eb9accc481c1e87adfe0a12ef69b16e6e3f736ddeccb323750b11e8ec526411`). The final focused readiness rerun and fresh review independently confirm the reporter selftest passes before both real prefixes.

## Verification and raw outputs

The final source commit reviewed independently is `1b1589a49f735195b9dbf89a238dc0518777335c`. All commands below exited zero:

| Command | Result | Retained output |
| --- | --- | --- |
| `ctest --preset ci -R '^retroarch.session.readiness$' --no-tests=error --verbose --output-on-failure` | 1/1 passed, 19.14 s. Reporter selftest ran before delayed and stalled prefixes; driver+cleanup was 7,457/10,667 ms. | Output appeared in the execution transcript; the post-review combined raw log below includes the readiness case. |
| `ctest --preset ci -R '^retroarch.(session.(readiness|controls)|marker[.]|progression[.])' --no-tests=error --output-on-failure` | 10/10 passed, 245.25 s. | `build/phase16-native-timing/review-fix-ci-controls.log`, SHA-256 `c90ab2254a5f1917dad3f1e877b52e503091c11c37174337f45dcc6564aedf9e` |
| `ctest --preset asan -R '^retroarch.session.(readiness|controls)$' --no-tests=error --verbose --output-on-failure` | 2/2 passed, 304.88 s; no sanitizer diagnostic. Readiness selftest preceded both real prefixes; full controls ended `VERDICT_OK`. | `build/phase16-native-timing/review-fix-asan-sessions.log`, SHA-256 `40cf25ba4c377788b85e477ea7fd04d5ceba4ab27219398e50a27bf388be6dd6` |
| `cmake --workflow --preset ci` | Configure and build passed; 649/649 tests passed in 263.37 s; CPack generated all three ZIPs. | `build/phase16-native-timing/review-fix-ci-workflow.log`, SHA-256 `a5709b1df1d551defc7884e9f7676e4cd13a650a8c38c21bbfd525dea78da35d` |

Final workflow packages:

| Package | SHA-256 |
| --- | --- |
| `nesturbator-0.1.10-library-macos-arm64.zip` | `5c1a1250575ce179f5f60ee651acb8adf851706003573b4589eb920e1f2ea5d0` |
| `nesturbator-0.1.10-libretro-macos-arm64.zip` | `80af4083d7db24ee77c0c7800fc5d1e1b32b5dd508a55fb0ca2e14aebfb87249` |
| `nesturbator-0.1.10-runner-macos-arm64.zip` | `ac7da4bc265dd72bd6b56358381124a81a6cf704f47925e0ffe3b0423f7af613` |

The fresh independent review is [16-NATIVE-REVIEW.md](16-NATIVE-REVIEW.md), bound to the final source commit above. It reviewed exactly `tests/retroarch/state_session.sh`, `tests/retroarch/test_state_session.sh`, `tests/retroarch/frame_index.c`, `README.md`, and `include/nesturbator.h`; it records `status: clean`, zero findings, and the ASan/CI/workflow results. [16-NATIVE-REVIEW-DISPOSITION.md](16-NATIVE-REVIEW-DISPOSITION.md) records WR-01 as resolved by the readiness selftest wiring and `open: 0`.

## Plan 07 and 08 handoff

Plan 07 resumes at Task 1. Reconcile preparation `30a009838b741a429863e7cb5143526fbaad8ed7`, completed repair/preparation through pushed PR head `3de23d4aed5d9696dc4020db3b8ef5e8205300fb`, local failure handoff `9b37c9a`, and the later planning, Plan 10 source, test, review, and evidence commits. Reuse open PR #50. Capture all final intended commits before pushing. Require a fresh native pull-request workflow on that exact head, including required, performance, and pinned-host checks; stop on any terminal failure. Local results do not waive the later exact-head owner-consent gate. No merge consent was requested or granted here.

Plan 08 remains after Plan 07 and must verify actual normal publication: all 18 archive checksums and attestations, all six platform metadata files, and CI for the exact release commit. Plan 10 does not change EVID-04, publication state, or the historical 21/22 verification. Keep EVID-04 and Phase 16 incomplete until actual release publication and independent verification close them.
