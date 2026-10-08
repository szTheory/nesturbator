---
phase: 01-a-test-frame-in-retroarch
reviewed: 2026-10-08T15:11:07Z
depth: standard
files_reviewed: 77
files_reviewed_list:
  - .gitattributes
  - .github/workflows/ci.yml
  - .github/workflows/nightly.yml
  - .github/workflows/release.yml
  - .release-please-manifest.json
  - CHANGELOG.md
  - CMakeLists.txt
  - CMakePresets.json
  - PROVENANCE.md
  - README.md
  - THIRD-PARTY-NOTICES.md
  - include/nesturbator.h
  - libretro/nesturbator_libretro.info
  - release-please-config.json
  - scripts/hygiene.sh
  - scripts/phase2_outcomes.sh
  - src/bus.c
  - src/cpu.c
  - src/frame.c
  - src/instance.c
  - src/internal.h
  - tests/CMakeLists.txt
  - tests/cmake/action_pins.cmake
  - tests/cmake/check_archives.cmake
  - tests/cmake/check_install_line.cmake
  - tests/cmake/check_ppm.cmake
  - tests/cmake/expect_output.cmake
  - tests/cmake/fetch_guard.cmake
  - tests/cmake/fetch_vectors.cmake
  - tests/cmake/float_fixture.cmake
  - tests/cmake/float_scan.cmake
  - tests/cmake/format_check.cmake
  - tests/cmake/global_symbols.cmake
  - tests/cmake/manifest_sha256.cmake
  - tests/cmake/nightly_workflow_policy.cmake
  - tests/cmake/palette_regen.cmake
  - tests/cmake/pins_check.cmake
  - tests/cmake/release_config.cmake
  - tests/cmake/release_markers.cmake
  - tests/cmake/script_policy.cmake
  - tests/cmake/vecconv_negative.cmake
  - tests/cmake/vector_api_policy.cmake
  - tests/cmake/vector_registration_policy.cmake
  - tests/cmake/vector_result_policy.cmake
  - tests/cmake/vector_source_policy.cmake
  - tests/cmake/vectors_fixture.cmake
  - tests/cmake/vectors_full_run.cmake
  - tests/cmake/vectors_regen.cmake
  - tests/cmake/vectors_sample_match.cmake
  - tests/cmake/vectors_stray_write.cmake
  - tests/cmake/vendored_sha256.cmake
  - tests/cmake/version_consistency.cmake
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
  - tests/release/two_versions.md
  - tests/retroarch/run_retroarch.cmake
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
  - version.txt
findings:
  critical: 0
  warning: 2
  info: 0
  total: 2
status: issues_found
---

# Phase 1: Code Review Report

**Reviewed:** 2026-10-08T15:11:07Z  
**Depth:** standard  
**Files Reviewed:** 77  
**Status:** issues_found

## Summary

Reviewed the 77 existing files returned by the phase scope resolver after filtering missing and duplicate paths. The resolver reported `status=degraded`, `reason=no-reachable-task-commits`; the file list is therefore a degraded scope, not a clean phase commit range. Two robustness defects remain: the destroy callback invocation can be broken by a conforming function-like `free` macro, and the CPU-vector harness's decimal parser can wrap before enforcing its limit. No tests were run, per the review task instructions.

## Narrative Findings (AI reviewer)

### WR-01: Calling the allocator member can expand a function-like `free` macro

**Classification:** WARNING  
**File:** `src/instance.c:117`  
**Issue:** The core includes `<stdlib.h>`, and C implementations may define `free` as a function-like macro (debug allocators commonly do). In `a.free(a.user, inst, sizeof *inst)`, the preprocessor still expands the `free(` token even though it follows `a.`. A macro expecting the standard one-argument `free(ptr)` then sees three arguments and prevents the core from compiling in that configuration.
**Fix:** Suppress function-like macro expansion when invoking the callback: `(a.free)(a.user, inst, sizeof *inst);`.

### WR-02: The vector harness count parser can accept overflowing inputs

**Classification:** WARNING  
**File:** `tests/cpu/test_vectors.c:42-44`  
**Issue:** `v * 10u + digit` wraps modulo 2^32 before the `v > max` check. A value such as `4294967297` wraps to `1` and is accepted as one chunk/test, despite exceeding the documented bound. This makes invalid command-line limits silently select a different validation run.
**Fix:** Before multiplying, reject when `v > (max - digit) / 10u`, then compute `v = v * 10u + digit`.

---

_Reviewed: 2026-10-08T15:11:07Z_  
_Reviewer: the agent (gsd-code-reviewer)_  
_Depth: standard_
