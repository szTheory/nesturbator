---
phase: 02-the-cpu-matches-the-public-vectors
verified: 2026-10-06T21:15:26Z
status: passed
score: 62/62 must-haves verified
covered_files: [".github/workflows/ci.yml", ".github/workflows/nightly.yml", ".github/workflows/release.yml", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-01-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-01-SUMMARY.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-02-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-02-SUMMARY.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-03-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-03-SUMMARY.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-04-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-04-SUMMARY.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-05-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-05-SUMMARY.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-06-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-06-SUMMARY.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-07-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-07-SUMMARY.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-08-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-08-SUMMARY.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-09-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-09-SUMMARY.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-10-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-10-SUMMARY.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-11-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-11-SUMMARY.md", "scripts/phase2_outcomes.sh", "tests/CMakeLists.txt", "tests/cmake/nightly_workflow_policy.cmake", "tests/cmake/vector_result_policy.cmake"]
covered_digest: "v3:sha256:05eb84fda9a73e9c4118a041e38f5f41f3bb80ab3c0ffd781a9a27f54d5c46e5"
behavior_unverified: 0
overrides_applied: 2
overrides:
  - must_have: "On a scheduled run, the `report` job opens or edits one rolling issue labelled `nightly` with the run URL and the failing `65x02/<xx>` keys, and closes it on success (D-24). It runs only on `schedule`, so it is first exercised after the merge"
    reason: "The owner authorized a manual post-merge dispatch instead of waiting for cron. The full-vector job and run-bound artifact passed on descendant main SHA c3e96382ae991d18f69f88096611d42e6a076e0e; the reporter independently completed successfully on the exact-merge main-push run 37509491346. Workflow policy tests verify the schedule trigger and report condition. No scheduled event is claimed: cron itself was intentionally not awaited."
    accepted_by: "owner"
    accepted_at: "2026-10-06T21:12:01Z"
  - must_have: "The first scheduled nightly on main passes, and the report job runs with issues: write"
    reason: "The owner authorized a manual post-merge dispatch instead of waiting for cron. Run 37531643137 on descendant main SHA c3e96382ae991d18f69f88096611d42e6a076e0e completed vectors-full successfully and its exact 258-test inventory/JUnit artifact passed vector_result_policy.cmake. The report job was correctly skipped for workflow_dispatch; the reporter independently succeeded on exact-merge main-push run 37509491346. Workflow policy tests verify schedule and permission scoping. No scheduled event is claimed: cron itself was intentionally not awaited."
    accepted_by: "owner"
    accepted_at: "2026-10-06T21:12:01Z"
re_verification:
  previous_status: human_needed
  previous_score: 60/62
  gaps_closed:
    - "The post-merge full-vector backstop is now evidenced by owner-authorized manual workflow_dispatch run 37531643137 on a descendant main SHA; artifact policy validates all 258 registered and completed passing tests."
    - "The reporter path is independently evidenced by its successful completion on exact-merge main-push run 37509491346."
    - "Owner-requested substitution closes the two exact scheduled-event-only plan backstops; the scheduled trigger remains wired and policy-tested."
  gaps_remaining: []
  regressions: []
---

# Phase 2: The CPU matches the public vectors — Verification Report

**Phase Goal:** The 6502 behaves as the public 65x02 vectors say on every opcode and every bus cycle, and the release that merging publishes carries it.
**Verified:** 2026-10-06T21:15:26Z
**Status:** passed
**Re-verification:** Yes — the prior human-needed backstop was reviewed against the completed UAT and owner-approved manual dispatch.

## Goal Achievement

### Roadmap Success Criteria

| # | Success criterion | Status | Evidence |
|---|---|---|---|
| 1 | CI runs committed vectors for all 256 opcodes and compares final state and every bus cycle. | ✓ VERIFIED | Supplied host-access `cmake --workflow --preset ci` passed 329/329 with packaging complete. The 256 `cpu.vectors.00`–`.ff` checks are registered in `tests/CMakeLists.txt`; `tests/cpu/test_vectors.c` checks PC, S/A/X/Y/raw P, final RAM pairs, cycle count, and each ordered bus cycle. |
| 2 | The full pinned vector set matches and CI runs it nightly. | ✓ VERIFIED (implementation and run evidence; scheduled-only plan backstops overridden) | `nightly.yml` contains the `17 4 * * *` schedule and cold pinned fetch; nightly policy tests guard trigger, fetch and reporter conditions. Manual `workflow_dispatch` run 37531643137 passed `vectors-full` on descendant main SHA `c3e96382ae991d18f69f88096611d42e6a076e0e`. The downloaded `vectors-full-evidence-37531643137` artifact passed `tests/cmake/vector_result_policy.cmake` (exit 0), validating the exact 258-test inventory and completed passing JUnit results. This is a manual substitute, not an observed cron run. |
| 3 | CI, ASan, no-FP and hygiene pass on six platforms with CPU in the library, and merge publishes a release. | ✓ VERIFIED | Host-access CI passed 329/329 and packaged; recorded hosted CI evidence covers six platform legs, with CPU in the library. The earlier phase checks record ASan, no-FP and hygiene passes. Published v0.1.1 has the exact 18 archives plus `SHA256SUMS`; release run 37513567481 passed CI, attestations, tamper check and publish, with only its obsolete metadata assertion failing. Collector identifies that narrow known false negative; merged release assets and ancestry are verified. |

### Observable Truths and Plan Must-Haves

All 62 plan truths across 02-01 through 02-11 were rechecked using the prior verification contract, current code/policy files, available automated evidence and the updated UAT. The two remaining items were the exact scheduled-event-only backstops in plans 02-07 and 02-08. They are PASSED (override) under the owner's explicit acceptance: full vectors/artifact on a post-merge manual dispatch, reporter success on the exact-merge main-push, and static policy evidence for the scheduled path. The UAT is now complete (4/4 pass).

| Plan | Verified or PASSED (override) | Total | Evidence summary |
|---|---:|---:|---|
| 02-01 | 7 | 7 | C converter/shared bounds-checked reader, malformed-input behavior, host-tool isolation, release pin guard and frame-hash checks. |
| 02-02 | 5 | 5 | CPU object and two bus hooks, vector test bus, bus tests, library linkage and no-FP checks. |
| 02-03 | 7 | 7 | Sample bytes, 256 chunks, pin/hash/provenance, regeneration, parser/reader rejection tests and licence notice. |
| 02-04 | 5 | 5 | Official load/store/ALU/compare/flag instructions, addressing and cycle behavior checked against vectors. |
| 02-05 | 5 | 5 | Control, stack and RMW instructions, status bits and JSR ordering checked against vectors. |
| 02-06 | 9 | 9 | All 256 opcodes, unofficial/JAM/profile/store behavior, raw status, no waiver list and JAM/JSR unit tests. |
| 02-07 | 7 | 7 | Full pinned set and sample provenance, cold-fetch/nightly policy, plus scheduled reporter truth accepted by owner override. |
| 02-08 | 8 | 8 | CI matrix/timeouts, PR full-vector lane, published release and manual post-merge backstop; exact scheduled-run truth accepted by owner override. |
| 02-09 | 3 | 3 | Parser/API/JSON-size and exact no-skip registration policies are registered and passed. |
| 02-10 | 3 | 3 | Nightly policy, exact-commit main-push evidence/artifact and reporter wiring; live reporter job succeeded. |
| 02-11 | 3 | 3 | Release assertion and read-only outcome collector verified; the collector still labels cron PENDING by design. |

**Score:** 62/62 truths verified, including 2 PASSED (override). `behavior_unverified: 0`.

### Prohibitions

| Prohibition | Status | Evidence |
|---|---|---|
| No external JSON library, Python or CMake JSON parser for vector data. | ✓ VERIFIED | `vectors.source_policy` and mutation checks passed; `vecconv` is C. |
| No public API declaration change in Phase 2. | ✓ VERIFIED | `vectors.api_policy` and its mutation check passed. |
| No GPL/LGPL emulator source opened or copied. | ✓ VERIFIED (owner judgment) | Owner's pass recorded in `02-UAT.md` on 2026-10-06. |
| Do not commit the 1.08 GB upstream JSON set. | ✓ VERIFIED | Hygiene/JSON-size checks pass; only manifest-listed fixtures and sample are tracked. |
| No opcode exclusion/waiver list. | ✓ VERIFIED | Exact 00–ff registration policy and all 256 sample tests passed. |
| Nightly cannot pass through cached or skipped fetch. | ✓ VERIFIED | Nightly policy/self-tests and hygiene guard cold pinned fetch; manual full run passed. |
| Nightly has no writeback/secrets beyond scoped issue reporting. | ✓ VERIFIED | Workflow policy checks permission scope; `issues: write` exists only in report; exact-merge reporter job succeeded. |

## Required Artifacts

| Artifact | Expected | Status | Details |
|---|---|---|---|
| `src/cpu.c`, `src/bus.c`, `src/internal.h` | CPU implementation and internal bus/state seam | ✓ VERIFIED | Substantive opcode implementation; CPU object is folded into library and separately linked with the test bus. CI passed all sample vectors. |
| `tests/cpu/test_vectors.c`, `tests/cpu/vector_bus.c` | Final-state, RAM and per-cycle comparison | ✓ VERIFIED | Harness compares every required field and cycle; all 256 opcode tests passed. |
| `tools/vecconv/vecconv.c`, `tests/vectors/n65v.c` | Owned C converter and bounded reader | ✓ VERIFIED | Converter re-reads output through shared reader; conversion and malformed-input tests passed. |
| `tests/vectors/65x02-sample.n65v`, `tests/vectors/pins.txt` | Provenanced sample and full-set hashes | ✓ VERIFIED | Sample CI and full-vector main-push/manual runs passed; downloaded evidence validated. |
| Fetch, full-run and registration policy scripts | Pinned fetch, exact test inventory, no-skip policy | ✓ VERIFIED | CI policy tests and remote inventory/JUnit evidence pass. |
| `.github/workflows/nightly.yml`, `CMakePresets.json` | Scheduled, manual and main-push vector lanes | ✓ VERIFIED | Workflow triggers and policy are checked; dispatch and exact-merge push runs both completed successfully. Cron itself was not observed and is covered by the accepted overrides for the plan's scheduled-only truths. |
| `.github/workflows/release.yml`, `scripts/phase2_outcomes.sh`, `tests/cmake/vector_result_policy.cmake` | Published release assertion and read-only evidence collector | ✓ VERIFIED | Collector confirms release and exact-merge run; manual artifact independently passed `vector_result_policy.cmake` with exit 0. |
| `tests/CMakeLists.txt` and policy scripts | Policies registered in CI/hygiene | ✓ VERIFIED | Host-access CI passed 329/329 including policy checks. |

## Key Link Verification

| From | To | Via | Status | Details |
|---|---|---|---|---|
| `CMakeLists.txt` | `src/cpu.c` | CPU object folded into `nesturbator` | ✓ WIRED | Target includes CPU object; CI rebuilt and linked it. |
| `tests/CMakeLists.txt` | CPU object and `vector_bus.c` | `cpu.vectors` target | ✓ WIRED | Harness links CPU object and test bus; all 256 cases ran. |
| `tools/vecconv/vecconv.c` | `tests/vectors/n65v.c` | shared `n65v_next` re-read | ✓ WIRED | Converter and reader tests pass. |
| `tests/CMakeLists.txt` | committed sample | 256 `cpu.vectors.<xx>` tests | ✓ WIRED | All 256 passed in host-access CI. |
| Full-vector fixture | `pins.txt` and fetched files | cold fetch then size/hash validation | ✓ WIRED | Main-push and manual-dispatch runs passed; artifacts validated. |
| `nightly.yml` | `vectors-full` preset | schedule, dispatch, PR and main-push workflow triggers | ✓ WIRED | Manual run 37531643137 passed; exact-merge run 37509491346 passed. `report` succeeded on push and was skipped by design on dispatch. |
| `release.yml` | published GitHub release | upload, attest, publish, metadata assertion | ✓ WIRED | Live v0.1.1 inventory and release workflow evidence verified. |
| `scripts/phase2_outcomes.sh` | GitHub release/run/artifact APIs | read-only `gh` calls and local validator | ✓ WIRED | Collector independently reports release and exact-merge pass; cron remains explicitly PENDING. |

The generic artifact query passed 10/11 plans; the sole legacy mismatch is plan 02-05's `NESTURBATOR_OFFICIAL_OPCODES`, superseded in 02-06 by all-opcode registration. Generic key-link checks passed 7/11 plans; remaining pattern mismatches were manually traced through dynamic CTest registration and `gh` API calls. No orphaned key link was found.

## Data-Flow Trace (Level 4)

| Artifact | Data variable | Source | Produces real data | Status |
|---|---|---|---|---|
| `cpu.vectors.<xx>` | expected CPU state and cycles | committed N65V sample tied to manifest | Yes | ✓ FLOWING |
| `cpu.vectors-full.<xx>` | expected CPU state and cycles | individually pinned upstream files | Yes | ✓ FLOWING |
| Full-vector workflow artifact | test registration and completed results | same-run CTest inventory and JUnit | Yes | ✓ FLOWING — main-push and dispatch artifacts validate |
| Release assets | platform archives and sums | CI-produced package artifacts | Yes | ✓ FLOWING — published inventory verified |

## Behavioral Spot-Checks

| Behavior | Command/evidence | Result | Status |
|---|---|---|---|
| Build library/runner/libretro and pass CI tests | Host-access `cmake --workflow --preset ci` | 329/329 passed; packaging completed, including 256 CPU vectors and RetroArch frame test | ✓ PASS |
| Exact-merge full-vector run and reporter | `gh run view 37509491346 --repo szTheory/nesturbator --json ...` | `vectors-full` and `report` succeeded on merge SHA `ea55b1f74b5e60ef088a4048bd15fcb2568e44fc` | ✓ PASS |
| Manual post-merge full-vector run | `gh run view 37531643137 --repo szTheory/nesturbator --json ...` | `workflow_dispatch`, success, descendant main SHA; `vectors-full` succeeded; `report` skipped by workflow condition | ✓ PASS (manual substitute) |
| Dispatch artifact result proof | `cmake -DINVENTORY=<artifact>/registered-tests.json -DJUNIT=<artifact>/vectors-full.junit.xml -P tests/cmake/vector_result_policy.cmake` | Downloaded run-bound artifact; exit 0, exact 258 names and completed passing results | ✓ PASS |
| Published release | `scripts/phase2_outcomes.sh ea55b1f74b5e60ef088a4048bd15fcb2568e44fc v0.1.1` | Release and exact-merge main-push PASS; scheduled event PENDING (exit 2) | ✓ PASS with scheduled-only plan truths overridden |

## Probe Execution

No phase-declared or conventional `scripts/*/tests/probe-*.sh` probes were found.

## Test Quality Audit

| Test File/Set | Linked requirement | Active | Skipped | Circular | Assertion level | Verdict |
|---|---|---:|---:|---:|---|---|
| `tests/cpu/test_vectors.c` and 256 CTest cases | CPU-01, CPU-02 | Yes | 0 | No | Value-level state and ordered per-cycle behavior | ✓ ADEQUATE |
| `tests/vectors/test_n65v.c`, converter tests | CPU-01, CPU-02 | Yes | 0 | No | Value-level format and malformed-input behavior | ✓ ADEQUATE |
| `tests/cpu/test_cpu_unit.c`, `tests/cpu/test_bus.c` | CPU-01 | Yes | 0 | No | Ordered multi-cycle assertions | ✓ ADEQUATE |
| `tests/cmake/vector_result_policy.cmake` | CPU-02 evidence integrity | Yes | 0 | No | Exact inventory/result names; rejects skips/failures | ✓ ADEQUATE |
| `tests/cmake/*policy*.cmake` mutation checks | CPU-01, CPU-02 | Yes | 0 | No | Bad inputs must be rejected | ✓ ADEQUATE |

Disabled requirement tests: 0. Circular expected-value generation: 0; vector expectations originate from pinned external MIT data. Insufficient assertions: 0.

## Requirements Coverage

| Requirement | Source plans | Description | Status | Evidence |
|---|---|---|---|---|
| CPU-01 | 02-01 through 02-09, 02-11 | All 256 opcodes match committed 65x02 vectors on final state and every bus cycle | ✓ SATISFIED | 329/329 host-access CI, 256 active tests, direct harness comparisons and CPU-object wiring. |
| CPU-02 | 02-07 through 02-11 | Full pinned 65x02 set matches and CI runs it nightly | ✓ SATISFIED (two exact scheduled-event-only plan statements PASSED by owner override) | Dispatch full run and validated artifact, exact-merge full run and reporter success, cron trigger and reporter conditions policy-tested. UAT records 4/4 passed. The first cron itself was not awaited. |

No orphaned requirements: `REQUIREMENTS.md` maps only CPU-01 and CPU-02 to Phase 2; both appear in the phase plans.

### Decision Coverage

All trackable CONTEXT.md decisions are honored by shipped artifacts: 24/24; none were unhonored. This is a non-blocking coverage check.

## Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|---|---:|---|---|---|
| None found | — | No unreferenced TBD/FIXME/XXX, placeholder, empty implementation or stub data flow found in the reviewed phase implementation and policy files | — | No blocker |

## Advisory (New Scope, Unevidenced)

None. The previous verification had no `gaps:` section; this pass rechecked its outstanding human-needed evidence contract and found no new-scope blocker.

## Human Verification Required

N/A — this phase delivers a core library, test harness and CI/release infrastructure with no user-facing elements. All remaining owner judgment is resolved in UAT, and there are no behavior-unverified truths. The scheduled-event-only plan wording is transparently handled by the two owner-approved overrides above.

## Gaps Summary

No unresolved phase gaps remain. The manual post-merge dispatch is a documented substitute for the first cron event, not evidence that a schedule event occurred. Its full-vector run passed on a descendant main SHA and its run-bound inventory/JUnit artifact passed the exact result validator. The reporter was independently exercised successfully on the exact-merge main-push. Static workflow policy checks cover the schedule trigger and report condition. The read-only collector continues to report the first scheduled event as PENDING, as designed; the two narrowly scoped plan truths that required the scheduled event are accepted overrides, not rewritten claims of cron execution.

---

_Verified: 2026-10-06T21:15:26Z_
_Verifier: the agent (gsd-verifier)_
