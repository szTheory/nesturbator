---
phase: 02-the-cpu-matches-the-public-vectors
verified: 2026-10-09T13:17:06Z
status: passed
score: 62/62 must-haves verified
covered_files: [".gitattributes", ".github/workflows/ci.yml", ".github/workflows/nightly.yml", ".github/workflows/release.yml", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-01-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-01-SUMMARY.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-02-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-02-SUMMARY.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-03-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-03-SUMMARY.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-04-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-04-SUMMARY.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-05-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-05-SUMMARY.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-06-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-06-SUMMARY.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-07-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-07-SUMMARY.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-08-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-08-SUMMARY.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-09-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-09-SUMMARY.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-10-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-10-SUMMARY.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-11-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-11-SUMMARY.md", "CMakeLists.txt", "CMakePresets.json", "PROVENANCE.md", "README.md", "THIRD-PARTY-NOTICES.md", "include/nesturbator.h", "libretro/libretro.c", "release-please-config.json", "scripts/hygiene.sh", "scripts/phase2_outcomes.sh", "src/bus.c", "src/cpu.c", "src/instance.c", "src/internal.h", "tests/CMakeLists.txt", "tests/cmake/fetch_vectors.cmake", "tests/cmake/manifest_sha256.cmake", "tests/cmake/nightly_workflow_policy.cmake", "tests/cmake/pins_check.cmake", "tests/cmake/vecconv_negative.cmake", "tests/cmake/vector_api_policy.cmake", "tests/cmake/vector_registration_policy.cmake", "tests/cmake/vector_result_policy.cmake", "tests/cmake/vector_source_policy.cmake", "tests/cmake/vectors_fixture.cmake", "tests/cmake/vectors_full_run.cmake", "tests/cmake/vectors_regen.cmake", "tests/cmake/vectors_sample_match.cmake", "tests/core/test_api.c", "tests/core/test_profile.c", "tests/cpu/test_bus.c", "tests/cpu/test_cpu_unit.c", "tests/cpu/test_vectors.c", "tests/cpu/vector_bus.c", "tests/cpu/vector_bus.h", "tests/embed/CMakeLists.txt", "tests/hygiene/CMakeLists.txt", "tests/hygiene/scan_selftest.sh", "tests/roms/manifest.txt", "tests/vectors/65x02-sample.n65v", "tests/vectors/fixtures/02-first3.json", "tests/vectors/fixtures/README.md", "tests/vectors/fixtures/a9-first3.json", "tests/vectors/fixtures/a9-tail.json", "tests/vectors/n65v.c", "tests/vectors/n65v.h", "tests/vectors/pins.txt", "tests/vectors/test_n65v.c", "tools/vecconv/CMakeLists.txt", "tools/vecconv/vecconv.c"]
covered_digest: "v3:sha256:500b4a3cf076a2797d4b17bafa044dbf6ab1a57ba8b99b0e99a19f901e8e01f7"
behavior_unverified: 0
overrides_applied: 2
overrides:
  - must_have: "On a scheduled run, the `report` job opens or edits one rolling issue labelled `nightly` with the run URL and the failing `65x02/<xx>` keys, and closes it on success (D-24). It runs only on `schedule`, so it is first exercised after the merge"
    reason: "Owner-authorized manual post-merge dispatch 37531643137 on descendant main SHA c3e96382ae991d18f69f88096611d42e6a076e0e passed all 258 registered tests and its run-bound inventory/JUnit artifact passed vector_result_policy.cmake. The reporter independently succeeded on exact-merge main-push run 37509491346. Workflow policy checks cover the scheduled trigger and reporting condition. No scheduled event is claimed."
    accepted_by: "owner"
    accepted_at: "2026-10-06T21:12:01Z"
  - must_have: "The first scheduled nightly on main passes, and the report job runs with issues: write"
    reason: "Owner-authorized manual post-merge dispatch 37531643137 on descendant main SHA c3e96382ae991d18f69f88096611d42e6a076e0e passed all 258 registered tests and its run-bound inventory/JUnit artifact passed vector_result_policy.cmake. The reporter independently succeeded on exact-merge main-push run 37509491346. Workflow policy checks cover the scheduled trigger and permission scope. No scheduled event is claimed."
    accepted_by: "owner"
    accepted_at: "2026-10-06T21:12:01Z"
re_verification:
  previous_status: passed
  previous_score: 62/62
  gaps_closed: []
  gaps_remaining: []
  regressions: []
advisory:
  - finding: "Review noted single-sample audio callback handling in libretro/libretro.c; its attribution to Phase 2 could not be established because historical task commit IDs were unreachable."
    category: other
    reason: "The review classified this as Phase 4 audio behavior, outside the Phase 2 CPU/vector/release requirements. No deterministic failing test or reproduction was supplied for this verification."
    evidence_status: "review-only; no deterministic reproduction provided"
---

# Phase 2: The CPU matches the public vectors — Verification Report

**Phase Goal:** The 6502 behaves as the public 65x02 vectors say on every opcode and every bus cycle, and the release that merging publishes carries it.
**Verified:** 2026-10-09T13:17:06Z
**Status:** passed
**Re-verification:** Yes — current code and gate evidence were checked against all 11 plan contracts and the prior 62/62 verification.

## Goal Achievement

### Roadmap Success Criteria

| # | Success criterion | Status | Evidence |
|---|---|---|---|
| 1 | CI runs committed vectors for all 256 opcodes and compares final state and every bus cycle. | ✓ VERIFIED | Current supplied `cmake --workflow --preset ci` regression result: 354/356 passed, 2 host-integration tests skipped because the host integration was unavailable; no failures. CPU vector tests are registered as `cpu.vectors.00` through `.ff` in `tests/CMakeLists.txt`. `tests/cpu/test_vectors.c` compares final CPU state, final RAM pairs, cycle count and each ordered bus cycle against committed N65V data. |
| 2 | Full pinned vector set matches and CI runs it nightly. | ✓ VERIFIED (two scheduled-event-only plan truths PASSED by accepted override) | `nightly.yml` wires scheduled, main-push and manual dispatch triggers to `vectors-full`; fetch policy checks enforce the pinned cold fetch. UAT records exact-merge push run 37509491346 and manual post-merge run 37531643137; the latter's archived exact 258-test inventory/JUnit evidence passed `vector_result_policy.cmake`. The first cron event was not observed, as recorded in the owner-approved overrides below. |
| 3 | CI, ASan, no-FP and hygiene presets pass across six platforms with CPU in the library, and merge publishes a release. | ✓ VERIFIED | Current local workflow completed package production with no failed tests. Prior hosted evidence and completed UAT document the six-platform CI, ASan, no-FP and hygiene results, published v0.1.1 with 18 platform archives plus SHA256SUMS, and exact merge/tag ancestry. Current run's two skips were `retroarch.testframe` and `retroarch.game`, both unavailable host integration; this does not affect CPU vector coverage. |

### Plan Must-Haves

All 62 truths from plans 02-01 through 02-11 were rechecked against the actual files and wiring. Current plan artifact/link query results are included below; where generic patterns miss an indirect connection or an intentionally superseded artifact, the live source path was inspected. No observable must-have failed.

| Plan | Result | Evidence |
|---|---:|---|
| 02-01 | 7/7 | C converter, shared bounded N65V reader and reread path exist; host-tool isolation, release pin and unchanged frame-hash checks are present. |
| 02-02 | 5/5 | CPU object is folded into the library; the vector harness links that same object against its test bus; bus implementation and CPU unit harness are substantive. |
| 02-03 | 7/7 | Committed 256-chunk sample, manifest/hash, pinned MIT notice, CMake regeneration path and malformed-input tests are present. |
| 02-04 | 5/5 | Official instruction/addressing logic is in `src/cpu.c`; vector tests are dynamically registered against the sample and compare state and cycle traces. |
| 02-05 | 5/5 | RMW, branch, stack, interrupt and JSR ordering truths are represented in CPU code and covered by sample vector harness. The old `NESTURBATOR_OFFICIAL_OPCODES` symbol is not present: plan 02-06 replaced it with generated all-opcode registration, which includes all 151 official opcodes and the full 256 opcode set. |
| 02-06 | 9/9 | CPU switch contains 256 opcode labels; all-opcode vector registration has no waiver list; profile, JAM, unstable opcode and JSR-overlap behavior have substantive unit/vector coverage. |
| 02-07 | 7/7 | Pinned full-vector fetch, manifest and sample-match artifacts are wired into `vectors-full`; scheduled-only reporter behavior is accepted by owner override. |
| 02-08 | 8/8 | Six-platform CI matrix, ASan/no-FP lanes, PR full-vector workflow and published release are supported by workflow and hosted/UAT evidence; scheduled-only backstop is accepted by owner override. |
| 02-09 | 3/3 | Parser/API/JSON-size and exact no-skip registration policies are present and registered. |
| 02-10 | 3/3 | Nightly security policy and exact main-push inventory/JUnit artifact/report wiring are present. |
| 02-11 | 3/3 | Release metadata assertion and read-only commit-bound outcome collector are present; UAT records publication and run-bound vectors evidence. |

**Score:** 62/62 must-haves verified, including 2 PASSED (override). `behavior_unverified: 0`.

### Prohibitions

| Prohibition | Status | Evidence |
|---|---|---|
| Do not use external JSON libraries, Python or CMake JSON parsing for vectors. | ✓ VERIFIED | Converter is C; parser source policy is registered. |
| Do not change public declarations in Phase 2. | ✓ VERIFIED | API policy checks enforce the Phase 1 declaration baseline. |
| Do not commit the full upstream JSON corpus. | ✓ VERIFIED | Only manifest-listed small fixtures and the N65V sample are present; hygiene includes size policy. |
| Do not exclude/waive opcode vectors. | ✓ VERIFIED | Generated 00–ff registration, exact registration policy and vector test harness. |
| No GPL/LGPL emulator source was consulted or copied. | ✓ VERIFIED (owner judgment) | UAT records the owner's provenance confirmation, 4/4 items complete. |
| Full-vector workflow cannot pass by caching/skipping fetch. | ✓ VERIFIED | Nightly policy and hygiene enforce cold fetch/no skip/no privileged trigger; post-merge full run succeeded. |
| Nightly permissions/writeback stay scoped. | ✓ VERIFIED | Root permissions empty; issue-write permission is scoped to reporting job and policy checked. |

## Required Artifacts

| Artifact | Expected | Status | Details |
|---|---|---|---|
| `src/cpu.c`, `src/bus.c`, `src/internal.h` | CPU implementation, bus seam and instance state | ✓ VERIFIED | Substantive CPU implementation and bus functions; CMake includes `nesturbator_cpu` object in the shipped library. |
| `tests/cpu/test_vectors.c`, `tests/cpu/vector_bus.c` | CPU vector execution and ordered bus trace comparison | ✓ VERIFIED | Harness calls `nesturbator__cpu_step`, supplies test bus hooks, and compares end state, RAM, cycle count and bus accesses. |
| `tools/vecconv/vecconv.c`, `tests/vectors/n65v.c` | Owned converter and shared bounded reader | ✓ VERIFIED | Converter writes N65V and reads it back through shared reader; malformed input paths are covered by tests. |
| `tests/vectors/65x02-sample.n65v`, `tests/vectors/pins.txt` | Committed sample and full-set pins | ✓ VERIFIED | Manifest-backed sample and pinned full inputs; prior run artifacts/UAT validate full-tier results. |
| `.github/workflows/ci.yml`, `.github/workflows/nightly.yml`, `.github/workflows/release.yml` | Cross-platform CI, full-vector lane and publish assertion | ✓ VERIFIED | Workflow paths and permission/trigger policy are wired; UAT records successful hosted outcomes and release. |
| Policy checks and `scripts/phase2_outcomes.sh` | Exact inventory, no-skip and release/run evidence validation | ✓ VERIFIED | Policy checks are registered; collector traces to `gh` release/run/artifact APIs and local result validator. |
| `tests/CMakeLists.txt` | CPU vectors and policy tests registered | ✓ VERIFIED | 256 sample tests generated by nested hex loops; full tier adds fetch/sample-match plus 256 tests. |

Plan-artifact query: plans 02-01, 02-02, 02-03, 02-04, 02-06 through 02-11 pass. Plan 02-05's only generic artifact mismatch is the superseded `NESTURBATOR_OFFICIAL_OPCODES` name; its intent is fulfilled by all-opcode generation in plan 02-06. Key-link query false negatives are pattern limitations: registration policy is invoked from CTest through CMake script commands, and `phase2_outcomes.sh` calls `gh release`, `gh api`, and `gh run` directly. Manual source trace confirmed these paths are connected.

## Key Link Verification

| From | To | Via | Status | Details |
|---|---|---|---|---|
| `CMakeLists.txt` | `src/cpu.c` | `nesturbator_cpu` object linked into library | ✓ WIRED | `target_sources(nesturbator PRIVATE $<TARGET_OBJECTS:nesturbator_cpu>)`. |
| `tests/CMakeLists.txt` | CPU object and `vector_bus.c` | `cpu.vectors` target | ✓ WIRED | Harness links the same CPU object with test hooks, not the library bus. |
| `tests/CMakeLists.txt` | committed sample | `cpu.vectors.<xx>` generation | ✓ WIRED | Nested hex loops generate 00–ff tests; each command receives the sample and opcode. |
| `tools/vecconv/vecconv.c` | `tests/vectors/n65v.c` | shared reader roundtrip | ✓ WIRED | Converter calls shared N65V reader on output. |
| `tests/CMakeLists.txt` | `tests/cmake/fetch_vectors.cmake` | fixture setup | ✓ WIRED | Fetch setup gates full-tier sample match and opcode tests. |
| `.github/workflows/nightly.yml` | `CMakePresets.json` | `cmake --workflow --preset vectors-full` | ✓ WIRED | Workflow configures and runs the full preset; run-specific evidence is uploaded. |
| Hygiene CTest | vector/nightly policy scripts | CMake script invocation | ✓ WIRED | Policy checks are added as CTest tests in hygiene/test CMake files. |
| `scripts/phase2_outcomes.sh` | GitHub release/run/artifact APIs | `gh` queries and result validator | ✓ WIRED | Direct calls to `gh release`, `gh api`, `gh run`; archived inventory and JUnit checked by CMake policy. |

## Data-Flow Trace (Level 4)

| Artifact | Data | Source | Produces real data | Status |
|---|---|---|---|---|
| `cpu.vectors.<xx>` | Expected state and cycles | Manifest-hashed committed N65V sample | Yes | ✓ FLOWING |
| `cpu.vectors-full.<xx>` | Expected state and cycles | Individually pinned fetched vector files | Yes | ✓ FLOWING |
| Run-bound full-vector evidence | Exact names and results | Same-run CTest inventory and JUnit | Yes | ✓ FLOWING — validated in hosted UAT run |
| Release assets | Platform archives and checksums | CI-produced release artifacts | Yes | ✓ FLOWING — published release inventory verified |

## Behavioral Spot-Checks

| Behavior | Command/evidence | Result | Status |
|---|---|---|---|
| CI regression and CPU vectors | Supplied `cmake --workflow --preset ci` invocation | 354/356 passed, 2 skipped (`retroarch.testframe`, `retroarch.game`) because host integration was unavailable; no failures and packaging completed. No tests were rerun by this verifier. | ✓ PASS |
| Exact-merge full vectors and reporter | UAT run 37509491346 | `vectors-full` and report succeeded at the phase merge SHA. | ✓ PASS |
| Post-merge full-vector backstop | UAT run 37531643137 plus archived inventory/JUnit validation | 258 exact tests completed successfully; result policy exited 0. Reporter was skipped for dispatch by design. | ✓ PASS (manual substitute) |
| Release publication | UAT evidence for v0.1.1 | Published release, exact assets, tag ancestry, CI/attestation/tamper/publish steps verified. | ✓ PASS |

## Probe Execution

No phase-declared probes or conventional `scripts/*/tests/probe-*.sh` probes are declared by the phase plans or summaries.

## Test Quality Audit

| Test File/Set | Linked requirement | Active | Skipped | Circular | Assertion level | Verdict |
|---|---|---:|---:|---:|---|---|
| `tests/cpu/test_vectors.c` and 256 sample vector cases | CPU-01 | Yes | 0 | No | Value-level final state and ordered bus cycles against external vector data | ✓ ADEQUATE |
| Full-tier vector cases and `vector_result_policy.cmake` | CPU-02 | Yes | 0 in archived result | No | Exact test inventory and completed JUnit result validation | ✓ ADEQUATE |
| `tests/vectors/test_n65v.c`, converter and bus tests | CPU-01, CPU-02 | Yes | 0 | No | Value and malformed-input behavior | ✓ ADEQUATE |
| Parser/API/size/registration/nightly policy tests | CPU-01, CPU-02 | Yes | 0 | No | Mutation-sensitive policy behavior | ✓ ADEQUATE |

Requirement-linked disabled tests: 0. Circular expected-value generation: 0; expected CPU values come from pinned public 65x02 vector data. Insufficient assertions: 0. Current CI's two skips are host integration tests and do not cover CPU vector checks.

## Requirements Coverage

| Requirement | Source plans | Description | Status | Evidence |
|---|---|---|---|---|
| CPU-01 | 02-01 through 02-06, 02-08, 02-09, 02-11 | All opcodes match committed 65x02 vectors on final state and every bus cycle | ✓ SATISFIED | 256 generated active tests, direct value/ordered-cycle comparisons, CPU object wiring and current regression gate with no failures. |
| CPU-02 | 02-07 through 02-11 | Pinned full 65x02 set matches and CI runs it nightly | ✓ SATISFIED (two exact scheduled-event-only plan truths PASSED by owner override) | Pinned cold fetch and schedule policy, exact merge full-vector run, post-merge manual run with validated 258-test artifact, and UAT 4/4. Cron itself is not claimed. |

No orphaned requirements: REQUIREMENTS.md maps only CPU-01 and CPU-02 to Phase 2, and both appear in the phase plans.

### Decision Coverage

All trackable CONTEXT.md decisions are honored by shipped artifacts: 24/24; none were unhonored. The coverage query was non-blocking.

## Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|---|---:|---|---|---|
| None | — | No unreferenced TBD/FIXME/XXX, placeholder, empty implementation or disconnected data flow in the phase implementation/policy scan. Nullable `return NULL` paths are normal control flow, not stubs. | — | No blocker |

## Advisory (New Scope, Unevidenced)

| # | Finding | Category | Why Advisory |
|---|---|---|---|
| 1 | Review noted single-sample libretro audio callback handling in `libretro/libretro.c`. | other | Review could not establish historical task attribution because commit IDs were unreachable. The finding concerns Phase 4 audio behavior, outside CPU/vector/release truths; no failing test or reproducible artifact was provided in this verification. |

## Human Verification Required

N/A — this is a core-library and CI/release infrastructure phase with no user-facing feature requiring manual evaluation. All required owner judgment is recorded in the completed UAT. No behavior-dependent truth remains untested; the two current CI skips are unavailable RetroArch host integration tests, and historical hosted evidence/UAT covers the release gate.

## Gaps Summary

No unresolved Phase 2 gaps. The current regression gate had 354 passes and two host-integration skips, with no failures and packaging complete. Archived hosted evidence and UAT independently cover the six-platform/release and full-vector requirements. The first scheduled cron event itself remains unobserved; that exact backstop expectation is transparently accepted through two owner-approved overrides and is not represented as a cron run. The separate Phase 4 audio review note remains advisory and does not contradict the Phase 2 goal.

---

_Verified: 2026-10-09T13:17:06Z_  
_Verifier: the agent (gsd-verifier)_
