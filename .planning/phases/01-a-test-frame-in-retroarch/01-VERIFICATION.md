---
phase: 01-a-test-frame-in-retroarch
verified: 2026-10-09T11:45:10Z
status: human_needed
score: 68/69 must-haves verified
covered_files: [".github/rulesets/main.json", ".github/workflows/ci.yml", ".github/workflows/release.yml", ".planning/phases/01-a-test-frame-in-retroarch/01-01-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-01-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-02-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-02-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-03-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-03-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-04-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-04-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-05-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-05-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-06-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-06-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-07-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-07-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-08-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-08-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-09-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-09-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-10-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-10-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-11-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-11-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-12-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-12-SUMMARY.md", "cmake/packaging.cmake", "host/convert.c", "host/convert.h", "include/nesturbator.h", "libretro/libretro.c", "libretro/libretro.h", "libretro/nesturbator_libretro.info", "runner/main.c", "runner/ppm.c", "runner/sha256.c", "scripts/hygiene.sh", "src/frame.c", "src/testcard.c", "tests/CMakeLists.txt", "tests/cmake/check_archives.cmake", "tests/cmake/check_install_line.cmake", "tests/consumer/main.c", "tests/core/test_api.c", "tests/core/test_frame.c", "tests/libretro/libretro_host.c", "tests/retroarch/run_retroarch.cmake"]
covered_digest: "v3:sha256:086920b9172316d55241563c9154b908fd8f5b9219f6f1d6c6a2ff9ccbc92ce2"
behavior_unverified: 0
overrides_applied: 0
human_verification:
  - test: "Review clean-room provenance of the palette generator and generated table"
    expected: "Confirm tools/palgen/palgen.c and src/palette_ntsc.c restate cited hardware facts in original structure and do not copy or paraphrase restricted decoder or emulator code."
    why_human: "Plan 01-03 marks this prohibition verification as judgment; source provenance cannot be established by presence checks or the automated test suite."
  - test: "Review release credential isolation enforcement"
    expected: "Confirm the release App credential cannot reach pull-request-controlled code; release.yml currently mints it only on push-to-main/workflow_dispatch and passes it only to release-please and auto-merge."
    why_human: "Plan 01-10 specifies test-tier enforcement, but no regression test checks this credential boundary. The implementation looks correctly scoped, but fail-closed prohibition policy requires human review."
  - test: "Verify a docs-only or chore-only merge does not publish a release"
    expected: "Confirm release-please creates no release PR/release for such a merge, while behavior-changing merges publish exactly 18 archives plus SHA256SUMS with valid attestations."
    why_human: "The live v0.1.4 release proves the positive publication path; no isolated test or direct observation exercises the negative docs/chore path in plan 01-12's backstop claim."
---

# Phase 1: A test frame in RetroArch Verification Report

**Phase Goal:** Anyone can build, test, download and install the library, the runner and the libretro core, and RetroArch shows the core's built-in test frame.
**Verified:** 2026-10-09T11:45:10Z
**Status:** human_needed
**Re-verification:** No. The prior report had no `gaps:` section; this is an independent initial goal-backward pass against current code and remote evidence.

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|---|---|---|
| 1 | `ci`/`ci-msvc` build and test the library, runner and libretro core on six platforms, including an installed-package consumer; CI runs the same workflows. | ✓ VERIFIED | Ran `cmake --workflow --preset ci` locally: configure/build/package succeeded and CTest reported 350 passed, 2 skipped, 0 failed (352 total). `install.stage`, `install.consumer`, `libretro.host` passed. Current hosted CI run 37862745508 succeeded with all six build legs, asan, both nofp legs, hygiene, hash-equality and `CI required`; the workflow uses `ci` on Linux/macOS and `ci-msvc` on Windows. |
| 2 | The no-cartridge runner writes the fixed image/hash and the libretro host receives the equal frame. | ✓ VERIFIED | Local `runner.dump`, `runner.hash`, `core.frame`, `libretro.host`, and `retroarch.compare*` passed. `tests/CMakeLists.txt` pins the frame output/hash; runner and adapter both call the same instance frame, palette and conversion APIs. Hosted `hash-equality` passed in run 37862745508. |
| 3 | README install and unattended RetroArch run compare the screenshot exactly; unsupported hosts skip. | ✓ VERIFIED | `01-UAT.md` records the Apple Silicon install and `retroarch.testframe` as Passed, with exact screenshot comparison and unchanged user directory. Current local workflow correctly skipped `retroarch.testframe` and `retroarch.game` because RetroArch is unavailable here; CTest reports 100% pass with those two expected skips. Current hosted CI run 37862745508 also succeeded its `retroarch-e2e` job. |
| 4 | ASan, no-float/undefined-symbol and hygiene checks pass. | ✓ VERIFIED | Presets and their CTest wiring are present. Hosted CI run 37862745508 succeeded `asan`, both `nofp` legs and `hygiene`; the local `ci` workflow also passed. Hygiene checks the tree and binary manifest. |
| 5 | PRs produce six-platform archives; a behavior-changing merge publishes 18 archives and `SHA256SUMS`. | ✓ VERIFIED | `.github/workflows/ci.yml` builds and uploads the per-platform library/runner/libretro zips and hashes. `release.yml` requires exactly 18 zips, writes checksums, attests, verifies each archive and publishes. Direct current release evidence: v0.1.4 is not draft and has exactly 19 assets (18 platform/component zips plus `SHA256SUMS`); Release workflow run 37862745747 succeeded. The separate docs/chore negative branch is a PLAN backstop item; see Human Verification. |

**Score:** 68/69 phase-plan must-have truths verified; all 5/5 roadmap success criteria are supported. One backstop claim remains for human verification.

### Plan Must-Haves

All 68 `must_haves.truths` parsed from plans 01-01 through 01-12 were checked against source, CMake wiring, named tests and current hosted/local evidence. The grouped evidence below records the checks and any pattern-check limitation; no plan truth is failed.

| Plan | Truths | Result and evidence |
|---|---:|---|
| 01-01 | 6/6 | Public API, fixed test card, deterministic runner hash, runner argument/error cases and test preset are implemented in `include/nesturbator.h`, `src/frame.c`, `src/testcard.c`, `runner/main.c`, and registered tests. Local frame/API/runner tests passed. |
| 01-02 | 8/8 | Instance API/state, frame invariants, header-alone C/C++ use, SHA-256 known answers and version consistency are implemented and registered. Local API/frame/header/hash tests passed. |
| 01-03 | 4/4 | Palette generator/table and API invariants are implemented; regeneration compares generated output with the checked-in table. `palette.regen` and `core.palette` passed. Source provenance remains a judgment-tier prohibition and is routed to human review. |
| 01-04 | 4/4 | Shared palette conversion feeds runner P6 output; bounds and exact byte layout are checked. `host.convert`, runner dump and frame hash tests passed. |
| 01-05 | 7/7 | Libretro entry points use the shared conversion; the host dynamically loads the module and compares against the generated runner PPM fixture. `libretro.host` passed. GSD's pattern check missed the fixture variable name, but `tests/CMakeLists.txt` declares the fixture setup/requirement and the host consumes that generated file. |
| 01-06 | 5/5 | Install components, relocatable CMake/pkg-config packages, installed public-header consumer and archive layout checks are wired. Local `install.stage`, `install.consumer`, `embed.subdirectory` passed; current v0.1.4 contains the three component archives for every platform. |
| 01-07 | 6/6 | ASan and no-float presets register their tests; float scan, undefined-symbol allowlist, global-symbol scan and negative fixtures are substantive. Hosted `asan` and both nofp jobs passed. |
| 01-08 | 3/3 | Hygiene preset checks formatting, pinned Actions, personal-data/binary/ROM tree policy. Current hosted hygiene job passed. The release history and ROM checks are connected through `scripts/hygiene.sh` and the CI job. |
| 01-09 | 7/7 | BMP/P6 parser/comparator covers malformed files, dimensions, precision and supported layouts; driver uses generated isolated config and compares RetroArch output. `retroarch.compare` and `.cli` passed locally; the actual screenshot run is backed by phase UAT and current hosted `retroarch-e2e`. Test-tier safety prohibitions are separately assessed below. |
| 01-10 | 8/8 | CI triggers/matrix/roll-up and release sequencing/permissions/pinning are present; latest hosted CI and release runs succeeded. `STATE.md` no longer carries the plan-era `release-as` todo because Phase 2 removed it after v0.1.0; current `release.no_release_as` passes. One static link-pattern miss is therefore an obsolete planning marker, not missing workflow wiring. |
| 01-11 | 4/4 | README's versioned macOS-arm64 install line is parsed and extracted by `check_install_line.cmake`; the macOS arm64 CI job invokes it. CI's six build legs and v0.1.4 asset corroborate the package. UAT records the actual RetroArch install/run. |
| 01-12 | 6/6 listed truths | Phase PR is merged; current hosted CI produced the six-platform checks and hash equality; v0.1.4 proves the positive release path and expected 19-asset count. The separate backstop statement covering the negative docs/chore path is not included in this count and remains human verification. The GSD link checker cannot resolve a remote ruleset, but the active ruleset was previously read back and the merged PR/current CI evidence is available. |

### Required Artifacts

| Artifact set | Expected | Status | Details |
|---|---|---|---|
| All PLAN-declared artifacts, plans 01-01 through 01-12 | 46 declared artifacts exist and pass artifact checks | ✓ VERIFIED | Ran `query verify.artifacts` for each plan: 46/46 passed with no missing/stub issues. Source review confirms substantive implementation; the public-header consumer, runner, adapter, image converter, comparator, packaging and workflows are connected. |
| Test-frame data path | Instance frame → test card → runner/adapter output | ✓ VERIFIED | `src/frame.c` fills frame pixels from `src/testcard.c`; `runner/main.c` hashes and converts those pixels; `libretro/libretro.c` returns converted frame data through the libretro callback. Host test compares callback pixels to runner PPM. |
| Build/install/release path | Presets → build/tests/packages → hosted CI/release | ✓ VERIFIED | CMake presets, install fixtures, six-platform matrix, artifact uploads, release workflow and v0.1.4 assets are present and wired. |

### Key Link Verification

| Link group | Status | Details |
|---|---|---|
| 25 PLAN-declared links | ✓ WIRED | Ran `query verify.key-links` for all plans. 22 passed pattern checks directly. Three pattern misses were manually resolved: plan 05's generated PPM fixture is declared and consumed through CTest fixture properties; plan 10's Phase 2 `release-as` todo was completed and removed after v0.1.0; plan 12's remote ruleset cannot be resolved from local text but remote ruleset/merged PR state is evidenced. No code wiring break found. |
| Runner → public core → frame/test card | ✓ WIRED | Runner calls public create/run/destroy; frame fill calls the per-pixel test-card function. |
| Libretro host → adapter → runner reference | ✓ WIRED | Host loads adapter exports and tests its callback pixels against the runner-produced PPM fixture. |
| CI → packages/hashes; release → CI → checksums/attestation/publish | ✓ WIRED | Workflow call and dependencies, 18-archive assertion, checksums, attestation verify, tamper-negative check, publish and metadata assertion are explicit in `release.yml`. |

### Data-Flow Trace (Level 4)

| Artifact | Data | Source | Real data | Status |
|---|---|---|---|---|
| `src/frame.c` → runner | Native frame pixels | Instance frame generation and `nesturbator__test_pixel` | Yes | ✓ FLOWING |
| Runner → image/hash | RGB image bytes and hash input | Palette API, host converter and SHA-256 | Yes | ✓ FLOWING |
| `libretro/libretro.c` → host/RetroArch | Video callback pixels | Same core frame and palette conversion APIs | Yes | ✓ FLOWING |
| Installed consumer | Created instance and rendered frame | Installed CMake package and public header | Yes | ✓ FLOWING |
| CI/release artifacts | Platform archives and hash files | Actual build/package jobs; v0.1.4 release assets | Yes | ✓ FLOWING |

### Behavioral Spot-Checks

| Behavior | Command/evidence | Result | Status |
|---|---|---|---|
| Full local `ci` workflow | `cmake --workflow --preset ci` | Exit 0; build and package passed; CTest 350 passed, 2 expected skips, 0 failed | ✓ PASS |
| Installed public consumer and libretro host | Same workflow: `install.consumer`, `libretro.host` | Both passed | ✓ PASS |
| RetroArch exact comparison and unavailable-host behavior | Same workflow: `retroarch.testframe`, `retroarch.game`; `.planning/.../01-UAT.md` | Local tests correctly skipped without RetroArch; Apple Silicon phase UAT records `retroarch.testframe` Passed; current hosted `retroarch-e2e` passed | ✓ PASS |
| Six-platform CI, sanitizers, no-float, hygiene, hashes | Run 37862745508 (`push`, head `93942b7a5d0fa5120e63865a37af29261dcbf825`) | Successful jobs include all six build legs, `asan`, both nofp legs, hygiene, hash-equality, RetroArch E2E and `CI required` | ✓ PASS |
| Published release contents | `gh release view v0.1.4` | `isDraft=false`; exactly 19 assets, 18 platform/component archives plus `SHA256SUMS`; Release run 37862745747 succeeded | ✓ PASS |

### Probe Execution

No plan declares a probe path and there is no conventional `scripts/*/tests/probe-*.sh` probe for this phase. Step 7c: SKIPPED.

### Requirements Coverage

| Requirement | Source plans | Status | Evidence |
|---|---|---|---|
| FRAME-01 | 01-01, 01-02, 01-10, 01-12 | ✓ SATISFIED | Local CI workflow passed; current hosted six-platform CI run succeeded all six builds and the `CI required` roll-up. |
| FRAME-02 | 01-02, 01-06 | ✓ SATISFIED | Installed consumer uses only the public header and passed in local `install.consumer`. |
| FRAME-03 | 01-01 through 01-04 | ✓ SATISFIED | Fixed frame/hash implementation, runner dump/hash tests and cross-platform hash-equality job passed. |
| FRAME-04 | 01-05 | ✓ SATISFIED | Dynamic libretro host test compares real callback pixels with runner PPM and passed. |
| FRAME-05 | 01-09, 01-11 | ✓ SATISFIED | Comparator and install-line checks passed; phase UAT records Apple Silicon RetroArch screenshot equality; current hosted RetroArch E2E passed. |
| FRAME-06 | 01-07, 01-08, 01-10, 01-12 | ✓ SATISFIED | Hosted ASan, two no-float jobs and hygiene passed; checks are wired into presets/CI. |
| FRAME-07 | 01-06, 01-10, 01-12 | ✓ SATISFIED | Current published v0.1.4 has all 18 zips and checksums. The docs/chore negative trigger remains a PLAN backstop human item. |

All seven Phase 01 requirement IDs are present in REQUIREMENTS.md and claimed by plans. No additional requirement is mapped to Phase 01 without a source plan.

### Decision Coverage

Skipped by `check.decision-coverage-verify`: no phase `CONTEXT.md` is available to audit.

### Test Quality Audit

The registered test targets and CTest wiring are present. Local CI's 352 tests and hosted CI jobs passed. No disabled test was found as the sole proof for the checked Phase 01 criteria. Exact expected frame values/hashes are pinned in test sources rather than generated at assertion time. The two local RetroArch tests skipped as designed because this environment lacks RetroArch; the phase UAT and hosted RetroArch E2E provide positive execution evidence.

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|---|---:|---|---|---|
| `libretro/libretro.c` | 298 | `return NULL` for `retro_get_memory_data` | ℹ️ Info | Correct documented libretro behavior: this core exposes no memory region. Not a stub for the test-frame path. |
| `01-REVIEW-DISPOSITION.md` | CR-01 | Open critical review finding: cartridge trainer handling, a Phase 03 input path | ⚠️ Warning, out of Phase 01 scope | Does not affect the no-cartridge test frame, installed library/runner/adapter, or release artifacts verified here. No Phase 01 truth depends on trainer behavior. |

No unreferenced `TBD`, `FIXME`, or `XXX` debt markers were found in the implementation paths scanned. The remaining `return NULL` is a protocol-defined no-memory response, not an empty implementation. No source stub, hardcoded empty user-visible frame, or disconnected data flow was found.

### Human Verification Required

#### 1. Clean-room palette provenance

**Test:** Review `tools/palgen/palgen.c` and `src/palette_ntsc.c` against the cited sources and project provenance.
**Expected:** Confirm they contain independently structured implementation based on cited hardware facts, with no copied or paraphrased restricted decoder/emulator code.
**Why human:** Plan 01-03 marks this prohibition as judgment-tier; provenance is not decidable through executable behavior or a textual stub scan.

#### 2. Release credential boundary enforcement

**Test:** Review the release workflow and decide whether the lack of a regression test for the App credential boundary is acceptable or should be closed with an automated policy test.
**Expected:** Pull-request-controlled code cannot access the App private key/token; the workflow currently scopes token minting to the release workflow and sends it only to release-please and auto-merge.
**Why human:** Plan 01-10 labels this test-tier, but no wired test enforces the boundary. The fail-closed prohibition rule requires a human-visible flag despite the source review showing the intended scoping.

#### 3. No release for docs-only/chore-only merges

**Test:** Confirm through a safe release-policy test or existing workflow evidence that a docs-only/chore-only merge does not open/publish a release, while behavior-changing merges publish.
**Expected:** Release-please makes no release for docs/chore-only changes; behavior-changing release has exactly 18 archives plus `SHA256SUMS` and verified attestations.
**Why human:** v0.1.4 directly proves the positive release path, but this phase's backstop claim also asserts a negative path that has no isolated test or direct run evidence.

### Gaps Summary

No code or wiring gaps were found. All 46 artifacts and 25 key links were checked; three automated link-pattern misses were resolved by direct source/remote evidence. Current local CI passed 350 tests with two expected RetroArch skips, and current hosted CI plus the published v0.1.4 release demonstrate the six-platform build and positive release paths. Status is `human_needed` because one plan backstop behavior lacks evidence, one judgment-tier clean-room prohibition needs human provenance review, and the credential boundary has no test-tier enforcement wired. The open CR-01 concerns Phase 03 cartridge trainer input and does not block Phase 01's no-cartridge frame goal.

---

_Verified: 2026-10-09T11:45:10Z_
_Verifier: the agent (gsd-verifier)_
