---
phase: 01-a-test-frame-in-retroarch
verified: 2026-10-08T15:20:00Z
status: passed
score: 5/5 must-haves verified
covered_files: [".clang-format", ".gitattributes", ".githooks/pre-commit", ".githooks/pre-push", ".github/dependabot.yml", ".github/rulesets/main.json", ".github/workflows/ci.yml", ".github/workflows/release.yml", ".planning/phases/01-a-test-frame-in-retroarch/01-01-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-01-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-02-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-02-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-03-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-03-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-04-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-04-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-05-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-05-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-06-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-06-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-07-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-07-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-08-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-08-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-09-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-09-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-10-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-10-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-11-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-11-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-12-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-12-SUMMARY.md", "ASSET_POLICY.md", "CMakeLists.txt", "CMakePresets.json", "CONTRIBUTING.md", "PROVENANCE.md", "README.md", "SECURITY.md", "THIRD-PARTY-NOTICES.md", "cmake/nesturbator.pc.in", "cmake/nesturbatorConfig.cmake.in", "cmake/packaging.cmake", "host/convert.c", "host/convert.h", "include/nesturbator.h", "libretro/CMakeLists.txt", "libretro/libretro.c", "libretro/libretro.h", "libretro/nesturbator_libretro.info", "release-please-config.json", "runner/CMakeLists.txt", "runner/main.c", "runner/ppm.c", "runner/ppm.h", "runner/sha256.c", "runner/sha256.h", "scripts/hygiene.sh", "scripts/phase2_outcomes.sh", "src/bus.c", "src/cpu.c", "src/frame.c", "src/instance.c", "src/internal.h", "src/palette.c", "src/palette_ntsc.c", "src/testcard.c", "tests/CMakeLists.txt", "tests/abi/CMakeLists.txt", "tests/abi/float_fixture.c", "tests/check.h", "tests/cmake/action_pins.cmake", "tests/cmake/check_archives.cmake", "tests/cmake/check_install_line.cmake", "tests/cmake/check_ppm.cmake", "tests/cmake/expect_output.cmake", "tests/cmake/fetch_guard.cmake", "tests/cmake/fetch_vectors.cmake", "tests/cmake/float_fixture.cmake", "tests/cmake/float_scan.cmake", "tests/cmake/format_check.cmake", "tests/cmake/global_symbols.cmake", "tests/cmake/manifest_sha256.cmake", "tests/cmake/nightly_workflow_policy.cmake", "tests/cmake/palette_regen.cmake", "tests/cmake/pins_check.cmake", "tests/cmake/release_config.cmake", "tests/cmake/release_markers.cmake", "tests/cmake/script_policy.cmake", "tests/cmake/undefined_symbols.cmake", "tests/cmake/vecconv_negative.cmake", "tests/cmake/vector_api_policy.cmake", "tests/cmake/vector_registration_policy.cmake", "tests/cmake/vector_result_policy.cmake", "tests/cmake/vector_source_policy.cmake", "tests/cmake/vectors_fixture.cmake", "tests/cmake/vectors_full_run.cmake", "tests/cmake/vectors_regen.cmake", "tests/cmake/vectors_sample_match.cmake", "tests/cmake/vectors_stray_write.cmake", "tests/cmake/vendored_sha256.cmake", "tests/cmake/version_consistency.cmake", "tests/cmake/write_hashes.cmake", "tests/consumer/CMakeLists.txt", "tests/consumer/main.c", "tests/core/test_api.c", "tests/core/test_frame.c", "tests/core/test_palette.c", "tests/core/test_profile.c", "tests/cpu/test_bus.c", "tests/cpu/test_cpu_unit.c", "tests/cpu/test_vectors.c", "tests/cpu/vector_bus.c", "tests/cpu/vector_bus.h", "tests/embed/CMakeLists.txt", "tests/header/header_c.c", "tests/header/header_cxx.cpp", "tests/host/test_convert.c", "tests/hygiene/CMakeLists.txt", "tests/hygiene/fixtures/pins_bad.yml", "tests/hygiene/fixtures/pins_good.yml", "tests/hygiene/scan_selftest.sh", "tests/libretro/libretro_host.c", "tests/release/two_versions.md", "tests/retroarch/CMakeLists.txt", "tests/retroarch/bmp_ppm.c", "tests/retroarch/bmp_ppm.h", "tests/retroarch/compare_frame.c", "tests/retroarch/run_retroarch.cmake", "tests/retroarch/test.cfg.in", "tests/retroarch/test_compare_frame.c", "tests/roms/manifest.txt", "tests/runner/hashes.txt", "tests/runner/test_sha256.c", "tests/vectors/65x02-sample.n65v", "tests/vectors/fixtures/02-first3.json", "tests/vectors/fixtures/README.md", "tests/vectors/fixtures/a9-first3.json", "tests/vectors/fixtures/a9-tail.json", "tests/vectors/n65v.c", "tests/vectors/n65v.h", "tests/vectors/pins.txt", "tests/vectors/test_n65v.c", "tools/palgen/CMakeLists.txt", "tools/palgen/palgen.c", "tools/vecconv/CMakeLists.txt", "tools/vecconv/vecconv.c", "version.txt"]
covered_digest: "v3:sha256:abd46d54001a67520cdd353dcb2c214e0f93ce67c506e4e4b859c6dda297fdf9"
behavior_unverified: 0
overrides_applied: 0
---

# Phase 1: A test frame in RetroArch Verification Report

**Phase Goal:** Anyone can build, test, download and install the library, the runner and the libretro core, and RetroArch shows the core's built-in test frame.
**Verified:** 2026-10-08T15:20:00Z
**Status:** passed
**Re-verification:** No. The previous report had no `gaps:` section; this is an initial goal-backward verification.

## Goal Achievement

### Observable Truths (ROADMAP success criteria)

| # | Truth | Status | Evidence |
|---|---|---|---|
| 1 | The CMake workflows build and test the library, runner and adapter on six platforms, including an installed-package consumer using only the public header. | ✓ VERIFIED | `CMakeLists.txt`, `CMakePresets.json`, `tests/consumer/main.c`, and `tests/CMakeLists.txt` wire `install.stage` → `install.consumer`; the consumer includes `nesturbator.h`, creates/runs/destroys an instance. Current GitHub CI run 37538402558 on `33911ba` passed all 13 jobs: six platform build legs, ASan, both no-float legs, hygiene, hash equality and `CI required`. The CI workflow invokes the declared CMake presets. |
| 2 | The no-cartridge runner emits the specified frame/hash and the libretro host receives an equal frame. | ✓ VERIFIED | `src/frame.c` fills every native pixel via `nesturbator__test_pixel`; `runner/main.c` hashes row-major native pixels and writes P6 through the shared host conversion. `tests/CMakeLists.txt` pins the expected hash `b49e9be4…b0453` and tick counts; `tests/libretro/libretro_host.c` dynamically loads the module, checks the native boundary pixels and compares its frame with the runner PPM. Current CI run 37538402558 passed. |
| 3 | The README install retrieves the Apple Silicon adapter and RetroArch's unattended screenshot comparison equals the runner frame; unsupported hosts skip the test. | ✓ VERIFIED | `README.md` has the versioned one-line release install; `tests/cmake/check_install_line.cmake` parses that line and validates the real archive layout; `.github/workflows/ci.yml` runs the check on macOS arm64. The isolated `tests/retroarch/run_retroarch.cmake` launches RetroArch with generated config and compares its screenshot through `compare_frame`; `tests/retroarch/CMakeLists.txt` defines skip behavior. Phase UAT records the automated installed-release comparison and `retroarch.testframe` pass; no owner-only visual judgment is part of this pixel-equality criterion. |
| 4 | ASan, no-float and hygiene lanes enforce sanitizer, ABI-symbol/floating-point, and clean-tree requirements. | ✓ VERIFIED | Presets and `tests/abi`, `tests/hygiene`, and `scripts/hygiene.sh` wire the checks. The current GitHub CI run 37538402558 passed ASan, both no-float jobs and hygiene. The tracked ROM manifest and hygiene checks enforce the clean-tree rule. |
| 5 | Pull requests produce six-platform archives and a behavior-changing merge publishes 18 archives plus checksums with no manual release step. | ✓ VERIFIED | `.github/workflows/ci.yml` builds/uploads per-platform archives and hashes; `.github/workflows/release.yml` wires release-please, CI, checksum/attestation and publish. Read-only GitHub evidence: current non-draft `v0.1.3` release has all 18 platform/component archives and `SHA256SUMS`; CI push run 37538402558 passed `hash-equality` and `CI required`. GitHub ruleset API reports active `main`, with squash-only, required `CI required`, matching the intent of `.github/rulesets/main.json`. |

**Score:** 5/5 truths verified (0 present, behavior-unverified)

### Required Artifacts

| Artifact set | Expected | Status | Details |
|---|---|---|---|
| Plans 01-01 through 01-12 declared artifacts | 46 declared paths exist and pass `verify.artifacts` checks | ✓ VERIFIED | Per-plan artifact checks returned 46/46 passed. Implementations include the public API/core, runner, palette, host conversion, adapter, install/package, ABI/hygiene checks, RetroArch test, and CI/release configuration. |
| `src/frame.c`, `src/testcard.c`, `runner/main.c` | Test frame generation, runner hash and PPM output | ✓ VERIFIED | Substantive implementations are present; runner calls public frame API and shared host conversion. |
| `libretro/libretro.c`, `tests/libretro/libretro_host.c` | Libretro entry points and runtime host exercise | ✓ VERIFIED | Adapter implements the API and host dynamically resolves entry points and compares rendered output. |
| `.github/workflows/{ci,release}.yml`, `cmake/packaging.cmake` | Six-platform build, archives, publishing | ✓ VERIFIED | Workflow and packaging artifacts are substantive and connected; live release evidence confirms published deliverables. |

### Key Link Verification

| From | To | Via | Status | Details |
|---|---|---|---|---|
| `runner/main.c` | `include/nesturbator.h` | create/run/destroy public API | WIRED | Calls `nesturbator_run_frame`; corresponding create/destroy calls present. |
| `src/frame.c` | `src/testcard.c` | per-pixel generator | WIRED | Calls `nesturbator__test_pixel` in frame fill loop. |
| `tests/CMakeLists.txt` | `tests/cmake/expect_output.cmake` | runner output assertions | WIRED | Runner tests invoke the exact-output helper. |
| `tests/libretro/libretro_host.c` | generated runner PPM | test fixture | WIRED | Pattern checker missed the fixture's generated variable name, but `add_test` passes `${NESTURBATOR_TESTFRAME_PPM}` and declares `FIXTURES_REQUIRED testframe_ppm`; the fixture is wired. |
| `.github/rulesets/main.json` | live GitHub ruleset | API read-back | WIRED | Pattern checker cannot resolve remote endpoints; read-only API confirmed ruleset `main` id 24400039 is active with only squash merge and required `CI required`. |
| `tests/cmake/check_install_line.cmake` | `README.md` | parse install line | WIRED | Test reads/parses the README release URL and tar arguments. |
| `.github/workflows/ci.yml` | install-line check | macOS arm64 leg | WIRED | Workflow invokes the script after packaging on the arm64 build leg. |
| `.github/workflows/release.yml` | `.github/workflows/ci.yml` | release CI dependency | WIRED | Reusable workflow call is present and release publish depends on CI. |

All 12 plan artifact checks passed. Key-link pattern checks passed 22/25; the three pattern misses were manually resolved above from explicit fixture setup and live ruleset API evidence. No disconnected key links remain.

### Data-Flow Trace (Level 4)

| Artifact | Data variable | Source | Produces Real Data | Status |
|---|---|---|---|---|
| `src/frame.c` → `runner/main.c` | native frame pixels | per-instance frame state and test-card pixel function | Yes | ✓ FLOWING |
| `runner/main.c` → PPM/hash | converted pixels and frame bytes | palette lookup, shared converter, SHA-256 | Yes | ✓ FLOWING |
| `libretro/libretro.c` | callback frame | same core frame API and shared converter used by runner | Yes | ✓ FLOWING |
| `tests/libretro/libretro_host.c` | callback pixels | dynamically loaded module callback, compared with runner PPM | Yes | ✓ FLOWING |
| `tests/retroarch/run_retroarch.cmake` | screenshot | RetroArch process output, converted and compared to runner PPM | Yes | ✓ FLOWING |

### Behavioral Spot-Checks

| Behavior | Command/evidence | Result | Status |
|---|---|---|---|
| Six-platform CI, including frame/hash equality | Existing GitHub CI run 37538402558 on `33911ba` | 13/13 jobs succeeded, including all six builds, ASan, no-float ×2, hygiene, hash-equality and roll-up | ✓ PASS |
| Installed package and adapter consumer | Test wiring in `tests/CMakeLists.txt`; latest CI run above | `install.consumer` and `libretro.host` are part of the `ci` test lane; no test was rerun in this verification | ✓ PASS (existing CI evidence) |
| RetroArch installed-release screenshot comparison | Existing `.planning/phases/01-a-test-frame-in-retroarch/01-UAT.md` machine-run record | Release core installed into isolated HOME; `retroarch.testframe` and direct installed-core comparison recorded as passing | ✓ PASS (existing automated evidence) |
| Published archives and integrity manifest | Read-only GitHub release metadata for `v0.1.3` | 18 named archives and `SHA256SUMS`, non-draft release | ✓ PASS |

No tests were added or run in this verification, per task instruction. Existing code wiring and machine evidence were inspected.

### Probe Execution

No phase plan declares a probe and no conventional `scripts/*/tests/probe-*.sh` is present. Step 7c: SKIPPED.

### Requirements Coverage

| Requirement | Source Plans | Description | Status | Evidence |
|---|---|---|---|---|
| FRAME-01 | 01-01, 01-02, 01-10, 01-12 | Six-platform CMake/CI build and test | ✓ SATISFIED | Presets/workflow plus six successful CI build jobs and `CI required` in run 37538402558. |
| FRAME-02 | 01-02, 01-06 | Public-header installed-package consumer | ✓ SATISFIED | `tests/consumer/main.c`, staged install fixture, and CI evidence. |
| FRAME-03 | 01-01 through 01-04 | Fixed test frame, runner image/hash and cross-platform identity | ✓ SATISFIED | Frame/runner implementations, pinned output assertions and passing CI `hash-equality`. |
| FRAME-04 | 01-05 | Libretro host receives runner-equivalent frame | ✓ SATISFIED | Dynamic module host, pixel assertions, PPM fixture and CI test lane. |
| FRAME-05 | 01-09, 01-11 | Unattended RetroArch screenshot comparison and install line | ✓ SATISFIED | Isolated RetroArch driver, README extraction check, and existing automated UAT pass. |
| FRAME-06 | 01-07, 01-08 | ASan, no-float ABI and hygiene | ✓ SATISFIED | Checks are wired and latest CI passed all three lanes (no-float has two platform jobs). |
| FRAME-07 | 01-06, 01-10, 01-12 | Per-platform packages and automated releases | ✓ SATISFIED | Packaging/release workflows plus live v0.1.3 assets and checksums. |

All seven requirement IDs named by the roadmap are accounted for in PLAN frontmatter. No additional Phase 1 requirement mapping is orphaned in `REQUIREMENTS.md`.

### Decision Coverage

All trackable CONTEXT.md decisions are honored by shipped artifacts (20/20; non-blocking decision gate).

### Test Quality Audit

| Test File Set | Linked Req | Disabled tests | Circular expected-value generation | Verdict |
|---|---|---|---|---|
| Requirement-linked tests under `tests/` | FRAME-01 through FRAME-07 | None found by disabled-test scan | None identified in relevant test sources | ✓ PASS |

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|---|---:|---|---|---|
| `libretro/libretro.h` | several | Upstream TODO comments in the vendored ABI header | ℹ️ Info | Inherited protocol-header notes; no empty implementation or phase blocker. |
| Phase review disposition | WR-01, WR-02 | Open review warnings | ⚠️ Warning | Allocator macro interaction and vector-count parser overflow are recorded review items; neither prevents Phase 1 outcomes, and no failing evidence was found in this verification. |

No unreferenced `TBD`, `FIXME`, or `XXX` debt markers were found in implementation files scanned. No implementation stubs or hardcoded empty rendered data were found.

### Human Verification Required

None. RetroArch acceptance is exact screenshot-to-runner pixel equality and has existing automated execution evidence; no visual judgment is required by the criterion.

### Gaps Summary

No gaps. The roadmap's five outcome truths are supported by substantive implementations, wiring, a current successful six-platform CI run, automated RetroArch UAT evidence, and the currently published cross-platform release assets. No blocking artifact or key-link failure remains.

---

_Verified: 2026-10-08T15:20:00Z_<br>
_Verifier: the agent (gsd-verifier)_
