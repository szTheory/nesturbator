---
phase: 02-the-cpu-matches-the-public-vectors
verified: 2026-10-09T19:36:29Z
status: passed
score: 62/62 must-haves verified
covered_files: [".gitattributes", ".github/workflows/ci.yml", ".github/workflows/nightly.yml", ".github/workflows/release.yml", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-01-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-01-SUMMARY.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-02-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-02-SUMMARY.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-03-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-03-SUMMARY.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-04-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-04-SUMMARY.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-05-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-05-SUMMARY.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-06-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-06-SUMMARY.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-07-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-07-SUMMARY.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-08-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-08-SUMMARY.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-09-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-09-SUMMARY.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-10-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-10-SUMMARY.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-11-PLAN.md", ".planning/phases/02-the-cpu-matches-the-public-vectors/02-11-SUMMARY.md", "CMakeLists.txt", "CMakePresets.json", "PROVENANCE.md", "README.md", "THIRD-PARTY-NOTICES.md", "include/nesturbator.h", "libretro/libretro.c", "release-please-config.json", "scripts/hygiene.sh", "scripts/phase2_outcomes.sh", "src/bus.c", "src/cpu.c", "src/instance.c", "src/internal.h", "tests/CMakeLists.txt", "tests/cmake/fetch_vectors.cmake", "tests/cmake/manifest_sha256.cmake", "tests/cmake/nightly_workflow_policy.cmake", "tests/cmake/pins_check.cmake", "tests/cmake/vecconv_negative.cmake", "tests/cmake/vector_api_policy.cmake", "tests/cmake/vector_registration_policy.cmake", "tests/cmake/vector_result_policy.cmake", "tests/cmake/vector_source_policy.cmake", "tests/cmake/vectors_fixture.cmake", "tests/cmake/vectors_full_run.cmake", "tests/cmake/vectors_regen.cmake", "tests/cmake/vectors_sample_match.cmake", "tests/core/test_api.c", "tests/core/test_profile.c", "tests/cpu/test_bus.c", "tests/cpu/test_cpu_unit.c", "tests/cpu/test_vectors.c", "tests/cpu/vector_bus.c", "tests/cpu/vector_bus.h", "tests/embed/CMakeLists.txt", "tests/hygiene/CMakeLists.txt", "tests/hygiene/scan_selftest.sh", "tests/roms/manifest.txt", "tests/vectors/65x02-sample.n65v", "tests/vectors/fixtures/02-first3.json", "tests/vectors/fixtures/README.md", "tests/vectors/fixtures/a9-first3.json", "tests/vectors/fixtures/a9-tail.json", "tests/vectors/n65v.c", "tests/vectors/n65v.h", "tests/vectors/pins.txt", "tests/vectors/test_n65v.c", "tools/vecconv/CMakeLists.txt", "tools/vecconv/vecconv.c"]
covered_digest: "v3:sha256:a6a574c130edd05dece329b369886d87d0a09a06ed72e5e8a0f83c44e657607f"
behavior_unverified: 0
overrides_applied: 2
overrides:
  - must_have: "On a scheduled run, the `report` job opens or edits one rolling issue labelled `nightly` with the run URL and the failing `65x02/<xx>` keys, and closes it on success (D-24). It runs only on `schedule`, so it is first exercised after the merge"
    reason: "Owner-authorized post-merge manual run 37531643137 passed the exact 258-test inventory and completed JUnit validation; the reporter succeeded on exact-merge main-push run 37509491346. The scheduled trigger and conditions are statically policy-checked; a cron event was not observed."
    accepted_by: "owner"
    accepted_at: "2026-10-06T21:12:01Z"
  - must_have: "The first scheduled nightly on main passes, and the report job runs with issues: write"
    reason: "Owner-authorized post-merge manual run 37531643137 passed the exact 258-test inventory and completed JUnit validation; the reporter succeeded on exact-merge main-push run 37509491346. The scheduled trigger and permission scope are statically policy-checked; a cron event was not observed."
    accepted_by: "owner"
    accepted_at: "2026-10-06T21:12:01Z"
re_verification:
  previous_status: passed
  previous_score: 62/62
  gaps_closed: []
  gaps_remaining: []
  regressions: []
---

# Phase 2: The CPU matches the public vectors — Verification Report

**Phase Goal:** The 6502 behaves as the public 65x02 vectors say on every opcode and every bus cycle, and the release that merging publishes carries it.
**Verified:** 2026-10-09T19:36:29Z
**Status:** passed
**Re-verification:** Yes — the full contract was rechecked because the previous passing report's evidence and fingerprint were stale.

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|---|---|---|
| 1 | CI runs committed sample vectors for all 256 opcodes and matches final state and every bus cycle. | ✓ VERIFIED | `src/cpu.c` contains 256 opcode cases. `tests/cpu/test_vectors.c` compares PC, S/A/X/Y/P, final RAM, cycle count, and each ordered cycle's address/value/kind. `tests/CMakeLists.txt` generates 00–ff tests from the committed 256-chunk sample. Configured `cmake --workflow --preset ci` completed 357 tests: 355 passed, 2 RetroArch checks skipped because the runtime was unavailable, and none failed. |
| 2 | Full pinned vectors are fetched, all tests match, and CI runs the suite nightly. | ✓ VERIFIED (two post-merge scheduled-event assertions PASSED by accepted override) | `tests/vectors/pins.txt`, `fetch_vectors.cmake`, `vectors_sample_match.cmake`, and the full preset enforce the pin and sample identity. `nightly.yml` runs `vectors-full`; UAT records exact-merge main-push run 37509491346 and manual post-merge run 37531643137 with all 258 registered CTest entries and completed passing JUnit results checked by `vector_result_policy.cmake`. The cron event itself was not observed; the two scheduled-only assertions are explicitly overridden with owner acceptance above. |
| 3 | CI, ASan, no-FP and hygiene pass across six platforms; merging publishes a release carrying the implementation. | ✓ VERIFIED | Current CI workflow defines the six-platform build matrix and the asan/nofp/hygiene gates; the CPU object is linked into the library in `CMakeLists.txt`. `.planning/phases/02-the-cpu-matches-the-public-vectors/02-UAT.md` records release v0.1.1 publication after the authorized merge, exact 18 archives plus nonempty SHA256SUMS, passing release CI/attestation/tamper/publication, and release tag ancestry from the phase merge. |

**Score:** 62/62 plan truths verified (including 2 PASSED by accepted override). `behavior_unverified: 0`.

### Plan Must-Haves

All truths from plans 02-01 through 02-11 were enumerated from frontmatter (truth counts: 7, 5, 7, 5, 5, 9, 7, 8, 3, 3, 3) and checked against implementation, tests, wiring, and available run evidence.

| Plan | Result | Evidence |
|---|---:|---|
| 02-01 | 7/7 | Owned C converter and bounded N65V reader round-trip; host-tool isolation, release pin removal and unchanged frame-hash test paths are present. Artifact and link queries pass. |
| 02-02 | 5/5 | CPU object is included in the library; vector harness links that same object to test-bus hooks. Bus and vector test paths are substantive and registered. |
| 02-03 | 7/7 | Sample blob, manifest/hash, upstream notice, regeneration workflow, and malformed-input tests exist and are connected. |
| 02-04 | 5/5 | Addressing and official opcode implementation is exercised against vectors with registers, RAM, and bus-cycle comparisons. |
| 02-05 | 5/5 | CPU cases and behavior exist. The plan's `NESTURBATOR_OFFICIAL_OPCODES` list was deliberately superseded in 02-06 by generated all-256 registration; the exact symbol is absent, while every opcode is covered. |
| 02-06 | 9/9 | All 256 opcode cases, no exclusion list, machine profile, JAM/unstable opcodes, and focused unit tests are present. |
| 02-07 | 7/7 | Pinned fetch, sample-match, full preset, and nightly workflow are present and connected. |
| 02-08 | 8/8 | Cross-platform CI and full-vector workflow wiring exists; recorded UAT supplies hosted and release evidence. Scheduled-only claims are covered by the accepted overrides. |
| 02-09 | 3/3 | Parser-source, API-baseline, JSON-size and exact registration policies are implemented. The generic link query missed indirect CTest script invocation; `tests/CMakeLists.txt` lines 339–344 register the checker directly. |
| 02-10 | 3/3 | Nightly trigger/security/no-skip policies and run-bound evidence/report wiring are implemented. Generic key-link query did not follow policy references through CMake; direct call sites exist in the hygiene CTest configuration. |
| 02-11 | 3/3 | Release metadata assertion and read-only outcome collector are present. The generic API-link query cannot parse `gh` shell calls; direct `gh release`, `gh api`, and `gh run` calls were inspected. |

### Required Artifacts

| Artifact set | Status | Details |
|---|---|---|
| CPU core and bus: `src/cpu.c`, `src/bus.c`, `src/internal.h`, `src/instance.c` | ✓ VERIFIED | Substantive implementation. `cpu.c` has 256 cases; library CMake includes the CPU object and obtains bus hooks from `bus.c`. |
| Vector execution: `tests/cpu/test_vectors.c`, `tests/cpu/vector_bus.c`, `tests/CMakeLists.txt` | ✓ VERIFIED | Harness executes one instruction through production CPU code and compares complete state plus ordered cycles; CTest generation covers all 00–ff opcodes. |
| Corpus tooling/data: `tools/vecconv/vecconv.c`, `tests/vectors/n65v.c`, `tests/vectors/65x02-sample.n65v`, `tests/vectors/pins.txt` | ✓ VERIFIED | Owned converter/parser, committed sample and pinned full corpus metadata are present; source and result checks are registered. |
| Workflows/evidence: CI, nightly, release workflows; policy scripts; `scripts/phase2_outcomes.sh` | ✓ VERIFIED | Build, full vectors, reporting and release publication paths are connected; hosted evidence is recorded in phase UAT. |
| Plan 02-05 artifact `NESTURBATOR_OFFICIAL_OPCODES` | ✓ VERIFIED (superseded) | Exact artifact pattern is missing, but 02-06 intentionally removes both fixed opcode lists and generates all 256 opcodes. This is an evidenced equivalent implementation, not an incomplete feature. |

Plan artifact queries passed except the superseded 02-05 symbol; its replacement was manually verified. Key-link query false negatives on 02-09 through 02-11 were traced through actual CMake registrations and shell calls. No required implementation artifact is missing or stubbed.

### Key Link Verification

| From | To | Via | Status | Evidence |
|---|---|---|---|---|
| `CMakeLists.txt` | `src/cpu.c` | CPU object linked into library | ✓ WIRED | `target_sources` adds `$<TARGET_OBJECTS:nesturbator_cpu>`. |
| `tests/CMakeLists.txt` | `tests/cpu/vector_bus.c` and CPU object | Vector/unit executable link | ✓ WIRED | The harness links production CPU object with test bus hooks. |
| `tests/CMakeLists.txt` | committed N65V sample | 00–ff test generation | ✓ WIRED | Each opcode test passes the blob to `cpu.vectors`; full output is checked. |
| `tests/CMakeLists.txt` | full fetch and sample check | CTest fixtures | ✓ WIRED | Full-vector tests require the fetch fixture; sample-match and per-opcode tests use fetched pinned data. |
| `.github/workflows/nightly.yml` | full preset and result policy | main/schedule run | ✓ WIRED | Workflow invokes `cmake --workflow --preset vectors-full`, saves exact inventory/JUnit, and routes failures to reporter. |
| `.github/workflows/release.yml` | published assets | release metadata assertion | ✓ WIRED | Publish requires CI, creates and attests 18 archives, writes SHA256SUMS, and asserts remote release metadata. |
| `scripts/phase2_outcomes.sh` | GitHub release and run APIs | read-only `gh` queries | ✓ WIRED | Script calls GitHub release/run/artifact APIs and validates exact archived test results. |

### Data-Flow Trace (Level 4)

| Artifact | Data | Source | Produces real data | Status |
|---|---|---|---|---|
| Sample vector tests | Expected registers, RAM and cycles | Manifest-hashed committed N65V sample | Yes | ✓ FLOWING |
| Full vector tests | Expected state and cycles | Files fetched from pinned upstream commit and hash checked | Yes | ✓ FLOWING |
| Hosted full-run proof | Exact test names and results | Same-run CTest inventory and JUnit artifact, checked by policy script | Yes; phase UAT records successful checks | ✓ FLOWING |
| Release artifacts | Library, runner, libretro archives and checksum file | CI package jobs and release publish workflow | Yes; phase UAT records publication | ✓ FLOWING |

### Behavioral Spot-Checks

| Behavior | Command/evidence | Result | Status |
|---|---|---|---|
| Sample CPU vector suite and repository regression | `cmake --workflow --preset ci` | 355 passed, 2 skipped (`retroarch.testframe`, `retroarch.game`) out of 357; no failures. No additional tests run by this verifier. | ✓ PASS |
| Exact-merge full-vector workflow and reporter | Phase UAT run 37509491346 | `vectors-full` and report job succeeded on the exact phase merge SHA. | ✓ PASS |
| Post-merge full-vector evidence | Phase UAT run 37531643137 and archived inventory/JUnit check | Manual dispatch on main passed; policy accepted exact 258-test inventory and all passing results. | ✓ PASS |
| Release publication | Phase UAT release evidence | v0.1.1 publication, expected archive/checksum inventory, tag ancestry, CI, attestations, tamper check and publish recorded passed. | ✓ PASS |

### Probe Execution

No phase-declared or conventional `scripts/*/tests/probe-*.sh` probes were found in the plans or summaries.

### Requirements Coverage

| Requirement | Source plans | Description | Status | Evidence |
|---|---|---|---|---|
| CPU-01 | 02-01–02-06, 02-08, 02-09, 02-11 | Committed sample, every opcode, final-state and per-cycle match | ✓ SATISFIED | 256 registered cases; vector harness comparisons; supplied CI result passed. |
| CPU-02 | 02-07–02-11 | Full pinned set matches; CI runs it nightly | ✓ SATISFIED | Full fetch/pin workflow; exact-merge and post-merge run evidence. Scheduled-only first-cron/reporter assertions accepted by overrides. |

All requirement IDs found in plan frontmatter are CPU-01 and CPU-02; both are mapped to Phase 2 in `.planning/REQUIREMENTS.md`. No orphaned Phase 2 requirement IDs.

### Anti-Patterns Found

| File | Pattern | Severity | Impact |
|---|---|---|---|
| None | No unreferenced TBD/FIXME/XXX debt markers or implementation stubs found in inspected phase artifacts. A `return NULL` in the N65V reader is its documented allocation-failure path. | — | No blocker. |

### Human Verification Required

None. The irreducible clean-room provenance confirmation and scheduled-event substitute were already recorded in the completed phase UAT/accepted overrides.

### Gaps Summary

No blocking gaps. The scheduled cron event itself was not observed; owner-accepted overrides record the post-merge manual full-vector run and exact-merge reporter evidence as the agreed substitute. The current source, registered tests, CI gate result, hosted vector evidence, and published release evidence support the phase goal.

---

_Verified: 2026-10-09T19:36:29Z_
_Verifier: the agent (gsd-verifier)_
