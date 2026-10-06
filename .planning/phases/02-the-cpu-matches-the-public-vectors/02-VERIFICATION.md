---
phase: 02-the-cpu-matches-the-public-vectors
verified: 2026-10-06T17:43:41Z
status: human_needed
score: 58/62 must-haves verified
covered_files:
  - .github/workflows/ci.yml
  - .github/workflows/nightly.yml
  - .github/workflows/release.yml
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-01-PLAN.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-01-SUMMARY.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-02-PLAN.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-02-SUMMARY.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-03-PLAN.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-03-SUMMARY.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-04-PLAN.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-04-SUMMARY.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-05-PLAN.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-05-SUMMARY.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-06-PLAN.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-06-SUMMARY.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-07-PLAN.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-07-SUMMARY.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-08-PLAN.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-08-SUMMARY.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-09-PLAN.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-09-SUMMARY.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-10-PLAN.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-10-SUMMARY.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-11-PLAN.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-11-SUMMARY.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-REVIEW-FIX.md
  - CMakeLists.txt
  - CMakePresets.json
  - PROVENANCE.md
  - README.md
  - THIRD-PARTY-NOTICES.md
  - include/nesturbator.h
  - release-please-config.json
  - scripts/phase2_outcomes.sh
  - src/bus.c
  - src/cpu.c
  - src/instance.c
  - src/internal.h
  - tests/CMakeLists.txt
  - tests/cmake/fetch_guard.cmake
  - tests/cmake/fetch_vectors.cmake
  - tests/cmake/manifest_sha256.cmake
  - tests/cmake/nightly_workflow_policy.cmake
  - tests/cmake/release_config.cmake
  - tests/cmake/vector_api_policy.cmake
  - tests/cmake/vector_registration_policy.cmake
  - tests/cmake/vector_result_policy.cmake
  - tests/cmake/vector_source_policy.cmake
  - tests/cmake/vectors_fixture.cmake
  - tests/cmake/vectors_full_run.cmake
  - tests/cmake/vectors_regen.cmake
  - tests/cmake/vectors_sample_match.cmake
  - tests/core/test_api.c
  - tests/core/test_profile.c
  - tests/cpu/test_bus.c
  - tests/cpu/test_cpu_unit.c
  - tests/cpu/test_vectors.c
  - tests/cpu/vector_bus.c
  - tests/cpu/vector_bus.h
  - tests/embed/CMakeLists.txt
  - tests/hygiene/CMakeLists.txt
  - tests/hygiene/scan_selftest.sh
  - tests/roms/manifest.txt
  - tests/vectors/65x02-sample.n65v
  - tests/vectors/fixtures/02-first3.json
  - tests/vectors/fixtures/README.md
  - tests/vectors/fixtures/a9-first3.json
  - tests/vectors/fixtures/a9-tail.json
  - tests/vectors/n65v.c
  - tests/vectors/n65v.h
  - tests/vectors/pins.txt
  - tests/vectors/test_n65v.c
  - tools/vecconv/CMakeLists.txt
  - tools/vecconv/vecconv.c
covered_digest: "v3:sha256:4bc9a8bdc68a81eca11c717ba8100fb6eec82a7612ec36a51a59ec14f203b3ad"
behavior_unverified: 0
overrides_applied: 0
human_verification:
  - test: "After the owner-authorized merge, run scripts/phase2_outcomes.sh <merge-sha> v0.1.1 and confirm release publication."
    expected: "A non-draft v0.1.1 release exists with exactly the 18 library, runner and libretro archives plus nonempty SHA256SUMS, and its release workflow run succeeded on the tag commit descended from the merge."
    why_human: "The merge and release have not occurred; a local repository cannot provide the identity-bound hosted release outcome."
  - test: "After merge, use scripts/phase2_outcomes.sh <merge-sha> v0.1.1 to collect the main-push full-vector run."
    expected: "The run is tied to the exact merge SHA, vectors-full succeeds, and its artifact records the exact 258-test inventory and completed passing JUnit results."
    why_human: "The main-push workflow has not run for the merge commit; its hosted result is pending and was not counted as passed."
  - test: "After the first scheduled run following merge, use scripts/phase2_outcomes.sh <merge-sha> v0.1.1 to collect the scheduled full-vector result."
    expected: "A scheduled run on a descendant of the merge has all 258 vector checks passing with valid run-bound evidence, and the scheduled reporter job completes."
    why_human: "No post-merge scheduled run exists yet; its hosted result is pending and was not counted as passed."
  - test: "Owner confirms whether any GPL or LGPL emulator source was opened or copied while implementing the CPU."
    expected: "The clean-room requirement was followed; no GPL or LGPL emulator source was consulted or copied."
    why_human: "Source-opening history is an identity-bound judgment that repository contents and automated scanners cannot establish."
---

# Phase 2: The CPU matches the public vectors — Verification Report

**Phase Goal:** The 6502 behaves as the public 65x02 vectors say on every opcode and every bus cycle, and the release that merging publishes carries it.
**Verified:** 2026-10-06T17:43:41Z
**Status:** human_needed
**Re-verification:** No — the previous report had no `gaps:` section; this is an initial verification against the current roadmap and all 11 plans.

## Goal Achievement

### Roadmap Success Criteria

| # | Success criterion | Status | Evidence |
|---|---|---|---|
| 1 | CI's `vectors` label passes committed samples for all 256 opcodes, final state and every bus cycle. | ✓ VERIFIED | Fresh `cmake --workflow --preset ci` completed 327/327 when run outside the restricted sandbox. It rebuilt `src/cpu.c`; all 256 `cpu.vectors.00`–`.ff` tests passed. The harness compares registers, raw P, RAM pairs, cycle count and each bus-cycle address/value/kind. Mutation evidence in the prior verification shows cycle/value mutations fail. |
| 2 | `vectors-full` fetches the pinned set and every vector matches; CI schedules it nightly. | ✓ VERIFIED (implementation); scheduled hosted outcome pending | Fresh `cmake --workflow --preset vectors-full` fetched/checked the pinned data and passed 258/258, including fetch, sample-match and all 256 10,000-vector opcode tests. `nightly.yml` has the scheduled trigger and invokes the full preset; hygiene policy tests enforce no cache/skip and the workflow wiring. The first scheduled post-merge result remains pending and is listed for human verification, not counted as a pass. |
| 3 | `ci`, `asan`, `nofp`, `hygiene` pass with the CPU in the library; merge publishes a release. | ? UNCERTAIN — release event pending | Fresh local results: CI 327/327 (including RetroArch after leaving the restricted sandbox), ASan 326/326, no-FP 5/5, hygiene 8/8. The six-platform workflow matrix and packaging are wired; prior CI evidence covered the phase CPU matrix. The owner-authorized merge/release has not occurred, so the merge-published release clause is unresolved rather than passed. See the pending release item. |

### Plan must-have truths

All 62 truths declared across the 11 plans were assessed. The table summarizes truth status per plan; the four unresolved hosted-event truths are identified separately below.

| Plan | Truths verified | Truths total | Evidence summary |
|---|---:|---:|---|
| 02-01 | 7 | 7 | Converter/reader fixtures pass; C-owned parser policy, host-only target, release pin guard, frame regressions and original branch/history condition checked. |
| 02-02 | 5 | 5 | Shared shipped CPU object and test bus wiring; bus ticks/mirroring/open-bus unit tests; vector tracer and ABI/no-FP checks pass. |
| 02-03 | 7 | 7 | Sample manifest/hash, exact 256 chunk structure, crafted truncation/rejection tests, regeneration path and MIT provenance verified. |
| 02-04 | 5 | 5 | Official opcode sample and addressing/cycle adjacency behavior passed through active value-comparison tests. |
| 02-05 | 5 | 5 | Official control/RMW/stack behavior, raw status bits and JSR cycle order exercised by sample and named unit tests. |
| 02-06 | 9 | 9 | All 256 opcodes run; unofficial/JAM/profile/store cases and JSR overlap unit tests passed; no opcode waiver is used. |
| 02-07 | 6 | 7 | Full set and pinned file hashes passed locally; offline failure, exact sample match, registration and workflow policy verified. The first scheduled reporter execution remains pending. |
| 02-08 | 6 | 8 | PR/full-vector wiring and timeout configuration verified; the post-merge release and first scheduled main run remain pending. |
| 02-09 | 3 | 3 | Source dependency policy, API declaration baseline, JSON size and exact no-skip registration checks pass in CI. |
| 02-10 | 2 | 3 | Nightly workflow security/policy gates pass. Main-push run-bound inventory/JUnit outcome remains pending until that hosted event occurs. |
| 02-11 | 3 | 3 | Release assertion, pending-aware read-only collector, evidence result validator and automation-first handoff rule are implemented and wired; local self-test covers success, pending, wrong SHA, missing assets/artifact and result mutations. |

**Score:** 58/62 plan truths verified; 4 hosted-event truths await post-merge evidence. `behavior_unverified: 0` — no state transition or cancellation invariant lacks a test.

### Prohibitions

| Plan | Must-not condition | Status | Evidence |
|---|---|---|---|
| 01 | No external JSON library, Python, or CMake JSON parsing of vector inputs. | ✓ VERIFIED | `vectors.source_policy` and its mutation self-test pass; `vecconv` is C. |
| 02 | No public API declaration change; CPU reached through internal hook. | ✓ VERIFIED | `vectors.api_policy` and mutation self-test pass; `cpu.vectors` links the CPU object directly. |
| 02 | No GPL/LGPL emulator source opened or copied. | ? UNCERTAIN — owner judgment | No repository scan can establish source-opening history; human item above. |
| 03 | Do not commit the 1.08 GB upstream JSON set. | ✓ VERIFIED | Hygiene and JSON-size policy/self-test pass; only committed small fixtures and the N65V sample are tracked. |
| 06 | No hidden opcode exclusion/waiver list. | ✓ VERIFIED | 00–ff tests are required by the registration policy; both sample and full inventories pass, and all opcode tests passed. |
| 07 | Nightly cannot pass through cache or skipped fetch. | ✓ VERIFIED | Nightly policy and self-test pass; cold pinned fetch passed; the policy rejects cache/skip behavior. |
| 07 | No workflow writeback/secrets outside scoped issue reporting. | ✓ VERIFIED | Workflow policy, permissions inspection and action pin checks pass. |

## Required Artifacts

| Artifact | Expected | Status | Details |
|---|---|---|---|
| `src/cpu.c`, `src/bus.c`, `src/internal.h` | CPU instruction implementation, bus seam and internal state | ✓ VERIFIED | Substantive; CPU object is folded into `nesturbator` and separately linked into `cpu.vectors` with the test bus. Current sample/full vector runs prove flow through the object. |
| `tests/cpu/test_vectors.c`, `tests/cpu/vector_bus.c` | Compare final state, RAM and every bus cycle | ✓ VERIFIED | CTest labels register exactly 256 opcodes per tier; each run passed. |
| `tools/vecconv/vecconv.c`, `tests/vectors/n65v.c` | JSON conversion and bounds-checked N65V reader | ✓ VERIFIED | Converter re-reads output through `n65v_next`; fixtures and negative-input tests pass. Converter is excluded from installed and embedded targets. |
| `tests/vectors/65x02-sample.n65v`, `tests/vectors/pins.txt`, manifest | Provenanced sample plus pinned full-set hashes | ✓ VERIFIED | Manifest/hash checks pass; fresh full run validates the upstream pin and byte-identical sample prefix. |
| `tests/cmake/fetch_vectors.cmake`, `tests/cmake/fetch_guard.cmake` | Pinned fetch and source-tree protection | ✓ VERIFIED | Current `vectors.fetch.guard` passes symlink/case spelling checks; script only deletes its marked `65x02-src` tree. |
| `.github/workflows/nightly.yml`, `CMakePresets.json` | Scheduled full-vector lane and evidence artifacts | ✓ VERIFIED (wiring) | Schedule, main push, preset, exact inventory and run-bound artifact/JUnit wiring verified; hosted post-merge outcomes remain pending. |
| `tests/cmake/vector_source_policy.cmake`, `vector_api_policy.cmake`, `vector_registration_policy.cmake`, `nightly_workflow_policy.cmake` | Automated prohibition and inventory guards | ✓ VERIFIED | Registered in CI/hygiene and exercised by live and mutation tests. |
| `.github/workflows/release.yml`, `scripts/phase2_outcomes.sh`, `tests/cmake/vector_result_policy.cmake` | Publication assertion and commit-bound outcome query | ✓ VERIFIED (implementation) | Workflow checks exactly 18 archives plus `SHA256SUMS`; collector uses read-only GitHub queries and distinguishes pending (exit 2) from failure (exit 1). Self-test passes. Remote release outcome remains pending. |
| `tests/CMakeLists.txt` in plan 02-05 artifact query | Plan named `NESTURBATOR_OFFICIAL_OPCODES` | ✓ VERIFIED (superseded naming) | Query's literal pattern is absent because later plan 02-06 replaced the 151-opcode list with `NESTURBATOR_ALL_OPCODES`; current registration checker requires all `00`–`ff` tests and passes. This is not a stub. |

`gsd-tools query verify.artifacts` passed all current artifacts except the expected plan-02-05 naming mismatch above (the superseded variable). `gsd-tools query verify.key-links` reported 15 direct pattern matches; its generic target matcher did not understand several function/test invocations and GitHub API targets. Those were traced manually: test registrations invoke the policy scripts; workflow jobs invoke the vectors preset and metadata checks; the result collector calls `gh` release, run and artifact APIs. No substantive artifact is orphaned.

## Key Link Verification

| From | To | Via | Status | Details |
|---|---|---|---|---|
| `CMakeLists.txt` | `src/cpu.c` | `nesturbator_cpu` object folded into `nesturbator` | ✓ WIRED | `target_sources(... $<TARGET_OBJECTS:nesturbator_cpu>)`; confirmed by CI build and library symbols. |
| `tests/CMakeLists.txt` | CPU object + `vector_bus.c` | `cpu.vectors` target | ✓ WIRED | Test target links `$<TARGET_OBJECTS:nesturbator_cpu>` and test bus, not the library bus. |
| `tools/vecconv/vecconv.c` | `tests/vectors/n65v.c` | `n65v_next` re-read | ✓ WIRED | Executed by converter tests. |
| `tests/CMakeLists.txt` | committed sample | 256 labeled test registrations | ✓ WIRED | All 256 ran and passed in current CI. |
| `cpu.vectors-full.fetch` | `pins.txt` + fetched files | CTest fixture then SHA-256/size validation | ✓ WIRED | Fetch and all dependent vector tests passed. |
| `nightly.yml` | `vectors-full` preset | scheduled/main triggers launch full workflow | ✓ WIRED | Source trigger and job command verified; hosted post-merge run is pending. |
| Hygiene/CI CTest registrations | policy scripts | direct `cmake -P` invocations | ✓ WIRED | Live policy tests passed in CI/hygiene; mutation self-tests passed. |
| `release.yml` | published GitHub release | upload, publish, bounded exact metadata assertion | ✓ WIRED | Workflow source and syntax verified; no live release exists yet. |
| `scripts/phase2_outcomes.sh` | GitHub APIs | `gh` read-only release/run/artifact queries | ✓ WIRED | Self-test confirms pending/success/failure handling; no real post-merge query was made. |
| `.planning/preparation/GSD-HANDOFF.md` | project UAT constraints | documents automated checks before owner prompts | ✓ WIRED | Source references project Verification and Hand-offs constraints. |

## Data-Flow Trace (Level 4)

| Artifact | Data variable | Source | Produces real data | Status |
|---|---|---|---|---|
| `cpu.vectors.<xx>` | expected CPU state/cycles | Committed N65V sample; manifest SHA-256 and sample provenance | Yes | ✓ FLOWING |
| `cpu.vectors-full.<xx>` | expected CPU state/cycles | 256 JSON files from the pinned upstream commit, individually size/hash checked | Yes | ✓ FLOWING |
| `vectors-full` archived evidence | test inventory/JUnit | CTest registration and completed CTest result on the same workflow run | Yes when the host run occurs | ✓ FLOWING (hosted run pending) |

## Behavioral Spot-Checks

| Behavior | Command | Result | Status |
|---|---|---|---|
| 256 sample opcodes match final state and every cycle | `cmake --workflow --preset ci` | 327/327 passed outside the restricted sandbox; 256 vector cases passed | ✓ PASS |
| All 2,560,000 pinned vectors match | `cmake --workflow --preset vectors-full` | Fetch, sample-match and all 256 opcode tests passed; 258/258 | ✓ PASS |
| JAM loop, JSR overlap, bus timing and CPU unit behavior | `cmake --workflow --preset ci` | `cpu.unit` and `bus.unit` passed; explicit transition/order tests are present | ✓ PASS |
| Output identity and workflows' release/vector outcomes | `scripts/phase2_outcomes.sh --self-test` | Success, pending, wrong-SHA, missing-artifact/assets, failed and skipped result cases passed | ✓ PASS |

## Probe Execution

No phase-declared or conventional `scripts/*/tests/probe-*.sh` probes were found.

## Test Quality Audit

| Test File | Linked requirement | Active | Skipped | Circular | Assertion level | Verdict |
|---|---|---:|---:|---:|---|---|
| `tests/cpu/test_vectors.c` | CPU-01/02 | Yes | 0 | No | Value + per-cycle behavioral | ✓ ADEQUATE |
| `tests/vectors/test_n65v.c` | CPU-01/02 | Yes | 0 | No | Value and malformed-input behavior | ✓ ADEQUATE |
| `tests/cpu/test_cpu_unit.c`, `tests/cpu/test_bus.c` | CPU-01 | Yes | 0 | No | Ordered multi-cycle assertions | ✓ ADEQUATE |
| `tests/cmake/vector_result_policy.cmake` and self-test | CPU-02 evidence | Yes | 0 | No | Exact inventory and completed JUnit values | ✓ ADEQUATE |

Disabled requirement tests: 0. Circular expected-value generation detected: 0. Expected vector values come from the pinned external MIT data; `vectors_sample_match` verifies the committed prefix against fetched data. No test-quality blocker found.

## Requirements Coverage

| Requirement | Source plans | Description | Status | Evidence |
|---|---|---|---|---|
| CPU-01 | 02-01 through 02-09, 02-11 | All 256 opcodes match committed 65x02 vectors on final state and every bus cycle | ✓ SATISFIED | Fresh CI sample pass, exact 256 registrations, harness comparisons and CPU object wiring. |
| CPU-02 | 02-07 through 02-11 | Full pinned 65x02 set matches and is run in the nightly lane | ✓ SATISFIED (implementation); hosted outcomes pending | Fresh pinned full run 258/258; schedule/main triggers and full preset are wired. Pending post-merge hosted outcomes are explicit human items, not evidence of a pass. |

No orphaned requirements: `REQUIREMENTS.md` assigns only CPU-01 and CPU-02 to Phase 2, and plans declare both.

## Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|---|---:|---|---|---|
| None | — | No unreferenced TBD/FIXME/XXX, TODO/HACK/PLACEHOLDER or stub return in the phase implementation/test/workflow files | — | No blocker found |

The initial hygiene run flagged a personal workspace path in the review-fix note. It was replaced with “Main checkout”; `sh scripts/hygiene.sh --tree` and the full hygiene preset then passed. The review-fix note is committed. The user-reserved `.planning/HANDOFF.json` deletion and `.planning/config.json` modification were preserved.

## Decision Coverage

The decision-coverage query found 24 trackable CONTEXT decisions and 24 honored by plans, summaries or shipped artifacts. This warning-only gate does not affect status.

## Human Verification Required

1. **Published release:** After the owner-authorized merge, run `scripts/phase2_outcomes.sh <merge-sha> v0.1.1`. Confirm a successful release run on the tag commit and the exact 18 platform archives plus `SHA256SUMS`.
2. **Main-push full-vector result:** Use the same command after merge. Confirm the run is tied to the exact merge SHA and that its 258-test inventory and completed JUnit artifact pass validation.
3. **First scheduled nightly and reporter:** After the first scheduled run following merge, use the same command. Confirm 258 passing checks on a descendant commit and a completed report job.
4. **Clean-room provenance:** The owner confirms no GPL/LGPL emulator source was consulted or copied. The repository cannot establish an individual's source-opening history.

## Gaps Summary

No implementation gap was found in the CPU or vector pipeline. The committed sample and the freshly fetched pinned full set pass all final-state and bus-cycle checks. CI, ASan, no-FP and hygiene pass locally; the CI rerun required leaving the restricted sandbox for RetroArch, whose process aborted with empty output inside that sandbox. The review's CR-01 asset ordering, WR-01 pending classification and WR-02 timeout findings are fixed and locally exercised. The earlier source-tree deletion hazard is also covered by the current fetch guard and its passing symlink/case self-test.

Status is `human_needed` because the merge-bound release, exact-merge main-push run and first scheduled nightly have not occurred, and clean-room provenance requires an owner judgment. None of these pending items is marked passed.

---

_Verified: 2026-10-06T17:43:41Z_
_Verifier: the agent (gsd-verifier)_
