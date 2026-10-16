---
phase: "16"
plan: "19"
record: source-acceptance
status: incomplete
candidate_source_commit: 16f7804205afb49459b46b936509e41e8d5566c6
execution_head: 95d995c5ae7c43ee08d1535a83e373147fd1f20b
---

# Phase 16 Plan 19 Source Acceptance

This partial record preserves the single Task 1 preflight result. Plan 19 stopped before any new runtime selector or full-workflow launch. Retained Plan 18 results are not adopted because the current registered-CI report failed the amended machine-check.

## Scope and immutable identity

- Candidate: `16f7804205afb49459b46b936509e41e8d5566c6`.
- Execution head before this record: `95d995c5ae7c43ee08d1535a83e373147fd1f20b` (`docs(16): plan retained-evidence acceptance recovery`).
- Branch: `gsd/phase-16-host-evidence-and-roadmap`.
- Configuration SHA-256: `0517388f5d422ae97afdc5dc1ae11bc43ebd481a757424f152ee8af61559ff5e`; it remains an unstaged user change.
- The Task 1 inline command verified all ten candidate/input working-tree and HEAD blobs against the candidate and exact configuration hash before reaching the retained report check. The complete rows are preserved in `build/phase16-acceptance19-execute/source-before.json`.

| Path | Candidate Git blob | SHA-256 |
| --- | --- | --- |
| `tests/retroarch/state_session.sh` | `435a0f5fd1c4444e3b45ca668bd5af0e1a87d7e0` | `56dabb21e19d4501bd920dc55413e069e3570f8fdd440deb08d5b84920063af0` |
| `tests/retroarch/interruption_trace.sh` | `2de1af0dce0fb7d1a512b0ca2398f83566070eaa` | `ab11f15c1e77e7d5a9655f37c7a76992222226cf3b550fb8ef805df2f388198a` |
| `tests/retroarch/test_state_session.sh` | `073f78a061f9bd5d2ca62cc84d8c26e7335ac6a2` | `5d075ae02fd4a266e5b0e028384fae52db6ccd191bbcfd12692ef7e47840c299` |
| `README.md` | `461b8e8efeab3c30a6b22bfa49ff9d97b4ff8bd0` | `62bad86cd7e1470a9280f7d1d7b92fad2e3ecd68154b74f1c4d526660cd1ac3d` |
| `include/nesturbator.h` | `a514ec862372f908db60298c4dab010fc22e0312` | `75b9f70dc091c32b59852f0f2c64a151dc194e72002be58a1eed0a0bc394d091` |
| `tests/retroarch/frame_index.c` | `204d3ff64cc288176b0e6f47a1788c122a36815c` | `ecab2d5fbf294596572617759ae55c8494ec9bd95705b70bf21ea90767ea5d8b` |
| `tests/retroarch/CMakeLists.txt` | `443cbc4b35286f56c9a5c6b29c89e37c14aaa4bf` | `84bbaf464d812c1142e85851203f56e41f04511fee8bc16cf09bcc898400ec0e` |
| `tests/retroarch/run_retroarch.cmake` | `cfe2e4e7942bc6728a0a41b56d1cf9eee09f1e45` | `6a387dc0ac1c2be498f5375d597e4d84ae69763a471e73e80c6a5400972e9581` |
| `CMakePresets.json` | `10a19b58652adb634cc0357de5f5cc91cd848409` | `a97957fcd87052b863be85a26609c0c985402b8eac645748907cec6ea237a024` |
| `cmake/packaging.cmake` | `7afbd7cc89a153ff9397a1bdcc6a913dafd4af96` | `0797be3f2ba69a96076f342174ca4393274719d34d9207ce0acb45cf4a9261e4` |

## Terminal Task 1 preflight failure

The one inline Task 1 automated command exited `1` after `0.104` seconds at the retained CTest report-count assertion. Its exception record reports `status: FAIL`, `full_started: false`, and no full-run metadata. The exact CI `LastTest.log` digest matched the amended expected digest, but it contained only eight lines exactly matching `Test Passed.` where ten were required:

- Report: `build/phase16-acceptance18-execute/ci/ctest-current/Temporary/LastTest.log`.
- SHA-256: `ae3e29a59e34735b091ba7bc7fc4edd03ec2ab5838fed573f2b5cbaa0eb596d3`.
- Observed exact pass lines: `8`; required: `10`.
- The same report's copied Plan 18 acceptance record says CI completed 10/10, so the mismatch makes its current report unusable for Plan 19's conditional adoption. The retained CI pass is therefore **not adopted**.

The script reached this failure after checking the retained OFFLINE/CI/ASan stdout, stderr and exit-file digests, the three before/after source snapshots, the ten candidate/input identities and the configuration hash. Its ordered report loop stopped at CI, before its ASan report, cache/binary/environment, resolver identity, smoke, argv, package-baseline or launch checks. A separate read-only inspection after the stop found the retained ASan report has two exact pass lines; that does not override the failed Task 1 gate or authorize adoption.

No Plan 19 CLI identity assertion or `cmake --version` smoke ran. No FULL19 argv was persisted, no preexisting package or CTest report was moved, and no FULL19 process launched. The raw `full-slot-state.txt` remains exactly `planned`; this is a terminal preflight failure, not a consumed full-workflow result. `verification-result.json`, the unchanged source snapshot, and the original Plan 18 raw records preserve the failure. No CTest selector or full workflow was rerun.

## Gate accounting and remaining boundaries

| Gate | Plan 19 result |
| --- | --- |
| Retained OFFLINE18 / CI18 / ASan18 adoption | Not accepted; CI current-report count failed |
| Runtime resolver identity / bounded smoke | Not reached |
| FULL19 normal workflow and three fresh ZIPs | Not launched; planned slot remains unconsumed |
| REVIEW19 | Not launched |
| Success-only SUMMARY | Not created |

This failure is terminal under the Plan 19 preflight contract. Further acceptance requires a substantive planning amendment that resolves the inconsistency in the retained CI report evidence. No source, test, documentation, preset, workflow, configuration or historical artifact was changed. The prior Plan 17 terminal stop, Plan 18 launcher failure, all causal RED/GREEN evidence, and UNKNOWN historical/native timing causes remain unchanged. Parent post-wave/execute:post gates, native exact-head CI, owner consent, and public release evidence remain unperformed and unclaimed.

## Append-only parent clarification: report parser defect

Read-only inspection after the executor returned localizes the preflight failure to Plan 19's exact-line parser, not to a failed retained test or changed report. The preserved CI report has eight `Test Passed.` outcomes and two `Test Pass Reason:` outcomes, each followed by `Required regular expression found.`. The latter belong to `retroarch.marker.controls` (`marker controls: 36 cases`) and `retroarch.marker.qualify` (`marker qualification: 9997 frames`), matching their unchanged `PASS_REGULAR_EXPRESSION` registrations in `tests/retroarch/CMakeLists.txt`. All ten unique report test IDs have exactly one successful outcome; the retained stdout also has ten passed test rows and `100% tests passed out of 10`. The report's expected SHA-256 still matches. The earlier wording "report inconsistency" describes rejection by the approved parser; these observations establish a parser assumption defect rather than inconsistent underlying test results.

The read-only explanation and per-test rows are retained in `build/phase16-acceptance19-execute/report-format-inspection.json`. This inspection is not another execution of Task 1, formal adoption, a test selector, a launcher preflight or a full workflow. The original `verification-result.json` remains `FAIL`, `full_started: false`, elapsed `0.17079462495166808` seconds; the record above separately quotes the tool invocation's reported wall time. No original record is replaced. No REVIEW or SUMMARY exists and no acceptance credit is granted.

Recommended next scope is a substantive amendment of both retained-report and fresh full-workflow report verification. Bind exactly one accepted result to every expected unique test ID/name, accept the documented registered regex-success form only with its exact expected reason, and continue rejecting failures, skips, missing/duplicate/unknown tests, absent end records and identity drift. Validate the amended parser against these immutable real reports and deterministic in-memory negative controls before spending any launcher or workflow slot. Preserve the genuine Plan 18 passes, Plan 19's terminal preflight, source/config identities, finite launcher/FULL/review scope and every later mandatory parent/native/consent/release gate. Do not relax a test assertion or infer historical causation. No such amendment is authored or executed in this execute-phase step.
