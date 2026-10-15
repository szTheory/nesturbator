---
phase: 16-host-evidence-and-roadmap
plan: "04"
subsystem: testing
tags: [libretro, cmake, fixture-identity, serialization, runahead]

requires:
  - phase: "16-03"
    provides: [verified host-side evidence and test-boundary decisions]
provides:
  - exact case-sensitive final-component roles for Nesteroids and the MMC1 battery fixture
  - copied-input SHA-256 authentication against exact manifest rows before registered host launch
  - adversarial path and identity controls for EVID-03
affects: [libretro tests, fixture validation, Phase 17 conformance]

actuals:
  tokens: 6755.25
  tasks: 3
  commits: 7
plan_head_before: c67ebdc0921a3edf7fddb5ff98f412101f8f62f3
plan_head_after: cd9de46cd7c02b38e71d667f4348ed085474d99f

tech-stack:
  added: []
  patterns:
    - registered CMake wrapper copies and hashes the exact file supplied to the host
    - private final-component equality keeps host role selection separate from file authentication

key-files:
  created: [tests/cmake/fixture_identity.cmake]
  modified: [tests/libretro/libretro_host.c, tests/CMakeLists.txt, README.md, include/nesturbator.h]

key-decisions:
  - "Special host roles require an exact case-sensitive final filename; slash always separates, while backslash also separates only on Windows."
  - "Only registered CMake tests authenticate copied special-file bytes; direct libretro.host calls select exact names without authenticating content."

patterns-established:
  - "Role selection compares the final filename as bytes and performs no path, case, or Unicode normalization."
  - "Registered tests hash the copied input against the exact canonical manifest row before executing libretro.host."

requirements-completed: [EVID-03]
coverage:
  - id: D1
    description: "Exact fixture-role selection and copied-byte identity gating for registered libretro matrices."
    requirement: EVID-03
    verification:
      - kind: unit
        ref: "ctest --preset ci -R '^libretro.fixture_(roles|identity)$' --no-tests=error"
        status: pass
      - kind: integration
        ref: "ctest --preset ci -R '^libretro\\.(fixture_roles|fixture_identity|serialize|runahead)$' --no-tests=error"
        status: pass
      - kind: integration
        ref: "cmake --workflow --preset ci (645/645 tests)"
        status: pass
    human_judgment: false

duration: 54min
completed: 2026-10-15
status: complete
---

# Phase 16 Plan 04: Host Fixture Identity Summary

**Registered libretro matrices authenticate exact fixture bytes and preserve adversarial ROM argument boundaries through the production CMake wrapper.**

## Performance

- **Duration:** 54 min
- **Started:** 2026-10-14T23:40:09Z
- **Completed:** 2026-10-15T00:34:08Z
- **Tasks:** 2 planned tasks and 1 argv-boundary follow-up
- **Files modified:** 5

## Accomplishments

- Replaced both full-path substring selectors with one private final-component helper. Exact names are case-sensitive; `/` separates on every platform and `\\` separates only on Windows. Null, empty and trailing-separator paths select no role.
- Routed the existing serializer and run-ahead matrices through one CMake wrapper. It copies each special input into a build-owned path with spaces, looks up the exact manifest record, hashes the copied bytes, and starts the real host only after every special input passes.
- Added 20 role-selection path cases and nine CMake identity controls. Both wrong-byte special copies fail as `FIXTURE_IDENTITY` before a deliberately unavailable host can launch; missing rows, duplicate rows and missing files have separate reasons.
- Updated the README and public-header test-contract comments. Direct host calls use exact role names but remain outside the registered tests' byte-authentication boundary.
- Added a production-wrapper argv capture control, then fixed scalar CORE forwarding and escaped ROM path semicolons so spaces, semicolons, and supported backslashes arrive as their original arguments.

## Task and Follow-up Commits

1. **Task 1 RED:** `bfd8880` — registered the failing selector-boundary assertion; the test identified both misleading parent paths as special.
2. **Task 1 GREEN:** `447cfcc` — added exact role selection, copied-input authentication, matrix routing and synchronized documentation.
3. **Task 2 RED:** `9c5b974` — added the planned path-control inventory assertion; it reported four existing cases against the required inventory of 18.
4. **Task 2 GREEN:** `3331b0f` — expanded role controls to 20 cases and added nine identity-wrapper controls.
5. **Argv follow-up RED:** `453e724` — recorded actual host argv; the production wrapper delivered seven arguments where four were expected.
6. **Argv follow-up GREEN:** `cd9de46` — preserved CORE as a scalar argument and escaped literal ROM semicolons for CMake list expansion.

## Files Created/Modified

- `tests/cmake/fixture_identity.cmake` — copies, authenticates and forwards registered ROM inputs; includes positive, negative, generic, manifest and actual argv-boundary controls.
- `tests/libretro/libretro_host.c` — shares exact final-component role selection across serialization and run-ahead tests, exercises platform-specific path edges, and records argv in the isolated wrapper self-test mode.
- `tests/CMakeLists.txt` — routes the existing matrices through the wrapper and registers `libretro.fixture_identity` and `libretro.fixture_roles`.
- `README.md` — documents the registered identity boundary and its adversarial coverage.
- `include/nesturbator.h` — records the matching private host-test contract.

## Decisions Made

- Only the exact final filenames `nesteroids.nes` and `M1_P128K_C128K_S8K.nes` select special roles. Parent text, lookalikes, case changes, empty names and trailing separators remain generic.
- The registered test wrapper authenticates the bytes of the file the host will open. Direct host invocations do not gain an authentication service.
- Existing serializer and run-ahead checks remain unchanged in scope and retain all original minimum guards and counters.
- The wrapper passes CORE as a quoted scalar and escapes literal semicolons in each ROM before expanding the argument list.

## Deviations from Plan

The Task 2 RED commit used a reduced root-equality and branch-protection guard rather than the verbatim supplied root-pin block. It verified the same pinned checkout and non-protected phase branch; all other task commits used the full supplied guard. This had no source or behavior impact.

## Follow-up Correction

An independent production probe found that CMake split a semicolon-bearing ROM path during host launch while still returning success. The new C-host capture regression proved the exact delivered argv, including a CORE value with spaces, semicolons and a backslash plus two ROM paths with spaces, semicolons and a backslash. The wrapper now forwards CORE separately and escapes each prepared ROM before list expansion; the adversarial capture passes with the expected four arguments.

## TDD Gate Compliance

| Task | RED evidence | GREEN verification | Refactor | Status |
|---|---|---|---|---|
| 1 | `build/ci/fixture-roles-red.json`: JUnit target failed on the two misleading-parent role assertions; `gsd_run check tdd-red-evidence` returned `RED_EVIDENCE_OK`. | `447cfcc`; focused matrices and full CI passed with baseline counters. | Not needed. | Pass |
| 2 | `build/ci/fixture-roles-task2-red.json`: JUnit target failed the planned self-test inventory assertion (4 cases present, 18 required); classifier returned `RED_EVIDENCE_OK`. | `3331b0f`; all 20 role cases and nine identity controls passed, followed by full CI. | Not needed. | Pass |
| Follow-up | Actual C-host capture failed with 7 delivered arguments instead of 4, proving CORE and ROM path splitting. | `cd9de46`; adversarial capture and all four focused libretro tests passed. | Not needed. | Pass |

## Verification

- Baseline and final serializer counters matched: `roms=5`, `min_refusals=13`, `frames_compared=987`, `battery_roms=1`, `context_queries=0`, `av_queries=0`.
- Baseline and final run-ahead counters matched: `roms=7`, `stress=2`, `ticks=3960`, `repeat_ticks=540`, `states=4525`, `controls_tripped=4`.
- Focused final libretro checks passed: `fixture_identity`, `fixture_roles`, `serialize`, and `runahead` (4/4); the argv capture delivered the exact expected argument count and contents.
- Task 1 tracer feedback gate: `cmake --workflow --preset ci` passed 644/644 tests and packaged the library, runner and libretro adapter.
- Final verification: `cmake --workflow --preset ci` passed 645/645 tests in 296.68 seconds and packaged all three deliverables. `scripts/hygiene.sh` and `git diff --check` passed.
- No ROM bytes were added to the repository; wrong-byte probes contain only text in ignored build output.

## Issues Encountered

The post-plan argv probe exposed one false-success argument-boundary defect; the RED/GREEN follow-up fixed it. No unresolved issues remain. The full CI workflow's existing long RetroArch controls test completed within its configured timeout.

## User Setup Required

None.

## Next Phase Readiness

EVID-03 is complete. The original registered matrices, copied-byte controls and path-role cases are ready for Phase 16 Plan 05; no host CI, release or merge step was needed here.

---
*Phase: 16-host-evidence-and-roadmap*
*Completed: 2026-10-15*

## Self-Check: PASSED

- Summary file exists at the required phase path.
- All six RED/GREEN and follow-up commits are ancestors of the current plan head.
- Persisted plan ledger measured seven commits from the recorded base through `cd9de46` before this summary update.
