# Phase 25 Candidate Acceptance Record

**Status:** Plan 25-05 Task 3 passed on the final code candidate `fc48e4575264422df9a739795bc5286b840b8dc9` (tree `e328e142f810f5029d59508280d19100a0fee4a8`). The exact local CI workflow passed all 712 unfiltered tests and packaged all three artifacts. Phase goal verification passed 23/23 must-haves with no gaps or UAT; see [25-VERIFICATION.md](25-VERIFICATION.md). The four descriptor-less fallback rows and both prohibitions remain flagged; PAL reset-release duration remains unverified.

## Limited execution authority

The owner reaffirmed standing authority for local CMake configure/build/test/workflow commands, including required full CI and its configured AccuracyCoin campaign. This satisfies plan-level requests for per-run approval for local CI. Bound each full workflow to a 30-minute outer wall-clock cap while preserving stricter CTest, fixture and campaign caps. Diagnose failures before rerunning; retries within this local scope and cap do not need another approval. The first full workflow on candidate `9fcfbda` failed and its six focused repairs passed; a later output-sink wrapper failure was diagnosed and corrected. The final workflow passed on the candidate below. No hosted workflows, pushes, PRs, merges, releases or Phase 26/27 are authorized. The older interrupted run's result remains unknown and is historical; it does not override the owner's standing local-CI authority for this bounded plan task.

## Candidate and build identity

| Field | Value |
|---|---|
| Final code candidate | `fc48e4575264422df9a739795bc5286b840b8dc9` (`e328e142f810f5029d59508280d19100a0fee4a8`) |
| Candidate branch | `gsd/phase-25-pal-region-support` |
| CI preset / directory | `ci` / `build/ci` |
| Build configuration | CMake 4.4.3, Ninja, Release, `/usr/bin/cc`, `NESTURBATOR_ACCURACYCOIN_FULL=ON` |
| Configure/build outcome | Passed in final full workflow |
| Source state at verification | The committed candidate source/test/build inputs were used; unrelated pre-existing planning edits and `docs/coordination/` remained outside the commit and build inputs |

The initial failed run used tree `4f030376454d39d36789dbdb747425aeba1ebb2a`, commit `9fcfbda`. The focused test-contract repairs were committed in `0481ffd` and a bounded workflow on that corrected candidate passed; subsequent source/document fixes and the build-input fingerprint update produced the final code candidate below. CI used `build/ci`, CMake 4.4.3, Ninja, Release, `/usr/bin/cc`, and `NESTURBATOR_ACCURACYCOIN_FULL=ON`.

The corrected test-contract candidate was commit `0481ffd6da00afc4209ab73afc11122fbe7915c8`, tree `b8e46bae53a694af2f4cdfec333381254e9d939d`. The final code candidate after API-revision documentation and reset-limit clarification is commit `fc48e4575264422df9a739795bc5286b840b8dc9`, tree `e328e142f810f5029d59508280d19100a0fee4a8`. Later commits contain review/plan bookkeeping only and do not alter source, tests, CMake inputs or package inputs. The final receipt below is bound to `fc48e45`.

## First full local CI result (historical failure)

Command:

```sh
cmake --workflow --preset ci
```

The workflow ran configure, build and test in that order. Configure and build passed. The `ci` test preset had no test filter and used four workers. The workflow's configure step refreshed CTest registration to 712, and the test step ran all 712. The package step did not run because CTest failed.

CTest result: **706/712 passed; 6 failed; exit code 8; 544.14 seconds test time** (9 minutes, 4.14 seconds). The workflow ran Nov 1, 2026, local time; CTest logged start at 19:20 EST and end at 19:29:54 EST. The full AccuracyCoin campaign and protected-observation fixture both passed within this run. Campaign caps were BOOT=32, navigation=5, execution=1800, page timeout=15 seconds, collection timeout=10 seconds and CTest timeout=370 seconds; the existing 22-page/41,503-frame bounds were retained. The protected-observation setup retained its 760-second CTest cap.

| Failed test | Evidence and attributable correction | Focused result after correction |
|---|---|---|
| `runner.write_hashes.content` | Only 12 native-state hashes differed: tick 0 and tick 60 for six boards. State format 6 and all non-state frame/audio hash rows were unchanged. The state header intentionally records behavior revision 16; the test fixture still held revision-14 bytes. Updated those 12 expected state hashes from the workflow-generated output. | Passed as part of the six-test selection; `runner.write_hashes` fixture setup also passed. |
| `phase21.hosted_evidence.controls` | The control reads current behavior revision 16 from the public header but still asserted 14. Updated the current-source expectation to 16. | Passed as part of the six-test selection. |
| `accuracycoin.input_fingerprint` | The fingerprint contract covers 56 emulator, runner, public-header, build and campaign inputs. Metadata held `f04cdd6e…` from the prior source; current inputs hash to `c10fe356…`, still 56 inputs. Updated only that fingerprint field. | Passed as part of the six-test selection. |
| `accuracycoin.activation` | It rejected the same stale input fingerprint. The run's report SHA256 remained exactly the pinned `f3b5bbf7…`; the full campaign passed, reported 126/146 pass, 20 fail, 0 skip, 0 not-run, 0 timeout, and its scoreboard hash matched the canonical scoreboard. No score, report digest, source revision or behavior-revision assertion was changed. | Directly ran `accuracycoin_activation.cmake` against the retained report; passed. This check reads the report and does not start the campaign fixture. |
| `core.synth` | The test set `nes->ticks` for 50 NTSC cycles but left the shared `cpu_cycle` cursor at zero. Phase 25-03's observer correctly reports that cursor. Seeded the fixture's `cpu_cycle=50`. | Passed as part of the six-test selection. |
| `libretro.serialize` | The test expected a malformed replacement to unload the previously loaded game. The adapter returns false before replacing state, matching the public rejected-load contract. Updated the test to require the prior state size and serialized bytes to remain unchanged after rejection. | Passed as part of the six-test selection. |

After these repairs, `cmake --build --preset ci --target core.synth libretro.host` passed. The bounded CTest selection passed **6/6** in 36.85 seconds; the direct AccuracyCoin activation check passed against the existing report. No AccuracyCoin campaign or campaign fixture was rerun for that focused retest. This historical failure was later superseded by the final green workflow receipt below, not erased.

Generated logs and campaign evidence remain under the existing ignored `build/ci` directory. No cleanup was needed. No hosted workflow, live RetroArch session, push, merge or release was run.

## Final authorized full local CI receipt

The exact command `cmake --workflow --preset ci` completed configure → build → full unfiltered CTest → package on final code candidate `fc48e4575264422df9a739795bc5286b840b8dc9` (tree `e328e142f810f5029d59508280d19100a0fee4a8`). It exited 0 in 524.72 seconds, well inside the 1,800-second outer limit; no timeout occurred. CTest ran all **712/712** registered tests with four workers in 522.75 seconds. Campaign and protected-observation fixtures passed with their configured 370- and 760-second CTest limits. Campaign limits remained BOOT 32s, navigation 5s, execution 1,800s, page 15s and collection 10s; there were no filters or exclusions. The workflow package step succeeded.

| Receipt item | Evidence |
|---|---|
| Workflow log | `build/ci/25-05-task3-final-fingerprint-fixed-workflow.log`; SHA256 `27ee7d746c83140d1f43be217f3fa27a97a4c0d22195a6f25b76e9b3326db0d8` |
| Outer wrapper | `build/ci/25-05-task3-final-fingerprint-fixed-workflow.wrapper.txt`; start `2026-11-02T01:28:34Z`, end `2026-11-02T01:37:19Z`, workflow PID/PGID `6471`, exit 0, `outer_timeout=False`, process group exited |
| AccuracyCoin report | SHA256 `f3b5bbf7ddcf4d18deabda6d875b441330dc71c97b8aca5fdcc38bf471f98f6c`; 22 pages, 146 scored, 126 pass, 20 fail, 0 skipped/not-run/timed out |
| Canonical scoreboard | `tests/accuracy/scoreboard.txt`; SHA256 `9ec5d50d291d2713f5281851f642270498a4e8b0dab273b8b72cd19b66ff111b` |
| 56-input fingerprint | `cf07b0022d450a5bd7e58342aca6b16ed593acd022b65826b24f01061fed1089` |
| Library package | `build/ci/packages/nesturbator-0.1.11-library-macos-arm64.zip`; SHA256 `9e3f4e7cffac1acc6e9304951b0c3cd6d21f5c9fec7ad432c814116f493de165` |
| Libretro package | `build/ci/packages/nesturbator-0.1.11-libretro-macos-arm64.zip`; SHA256 `2a656331e587665f216279fc6a32a632b77da028ded09370a1e1a97072cfb3ff` |
| Runner package | `build/ci/packages/nesturbator-0.1.11-runner-macos-arm64.zip`; SHA256 `9d96ce56d473fa1187faa68cfad405641fd60b3d7590c37f3952552f430ab8d1` |

The fingerprint-only correction in `fc48e45` was made after verifying the retained AccuracyCoin report and canonical scoreboard hashes; `accuracycoin.input_fingerprint` and direct activation against that report both passed before the final workflow. The activation check did not set up or rerun the campaign fixture. An earlier wrapper attempt failed while relaying output through a pipe; its evidence was retained, the wrapper process group exited, and the final run redirected child output directly to a new ignored file with status-only polling. The historical RetroArch session-control corpus was preserved. No efficiency or package-size improvement is claimed. Unrelated planning working-tree edits were present during the campaign metadata source-dirty check and remain unstaged.

## Focused verification

The regional mapper and timing selection was:

```sh
cmake --preset dev
cmake --build --preset dev --target core.mapper core.mapper_discrete core.mapper_mmc1 core.mapper_mmc3 core.cartridge core.frame core.apu runner.spectral libretro.host
ctest --test-dir build/dev -R '^(core\.(mapper(_discrete|_mmc1|_mmc3)?|cartridge|frame|apu)|runner\.spectral|libretro\.host)$' --output-on-failure --no-tests=error
```

Result: **10/10 tests passed**, including the `runner.dump` fixture prerequisite selected by CTest. This covers the six-mapper matrix, existing discrete/MMC1/MMC3 tests, cartridge and frame boundary behavior, APU timing, the independent public-tone spectral suite, and the real libretro module harness.

Metadata and documentation checks were:

```sh
ctest --test-dir build/dev -R '^(libretro\.(vendored|info_claims(\.self_test)?|info_support|no_quirks)|docs\.links)$' --output-on-failure --no-tests=error
```

Result: **6/6 tests passed**. Clean option claims pass, false/missing/duplicate metadata mutation controls remain sensitive, the vendored header is unchanged, and documentation links pass. Plan 04's real module lifecycle test had already demonstrated option registration, query, reload latching and effective-region reporting before `core_options=true` was advertised.

The full workflow was subsequently run and passed as recorded above. No hosted workflow, live RetroArch session, push, merge or release was run.

## Six-mapper regional fixture and independent expectations

The owned fixture in `tests/core/test_mapper.c` builds NROM, MMC1, UxROM, CNROM, MMC3 and AxROM images in memory and runs them through the public region-load and frame APIs. Each mapper has an actual board-register behavior in its guest program. PAL NES 2.0 Auto and legacy iNES with a host PAL preference are compared as a profile control; a distinct explicit NTSC NES 2.0 instance runs alongside both PAL instances.

| Mapper | Board behavior exercised by the guest | Rendering setup |
|---:|---|---|
| 0 | NROM fixed PRG mapping | Off |
| 1 | MMC1 consecutive serial writes, fixed-last-bank PRG and mirroring | Off |
| 2 | UxROM PRG bank selection; legacy bus-conflict ROM byte is explicitly high | Off |
| 3 | CNROM CHR bank selection with distinct owned CHR-bank data | Off |
| 4 | MMC3 IRQ latch/enable and A12 clocks | On |
| 7 | AxROM single-screen upper-page selection | Off |

The arithmetic is independent of the implementation helpers:

- PAL: `312 × 341 × 10 = 1,063,920` half-master ticks per frame; `53,203,425` ticks/second; `32` ticks/CPU cycle; `640/709379` samples/tick. One nominal frame yields 959 samples plus a carried fraction. Eight frames yield 7,678 samples, compared with 7,680 for a rounded 960-sample-per-frame model.
- NTSC control: `714,732` ticks/frame; `24` ticks/CPU cycle; `352/315000` samples/tick. Eight frames yield 6,389 samples.
- The guest pulse timer is 100. Its source-derived edge period is `16 × (100 + 1) = 1,616` CPU cycles. The expected 48 kHz periods are 11,944 Q8 sample units for PAL (about 46.66 samples) and 11,095 Q8 units for NTSC (about 43.34 samples). The test checks transition-cycle spacing and measures public PCM autocorrelation against each rational expectation; the PAL and NTSC intervals must differ.
- Frame ticks use the owned three-cycle loop's independently calculated boundary rounding: PAL carries instruction overshoot against absolute nominal endpoints, while the existing NTSC behavior rounds each nominal frame. The fixture also checks fixed 256×240 geometry, profile-specific frame-rate fractions, per-instance PAL/NTSC clock units, and the PAL dot/scanline position from the 312×341 raster.

Adapter equality establishes forwarding and reporting parity only. The owned fixtures are source-derived behavioral evidence, not direct-console or universal-accuracy evidence. Plan 25-03's raster-row, APU edge and PAL signal suites supply the narrower subsystem acceptance; their source pins and the PAL `$4017` GET/PUT assumption remain documented in [25-PAL-EVIDENCE.md](25-PAL-EVIDENCE.md). The NTSC source hashes, existing mapper checks, and native-state tests were not changed. The only prior PAL frame assertions changed in `tests/core/test_frame.c` were stale additive-tick expectations; they now model the established absolute nominal PAL endpoints with bounded instruction overshoot.

## Bounded source, test and documentation review

The review covered the shared metadata resolver and no-content default, the library's region consumers, runner and adapter preference flow, mapper fixture construction, PAL frame/sample and transition arithmetic, prior raster/APU evidence, metadata negative controls, public 256×240 claims, and the Phase 26/27 boundaries. No additional in-scope source defect or unsupported PAL accuracy claim was found. The stale PAL expected-tick assertions described above were corrected and the focused frame and regional selections passed afterward.

### Reviewed fallback rows (retained unresolved)

Each inherited fallback row was reviewed against its cited source and current tests. Coverage is recorded below, but these descriptor-less rows remain **flagged/unresolved**; this evidence record does not silently dismiss them.

| Fallback row | Review evidence | Disposition |
|---|---|---|
| REG-01, unresolved/unclassified resolver and deliverable matrix | `tests/core/test_cartridge.c` covers all NES 2.0 timing values against Auto/NTSC/PAL preferences, legacy images, invalid/Dendy/truncated inputs, instance isolation, reset/unload and failed replacement. Plan 25-04 and `libretro.host` cover runner, library and real-module load/query/reload behavior. | Reviewed; matrix coverage exists. Retain flagged/unresolved because the inherited fallback has no wired acceptance descriptor to classify its original probe. |
| PAL-01, adjacent raster/APU/sample edges and equal-time ordering | `tests/ppu/test_nmi_races.c` covers the PAL raster, row and status boundaries; `tests/core/test_apu.c` covers source-derived PAL cursor/IRQ edges and register ordering; `tests/core/test_frame.c` covers sample capacity, remainder, reload and frame-boundary behavior. | Reviewed; the source-derived model and neighbor cases are covered. Retain flagged/unresolved for physical equal-timestamp CPU/PPU/APU precedence, which is not hardware-measured. PAL reset-release duration is also unverified: source clears reset inhibition at PAL pre-render dot 1 (line 311), and no pinned source establishes that duration. The public header/API now call it an unverified model behavior; no timing was invented. |
| PAL-01, empty/null/zero/exact/undersized boundaries | `tests/core/test_cartridge.c` covers null, zero/truncated and invalid image cases, rejected replacement and allocation failure. `tests/core/test_frame.c` covers null buffers, exact/undersized PAL audio capacity, fixed dimensions and refused-call state preservation. No-content region queries remain NTSC. | Reviewed; the named boundary cases have focused coverage. Retain flagged/unresolved as an inherited descriptor-less fallback rather than treating test existence as an automatic disposition. |
| PAL-01, interleaving/reset/reload ordering | `tests/core/test_frame.c` interleaves PAL and NTSC instances and checks reload/reset remainder behavior; `tests/core/test_cartridge.c` checks per-instance selection and reset/unload; `libretro.host` checks option latching and reload; the six-mapper matrix advances PAL peers and NTSC independently. | Reviewed; instance and lifecycle controls are present. Retain flagged/unresolved for the inherited descriptor-less row, physical equal-time ordering, and the unverified PAL reset-inhibit duration described above. |

### Retained descriptor-less prohibitions

| Prohibition | Status | Evidence and limit |
|---|---|---|
| PAL transparency | **Flagged/unverified** | Source-derived PAL frame, raster and pitch behavior is not direct hardware proof, universal playability, a modeled-palette qualification or 2A07 DMA acceptance. Palette presentation and loaded-game AV acceptance remain Phase 27; DMA remains Phase 26. |
| Campaign safety | **Flagged/unverified** | The final authorized local full workflow passed all 712 tests and packaging; the campaign fixture and protected-observation fixture passed, and report and canonical scoreboard hashes matched. Keep flagged because this inherited prohibition has no descriptor-backed disposition and an older interrupted run remains unknown. Standing authority covers bounded local CI for this plan; hosted workflows and publication remain outside scope. |

## Required CI gate and remaining limits

The first full-workflow attempt failed 706/712 on `9fcfbda`; six stale test contracts were corrected and focused retests passed. A subsequent wrapper/output-sink failure was diagnosed without removing historical test artifacts. The final full workflow is green on `fc48e45`, all three packages were created, and the final receipt is above. WR-01 is fixed; WR-02 remains deferred and explicitly disclosed because available pinned evidence does not establish PAL reset-release duration. The review ledger is [25-REVIEW-DISPOSITION.md](25-REVIEW-DISPOSITION.md).

Phase 25 Task 3 and goal-backward phase verification are complete. The verifier marked 23/23 truths verified with no gaps or UAT; all four inherited fallback rows and both descriptor-less prohibitions remain flagged. PAL reset-inhibit duration is outside the source-supported claims and remains an explicit limitation. Phase 26/27 and publication were not run or authorized here.
