---
phase: "16"
plan: "20"
record: source-acceptance
status: incomplete
task1_status: passed
review_status: pending
candidate_source_commit: 16f7804205afb49459b46b936509e41e8d5566c6
execution_head_before_task1: c0f82341033bdf5151ef44da1f150d611b7f6c35
---

# Phase 16 Plan 20 Source Acceptance

Task 1 passed on the immutable candidate source. This record distinguishes the retained Plan 18 passes adopted by the amended parser from the one new Plan 20 full workflow. Plan 20 remains incomplete until its fresh independent review and final handoff tasks pass.

## Scope and immutable identity

- Candidate source commit: `16f7804205afb49459b46b936509e41e8d5566c6`.
- Execution HEAD before Task 1: `c0f82341033bdf5151ef44da1f150d611b7f6c35` on `gsd/phase-16-host-evidence-and-roadmap`.
- The supplied repository-root pin passed before raw-slot initialization, this record write and the Task 1 commit.
- Parent dispatch evidence established only Plan 20 as DAG-ready, Plans 07/08 as blocked, and an empty fresh Plan 20 evaluation scope. The substantive amendment and parent state setup preceded execution.
- `.planning/config.json` remains an unstaged user change with SHA-256 `0517388f5d422ae97afdc5dc1ae11bc43ebd481a757424f152ee8af61559ff5e`; it is excluded from the Plan 20 commits.
- The Task 1 raw evidence is retained under ignored `build/phase16-acceptance20-execute/`. The slot began as `planned` and was changed to `consumed` immediately before FULL20. The root contains the original streams, statuses, timestamps, parser validations, source snapshots, current CTest report, package copies and archive-check output.

### Candidate source and registered-input identities

The Task 1 assertion bound each working-tree file and committed `HEAD` blob to the candidate commit, then repeated the checks after FULL20. The `source-before.json` and `source-after.json` maps in the raw root are identical.

| Path | Candidate Git blob | SHA-256 |
| --- | --- | --- |
| `tests/retroarch/state_session.sh` | `435a0f5fd1c4444e3b45ca668bd5af0e1a87d7e0` | `56dabb21e19d4501bd920dc55413e069e3570f8fdd440deb08d5b84920063af0` |
| `tests/retroarch/interruption_trace.sh` | `2de1af0dce0fb7d1a512b0ca2398f83566070eaa` | `ab11f15c1e77e7d5a9655f37c7a76992222226cf3b550fb8ef805df2f388198a` |
| `tests/retroarch/test_state_session.sh` | `073f78a061f9bd5d2ca62cc84d8c26e7335ac6a2` | `5d075ae02fd4a266e5b0e028384fae52db6ccd191bbcfd12692ef7e47840c299` |
| `README.md` | `461b8e8efeab3c30a6b22bfa49ff9d97b4ff8bd0` | `62bad86cd7e1470a9280f7d1d7b92fad2e3ecd68154b74f1c4d526660cd1ac3d` |
| `include/nesturbator.h` | `a514ec862372f908db60298c4dab010fc22e0312` | `75b9f70dc091c32b59852f0f2c64a151dc194e72002be58a1eed0a0bc394d091` |
| `tests/retroarch/frame_index.c` (read-only review path) | `204d3ff64cc288176b0e6f47a1788c122a36815c` | `ecab2d5fbf294596572617759ae55c8494ec9bd95705b70bf21ea90767ea5d8b` |
| `tests/retroarch/CMakeLists.txt` | `443cbc4b35286f56c9a5c6b29c89e37c14aaa4bf` | `84bbaf464d812c1142e85851203f56e41f04511fee8bc16cf09bcc898400ec0e` |
| `tests/retroarch/run_retroarch.cmake` | `cfe2e4e7942bc6728a0a41b56d1cf9eee09f1e45` | `6a387dc0ac1c2be498f5375d597e4d84ae69763a471e73e80c6a5400972e9581` |
| `CMakePresets.json` | `10a19b58652adb634cc0357de5f5cc91cd848409` | `a97957fcd87052b863be85a26609c0c985402b8eac645748907cec6ea237a024` |
| `cmake/packaging.cmake` | `7afbd7cc89a153ff9397a1bdcc6a913dafd4af96` | `0797be3f2ba69a96076f342174ca4393274719d34d9207ce0acb45cf4a9261e4` |

## Retained Plan 18 evidence adopted

Task 1 independently checked the immutable Plan 18 evidence before accepting it; no OFFLINE, CI or ASAN selector was run for Plan 20.

| Gate | Adopted result | Retained evidence |
| --- | --- | --- |
| OFFLINE18 | Passed, exit 0; required 7 capture, 51 trace and 13 real-ledger controls, nested identity and ownership controls | `build/phase16-acceptance18-execute/offline/`; stdout SHA-256 `8beff25d0f75bcc521b7d577186f715816cbb279cd5f3964b24f706a09d31fbe` |
| CI18 | Passed 10/10, no selected skips; marker, progression and session controls | Current selected report SHA-256 `ae3e29a59e34735b091ba7bc7fc4edd03ec2ab5838fed573f2b5cbaa0eb596d3`; stdout SHA-256 `493300043509a30d9687cfde18a53262e4fb7d63dce9622ab7935f4e2f5ec818` |
| ASAN18 | Passed 2/2 with no sanitizer diagnostics | Current selected report SHA-256 `bdac9ab97fba20d24cee020e7599e213c2050ed638ae5273ebff32bcc2744c80`; stdout SHA-256 `77e0e31b79b0515a675c8db278b11d5aca714ed71be14511a8244bc8f2890a69` |

The amended report parser queried CI/ASAN registration metadata with bounded CTest show-only JSON calls. On retained CI18 it accepted eight ordinary outcomes and two exact registered regular-expression outcomes; ASAN18 had two ordinary outcomes. Historical actual FULL18 was used only as a format control: 649 results, 611 ordinary, 38 registered-regex and two registered `WILL_FAIL` results. Its report/stdout digests matched the amendment. The parser's four positive controls passed, and all 20 deterministic malformed, missing, duplicate, unknown, skipped, failed, truncated-trailer and contradictory-stdout controls were rejected. This corrected the prior Plan 19 parser assumption; the Plan 19 failure remains preserved as a failed parser gate, not a failed underlying retained test run.

Retained18 source-before/after identities, status files, report digests, cache/binary identities and exact configuration hash passed the Task 1 assertions. Existing stale `LastTestsFailed` / failed-list copies were not treated as current results. The full immutable evidence remains under `build/phase16-acceptance18-execute/`; Plan 18's original terminal FULL command-path failure and Plan 19's terminal preflight remain unchanged in their records and raw roots.

## Plan 20 runtime and full workflow

The installed resolver-selected CLI was bound once. Its runtime identity output was `@opengsd/gsd-core` version `1.16.0`; the selected CLI byte SHA-256 is `4a2774598f6a1de9b63bbcbe71b4dff795b036fd061a80a3f2ab37a35bd63c3a`. Node was `v26.10.0`; CMake and CTest were `4.4.3`; the asserted host identity was Darwin `25.6.0` arm64 with `/usr/bin/cc`. The bounded 15-second `cmake --version` smoke passed in 0.068 seconds.

The one normal FULL20 argv was `node <resolver-selected-gsd-tools> run-with-timeout 900 -- cmake --workflow --preset ci`. The slot was consumed before launch. Process PID `50126` exited `0`, starting `2026-10-16T19:11:00.223590Z` and ending `2026-10-16T19:15:23.462295Z` after 263.240 seconds. The four expected configure/build/test/package workflow stages ran with CI jobs set to 4. The complete captured stdout SHA-256 is `5574327d2546d93e312231c4e06e6602114dea238015202bbba6a4bb5609e5ea`; stderr is empty with SHA-256 `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855`.

The fresh current report has 649 unique completed tests: 611 ordinary, 38 registered-regex and two registered `WILL_FAIL` outcomes. Every result matched its registration metadata; there were no skips or failures, and the post-configure registration metadata matched the preflight snapshot. `LastTest.log` SHA-256 is `1896a11aed6c8c2ec46f4a2ec0b02f39d3214c3c8a166f397f0967394225482d`. The machine verification is retained in `full-report-validation.json` and `verification-result.json`.

All three preexisting package paths were moved into the raw `preexisting/` baseline before FULL20, and all three outputs were newly created during this workflow. The normal package stage named them, ZIP integrity and complete inventories were checked, and the existing archive checker returned the exact three `ok` records with exit 0. The runner and libretro ZIP bytes equal their preexisting bytes, but the prelaunch move, fresh post-launch timestamps and package output establish new outputs; byte inequality was not used as a freshness criterion.

| Fresh package | Size | SHA-256 |
| --- | ---: | --- |
| `nesturbator-0.1.10-library-macos-arm64.zip` | 100324 | `88a8768c127c5475f5812ba885e3d42740b37da49a7ee7ef297117c9b94ffbd7` |
| `nesturbator-0.1.10-runner-macos-arm64.zip` | 92828 | `de5584b735a4cfc27418f350d281f235ed217609522865328300238bbcca4460` |
| `nesturbator-0.1.10-libretro-macos-arm64.zip` | 81628 | `b1bed4ad2ba10d7beea2d735a76d1b80e5f767defcb66ffc640361f222ec6dc4` |

The full invocation, current report, registration metadata, parser controls, source snapshots, package inventories and checker logs are retained under `build/phase16-acceptance20-execute/`. This local package result is not publication or release evidence.

## Execution note and remaining gates

One auxiliary command-extraction wrapper stopped on its own trailing-newline assertion before launching the Task 1 command; it ran no plan gate, test selector, parser, runtime identity check or application. The corrected extractor then launched the exact plan command once. The Task 1 automated command completed once, exited 0 and recorded `status: PASS`; no workflow retry or separate test selector followed.

Plan 17 remains partial; Plans 18 and 19 remain incomplete, with no replacement SUMMARY files. Plan 20's fresh independent exact-committed six-path review and final handoff are still pending. Parent post-wave and execute:post gates, the separate configured full-workflow repeat, native exact-final-head required/performance/pinned-host CI, affirmative exact-head owner consent, normal release publication, and Plan 08 actual release-byte/checksum/attestation/metadata/CI evidence remain distinct unperformed obligations. Historical assertion/native timing causes remain UNKNOWN. Nothing here claims Plan 07 readiness, phase completion, consent or publication.
