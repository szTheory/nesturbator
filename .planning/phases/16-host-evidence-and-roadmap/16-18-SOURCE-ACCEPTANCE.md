---
phase: "16"
plan: "18"
record: source-acceptance
status: in_progress
candidate_source_commit: 16f7804205afb49459b46b936509e41e8d5566c6
---

# Phase 16 Plan 18 Source Acceptance

This record covers only the current Plan 18 finite acceptance. It preserves Plan 17's terminal stop, partial work, and raw failure evidence. No result is claimed before its single planned gate runs and its source identity is checked again.

## Candidate and preflight

- Candidate source commit: `16f7804205afb49459b46b936509e41e8d5566c6`.
- Current full HEAD at preflight: `8b1b66bbed8f01f45cccb32558173ef17ed1c229`.
- Branch: `gsd/phase-16-host-evidence-and-roadmap`.
- The supplied repository-root pin passed; the working root is the pinned checkout.
- The substantive Plan 18 amendment is committed at `8b1b66bbed8f01f45cccb32558173ef17ed1c229`. Parent dispatch evidence records only Plan 18 ready, Plans 07/08 not ready, and fresh Plan 18 evaluation scope resolved with no matching commits. Plan 17 remains superseded and incomplete, without a SUMMARY or completion credit.
- The exact `.planning/config.json` SHA-256 is `0517388f5d422ae97afdc5dc1ae11bc43ebd481a757424f152ee8af61559ff5e`; it remains an unstaged user change and is excluded from commits.
- Installed runtime identity: `@opengsd/gsd-core` `1.16.0`, invoked through `node "$HOME/.codex/gsd-core/bin/gsd-tools.cjs"`.
- Host/tool identity: Darwin 25.6.0 arm64; CMake and CTest 4.4.3; Node v26.10.0; `/usr/bin/cc`; shell `/bin/sh`; Perl `/usr/bin/perl`.

### Corrected candidate source/doc blobs and byte digests

| Path | Git blob | SHA-256 |
| --- | --- | --- |
| `tests/retroarch/state_session.sh` | `435a0f5fd1c4444e3b45ca668bd5af0e1a87d7e0` | `56dabb21e19d4501bd920dc55413e069e3570f8fdd440deb08d5b84920063af0` |
| `tests/retroarch/interruption_trace.sh` | `2de1af0dce0fb7d1a512b0ca2398f83566070eaa` | `ab11f15c1e77e7d5a9655f37c7a76992222226cf3b550fb8ef805df2f388198a` |
| `tests/retroarch/test_state_session.sh` | `073f78a061f9bd5d2ca62cc84d8c26e7335ac6a2` | `5d075ae02fd4a266e5b0e028384fae52db6ccd191bbcfd12692ef7e47840c299` |
| `README.md` | `461b8e8efeab3c30a6b22bfa49ff9d97b4ff8bd0` | `62bad86cd7e1470a9280f7d1d7b92fad2e3ecd68154b74f1c4d526660cd1ac3d` |
| `include/nesturbator.h` | `a514ec862372f908db60298c4dab010fc22e0312` | `75b9f70dc091c32b59852f0f2c64a151dc194e72002be58a1eed0a0bc394d091` |
| `tests/retroarch/frame_index.c` (read-only) | `204d3ff64cc288176b0e6f47a1788c122a36815c` | `ecab2d5fbf294596572617759ae55c8494ec9bd95705b70bf21ea90767ea5d8b` |

### Registered input blobs and byte digests

| Path | Git blob | SHA-256 |
| --- | --- | --- |
| `tests/retroarch/CMakeLists.txt` | `443cbc4b35286f56c9a5c6b29c89e37c14aaa4bf` | `84bbaf464d812c1142e85851203f56e41f04511fee8bc16cf09bcc898400ec0e` |
| `tests/retroarch/run_retroarch.cmake` | `cfe2e4e7942bc6728a0a41b56d1cf9eee09f1e45` | `6a387dc0ac1c2be498f5375d597e4d84ae69763a471e73e80c6a5400972e9581` |
| `CMakePresets.json` | `10a19b58652adb634cc0357de5f5cc91cd848409` | `a97957fcd87052b863be85a26609c0c985402b8eac645748907cec6ea237a024` |
| `cmake/packaging.cmake` | `7afbd7cc89a153ff9397a1bdcc6a913dafd4af96` | `0797be3f2ba69a96076f342174ca4393274719d34d9207ce0acb45cf4a9261e4` |

### Existing cache and binary identities

Both caches point to this checkout. No selector was run during preflight.

| Artifact | SHA-256 | Observed identity |
| --- | --- | --- |
| `build/ci/CMakeCache.txt` | `af03303ad33e142dde959b50627ec4720d62635c88eeb032de79aacbf1a910d4` | Release, `/usr/bin/cc`, project root is this checkout |
| `build/ci/tests/retroarch/frame_index` | `10fba3ddd9bdbe01d02f16b7d2938263938533a49d8619aa960f4f44534270f9` | Existing CI frame-index binary |
| `build/asan/CMakeCache.txt` | `875a51bed2081f932dfa23ca0c07f1f3ed7effe8a934c557f8db2e53012cd304` | Debug, `/usr/bin/cc`, project root is this checkout |
| `build/asan/tests/retroarch/frame_index` | `1a137b9ac1f782285ec9ea1362f9cd747bebe0e71362cf3930c7e795ef42b917` | Existing ASan frame-index binary |

The parent reconciled and retained the nine partial Plan 17 commits: `b25fd8fc5abb38c492ed8db15471d702b7a9589f`, `e47917f839a951825aeab553ca10c5c4e33ea9d3`, `483bdceb69f8b33d8c9ff3add86f2dbf665be791`, `4d1d193389bf2da13f6756e265daeba8b90d8428`, `18ecd154ac5fbe31036b66f6cc14b072a960cf98`, `5f50bbc773bf891ef40c97ff12feac08003ba727`, `16f7804205afb49459b46b936509e41e8d5566c6`, `db0494378d773249ae7fb2cb9155fd7fa46700ab`, and `aac195c6bfd7f5f90e966213a109d4c686b68331`. Parent reconciliation `228ba4e2fee45910ff86704ad3a15d6872c1b36a` preserves the 60-second outer-guard mismatch. Actual helper RED/GREEN work is retained; the 60-second registered CI stop after nine passes did not establish the controls outcome or diagnose a session defect. Its raw evidence remains immutable under `build/phase16-review-corrections-execute/evidence/ci-selection/`.

## Finite acceptance ledger

One slot exists for each of OFFLINE, CI, ASAN, FULL, and REVIEW. A slot is consumed before launch. A started-but-unresolved, failed, or unusable slot is terminal and is never reset. The append-only raw ledger and gate outputs live under ignored `build/phase16-acceptance18-execute/`.

| Slot | State | Command/output tree | Expected result |
| --- | --- | --- | --- |
| OFFLINE | passed, exit 0 | exact offline selector / `offline/work` | ownership cases, 7 capture, 51 trace, 13 real-ledger, nested identity controls; 60-second process guard |
| CI | passed, exit 0 | exact registered CI selector / `ci/` | 10/10, including real controls; 1200-second process guard |
| ASAN | planned, not consumed | exact ASan selector / `asan/` | 2/2, no sanitizer diagnostics; 1200-second process guard |
| FULL | planned, not consumed | exact normal workflow / `full/` | 649/649, jobs 4, three actual ZIPs; 900-second process guard |
| REVIEW | planned, not consumed | fresh independent separate-context review | exact committed six-path review and a matching disposition, only after successful prior gates |

### OFFLINE launch record

- Local slot ID: `OFFLINE-01`.
- Launch state: consumed before launch; process/session ID pending tool launch.
- Candidate: `16f7804205afb49459b46b936509e41e8d5566c6`.
- Process deadline: 60 seconds.
- Expected raw output: stdout, stderr, exit status, TAP/case summaries, and all owned files under `build/phase16-acceptance18-execute/offline/`.
- Exact gate argv: `node "$HOME/.codex/gsd-core/bin/gsd-tools.cjs" run-with-timeout 60 -- cmake -E env STATE_SESSION_CONTROL=interruption-capture-selftest sh tests/retroarch/test_state_session.sh tests/retroarch/state_session.sh build/ci/tests/retroarch/frame_index build/phase16-acceptance18-execute/offline/work cmake tests/retroarch/run_retroarch.cmake`.
- Prelaunch source and registered-input table matches the candidate table above; working-tree diff from the candidate is empty. Current configuration hash remains unchanged.
- Prelaunch accounting timestamp: 2026-10-16T15:27:26Z; this is not the process start time. The wrapper will record the actual gate start/end separately.

### OFFLINE-01 result

- Tool session `53373`; wrapper PID `2367`.
- Actual start `2026-10-16T15:28:16Z`, end `2026-10-16T15:28:21Z`, elapsed 5 seconds; process status `0`.
- The actual-helper collision and owner-path replacement controls preserved foreign bytes/identity and refused before host or transport launch. Nested identity controls reported two positive sessions and four ownership negatives; the invalid-ID recorder failure remained separate from assertion status.
- All 7 capture cases passed. The actual 21-row ledger selector passed 13 cases: one valid positive and 12 intended negatives with `EVENT_FIELDS`, `EVENT_SEQUENCE`, `EVENT_IDENTITY`, `EVENT_VALUE`, and `EVENT_TIME` reasons. The trace selector passed all 51 cases (40 retained plus 11 lifecycle/QUIT), including positive absent-observation and numeric-PID/unavailable-liveness cases.
- Complete stdout SHA-256: `8beff25d0f75bcc521b7d577186f715816cbb279cd5f3964b24f706a09d31fbe`; empty stderr SHA-256: `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855`; exit-file SHA-256: `9a271f2a916b0b6ee6cecb2426f0b3206ef074578be55d9bc94f6f3fe3ab86aa`.
- The post-gate source/input blobs and file digests equal the candidate table. Candidate diff exit was `0`; config SHA-256 remains exact. `offline/source-before.txt` and `offline/source-after.txt` differ only because the prelaunch snapshot omitted the printed trailing diff exit value; the source/input rows, config digest, and candidate diff result are the same.
- Full offline stdout/stderr, status, timing, source snapshots and generated owned traces/ledgers remain under `build/phase16-acceptance18-execute/offline/`.

### CI launch record

- Local slot ID: `CI-01`; state: consumed before launch, process/session ID pending tool launch.
- Candidate: `16f7804205afb49459b46b936509e41e8d5566c6`.
- Process deadline: 1200 seconds. Expected completed tests: 10/10; registered per-test limits remain unchanged, including session controls 900 seconds, readiness 60 seconds, and marker qualification 600 seconds.
- Exact gate argv: `node "$HOME/.codex/gsd-core/bin/gsd-tools.cjs" run-with-timeout 1200 -- ctest --preset ci -R '^retroarch.(session.(readiness|controls)|marker[.]|progression[.])' --no-tests=error --output-on-failure`.
- Captured preexisting `build/ci/Testing/Temporary` logs under `build/phase16-acceptance18-execute/ci/preexisting-ctest/` before launch; the copied stale LastTest log is historical only. CTest's selected test artifacts are retained separately under `build/ci/tests/retroarch/` and the current CTest tree will be copied after the run.
- Source/input/config/cache/binary identities are recorded in `ci/source-before.txt` after the OFFLINE gate and before launch. The candidate diff must remain empty.
- Prelaunch accounting timestamp: 2026-10-16T15:29:35Z; this is not the process start time.

### CI-01 result

- Tool execution session `98372`; wrapper PID `7630`.
- Actual start `2026-10-16T15:29:54Z`, end `2026-10-16T15:33:52Z`, elapsed 238 seconds; process status `0`.
- The exact selector completed 10/10 with no selected tests skipped: marker generation/fixtures/controls/qualification, progression tracer/prefix/complete-required/controls, and session readiness/controls. CTest reported `100% tests passed out of 10`.
- Current CTest `LastTest.log` SHA-256: `ae3e29a59e34735b091ba7bc7fc4edd03ec2ab5838fed573f2b5cbaa0eb596d3`. It records the passing results, including the 21-row real-ledger positive, TERM lock release, concurrent `ENDPOINT_BUSY`, retained TERM with one QUIT and reaped host, and completion-control boundaries.
- CTest's `LastTestsFailed.log` and `LastTestsDisabled.log` were not refreshed by the selector: their timestamps predate this run and their hashes match the prelaunch copies. They are retained as historical reports only, not treated as this run's result. The authoritative current report and stdout both show all ten selected tests passed.
- Complete stdout SHA-256: `493300043509a30d9687cfde18a53262e4fb7d63dce9622ab7935f4e2f5ec818`; empty stderr SHA-256: `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855`; exit-file SHA-256: `9a271f2a916b0b6ee6cecb2426f0b3206ef074578be55d9bc94f6f3fe3ab86aa`.
- `ci/source-after.txt` SHA-256: `63250637271b79651f8b59baf03cf0719a2d951db9d454bf40fc2eb154fda26a`. Source and registered-input rows still match the candidate, config SHA-256 is unchanged, and candidate diff exit is `0`.
- Complete raw command output, process metadata, current CTest report, and selected test artifacts are retained under `build/phase16-acceptance18-execute/ci/`.

## Gate results

The OFFLINE and CI gates are complete as recorded above. ASAN, FULL, and REVIEW remain unconsumed and are not implied by historical Plan 16 or Plan 17 outputs.

## Remaining boundaries

Historical assertion and native timing causes remain UNKNOWN. A successful local acceptance and review would still be provisional until every mandatory parent post-wave/post-merge gate passes. Plan 07 retains native exact-head CI, affirmative exact-head owner consent, and the normal release flow. Plan 08 retains proof of the actual 18 published assets, checksums, attestations, six metadata files, and exact-release CI. No local evidence grants consent, publication, phase completion, or Plan 07 readiness.
