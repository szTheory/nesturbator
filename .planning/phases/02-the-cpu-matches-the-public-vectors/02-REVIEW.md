---
phase: 02-the-cpu-matches-the-public-vectors
reviewed: 2026-10-06T14:00:00Z
depth: standard
files_reviewed: 51
files_reviewed_list:
  - .gitattributes
  - .github/workflows/ci.yml
  - .github/workflows/nightly.yml
  - .github/workflows/release.yml
  - CMakeLists.txt
  - CMakePresets.json
  - PROVENANCE.md
  - README.md
  - THIRD-PARTY-NOTICES.md
  - include/nesturbator.h
  - release-please-config.json
  - scripts/hygiene.sh
  - scripts/phase2_outcomes.sh
  - src/bus.c
  - src/cpu.c
  - src/instance.c
  - src/internal.h
  - tests/CMakeLists.txt
  - tests/cmake/manifest_sha256.cmake
  - tests/cmake/nightly_workflow_policy.cmake
  - tests/cmake/pins_check.cmake
  - tests/cmake/release_config.cmake
  - tests/cmake/vecconv_negative.cmake
  - tests/cmake/vector_api_policy.cmake
  - tests/cmake/vector_registration_policy.cmake
  - tests/cmake/vector_result_policy.cmake
  - tests/cmake/vector_source_policy.cmake
  - tests/cmake/vectors_fixture.cmake
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
  - tests/vectors/test_n65v.c
  - tools/vecconv/CMakeLists.txt
  - tools/vecconv/vecconv.c
findings:
  critical: 1
  warning: 2
  info: 0
  total: 3
status: issues_found
---

# Phase 02: Code Review Report

**Reviewed:** 2026-10-06T14:00:00Z
**Depth:** standard
**Files Reviewed:** 51
**Status:** issues_found

## Summary

Reviewed all 51 scoped files, including the CPU, vector conversion and reader, public API boundary, build/test policies, workflows, hygiene tooling, and phase outcome script. The CPU implementation and vector pipeline show strong coverage, but the phase outcome script cannot recognize a correctly published release because its expected archive list is not sorted. It also reports an in-progress validation run as a failure.

## Critical Issues

### CR-01: [BLOCKER] Valid release assets are always reported as mismatched

**File:** `scripts/phase2_outcomes.sh:28`
**Issue:** `actual` is sorted at line 33, but this command substitution sorts only the final `echo SHA256SUMS` pipeline element. The archive names remain in generation order, which differs from lexical order (for example, `runner` precedes `libretro`, and `x64` precedes `arm64`). Therefore a release with exactly the expected 18 archives plus `SHA256SUMS` fails the `actual != expected` comparison and is reported as a release asset mismatch.
**Fix:** Sort the complete generated list, for example:

```sh
expected=$(
  {
    for c in library runner libretro; do
      for o in linux macos windows; do
        for a in x64 arm64; do
          echo "nesturbator-$version-$c-$o-$a.zip"
        done
      done
    done
    echo SHA256SUMS
  } | LC_ALL=C sort
)
```

## Warnings

### WR-01: [WARNING] In-progress nightly run is classified as a failure

**File:** `scripts/phase2_outcomes.sh:58-61`
**Issue:** When the run is not yet completed, the script prints `PENDING (...)` but sets `failed=1`. Its final exit path then returns 1 instead of the documented pending result (exit 2), making polling/reporting callers treat ordinary in-progress work as a failure.
**Fix:** Branch on `.status` before the completion/conclusion check: set `pending=1` and return when status is not `completed`; set `failed=1` only for a completed unsuccessful run.

### WR-02: [WARNING] Required no-FP CI job has little timeout headroom

**File:** `.github/workflows/ci.yml:134`
**Issue:** The `nofp` job has a one-minute timeout. The phase verification records a run of 38 seconds on `ubuntu-24.04` (over 60% of that limit), while the timeout comment is based on the 15-second ARM run. Normal runner startup or test-time variation can cancel this required job despite correct code.
**Fix:** Set a timeout with meaningful headroom for the slower matrix leg (for example, two minutes) and update the measurement comment to reflect the slowest runner.

---

_Reviewed: 2026-10-06T14:00:00Z_
_Reviewer: the agent (gsd-code-reviewer)_
_Depth: standard_
