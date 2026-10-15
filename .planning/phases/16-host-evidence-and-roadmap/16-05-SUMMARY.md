---
phase: 16-host-evidence-and-roadmap
plan: "05"
subsystem: testing
tags: [libretro, cmake, metadata, mmc3, packaging]

requires:
  - phase: "16-04"
    provides: [verified host evidence and fixture identity controls]
provides:
  - truthful localized NTSC MMC3 support metadata for mapper 4 submappers 0/4
  - automated source, documentation, staged-install and package consistency checks
affects: [libretro metadata, package verification, EVID-04, release evidence]

actuals:
  tokens: 5916.75
  tasks: 2
  commits: 3
commits: 3
plan_head_before: 12cecd7afcff1dbda027d52b335e1da3c2d088f7
plan_head_after: c50e6962b40ec0f663db43f5860533199618410b

tech-stack:
  added: []
  patterns:
    - localized support facts are checked independently of surrounding product prose
    - installed and archived metadata are byte-compared before the shared support parser runs

key-files:
  created: []
  modified:
    - libretro/nesturbator_libretro.info
    - tests/cmake/info_claims.cmake
    - tests/cmake/check_archives.cmake
    - tests/CMakeLists.txt
    - README.md
    - include/nesturbator.h

key-decisions:
  - "Keep the support contract as one narrow localized clause: NTSC MMC3 mapper 4, submappers 0/4, with MMC6 and four-screen excluded."
  - "Record current staged and package acceptance, but leave EVID-04 pending until ordinary release publication evidence exists."

patterns-established:
  - "Use the same narrow support parser for source metadata, the fresh component install, and the extracted package entry."
  - "Archive controls require specific absent/stale reasons so extraction errors cannot masquerade as metadata validation."

requirements-completed: []
requirements-pending: [EVID-04]
coverage:
  - id: D1
    description: "The libretro description and documentation agree on the implemented NTSC MMC3 mapper and submapper scope, with MMC6 and four-screen excluded."
    verification:
      - kind: unit
        ref: "ctest --preset ci -R '^(libretro.info_(support|claims.*)|core.cartridge)$' --no-tests=error"
        status: pass
      - kind: integration
        ref: "cmake --workflow --preset ci (646/646 tests for Task 1)"
        status: pass
    human_judgment: false
  - id: D2
    description: "Fresh staged and current package metadata matches the checked source and passes the same localized support checks."
    verification:
      - kind: integration
        ref: "cmake --workflow --preset ci (648/648 tests and CPack)"
        status: pass
      - kind: integration
        ref: "cmake -DDIR=build/ci -P tests/cmake/check_archives.cmake (three current archives)"
        status: pass
      - kind: unit
        ref: "libretro.info_archives.self_test (absent, stale and current synthetic archives)"
        status: pass
    human_judgment: false
  - id: D3
    description: "Published release evidence for EVID-04 remains outstanding."
    verification: []
    human_judgment: true
    rationale: "This plan verifies source, staged and packaged bytes; it does not publish a release or observe its tag and artifact."

duration: 25min
completed: 2026-10-15
status: complete
---

# Phase 16 Plan 05: Truthful MMC3 Metadata Summary

**Libretro metadata, documentation, staged installs and package archives now agree on the implemented NTSC MMC3 profile without implying release publication.**

## Performance

- **Duration:** 25 min
- **Started:** 2026-10-15T00:46:46.972Z
- **Completed:** 2026-10-15T01:12:14Z
- **Tasks:** 2
- **Files modified:** 6

## Accomplishments

- Replaced the outdated `.info` description with a localized support clause for NTSC MMC3 mapper 4, submappers 0/4, explicitly excluding MMC6 and four-screen.
- Added a focused shared parser and independent controls for metadata, README and header facts while retaining the existing capability, savestate and cartridge-admission checks.
- Added a fresh isolated libretro component install check and actual ZIP extraction, byte agreement and support validation. Synthetic archives prove absent and stale metadata fail for their intended reasons while current metadata passes.
- Kept release status accurate: current staged and package contents are verified, while EVID-04 remains pending until normal release publication evidence is available.

## Task Commits

1. **Task 1 RED:** `7539ae9` — require the truthful MMC3 metadata clause and register its support probe.
2. **Task 1 GREEN:** `76f8b29` — publish and validate the localized MMC3 support facts.
3. **Task 2:** `c50e696` — verify staged and archived metadata.

The final SDK metadata commit records this summary and state updates separately.

## Files Created/Modified

- `libretro/nesturbator_libretro.info` — frontend-facing support description.
- `tests/cmake/info_claims.cmake` — narrow shared support parser, controls and isolated staged-install check.
- `tests/cmake/check_archives.cmake` — source agreement and parser validation for actual ZIP metadata, with synthetic controls.
- `tests/CMakeLists.txt` — ordinary CI registration for source, staging and archive checks.
- `README.md` and `include/nesturbator.h` — matching supported-profile fact, with existing detailed limits preserved.

## Decisions Made

- The exact localized support clause is the checked interface; surrounding natural-language prose stays flexible.
- Source, staged and archived facts are verified as one path, but successful packaging is not represented as a published release.
- EVID-04 stays pending until the normal release process supplies actual tag, artifact and publication evidence.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Trimmed whitespace around semicolon-separated support facts**
- **Found during:** Task 1
- **Issue:** The parser treated the intended spaces after semicolons as part of the exclusion facts and rejected the valid support clause.
- **Fix:** Normalize each localized fact before exact comparison, while preserving the exact fact count.
- **Files modified:** `tests/cmake/info_claims.cmake`
- **Verification:** Focused source/support/cartridge tests and the full CI workflow passed.
- **Committed in:** `76f8b29`

**2. [Rule 1 - Bug] Corrected the stale archive control fixture content**
- **Found during:** Task 2
- **Issue:** The synthetic stale archive was initially empty because the CMake string replacement used the wrong output argument, so the control failed before reaching its intended source-disagreement check.
- **Fix:** Generate the stale `.info` file from the current source with only surrounding description wording changed, then package that file into the synthetic ZIP.
- **Files modified:** `tests/cmake/check_archives.cmake`
- **Verification:** Archive self-test reports the expected stale-metadata disagreement; the absent and current controls also pass.
- **Committed in:** `c50e696`

**Total deviations:** 2 auto-fixed (Rule 1). Both were directly related to the task's new checks.

## TDD Gate Compliance

| Task | RED evidence | GREEN verification | Status |
|---|---|---|---|
| 1 | `build/ci/info-support-red.json`; `gsd_run check tdd-red-evidence` returned `RED_EVIDENCE_OK` for the intended missing-clause failure. | `76f8b29`; focused checks passed and full CI passed 646/646. The post-commit tracer rerun also passed 646/646 and packaged all three components. | Pass |
| 2 | Not a TDD task. | `c50e696`; focused metadata checks passed 6/6, full CI passed 648/648 and the three actual archives passed the explicit checker. | Pass |

## Verification

- `cmake --workflow --preset ci` passed 648/648 tests and produced the library, libretro and runner archives.
- `cmake --preset ci` followed by focused source, staged, archive-control and cartridge checks passed 6/6.
- `cmake -DDIR=build/ci -P tests/cmake/check_archives.cmake` confirmed exactly three current archives and validated the actual libretro `.info` entry.
- `bash scripts/hygiene.sh` and `cmake --workflow --preset hygiene` passed; hygiene reported 13/13 tests. The explicit clang-format 18 check for `include/nesturbator.h` and `git diff --check` also passed.
- The staged test installs the current libretro component into a freshly cleared, build-owned directory, checks the source/staged SHA-256 and validates that installed file with the shared parser.
- No emulator admission behavior, package version, dependency or public declaration changed. No release was published.

## Issues Encountered

The existing `retroarch.session.controls` test took approximately five minutes in each full CI run and completed within its configured timeout. No failures remain.

## User Setup Required

None.

## Next Phase Readiness

Plan 16-05's source, documentation, staging and package checks are complete. EVID-04 remains pending until actual release publication evidence is recorded; no tag, upload or publication action was part of this plan.

---
*Phase: 16-host-evidence-and-roadmap*
*Completed: 2026-10-15*

## Self-Check: PASSED

- The summary file exists at the required phase path.
- All three task commits are ancestors of the plan head recorded above.
- The plan ledger measured three commits from `12cecd7afcff1dbda027d52b335e1da3c2d088f7` through `c50e6962b40ec0f663db43f5860533199618410b` before this summary update.
