---
phase: 25-region-selection-and-pal-timing
verified: 2026-11-03T03:10:02Z
status: passed
score: 23/23 must-haves verified
covered_files:
  - .planning/phases/25-region-selection-and-pal-timing/25-01-PLAN.md
  - .planning/phases/25-region-selection-and-pal-timing/25-01-SUMMARY.md
  - .planning/phases/25-region-selection-and-pal-timing/25-02-PLAN.md
  - .planning/phases/25-region-selection-and-pal-timing/25-02-SUMMARY.md
  - .planning/phases/25-region-selection-and-pal-timing/25-03-PLAN.md
  - .planning/phases/25-region-selection-and-pal-timing/25-03-SUMMARY.md
  - .planning/phases/25-region-selection-and-pal-timing/25-04-PLAN.md
  - .planning/phases/25-region-selection-and-pal-timing/25-04-SUMMARY.md
  - .planning/phases/25-region-selection-and-pal-timing/25-05-PLAN.md
  - .planning/phases/25-region-selection-and-pal-timing/25-05-SUMMARY.md
  - .planning/phases/25-region-selection-and-pal-timing/25-ACCEPTANCE.md
  - .planning/phases/25-region-selection-and-pal-timing/25-AUTOMATION-DISPOSITION.md
  - .planning/phases/25-region-selection-and-pal-timing/25-PAL-EVIDENCE.md
  - .planning/phases/25-region-selection-and-pal-timing/25-REVIEW-DISPOSITION.md
  - .planning/phases/25-region-selection-and-pal-timing/25-REVIEW.md
  - README.md
  - docs/api.md
  - docs/runner.md
  - docs/testing.md
  - include/nesturbator.h
  - libretro/libretro.c
  - libretro/nesturbator_libretro.info
  - runner/main.c
  - src/apu.c
  - src/bus.c
  - src/cartridge.c
  - src/frame.c
  - src/instance.c
  - src/internal.h
  - src/ppu.c
  - src/synth.c
  - tests/CMakeLists.txt
  - tests/accuracy/baseline-metadata.txt
  - tests/cmake/info_claims.cmake
  - tests/cmake/phase21_hosted_evidence_controls.cmake
  - tests/cmake/vector_api_policy.cmake
  - tests/core/test_api.c
  - tests/core/test_apu.c
  - tests/core/test_cartridge.c
  - tests/core/test_frame.c
  - tests/core/test_mapper.c
  - tests/core/test_synth.c
  - tests/cpu/test_bus.c
  - tests/libretro/libretro_host.c
  - tests/ppu/test_nmi_races.c
  - tests/runner/hashes.txt
  - tests/runner/test_spectral.c
covered_digest: "v4:sha256:7df483fca28e16619cd80fddf29a51dc165ce2bcb2a9a7e656b87fc0d4c73e63"
behavior_unverified: 0
overrides_applied: 0
flagged_prohibitions: 2
decision_coverage:
  honored: 6
  total: 6
  not_honored: []
human_verification: []
---

# Phase 25: Region Selection and PAL Timing Verification Report

**Phase Goal:** A library user can select supported content under the agreed region policy and run PAL content at its intended frame timing and audio pitch on every existing mapper family.

**Verified:** 2026-11-03T03:10:02Z
**Status:** passed, with two flagged/unresolved prohibitions retained.
**Re-verification:** No; the existing report had no gaps, so this is a fresh goal-backward pass.

## Goal Achievement

The four roadmap success criteria and the additional plan truths were checked against current code, active tests, and the current local workflow receipt. `HEAD` is `6a3cf00607a6cbe33bdaac971dae382aab3fb00a` with tree `8f33d6f4376a8781d0a020a0611a040109265460`, matching the candidate supplied for this verification. The prior report had no `gaps:` section, so this is an initial-mode goal-backward pass.

### Observable Truths

| # | Truth | Status | Evidence |
|---:|---|---|---|
| 1 | Explicit single-region NES 2.0 metadata determines NTSC or PAL despite a contradictory host preference. | ✓ VERIFIED | src/cartridge.c:233-247 resolves explicit timing before the preference. The complete resolver matrix in tests/core/test_cartridge.c passed as core.cartridge in the current candidate's unfiltered workflow log. |
| 2 | Multi-region NES 2.0 and legacy iNES default to NTSC, accept an explicit PAL request, and refuse Dendy. | ✓ VERIFIED | src/cartridge.c:236-251; the library, runner, and real adapter cases passed in core.cartridge and libretro.host in the current 722-test workflow. |
| 3 | PAL content on mapper IDs 0, 1, 2, 3, 4, and 7 uses PAL cadence and frequency tables and produces independently checked frame timing and pitch. | ✓ VERIFIED | tests/core/test_mapper.c:612-803 creates all six board fixtures and checks independent frame/sample arithmetic, CPU-cycle periods, and PCM pitch. src/internal.h, src/bus.c, src/ppu.c, src/apu.c, and src/synth.c consume the loaded profile. core.mapper, core.apu, ppu.nmi_races, and runner.spectral passed in the current workflow. Evidence is source-derived model agreement, not physical PAL hardware qualification. |
| 4 | Advancing many PAL frames gives deterministic exact frame/sample accounting without rounding to 50 Hz. | ✓ VERIFIED | tests/core/test_frame.c:345-503 and the mapper matrix use independent rational counts, reject the rounded 960-samples/frame model, and check carried remainders. core.frame and core.mapper passed in the current workflow. |
| 5 | A failed cartridge replacement preserves the active machine and region; a successful load latches the profile until unload/reload. | ✓ VERIFIED | src/cartridge.c:308-346 validates and allocates before replacing cartridge state; tests/core/test_cartridge.c checks allocator failure, rejected replacement, subsequent execution, reload, and unload. core.cartridge passed. |
| 6 | The original loader remains Auto and the public ABI and output-structure prefix remain usable. | ✓ VERIFIED | src/cartridge.c:412-420 delegates the old loader to Auto and returns NTSC when empty. tests/core/test_api.c and tests/cmake/vector_api_policy.cmake cover the public contract; core.api and header build checks passed in the current unfiltered workflow. |
| 7 | PAL row-239 representation and APU GET/PUT-to-CPU event units have independent, source-pinned dispositions before dependent expectations are used. | ✓ VERIFIED | 25-PAL-EVIDENCE.md identifies the pinned PAL video, circuit-analysis, and APU sources; derives physical rows and event cursors with explicit units/origins; and defines independent row/event fixtures. The record excludes direct hardware measurement and emulator output as an oracle. |
| 8 | PAL CPU cycles use 32 half-master ticks with M2 at +13, and PAL PPU dots use 10 ticks. | ✓ VERIFIED | src/internal.h:279-292 provides the profile values; src/bus.c and src/ppu.c consume them. bus.unit and the current PAL PPU/mapper checks passed. |
| 9 | PAL information reports 322445/6448 frames per second and 48000/1 samples per second while retaining 256×240 output. | ✓ VERIFIED | src/internal.h:294-317 provides the rate fractions; src/frame.c reports them and keeps the fixed buffer dimensions. core.frame, core.mapper, and libretro.host passed. |
| 10 | Exact many-frame sample totals, atomic buffer refusal, and independent interleaved NTSC/PAL clocks and remainders are covered. | ✓ VERIFIED | tests/core/test_frame.c:345-503 exercises rational periods, NULL/exact/undersized buffers, reload/reset, and interleaved instances. core.frame passed in the current workflow. |
| 11 | Established NTSC frame, audio-count, raster, mapper-clock, and interrupt behavior remains protected. | ✓ VERIFIED | Retained NTSC assertions in frame, APU, PPU, bus, mapper, and hash tests passed in the unfiltered current workflow; Phase 27 acceptance also records no NTSC palette/hash/scoreboard repin. |
| 12 | PAL raster uses 10-tick dots over 312 lines without an NTSC odd-frame skip, while native output remains 256×240. | ✓ VERIFIED | src/ppu.c selects the PAL raster and suppresses the NTSC-only skip; tests/ppu/test_nmi_races.c checks raster boundaries, row markers, and output canaries. ppu.nmi_races passed. |
| 13 | PAL pulse/triangle pitch uses the PAL CPU clock, and PAL noise/DMC timers use the source-derived table entries. | ✓ VERIFIED | src/apu.c selects the regional timers; tests/core/test_apu.c checks table entries/events and tests/runner/test_spectral.c checks public pulse/triangle PCM against independent rational expectations. core.apu and runner.spectral passed. |
| 14 | PAL four/five-step sequencing, IRQ edges, and modeled register-edge order match the pinned event table. | ✓ VERIFIED | src/apu.c and tests/core/test_apu.c cover the source-derived cursor thresholds and GET/PUT neighbors; core.apu passed. The PAL $4017 3/4-cycle equivalence remains a stated source assumption, not a measured hardware race. |
| 15 | PAL PCM transitions use the regional clock, carry sample phase across frames, and remain deterministic across instances. | ✓ VERIFIED | Per-instance clock/sample state in src/internal.h, src/apu.c, src/synth.c, and src/frame.c feeds the real PCM path. core.apu, core.frame, and runner.spectral passed their phase, sample, and instance checks. |
| 16 | NTSC transition/PCM expectations remain protected; Phase 25 makes no 2A07 DMA or direct-hardware claim. | ✓ VERIFIED | Existing NTSC tests and hashes passed in the current workflow. 25-PAL-EVIDENCE.md, 25-ACCEPTANCE.md, and public docs label PAL evidence as source-derived and defer DMA/state and palette/presentation claims. |
| 17 | Runner and libretro forward Auto/NTSC/PAL preferences through the shared loader. | ✓ VERIFIED | runner/main.c:1120 and libretro/libretro.c:471-475 call nesturbator_load_cartridge_region; region precedence remains in src/cartridge.c. |
| 18 | A changed libretro option takes effect only on the next load, keeping the active clock and reported region aligned. | ✓ VERIFIED | libretro/libretro.c:128-139,471-475 samples the option at content load; tests/libretro/libretro_host.c exercises reload latching and refused replacement. libretro.host passed. |
| 19 | Libretro region, AV rates, and callback geometry reflect the loaded profile and fixed 256×240 contract. | ✓ VERIFIED | libretro/libretro.c:273-292,523-529 queries library info/effective region and sets fixed geometry. Host lifecycle assertions passed in libretro.host. |
| 20 | The registered core option was exercised before its metadata capability was advertised, and metadata checks still reject false claims. | ✓ VERIFIED | tests/libretro/libretro_host.c, tests/cmake/info_claims.cmake, and libretro/nesturbator_libretro.info cover option lifecycle and negative controls. libretro.host, libretro.info_claims, and docs.links passed in the current workflow. |
| 21 | PAL acceptance uses independent expectations and does not rely on repeated hashes or adapter equality alone; NTSC expectations remain intact. | ✓ VERIFIED | tests/core/test_mapper.c, tests/core/test_apu.c, tests/ppu/test_nmi_races.c, and tests/runner/test_spectral.c compare source arithmetic, event values, pixels, and PCM values. The same files retain NTSC controls, all passing in the current workflow. |
| 22 | The required full local CI is accepted only after authorized execution on the changed candidate, while the older interrupted-campaign stop remains effective. | ✓ VERIFIED | The current run on `6a3cf00607a6cbe33bdaac971dae382aab3fb00a` / tree `8f33d6f4376a8781d0a020a0611a040109265460` ran `cmake --workflow --preset ci` under standing local-CMake authority; the CTest log contains 722 test records, and the supplied run result reports 722/722 passed plus configure, build, and packaging success. This does not resolve the older interrupted campaign or authorize hosted CI or release actions. |
| 23 | Focused PAL/NTSC evidence, full-workflow receipts, and scope limits are recorded separately. | ✓ VERIFIED | `25-ACCEPTANCE.md`, `25-PAL-EVIDENCE.md`, and `25-AUTOMATION-DISPOSITION.md` separate source-derived model evidence, test results, unresolved descriptor-less flags, and the operational stop. The current CTest log is fingerprinted with this report. |

**Score:** 23/23 truths verified (0 present, behavior-unverified).

Phase 25 covers region selection and PAL execution timing/pitch. Regional state continuation, PAL color presentation, and 2A07 DMA behavior are outside this phase's contract; this report does not use their later implementation or evidence to expand the Phase 25 claim.

## Required Artifacts

All 17 artifacts declared across the five plans passed the GSD artifact checks (17/17: present, substantive, and no declared-pattern issues). Their consumers and test registrations were manually traced where the plan links use symbolic names.

| Artifact | Expected | Status | Details |
|---|---|---|---|
| src/cartridge.c | Shared resolver and transactional load | ✓ VERIFIED | Explicit metadata precedes preference; effective region commits after validation/allocation. |
| tests/core/test_cartridge.c | Resolver and failed-replacement controls | ✓ VERIFIED | Preference matrix, Dendy/invalid cases, allocation failure, isolation, and live-machine continuation. |
| 25-PAL-EVIDENCE.md | PAL row mapping and APU edge oracle | ✓ VERIFIED | Pinned sources, units, origins, derivations, and independent fixture designs. |
| src/internal.h | Regional profile accessors | ✓ VERIFIED | Integer CPU/M2/PPU/frame/sample/rate values derive from instance region. |
| src/frame.c | Exact frame/sample accounting | ✓ VERIFIED | Regional target and remainder logic on the public frame path. |
| src/ppu.c | PAL raster and raw row behavior | ✓ VERIFIED | 312-line model, 10-tick dots, no PAL odd-frame skip. |
| tests/ppu/test_nmi_races.c | Independent raster/row controls | ✓ VERIFIED | Boundary, physical row, and output guard assertions. |
| src/apu.c | PAL APU periods and event cursors | ✓ VERIFIED | Region-selected PAL frequency tables and four/five-step sequence. |
| src/synth.c | Profile-aware sample interpolation | ✓ VERIFIED | Uses regional sample phase through transition, sampling, and drain. |
| tests/core/test_apu.c | PAL table and event controls | ✓ VERIFIED | All noise/DMC rates and adjacent source-derived event edges checked. |
| tests/runner/test_spectral.c | Public PAL pitch oracle | ✓ VERIFIED | Public PCM measured against independently derived rational tone frequencies. |
| runner/main.c | CLI preference forwarding | ✓ VERIFIED | Parsed preference reaches the shared region loader. |
| libretro/libretro.c | Option forwarding and effective-profile report | ✓ VERIFIED | Load-time preference reaches core; region and AV data use loaded profile. |
| tests/libretro/libretro_host.c | Real adapter lifecycle | ✓ VERIFIED | Option registration, reload, refusal, region, AV, and geometry are exercised. |
| tests/cmake/info_claims.cmake | Metadata claim guard | ✓ VERIFIED | False/missing option claims are rejected. |
| tests/core/test_mapper.c | Six-mapper acceptance | ✓ VERIFIED | Owned boards 0, 1, 2, 3, 4, and 7 use public load/frame APIs and independent checks. |
| 25-ACCEPTANCE.md | Candidate-bound acceptance record | ✓ VERIFIED | CI receipt, focused checks, evidence boundaries, and retained flags are recorded. |

**Artifacts:** 17/17 verified.

## Key Link Verification

The key-link query returned false because plan from/to values are symbolic component names rather than repository-relative file paths. I traced each connection directly through the current source.

| From | To | Via | Status | Details |
|---|---|---|---|---|
| Legacy loader | Region-aware loader | Auto wrapper | ✓ WIRED | src/cartridge.c:412-415. |
| Validated cartridge layout | cart.effective_region | Commit after allocation | ✓ WIRED | src/cartridge.c:308-346; failed validation/allocation returns before replacing the active cartridge. |
| cart.effective_region | bus.c, frame.c, instance.c | Shared integer profile helpers | ✓ WIRED | src/internal.h:265-317; consumers read the profile for bus/frame clocks and reported rates. |
| Effective region and pinned PAL row evidence | ppu.c and tests/ppu/test_nmi_races.c | Regional raster accessors and independently derived row/boundary expectations | ✓ WIRED | src/ppu.c consumes the loaded region; the test checks PAL line/dot behavior, physical rows, and guards. |
| cart.effective_region | apu.c period/event selection | Regional frequency tables and frame-event cursors | ✓ WIRED | src/apu.c selects PAL tables/events from the loaded profile; tests/core/test_apu.c checks all timer entries and adjacent source-derived cursors. |
| regional sample_phase | synth interpolation and PCM staging | Shared regional denominator through transition, sampling, and drain | ✓ WIRED | src/apu.c and src/synth.c use the regional sample clock; tests/core/test_mapper.c and tests/runner/test_spectral.c exercise profile-specific PCM, while test_synth.c covers synthesis invariants. |
| Runner/libretro preference | Shared public loader | Preference forwarding | ✓ WIRED | runner/main.c:1120; libretro/libretro.c:471-475. |
| Loaded region | Libretro region and AV info | Public effective-profile query | ✓ WIRED | retro_get_region() uses nesturbator_get_region() and AV info uses nesturbator_get_info(). |
| Six-mapper fixture matrix | Production load/frame/bus/PPU/APU paths | Public API plus independent rational and period expectations | ✓ WIRED | tests/core/test_mapper.c:612-803 loads each generated image, advances frames, and checks timing, samples, board behavior, and PCM pitch. |
| Required CI receipt | Exact candidate source/build identity | Current run and repository commit/tree | ✓ WIRED | The supplied run is on HEAD `6a3cf00607a6cbe33bdaac971dae382aab3fb00a` / tree `8f33d6f4376a8781d0a020a0611a040109265460`; the current LastTest log contains all 722 test records. |

**Wiring:** all planned connections manually traced. The failed query patterns reflect symbolic plan links, not broken product wiring.

## Data-Flow Trace (Level 4)

| Artifact | Data | Source and flow | Status |
|---|---|---|---|
| Region resolver | Effective profile | NES header plus per-load preference → validated layout → instance cartridge state | ✓ FLOWING |
| Regional timing | CPU/PPU/APU clocks and reported rates | Instance profile → shared integer helpers → actual bus/frame/PPU/APU/synth paths | ✓ FLOWING |
| Public frame/audio output | Pixels, PCM, counts, ticks | Loaded guest executes on CPU; PPU writes pixels and APU/synth produce PCM through frame API | ✓ FLOWING |
| Runner and libretro | Preference, region and AV rates | Host preference → shared loader → effective-profile query and callbacks | ✓ FLOWING |

No rendered value ends at a static return or mock. Generated owned ROMs are test inputs; expected frame/sample/pitch values are derived independently from pinned source facts and rational arithmetic. This supports the bounded model claim only.

## Behavioral Spot-Checks

No tests were rerun by this verifier. The current run supplied with the verification task executed `cmake --workflow --preset ci` on the exact `HEAD` above: configure, build, 722/722 unfiltered CTest checks, and packaging of the library, libretro adapter, and runner passed. `build/ci/Testing/Temporary/LastTest.log` has 722 test starts and 722 pass records (684 ordinary passes and 38 positive-regex passes), SHA256 `162ea1d7d82371d69c59ea27bc34df76c86a0eacbe60bbf102e2d7e31ce93fb4`. The adjacent `LastTestsFailed.log` is timestamped 20:47 EST, before this log's 21:56 EST completion, and was not treated as this run's result. This is local software evidence only; it does not establish direct PAL hardware correctness or hosted CI status.

| Behavior | Saved evidence | Result | Status |
|---|---|---|---|
| Resolver matrix and failed replacement | Current unfiltered workflow: `core.cartridge` (0.08 s) | Passed | ✓ PASS |
| PAL bus and exact frame/sample accounting | `bus.unit` (0.08 s), `core.frame` (2.70 s) | Passed | ✓ PASS |
| Six-mapper timing and pitch matrix | `core.mapper` (0.15 s) | Passed | ✓ PASS |
| PAL raster and row boundaries | `ppu.nmi_races` (0.02 s) | Passed | ✓ PASS |
| PAL APU tables/events and public pitch | `core.apu` (0.01 s), `runner.spectral` (0.24 s) | Passed | ✓ PASS |
| Public API and real libretro selection/reporting lifecycle | `core.api` (0.03 s), `libretro.host` (1.33 s) | Passed | ✓ PASS |
| Option metadata and false-claim controls | `libretro.info_claims` in the same unfiltered workflow | Passed | ✓ PASS |
| Overall current workflow and packages | `cmake --workflow --preset ci` on the current commit | 722/722, packages passed | ✓ PASS |

The supplied workflow receipt reports 506.94 seconds for the CTest stage, within the configured 1,500-second gate and the project 30-minute cap. The current CTest log's largest single test was `retroarch.session.controls` at 287.50 seconds. No safe check reduction is indicated: the longest case is acceptance coverage, and the evidence does not show avoidable duplicate execution. No CI configuration was changed.

## Probe Execution

| Probe | Command | Result | Status |
|---|---|---|---|
| None declared | No Phase 25 plan/summary declares a probe; no scripts/*/tests/probe-*.sh exists. | Not applicable | N/A |

## Requirements Coverage

| Requirement | Source Plans | Description | Status | Evidence |
|---|---|---|---|---|
| REG-01 | 25-01, 25-02, 25-04, 25-05 | Explicit NES 2.0 precedence; multi-region/legacy defaults and host preference; Dendy refusal | ✓ SATISFIED | Resolver matrix plus library, runner, and real-module lifecycle tests passed in the current workflow. |
| PAL-01 | 25-01 through 25-05 | PAL CPU/PPU/APU timing and frequency tables with independent timing/pitch checks across six supported mappers | ✓ SATISFIED | Source-pinned model evidence, subsystem tests, public spectral checks, six-mapper matrix, and current full workflow. |

REQUIREMENTS.md assigns only REG-01 and PAL-01 to Phase 25; no Phase 25 requirement is orphaned.

## Decision Coverage

The decision-coverage query checked six trackable decisions in 25-CONTEXT.md; all six are represented in shipped artifacts. The gate is advisory and does not affect the must-have score.

## Test Quality Audit

| Test/Fixture | Requirement | Active | Skipped | Expected-value source | Strongest assertion | Verdict |
|---|---|---:|---:|---|---|---|
| tests/core/test_cartridge.c | REG-01 | Yes | 0 | Explicit metadata/preference cases | Value and transactional replacement behavior | PASS |
| tests/core/test_frame.c | PAL-01 | Yes | 0 | Independent rational frame/sample arithmetic | Exact counts, atomic refusal, reset/reload/interleaving | PASS |
| tests/core/test_mapper.c | PAL-01 | Yes | 0 | Independent PAL/NTSC rational clocks and timer math | Six-board behavior plus frame/sample/pitch values | PASS |
| tests/core/test_apu.c | PAL-01 | Yes | 0 | Pinned source table and event derivation | Value-level table and adjacent event checks | PASS |
| tests/ppu/test_nmi_races.c | PAL-01 | Yes | 0 | Pinned physical-row and raster expectations | Raster and pixel-row boundary values | PASS |
| tests/runner/test_spectral.c | PAL-01 | Yes | 0 | Independent rational tone frequencies | Public PCM pitch and phase | PASS |
| tests/core/test_synth.c | PAL-01 (NTSC regression control) | Yes | 0 | Fixed-point/Q15 invariants | Value-level integrator and chunked-drain checks | Supporting control; not used as the PAL pitch oracle |
| tests/core/test_api.c | REG-01 | Yes | 0 | Public API/ABI contract | Status and structure-prefix values | PASS |
| tests/cmake/vector_api_policy.cmake | REG-01 | Yes | 0 | Declared ABI policy | Compile/configuration guard | PASS |
| tests/cpu/test_bus.c | PAL-01 | Yes | 0 | Independent regional cycle arithmetic and bus-trace points | Cycle/timestamp values and NTSC controls | PASS |
| tests/libretro/libretro_host.c | REG-01, PAL-01 | Yes | 0 | Generated owned inputs | Real module load/query/reload behavior | PASS |
| tests/runner/hashes.txt | PAL-01 (NTSC regression control) | Yes | 0 | Retained pre-change output baselines | Exact frame/audio hash comparison | Supporting control; not used as a PAL oracle |
| tests/cmake/info_claims.cmake | REG-01 | Yes | 0 | Pinned capability and mutation controls | Negative metadata values are rejected | PASS |

The disabled-test scan found no skipped requirement tests; `.pending` matches in bus tests are state-field names, not disabled-test markers. Test-side writes in `libretro_host.c` create ROM/movie fixtures or output files; they do not generate expected timing or pitch values from the system under test. The PAL expectations come from pinned source facts and independent rational arithmetic. Repeated hashes establish determinism only, and adapter equality is not used as a timing/pitch oracle.

## Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|---|---:|---|---|---|
| None | — | No TBD, FIXME, XXX, TODO/HACK placeholder, disabled test, or empty implementation was found in the Phase 25 implementation/test files. | — | No Phase 25 stub or debt-marker blocker. |

The phrase “not available with AccuracyCoin modes” in README.md describes a documented testing mode limit; it is not placeholder copy. The search for test-side writes found only generated fixtures and output files, not an expected-value generator.

## Prohibition Flags and Automation Disposition

The two canonical Plan 25-05 prohibitions remain flagged and unresolved (`flagged_prohibitions: 2`); this report does not turn them green.

| Prohibition | Status | Evidence and limit |
|---|---|---|
| PAL evidence and accuracy | Flagged/unresolved | The bounded source/documentation review found no claim that PAL model results are direct hardware proof, universal accuracy/playability, palette qualification, or DMA acceptance. The PAL `$4017` delay and reset-inhibit duration remain disclosed model assumptions/limits. |
| Interrupted-campaign authority | Flagged/unresolved | The older interrupted campaign remains stopped and its outcome unknown. The current local CI run was performed under standing local-CI authority; it does not resolve that older run or authorize hosted workflows, pushes, merges, publication, or another phase. |

The project policy and `25-AUTOMATION-DISPOSITION.md` supersede the two former human prompts after a bounded source/documentation review. The flags remain visible, and no owner acknowledgment is needed merely to preserve these limits or the existing stop. This is not a hardware qualification or permission to restart the stopped campaign.

## Human Verification Required

None. The Phase 25 automation disposition records why the two retained flags need no owner acknowledgment merely to preserve their limits. No visual, sensory, or manual user-flow test is part of this core-library phase.

## Gaps Summary

No Phase 25 roadmap or plan truth failed. Its region policy, PAL timing, sample accounting, six-mapper pitch, host forwarding, and NTSC safeguards are present, connected, and covered by current-source test receipts. Status is passed with two prohibition flags still explicitly unresolved. The local workflow confirms the software model and regressions on this candidate; it does not establish hardware accuracy or hosted CI status.

---
_Verified: 2026-11-03T03:10:02Z_
_Verifier: the agent (gsd-verifier)_
