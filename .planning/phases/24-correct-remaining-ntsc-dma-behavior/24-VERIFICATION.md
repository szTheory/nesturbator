---
phase: 24-correct-remaining-ntsc-dma-behavior
verified: 2026-11-01T01:34:41Z
status: gaps_found
score: "8/9 must-haves verified"
covered_files:
  - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-01-PLAN.md
  - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-01-SUMMARY.md
  - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-02-PLAN.md
  - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-02-SUMMARY.md
  - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-03-PLAN.md
  - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-03-SUMMARY.md
  - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-04-PLAN.md
  - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-04-SUMMARY.md
  - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-ACCEPTANCE.md
  - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-CI-RUN-20261101.md
  - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-DMA-EVIDENCE.md
  - README.md
  - docs/api.md
  - docs/reproduction.md
  - include/nesturbator.h
  - src/apu.c
  - src/bus.c
  - src/internal.h
  - src/state.c
  - tests/accuracy/baseline-metadata.txt
  - tests/accuracy/measurements.md
  - tests/cmake/phase21_hosted_evidence_controls.cmake
  - tests/cmake/reproduction_contract.cmake
  - tests/cmake/vector_api_policy.cmake
  - tests/core/test_api.c
  - tests/core/test_apu.c
  - tests/core/test_state_fork.c
  - tests/core/test_state_reject.c
  - tests/cpu/dma_abort_sweep.h
  - tests/cpu/test_bus.c
  - tests/runner/hashes.txt
covered_digest: "v4:sha256:99abdace35ac25efd7689eb6dcd8082dd017cd7bc1f10ff4eba0c20d58003e8c"
behavior_unverified: 0
overrides_applied: 0
gaps:
  - truth: "Protected PASS behavior is independently verified, identical frame/PCM/transition outputs are established across supported platforms, and the existing performance gate passes unchanged."
    status: failed
    reason: "The local run observed 119 prior protected PASS keys, but used the checkout scoreboard-main fallback. No independent cold-main authority, current-head hosted result, six-platform hash equality, or Cachegrind +1% result is present."
    artifacts:
      - path: .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-CI-RUN-20261101.md
        issue: "Records only the local campaign and explicitly says its protected baseline came from the checkout fallback; hosted platform and performance evidence are absent."
      - path: .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-ACCEPTANCE.md
        issue: "The required independent protected-main, hosted hash, performance, and current-head CI results remain outstanding."
    missing:
      - "Independently prepared cold protected-main evidence for the current candidate."
      - "Current-head hosted CI lane results."
      - "Linux/macOS/Windows x64/arm64 equality for the complete frame, PCM, and transition hash inventory."
      - "The unchanged Cachegrind base/head comparison within the existing +1% allowance."
---

# Phase 24: Correct Remaining NTSC DMA Behavior — Verification Report

**Phase Goal:** Hosts using the library, runner or libretro adapter receive source-supported NTSC DMA behavior for the five selected failures, with existing passing behavior and deterministic continuation preserved.

**Verified:** 2026-11-01T01:34:41Z  
**Status:** `gaps_found`  
**Re-verification:** No — initial verification

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|---|---|---|
| 1 | All five selected rows pass at the unchanged AccuracyCoin pin for the declared target. | ✓ VERIFIED | The archived complete local run binds ROM pin `6bee919529289c555803b612b8c9e4f7eaaeeabb` and implementation source `324a769e413003cce3aa30d6a5a78a441ce558d8`. Its raw `report.tsv` has individual terminal PASS rows for Landing `0x01`, DMC+OAM `0x01`, Bus Conflicts `0x01`, Explicit Abort `0x01`, and the accepted Implicit Abort `0x09` variant. `git diff --exit-code a12a79a --` over the emulation sources, tests, public header, README and CMake wiring returns 0 at this checkout. This is pinned compatibility evidence, not silicon proof. |
| 2 | Every corrected row has a separate first-cause record, cycle-level observations, a named wrong model, and an owned fixture that observes the real GET/resume effects. | ✓ VERIFIED | `24-DMA-EVIDENCE.md` contains five independent source/test/core records and separate model controls. `tests/cpu/test_bus.c` includes the production `src/bus.c` with test-only observer hooks and compares results with the normal linked path. The landing fixture asserts sample address/data/phase and resumed CPU read; the full local CTest log shows `bus.unit` passed. The hardware reference is identified as NESdev DMA rev. 23450 (HWC.01, T2); the pinned test contract and observed core events are kept in separate evidence columns. |
| 3 | DMC GET collisions preserve OAM bytes, resume position, and source/destination timing through interior and final windows. | ✓ VERIFIED | `src/bus.c:oam_dma` advances local OAM stages and steals only the DMC GET source slot. The owned matrix asserts 256 destination writes and phase/resume positions. Existing `bus.unit` output records the interior +2 case and final +1/+3 cases; the local run's `bus.unit` entry passed. |
| 4 | DMC bus conflicts preserve distinct internal register effects and external sample data/latch behavior. | ✓ VERIFIED | `src/bus.c:dmc_sample_get` fetches from the mapper using the DMC address, updates the external latch, then applies internal `$4015-$4017` selection from the parked CPU address and sample low bits. `tests/cpu/test_bus.c:test_dma_bus_conflicts` checks sample GET address/data/phase, IRQ acknowledgement, controller shifts, returned value, and latch values. The same full local `bus.unit` entry passed. |
| 5 | Explicit and implicit abort behavior is separated from ordinary write-deferred retries, with the selected early-G model and both pinned accepted implicit outcomes preserved. | ✓ VERIFIED | `src/internal.h:nesturbator__dmc_stop_window`, `src/apu.c:nesturbator__apu_clock`, and `src/bus.c:bus_read_cycle` implement the expiring abort window separately from `dma_pending`. The test fixtures sweep neighboring counter/phase windows, writes, looping and length controls. The source comments and README label the early-G ordering as an inference and avoid a universal chip claim. The raw local ROM result records the pinned accepted `0x09` variant. |
| 6 | Every changed in-flight DMA state resumes identically from production-reachable native save boundaries, with format/revision and compatibility consequences explicit. | ✓ VERIFIED | `src/state.c` serializes the internal/external bus latches, DMC silence state and abort countdown with range validation. `include/nesturbator.h` declares native format 6, behavior revision 14 and ABI 1; it documents exact-format loading and that behavior revision is recorded, not enforced. `tests/core/test_state_fork.c` reaches real public frame boundaries, saves/loads through the public API, and compares pixels, PCM, transitions, effects and final state; `core.state_fork.stress` passed in the archived CTest log. |
| 7 | The public README/header explain the corrections and state consequences, and the owned regressions are registered in the existing CI entrypoint for all three host deliverables. | ✓ VERIFIED | The root CMake library includes `src/bus.c` and `src/apu.c`; the runner and `libretro/CMakeLists.txt` link that library. `tests/CMakeLists.txt` registers `bus.unit` and `core.state_fork.stress`, whose stress path invokes the DMA forks. README/header contain the behavior 11–14 and format 4–6 compatibility notes. The raw local CTest log includes passing `bus.unit` and `core.state_fork.stress` entries. |
| 8 | Protected PASS behavior is independently verified, cross-platform frame/PCM/transition hashes match, and unchanged performance limits pass. | ✗ FAILED | Local evidence says 119 old protected PASS keys passed, but protected observations used the checkout `scoreboard-main` fallback, not independent cold-main authority. The run produced only a macOS arm64 package. No current-head hosted lanes, six-platform hash equality, or unchanged Cachegrind `+1%` result is present. This is the blocking DMA-03 gap. |
| 9 | The acceptance boundary, authority limits, and any missing evidence are recorded without inferring acceptance from local fixtures. | ✓ VERIFIED | The exact local workflow, limits, ROM/build identities, complete report identity, observed rows, and missing results are recorded in `24-CI-RUN-20261101.md` and `24-ACCEPTANCE.md`. The run record says no hosted workflow, push, PR, merge, or release occurred. The current `.planning/STATE.md` keeps the phase and all four requirements open. |

**Score:** 8/9 merged roadmap and plan-specific truths verified.

The source-supported behavior is demonstrated for all five selected rows on the tested local candidate. The wider goal is not complete because the protected-main, cross-platform determinism, and performance gates remain unverified. The evidence supports the declared source model and pinned test contract; it does not establish silicon behavior or a universal RP2A03G implicit-abort result.

## Required Artifacts

| Artifact | Expected | Status | Details |
|---|---|---|---|
| `src/apu.c`, `src/bus.c`, `src/internal.h` | Instance-local DMC/OAM correction and bus arbitration | ✓ VERIFIED | Substantive code is in the core library and is unchanged from the tested behavior14 source commit. |
| `src/state.c`, `tests/core/test_state_fork.c`, `tests/core/test_state_reject.c` | Native serialization, continuation and rejection coverage | ✓ VERIFIED | Visitor saves/validates the changed fields; public-boundary fork and old-format refusal paths are implemented and present in the full local CTest log. |
| `tests/cpu/test_bus.c` | Landing, collision, bus-conflict, abort fixtures and wrong-model controls | ✓ VERIFIED | Substantive event-level assertions; linked as the existing `bus.unit` test. |
| `include/nesturbator.h`, `README.md` | Public behavior, revision and native compatibility comments | ✓ VERIFIED | Format 6, behavior revision 14, ABI 1, inferred behavior limits, and exact-format state policy are documented. |
| `24-DMA-EVIDENCE.md` | Five distinct causal records and state dispositions | ✓ VERIFIED | Five row sections retain separate hardware, pinned-test, observed-core, source-lineage and wrong-model evidence. |
| `24-ACCEPTANCE.md`, `24-CI-RUN-20261101.md`, `tests/accuracy/measurements.md` | Attributable local acceptance and explicit remaining evidence | ✓ VERIFIED | The full local result and its limits are recorded; no hosted or cross-platform result is represented as complete. |
| Cold protected-main, hosted hash and performance result artifacts | Independent protected evidence, six-platform hash equality, unchanged Cachegrind gate | ✗ MISSING | These results do not exist in the phase evidence; the acceptance record explicitly keeps them open. |

## Key Link Verification

| From | To | Via | Status | Details |
|---|---|---|---|---|
| `CMakeLists.txt` | `src/bus.c`, `src/apu.c` | `nesturbator` static library source list | ✓ WIRED | The production library compiles the corrected core files. |
| `runner` | `nesturbator::nesturbator` | CMake target link | ✓ WIRED | The headless runner consumes the same core library. |
| `libretro/CMakeLists.txt` | `nesturbator::nesturbator` | `target_link_libraries` | ✓ WIRED | The libretro module consumes the same core library. |
| `src/bus.c` | `src/apu.c` | CPU `cycle()` calls `nesturbator__apu_clock`; request state is consumed by DMC/OAM arbitration | ✓ WIRED | Manual call-path inspection confirms the link. The generic key-link query did not follow this source call. |
| `tests/cpu/test_bus.c` | `src/bus.c` | Test-only observer macro and renamed entrypoints around production source inclusion | ✓ WIRED | Test observer and production implementation are explicitly connected; normal and observed paths are compared. |
| `tests/core/test_state_fork.c` | `src/state.c` | Public `nesturbator_save_state`/`nesturbator_load_state` and whole-output continuation comparisons | ✓ WIRED | `src/state.c` is in the library; tests exercise the public path. The generic query did not infer this API-to-implementation edge. |
| `tests/CMakeLists.txt` | `tests/cpu/test_bus.c` | `bus.unit` source and CTest registration | ✓ WIRED | `bus.unit` is registered in the existing suite and passed in the archived run. |
| `tests/CMakeLists.txt` | `tests/core/test_state_fork.c` | `core.state_fork.stress` registration and `--stress` dispatch | ✓ WIRED | Stress invokes DMA, conflict-latch and abort native forks; the archived CTest entry passed. |
| `.github/workflows/ci.yml` | `tests/CMakeLists.txt` | Primary CI CMake workflow and FULL campaign selection | ✓ WIRED | The CI path is present; no current-head hosted result was run or claimed. |
| `tests/accuracy/scoreboard.txt` | `tests/cmake/verify_scoreboard_regression.cmake` | Exact-key protected regression verification | ✓ WIRED | Code path exists, but this local run did not have independent cold-main authority. |
| `tests/cmake/write_hashes.cmake` | `tests/cmake/hash_inventory.cmake` | Cross-platform frame/PCM/transition inventory | ✓ WIRED | The output collection path exists; required hosted results are missing. |
| `tests/cmake/perf_compare.cmake` | `tests/perf/allow.txt` | Existing Cachegrind allowance | ✓ WIRED | The `+1%` gate remains configured; no qualifying current result is recorded. |

## Data-Flow Trace (Level 4)

| Artifact | Data variable | Source | Produces real data | Status |
|---|---|---|---|---|
| `src/bus.c:dmc_sample_get` | `dmc->sample_buffer` | `nesturbator__map_cpu_read(nes, dmc->address)` resolves the loaded cartridge mapper/PRG data | Yes | ✓ FLOWING |
| `src/bus.c:oam_dma` | OAM transfer byte | Per-byte `bus_read_data` from the selected page, then PPU `$2004` write | Yes | ✓ FLOWING |
| `tests/cpu/test_bus.c` | observed sample/address/phase events | The same production bus source under a compile-time test observer | Yes; synthetic bytes are explicit fixture inputs, not production data fallbacks | ✓ FLOWING |

## Behavioral Spot-Checks

The verifier did not run the application, tests, or probes, as requested. These rows use the already archived run and its raw outputs.

| Behavior | Existing evidence inspected | Result | Status |
|---|---|---|---|
| Five selected AccuracyCoin rows | `build/phase24-acceptance/20261101T010016Z/observed/accuracycoin/full/report.tsv` individual rows and its run identity record | Five terminal PASS results at the pinned build; implicit row is accepted `0x09` | ✓ PASS |
| Owned DMA event regressions | CTest entry 354/707, `bus.unit`, in archived `LastTest.log`; fixture source and registration inspected | `Test Passed`; source/destination counts and GET/PUT event output present | ✓ PASS |
| Native DMA continuation | CTest entry 237/707, `core.state_fork.stress`, in archived `LastTest.log`; test source inspected | `Test Passed`; silent, pending, collision, bus-conflict and abort cuts reported output/state equality | ✓ PASS |
| Cross-platform hashes and performance | `24-ACCEPTANCE.md` and `24-CI-RUN-20261101.md` | No current-head six-platform inventory or Cachegrind result | ? SKIP — acceptance evidence absent |

## Probe Execution

No separate shell probe is declared. Plan 24-01's `bus.unit --dma-trace remaining` mode is a diagnostic executable mode; it was not re-run under the no-tests instruction. Existing row traces are retained in `24-DMA-EVIDENCE.md` and the ignored build-tree trace files, while the default `bus.unit` regression ran in the archived full CTest workflow.

## Requirements Coverage

| Requirement | Source plan(s) | Description | Status | Evidence |
|---|---|---|---|---|
| DMA-01 | 24-01, 24-02, 24-03, 24-04 | Five selected rows pass at the pinned AccuracyCoin build and declared target | ✓ SATISFIED | Individual rows in the complete local report are PASS; the current implementation sources match the tested behavior14 source commit. |
| DMA-02 | 24-01, 24-02, 24-03, 24-04 | Separate source-backed cycle evidence and owned wrong-model controls | ✓ SATISFIED | Five causal records, event fixtures, named controls and source identity are present and connected to the passing `bus.unit` test. |
| DMA-03 | 24-02, 24-03, 24-04 | Protected PASS keys, deterministic cross-platform outputs and existing performance limit | ✗ BLOCKED | Local protected observations used a fallback baseline; independent cold-main authority, current-head hosted results, six-platform hash equality and Cachegrind `+1%` evidence are absent. |
| DMA-04 | 24-01, 24-02, 24-03, 24-04 | Native save/load continuation and explicit format/revision and documentation effects | ✓ SATISFIED | Format 6 / behavior revision 14 / ABI 1 disposition, state visitor, public-boundary forks, old-format rejection and public comments are present. |

All IDs found in each of the four plan frontmatters exist in `REQUIREMENTS.md`; all four are mapped to Phase 24. No additional Phase 24 requirement is orphaned. The roadmap contains no later phase that explicitly schedules the missing regression, platform-hash, or performance evidence, so no gap is deferred.

## Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|---|---|---|---|---|
| `README.md` | 44 | “Fresh full-ROM acceptance of this revision is still pending” | ⚠️ Warning | A complete local full-ROM campaign now exists. The broader phase acceptance is still incomplete for independent protected-main, hosted platform/hash and performance evidence, so this sentence is stale or ambiguous and should distinguish local acceptance from those remaining gates. |
| `24-ACCEPTANCE.md` | — | Existing branch-wide `git diff --check main...HEAD` reports 159 inherited whitespace diagnostics across 50 files | ⚠️ Warning | The scoped acceptance delta is recorded clean, but the branch-wide planning/history hygiene check remains unresolved. This is not a DMA behavior failure and was not bulk-rewritten. |

The phase implementation files contain no `TBD`, `FIXME`, `XXX`, `TODO`, `HACK`, or placeholder implementations. The `return NULL` matches in test helpers are ordinary lookup/allocation failure paths, not stubs. The archived full local workflow reports all 707 CTest entries passing.

## Escalation Gate

The deferred Plan 24-04 human check remains open for the remote boundary: the owner must separately authorize any hosted PR/check trigger before current-head hosted lanes can run. Expected evidence is independent cold protected-main authority, Linux/macOS/Windows x64/arm64 frame/PCM/transition hash equality, and Cachegrind within the unchanged `+1%` allowance. A missing remote artifact cannot be replaced by local test evidence.

Plan 24-04 also carries three unresolved prohibition records, each lacking its required `verification` tier. The available source and run records show no observed violation: the local run had explicit local CMake authority and no hosted action occurred; documentation bounds claims to pinned compatibility and an inferred early-G model; and the exact-format/no-converter policy is documented. These prohibitions remain flagged `UNRESOLVED` and are not counted as verified until a human resolves their tier/status:

1. Do not infer renewed campaign or hosted-trigger authority from scope/planning approval.
2. Do not present pinned tests, emulator agreement, or deterministic hashes as silicon proof or a universal RP2A03G result.
3. Do not imply an old-format converter, behavior-revision load enforcement, or compatibility beyond the documented exact-format contract.

## Gaps Summary

The local implementation and its five selected ROM rows are verified against the pinned test contract. The phase remains incomplete because DMA-03's independent protected-main, six-platform determinism, hosted current-head, and unchanged performance evidence is absent. The inherited branch-wide whitespace diagnostics are also still open. No subsequent milestone phase claims these results.

---

_Verified: 2026-11-01T01:34:41Z_  
_Verifier: the agent (gsd-verifier)_
