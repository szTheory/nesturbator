---
phase: 02-the-cpu-matches-the-public-vectors
verified: 2026-10-06T20:56:59Z
status: human_needed
score: 60/62 must-haves verified
covered_files: [".github/workflows/ci.yml", ".github/workflows/release.yml", "scripts/phase2_outcomes.sh", "tests/CMakeLists.txt", "tests/cmake/vector_result_policy.cmake"]
covered_digest: "v3:sha256:f1009c704d2e6b7a6f831ed7b175dab785fe287bf9c4555da26417de37a89870"
behavior_unverified: 0
overrides_applied: 0
re_verification:
  previous_status: human_needed
  previous_score: 58/62
  gaps_closed:
    - "Published v0.1.1 release has the exact assets and a passing release run descended from the Phase 2 merge."
    - "The exact-merge main-push full-vector run and its evidence artifact validate."
    - "The owner answered pass for clean-room source provenance on 2026-10-06."
  gaps_remaining:
    - "First post-merge scheduled nightly run and its reporter result have not occurred; automated evidence is pending."
  regressions: []
human_verification:
  - test: "After the first scheduled nightly run following merge, rerun scripts/phase2_outcomes.sh ea55b1f74b5e60ef088a4048bd15fcb2568e44fc v0.1.1."
    expected: "The collector reports a scheduled run on a descendant of the merge with all 258 vector checks passing and valid run-bound inventory/JUnit evidence; the reporter job completes."
    why_human: "This is an external scheduled GitHub Actions event that has not started. The wait is automated and needs no owner decision or intervention; a human must only review the eventual evidence."
---

# Phase 2: The CPU matches the public vectors — Verification Report

**Phase Goal:** The 6502 behaves as the public 65x02 vectors say on every opcode and every bus cycle, and the release that merging publishes carries it.
**Verified:** 2026-10-06T20:56:59Z
**Status:** human_needed
**Re-verification:** Yes — refreshed the previous human-needed report against post-merge release/vector evidence and the owner’s clean-room response.

## Goal Achievement

### Roadmap Success Criteria

| # | Success criterion | Status | Evidence |
|---|---|---|---|
| 1 | CI runs the committed vector sample for all 256 opcodes and compares final state and every bus cycle. | ✓ VERIFIED | The host-access `cmake --workflow --preset ci` result supplied for this verification passed 329/329, including all 256 `cpu.vectors.00`–`.ff`. I independently reran the same command in the restricted environment: build succeeded, 328/329 passed, and `retroarch.testframe` aborted without output. The supplied host-access rerun resolved that environment-specific failure. `tests/cpu/test_vectors.c` compares PC, S/A/X/Y/raw P, final RAM pairs, cycle count, and every cycle's address/value/kind. |
| 2 | Full vectors are fetched at the pin, every vector matches, and CI runs the suite nightly. | ✓ VERIFIED (implementation and exact-merge evidence); first scheduled run pending | The exact-merge GitHub run `37509491346` has successful `vectors-full` and `report` jobs. The read-only collector downloaded and validated its run-bound exact 258-test inventory and JUnit result. The workflow retains the scheduled trigger, cold pinned fetch and no-skip policy. The first post-merge cron execution is not yet available and remains a future automated evidence wait. |
| 3 | CI, ASan, no-FP and hygiene pass with CPU in the library on six platforms, and merge publishes a release. | ✓ VERIFIED | The supplied host-access CI regression passed 329/329 with packaging complete; recorded CI evidence covers six platform legs, with the CPU object linked into `nesturbator`. Previous phase evidence records ASan, no-FP and hygiene passes; current CI rerun also passes its policy and CPU tests. `scripts/phase2_outcomes.sh ea55b1f74b5e60ef088a4048bd15fcb2568e44fc v0.1.1` verified the published non-draft v0.1.1 inventory (18 platform archives plus `SHA256SUMS`) and release run `37513567481`, whose successful CI, attestations, tamper check and publish preceded an obsolete metadata assertion failure. The collector classifies that narrowly verified false negative as PASS. |

### Observable Truths and Plan Must-Haves

All 62 truths across plans 02-01 through 02-11 were reviewed against current files and evidence. The previous report recorded 58/62 with four post-merge items unresolved. Release publication and exact-merge main-push vector evidence are now resolved; the owner also answered the clean-room provenance judgment. Two scheduled-run truths remain pending until the first post-merge cron execution, so 60/62 plan truths are verified.

| Plan | Verified | Total | Verification summary |
|---|---:|---:|---|
| 02-01 | 7 | 7 | Converter/reader, bounds and malformed input behavior, host-tool isolation, release pin guard, and frame hash regression checks. |
| 02-02 | 5 | 5 | Shared CPU object, two bus hooks, vector test bus, bus tests, library link and no-FP policy. |
| 02-03 | 7 | 7 | Sample bytes, 256 opcode chunks, pin/hash/provenance, regeneration path, parser and reader rejection tests. |
| 02-04 | 5 | 5 | Official load/store/ALU/compare/flag opcodes and addressing/cycle behavior checked by sample vectors. |
| 02-05 | 5 | 5 | Remaining official control, stack and RMW cases, status bits and JSR ordering checked. |
| 02-06 | 9 | 9 | All 256 opcode cases; unofficial/JAM/profile/store behavior; raw status, no waiver list, JSR overlap and JAM unit tests. |
| 02-07 | 6 | 7 | Full pinned set and sample provenance passed; cold fetch and nightly policy are wired. The scheduled report behavior is re-observed below through the exact-merge main-push reporter; schedule-specific execution remains separately pending. |
| 02-08 | 7 | 8 | CI matrix, measured timeouts, PR full-vector lane, release and first-main-run claims checked against post-merge release and exact-merge workflow evidence. |
| 02-09 | 3 | 3 | Parser/API/JSON-size and exact no-skip registration policies are registered and pass in current CI. |
| 02-10 | 3 | 3 | Nightly policy, exact-commit main-push run evidence, artifact and reporter wiring verified; direct live run shows both jobs succeeded. |
| 02-11 | 3 | 3 | Release assertion and outcome collector verified, including success/pending/failure handling. The first scheduled backstop event is pending. |

**Score:** 60/62 truths verified; 1 scheduled external event pending. `behavior_unverified: 0` — no state-transition or cancellation/cleanup/ordering invariant is left without a behavioral test.

### Prohibitions

| Prohibition | Status | Evidence |
|---|---|---|
| No external JSON library, Python or CMake JSON parsing for vector data. | ✓ VERIFIED | `vectors.source_policy` and its mutation test passed in current CI; `vecconv` is implemented in C. |
| No public API declaration change in Phase 2. | ✓ VERIFIED | `vectors.api_policy` and its mutation self-test passed. |
| No GPL/LGPL emulator source opened or copied. | ✓ VERIFIED (owner judgment) | Owner answered `pass` on 2026-10-06; captured in `02-UAT.md`. |
| Do not commit the 1.08 GB upstream JSON set. | ✓ VERIFIED | Hygiene and JSON-size checks passed; only the manifest-listed fixtures and sample are tracked. |
| No opcode exclusion/waiver list. | ✓ VERIFIED | Exact 00–ff registration policy and all 256 sample vector tests passed. |
| Nightly cannot pass through cache or skipped fetch. | ✓ VERIFIED | Nightly policy/self-tests, current hygiene lane and cold pinned full-vector run passed. |
| Nightly has no writeback/secrets beyond scoped issue reporting. | ✓ VERIFIED | Workflow policy passed; `nightly.yml` scopes `issues: write` to `report`, and live report job succeeded. |

## Required Artifacts

| Artifact | Expected | Status | Details |
|---|---|---|---|
| `src/cpu.c`, `src/bus.c`, `src/internal.h` | CPU implementation and internal bus/state seam | ✓ VERIFIED | Substantive opcode switch and helpers; CPU object is folded into the library and separately linked with the test bus. Current CI rebuilds it and passes all sample vectors. |
| `tests/cpu/test_vectors.c`, `tests/cpu/vector_bus.c` | Final-state, RAM and per-cycle comparison | ✓ VERIFIED | Active harness compares every asserted field and cycle. CI passed all 256 registered opcode tests. |
| `tools/vecconv/vecconv.c`, `tests/vectors/n65v.c` | C converter and bounded N65V reader | ✓ VERIFIED | Converter re-reads output through shared reader; conversion and malformed-input tests passed. |
| `tests/vectors/65x02-sample.n65v`, `tests/vectors/pins.txt` | Provenanced sample and pinned full-set hashes | ✓ VERIFIED | CI sample tests and exact-merge full run passed; full run artifact passed exact inventory/JUnit validation. |
| `tests/cmake/fetch_vectors.cmake`, `tests/cmake/vector_registration_policy.cmake` | Pinned fetch/source-tree guard and exact inventory policy | ✓ VERIFIED | Guard and registration policy/self-test passed; full workflow recorded exactly 258 required tests. |
| `.github/workflows/nightly.yml`, `CMakePresets.json` | Scheduled and main-push full-vector lanes | ✓ VERIFIED (wiring); first scheduled event pending | Workflow has scheduled/main triggers, cold fetch, run-bound evidence and scoped issue reporter. Exact-merge main push passed; separate cron run not yet started. |
| `.github/workflows/release.yml`, `scripts/phase2_outcomes.sh`, `tests/cmake/vector_result_policy.cmake` | Publication assertion and read-only evidence collector | ✓ VERIFIED | Current code and self-tests pass. The collector independently queried the live release and exact-merge full-vector artifact; pending schedule returns exit 2. |
| `tests/CMakeLists.txt` and policy scripts | Policy tests registered in CI/hygiene | ✓ VERIFIED | Registration, result, outcome, API/source policies are registered and passed in the 329-test host-access CI evidence. |

The generic artifact query passed 10 of 11 plans. Its sole mismatch is the legacy plan-02-05 artifact `NESTURBATOR_OFFICIAL_OPCODES`, superseded in plan 02-06 by all-opcode registration; the current policy requires exactly 00–ff. Generic key-link checks passed 7 of 11 plans. The reported misses in plans 02-09 through 02-11 are pattern-matcher limitations: the policy target is invoked by CTest through `${CMAKE_CURRENT_SOURCE_DIR}` and the GitHub API endpoints are dynamic `gh` arguments. Manual trace confirmed each caller and invocation; no link is orphaned.

## Key Link Verification

| From | To | Via | Status | Details |
|---|---|---|---|---|
| `CMakeLists.txt` | `src/cpu.c` | CPU object folded into `nesturbator` | ✓ WIRED | Top-level target includes `$<TARGET_OBJECTS:nesturbator_cpu>`; current build links the library. |
| `tests/CMakeLists.txt` | CPU object and `vector_bus.c` | `cpu.vectors` target | ✓ WIRED | Harness links CPU object and test bus, and current CTest executes 256 opcode cases. |
| `tools/vecconv/vecconv.c` | `tests/vectors/n65v.c` | shared `n65v_next` re-read | ✓ WIRED | Converter and reader tests pass. |
| `tests/CMakeLists.txt` | committed sample | 256 `cpu.vectors.<xx>` tests | ✓ WIRED | All 256 ran and passed in host-access CI. |
| full-vector fixture | `pins.txt` and fetched files | cold fetch then size/hash verification | ✓ WIRED | Exact-merge full-vector lane and evidence validation passed. |
| `nightly.yml` | `vectors-full` preset | scheduled and main-push workflow | ✓ WIRED | Exact-merge run passed `vectors-full`; its `report` job also completed successfully. First cron result is still pending. |
| `release.yml` | published GitHub release | upload, attest, publish, metadata assertion | ✓ WIRED | Live release contains exact expected assets; outcome collector validated release run and tag ancestry. |
| `scripts/phase2_outcomes.sh` | GitHub release/run/artifact APIs | read-only `gh` calls and local evidence validator | ✓ WIRED | Live exact-SHA results validated; schedule is distinctly reported PENDING (exit 2). |

The `verify.key-links` query's dynamic-target misses above were manually resolved by tracing the policy CTest registration and the collector's `gh release`, `gh run` and `gh api` commands into their consumers.

## Data-Flow Trace (Level 4)

| Artifact | Data variable | Source | Produces real data | Status |
|---|---|---|---|---|
| `cpu.vectors.<xx>` | expected CPU state and cycles | committed N65V sample, checked against manifest | Yes | ✓ FLOWING |
| `cpu.vectors-full.<xx>` | expected CPU state and cycles | individually pinned upstream files, fetched at fixed commit | Yes | ✓ FLOWING |
| full-vector workflow artifact | registration and completed test results | same-run CTest inventory and JUnit | Yes | ✓ FLOWING — exact-merge artifact validated |
| release assets | platform archives and sums | CI-produced package artifacts | Yes | ✓ FLOWING — live published inventory verified |

## Behavioral Spot-Checks

| Behavior | Command/evidence | Result | Status |
|---|---|---|---|
| Build library/runner/libretro and pass CI tests | `cmake --workflow --preset ci` (host-access result supplied) | 329/329 passed; packaging completed, including 256 CPU vector tests and RetroArch frame test | ✓ PASS |
| Restricted-environment comparison | `cmake --workflow --preset ci` (independent rerun here) | 328/329; only `retroarch.testframe` aborted with empty stdout/stderr in restricted execution | Environment failure, superseded by host-access pass |
| Exact merge full vector inventory and JUnit | `scripts/phase2_outcomes.sh ea55b1f74b5e60ef088a4048bd15fcb2568e44fc v0.1.1` | Exact-merge run `37509491346` passed; collector validated 258 names and completed passing JUnit | ✓ PASS |
| Exact-merge reporter job | `gh run view 37509491346 --repo szTheory/nesturbator --json headSha,status,conclusion,jobs,url` | `vectors-full` and `report` jobs both completed successfully on merge SHA `ea55b1f74b5e60ef088a4048bd15fcb2568e44fc` | ✓ PASS |
| Release publication | same evidence collector command | v0.1.1 published, exact 18 archives plus `SHA256SUMS`; release run `37513567481` passed the publication prerequisites; obsolete final assertion was its only failure | ✓ PASS (narrow collector exception) |
| Outcome policy | CI `phase2.outcomes.self_test` | success, pending, wrong SHA, missing assets/artifacts, and failure classifications passed | ✓ PASS |

## Probe Execution

No phase-declared or conventional `scripts/*/tests/probe-*.sh` probes were found.

## Test Quality Audit

| Test File/Set | Requirement | Active | Skipped | Circular | Assertion strength | Verdict |
|---|---|---:|---:|---:|---|---|
| `tests/cpu/test_vectors.c` and 256 CTest cases | CPU-01, CPU-02 | Yes | 0 | No | Value-level final state and ordered per-cycle behavior | ✓ ADEQUATE |
| `tests/vectors/test_n65v.c`, converter tests | CPU-01, CPU-02 | Yes | 0 | No | Value-level data validation and malformed-input behavior | ✓ ADEQUATE |
| `tests/cpu/test_cpu_unit.c`, `tests/cpu/test_bus.c` | CPU-01 | Yes | 0 | No | Explicit ordered multi-cycle assertions | ✓ ADEQUATE |
| `tests/cmake/vector_result_policy.cmake` | CPU-02 evidence integrity | Yes | 0 | No | Exact inventory/result names, no skips/failures | ✓ ADEQUATE |
| `tests/cmake/*policy*.cmake` mutation checks | CPU-01, CPU-02 guardrails | Yes | 0 | No | Deliberate bad inputs must be rejected | ✓ ADEQUATE |

Disabled requirement tests: 0. Circular expected-value generation: 0; vector expectations originate from the pinned external MIT data, and sample-match checks committed bytes against the pinned source. Insufficient assertions: 0.

## Requirements Coverage

| Requirement | Source plans | Description | Status | Evidence |
|---|---|---|---|---|
| CPU-01 | 02-01 through 02-09, 02-11 | All 256 opcodes match committed 65x02 vectors on final state and every bus cycle | ✓ SATISFIED | CI host-access pass, 256 active tests, direct harness comparisons and CPU-object wiring. |
| CPU-02 | 02-07 through 02-11 | Full pinned 65x02 set matches and is run in the nightly lane | ✓ SATISFIED (scheduled future observation pending) | Exact-merge full run passed with validated run-bound evidence; schedule and issue-report workflow are wired and exact-merge report job passed. The next scheduled run remains a future evidence wait. |

No orphaned requirements. `REQUIREMENTS.md` maps only CPU-01 and CPU-02 to Phase 2; both are declared in phase plans.

## Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|---|---:|---|---|---|
| None | — | No unreferenced TBD/FIXME/XXX, placeholder, empty implementation or stub data flow found in phase implementation/test/workflow files | — | No blocker |

The current hygiene tree check passed. The independent sandbox `ci` attempt's RetroArch abort is reported above as an environment-specific run failure, not hidden; host-access CI passed the same test.

## Advisory (New Scope, Unevidenced)

Not applicable — this report refresh is based on a previous `human_needed` report with no `gaps:` block; no new-scope anti-pattern blocker was raised.

## Automated Evidence Pending

The first scheduled nightly after merge has not started. The recorded schedule is 2026-10-07 04:17 UTC. This is a third-party automated wait, not an owner-only judgment and requires no owner action. After it runs, the read-only collector will validate the descendant run, all 258 checks, the artifact/JUnit results and reporter completion. It is not counted as passed here.

## Human Verification Required

1. **Review the first scheduled nightly evidence after it exists.** Run `scripts/phase2_outcomes.sh ea55b1f74b5e60ef088a4048bd15fcb2568e44fc v0.1.1` after the scheduled run.
   - **Expected:** the collector reports the scheduled descendant run and reporter as passing, with exact 258-test evidence.
   - **Why human:** the future GitHub schedule has not occurred. The run and report are automated; this item asks only for review of the resulting evidence. The clean-room provenance question is resolved by the owner's 2026-10-06 `pass` in `02-UAT.md` and does not need another owner response.

## Gaps Summary

The CPU goal is achieved in code and the release carries the implementation. The committed sample passed all 256 opcodes with final-state and per-cycle comparisons. The pinned full suite passed on the exact merge commit and its run-bound artifact passed the evidence validator; v0.1.1 is published with the required archives and sums. Current host-access CI passed 329/329 and packaged successfully. A restricted-sandbox rerun independently reproduced only the known RetroArch subprocess abort; the supplied host-access rerun passed it.

Status remains `human_needed` because one non-inferable hosted backstop observation—the first post-merge scheduled nightly and its reporter—does not yet exist. It is a future automated evidence wait, with no owner action needed. The separately owner-only clean-room judgment has already passed.

---

_Verified: 2026-10-06T20:56:59Z_
_Verifier: the agent (gsd-verifier)_
