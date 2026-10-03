---
phase: 01-a-test-frame-in-retroarch
plan: 06
subsystem: packaging
tags: [install, cmake-package, pkg-config, cpack, archives, frame-02, frame-07]
status: complete

requires:
  - phase: 01-05
    provides: "nesturbator_libretro MODULE, nesturbator_libretro.info and THIRD-PARTY-NOTICES.md"
provides:
  - "Install components library, runner and libretro, each with LICENSE; THIRD-PARTY-NOTICES.md in libretro"
  - "Relocatable CMake package (nesturbatorConfig.cmake, ConfigVersion SameMinorVersion, export nesturbator::nesturbator) and a ${pcfiledir}-relative nesturbator.pc"
  - "ctest install.stage (fixture staged) and install.consumer (FRAME-02), forwarding compiler, build type, CMAKE_C_FLAGS and CMAKE_EXE_LINKER_FLAGS"
  - "cmake/packaging.cmake: NESTURBATOR_OS, NESTURBATOR_ARCH, three CPack ZIP component archives in <build>/packages"
  - "packagePresets ci and ci-msvc and a package step closing the ci and ci-msvc workflows"
  - "tests/cmake/check_archives.cmake (-DDIR or -DPACKAGES, optional -DVERSION) for local and CI use"
affects: [01-07, 01-10, 01-12]

actuals:
  tokens: 4654
  tasks: 2
  commits: 2
plan_head_before: f35747d38908a4af1dc029f958307e5bee2622e0
plan_head_after: 6420a664112eaff7f57532015418a1674d576117

tech-stack:
  added: ["CPack ZIP generator (built into CMake)"]
  patterns:
    - "Install fixture: ctest install.stage sets up `staged`; install.consumer runs ctest --build-and-test against it"
    - "Archive check as a cmake -P script with an allow-list of entry regexes per component: missing and extra entries both fail"

key-files:
  created:
    - cmake/nesturbatorConfig.cmake.in
    - cmake/nesturbator.pc.in
    - cmake/packaging.cmake
    - tests/consumer/CMakeLists.txt
    - tests/consumer/main.c
    - tests/cmake/check_archives.cmake
  modified:
    - CMakeLists.txt
    - runner/CMakeLists.txt
    - libretro/CMakeLists.txt
    - tests/CMakeLists.txt
    - CMakePresets.json
    - README.md

key-decisions:
  - "nesturbator.pc computes its prefix from CMAKE_INSTALL_LIBDIR with file(RELATIVE_PATH), so lib, lib64 or a deeper libdir all resolve from ${pcfiledir}"
  - "install.stage passes --config $<CONFIG> so it installs the built configuration under single- and multi-config generators alike"
  - "check_archives ignores directory entries and fails on any file not on its component's allow-list, so the layout check is exact"

patterns-established:
  - "cmake/ holds package templates and packaging.cmake, included last from the top-level CMakeLists.txt"

requirements-completed: [FRAME-02, FRAME-07]

coverage:
  - id: D1
    description: "A consumer using only <nesturbator.h> builds with find_package against the staged install and runs one frame"
    requirement: FRAME-02
    verification:
      - kind: integration
        ref: "tests/CMakeLists.txt#install.consumer"
        status: pass
      - kind: command
        ref: "ls build/ci/stage/include/nesturbator.h build/ci/stage/lib/cmake/nesturbator/nesturbatorConfig.cmake build/ci/stage/lib/pkgconfig/nesturbator.pc"
        status: pass
    human_judgment: false
  - id: D2
    description: "cmake --workflow --preset ci writes exactly the three correctly named archives with exact layouts"
    requirement: FRAME-07
    verification:
      - kind: command
        ref: "cmake --workflow --preset ci (3 'CPack: - package:' lines)"
        status: pass
      - kind: command
        ref: "cmake -DDIR=build/ci -P tests/cmake/check_archives.cmake"
        status: pass
    human_judgment: false
  - id: D3
    description: "The pkg-config file resolves for an outside consumer"
    requirement: FRAME-02
    human_judgment: true
    rationale: "pkg-config is not installed on the dev machine and no test exercises nesturbator.pc in Phase 1 (plan's flagged assumption)"

duration: 3 min
completed: 2026-10-02
---

# Phase 01 Plan 06: Install package and release archives Summary

**The library installs as a relocatable CMake package plus nesturbator.pc, a find_package consumer proves FRAME-02 in ctest, and every `ci` run ends with the three exactly named and exactly laid-out release zips.**

## Performance

- **Duration:** 3 min
- **Started:** 2026-10-02T21:11:49Z
- **Completed:** 2026-10-02T21:14:24Z
- **Tasks:** 2
- **Files modified:** 12

## Accomplishments

- Install components `library`, `runner` and `libretro`; `nesturbator_host`, `palgen` and the tests are not installed.
- `install.stage` + `install.consumer` (25 tests in total now) build `tests/consumer` against `build/ci/stage` with the parent's compiler and C/link flags, so the asan lane in plan 07 can link it.
- `cmake --workflow --preset ci` ends with `nesturbator-0.0.0-{library,runner,libretro}-macos-arm64.zip` in `build/ci/packages/`.
- `check_archives.cmake` passed on the real output and failed as expected on four hand-made bad sets: an extra file, a missing LICENSE, a fourth zip of the same version, and (ignored, correctly) a zip of another version.
- README gained "Using the library" and "Downloads and archives" sections.

## Task Commits

1. **Task 1: Install components, CMake package and pkg-config, FRAME-02 consumer** - `63af915` (feat)
2. **Task 2: CPack ZIP archives, package step, exact-archive check** - `6420a66` (feat)

## Files Created/Modified

- `cmake/nesturbatorConfig.cmake.in` - package config including nesturbatorTargets.cmake
- `cmake/nesturbator.pc.in` - pkg-config file with a ${pcfiledir}-relative prefix
- `cmake/packaging.cmake` - OS/arch mapping and CPack component archive names
- `tests/consumer/` - FRAME-02 consumer project and program
- `tests/cmake/check_archives.cmake` - exact archive name and layout check
- `CMakeLists.txt`, `runner/CMakeLists.txt`, `libretro/CMakeLists.txt` - install rules
- `tests/CMakeLists.txt` - install.stage and install.consumer
- `CMakePresets.json` - packagePresets and package workflow steps
- `README.md` - install, package and archive documentation

## Decisions Made

See key-decisions in the frontmatter.

## Deviations from Plan

None - plan executed exactly as written. (The relative-prefix computation for nesturbator.pc and `--config $<CONFIG>` on install.stage are implementation details within the plan's instructions; the README install section was added in Task 1 per CLAUDE.md rule 6.)

## Issues Encountered

None. A fresh `build/ci` and the `dev` workflow both pass; the unpacked archives contain no home-directory paths.

## Next Phase Readiness

Ready for 01-07. The CI legs in plan 10 can upload `build/ci/packages/*.zip` and run `check_archives.cmake -DPACKAGES=...`. A13 (MSVC `CMAKE_C_COMPILER_ARCHITECTURE_ID`) is still to be confirmed by the first `ci-msvc` run.

## Self-Check: PASSED

- All six created files exist; commits 63af915 and 6420a66 are in the log.
- `cmake --workflow --preset ci`: 25/25 tests passed, three CPack packages.
- `cmake -DDIR=build/ci -P tests/cmake/check_archives.cmake`: exit 0.
- Acceptance greps: includes 3, nesturbator.h 1, CMAKE_EXE_LINKER_FLAGS 1, package steps 2.
