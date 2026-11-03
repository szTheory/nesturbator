---
phase: 28-qualified-nes-gameplay-package
plan: "02"
subsystem: testing
tags: [cmake, ctest, release-provenance, ntsc]
requires:
  - phase: 28-qualified-nes-gameplay-package/28-01
    provides: relocated consumer replay and the permitted DABG gameplay recipe
provides:
  - Source-checked NTSC provider profile with candidate-only publication status
  - Offline profile and release-receipt controls plus a fail-closed published receipt gate
affects: [28-03 acceptance, package publication, provider integration]
actuals:
  tokens: 25070
  tasks: 2
  commits: 5
  plan_head_before: 1181d61e94a4c47bbe6a2d15076142dd21befe9a
  plan_head_after: d080fdee024f23b6ee83b6380fa3cee9d94e38a9
tech-stack:
  added: []
  patterns:
    - Offline CMake mutation controls for provider facts and receipt identities
    - Published evidence is generated only after source, archive, provenance, and native replay checks
key-files:
  created:
    - docs/provider-ntsc.md
    - docs/provider-ntsc.json
    - tests/cmake/provider_profile.cmake
    - tests/cmake/provider_receipt.cmake
  modified:
    - docs/api.md
    - docs/releases.md
    - docs/testing.md
    - docs/reproduction.md
    - tests/CMakeLists.txt
key-decisions:
  - "The profile stays candidate-only until a verified receipt binds the exact published package and scenario."
  - "Receipt CI entries must match the expected candidate or release source, workflow, and trigger event."
patterns-established:
  - "Keep cheap offline receipt controls in CTest and run network-bound publication validation only at closeout."
requirements-completed: []
coverage:
  - id: D1
    description: "Source-checked provider profile states the exact NTSC API contract and its qualification limits."
    requirement: PROF-01
    verification:
      - kind: unit
        ref: "ctest --test-dir build/ci -R '^provider\\.profile$'"
        status: pass
      - kind: unit
        ref: "ctest --test-dir build/ci -R '^provider\\.profile\\.controls$'"
        status: pass
    human_judgment: true
    rationale: "The final published-source and package binding is intentionally pending until the live receipt passes."
  - id: D2
    description: "Offline controls reject inconsistent receipt identities; published mode checks release assets, attestations, and relocated native replay."
    requirement: PKG-01
    verification:
      - kind: unit
        ref: "ctest --test-dir build/ci -R '^provider\\.receipt\\.controls$'"
        status: pass
    human_judgment: true
    rationale: "No actual release was verified in this plan; Plan 03 and the separate shipping step own publication acceptance."
duration: 49min
completed: 2026-11-03
status: complete
---

# Phase 28 Plan 02: NTSC provider profile and publication receipt Summary

**A source-backed NTSC provider contract and offline receipt gate now prepare the exact published-package handoff without claiming that publication has passed.**

## Performance

- **Duration:** 49 min
- **Started:** 2026-11-03T15:56:59Z
- **Completed:** 2026-11-03T16:45:37Z
- **Tasks:** 2
- **Files modified:** 9

## Accomplishments

- Added a readable and machine-checkable NTSC profile with exact cadence, buffer, pixel, input, lifecycle, region, and qualification limits; offline controls compare it with the public header, source arithmetic, scenario recipe, and optional runtime report.
- Added a receipt checker that separates structural fixtures from live evidence, checks all 18 release archives and `SHA256SUMS`, verifies archive attestations and source identity, reuses the project archive checker for six targets, and replays the native package after relocation.
- Bound receipt CI records to the expected candidate/release source, workflow, and event. Documented the recurring offline checks and the one-time published receipt handoff.

## Task Commits

1. **Task 1 RED: provider contract controls** — `5dd923c` (`test(28-02): define provider contract checks`)
2. **Task 1 GREEN: provider profile** — `3d220e5` (`feat(28-02): document qualified NTSC provider profile`)
3. **Task 2: published receipt gate and documentation** — `33403f8` (`feat(28-02): add fail-closed published package receipt gate`)
4. **Task 2 RED: CI source identity control** — `cce4d23` (`test(28-02): reject receipt CI source identity drift`)
5. **Task 2 GREEN: bind CI evidence** — `d080fde` (`fix(28-02): bind receipt CI runs to expected commits`)

The plan metadata commit is created after the summary and state updates.

## Verification

- `cmake --preset ci` — passed.
- `provider.profile`, `provider.profile.controls`, and `provider.receipt.controls` — all passed individually through CTest.
- The new CI identity control first failed on its intended assertion: a valid but unrelated main CI head was accepted. After the fix, the synthetic complete receipt passes and the mismatched head reaches `ci-identity-mismatch`.
- `git diff --check` — passed.
- The bounded full workflow and live release check were not run here; Plan 03 owns the frozen-candidate workflow, and actual publication remains a separate shipping/closeout gate.

## Files Created/Modified

- `docs/provider-ntsc.md` and `docs/provider-ntsc.json` — provider-side contract and fact table.
- `tests/cmake/provider_profile.cmake` — source/runtime fact checks and offline mutations.
- `tests/cmake/provider_receipt.cmake` — offline receipt controls and bounded published-package validation.
- `tests/CMakeLists.txt` — registers the receipt controls in CTest.
- `docs/api.md`, `docs/releases.md`, `docs/testing.md`, and `docs/reproduction.md` — link the profile and describe the release/replay boundary.

## Decisions Made

- Keep publication status candidate-only until the exact released source, package bytes, and scenario pass the live receipt gate.
- Store exact candidate, main, and release-source CI identities in the receipt and validate each against its expected source, workflow, and event.
- Leave PROF-01 and PKG-01 open until Plan 03 and the actual published receipt complete their exact-package binding.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Reject receipt CI evidence bound to the wrong commit**
- **Found during:** Task 2 receipt review
- **Issue:** The structural validator accepted a successful main CI record with a valid SHA that did not match the release source.
- **Fix:** Added a failing control, then required each linked CI record to match its expected source SHA, workflow, and event.
- **Files modified:** `tests/cmake/provider_receipt.cmake`
- **Verification:** The targeted control failed before the fix on the intended mismatch and passed afterward; all three focused provider CTest targets pass.
- **Committed in:** `cce4d23` (control) and `d080fde` (fix)

**Total deviations:** 1 auto-fixed correctness issue. The additional check tightens the planned exact-CI identity binding without changing workflow scope.

## TDD Gate Compliance

Task 1 followed RED then GREEN (`5dd923c`, `3d220e5`). Task 2’s initial receipt implementation was committed before its regression test was written. A later focused CI identity gap was handled with a valid RED/GREEN pair (`cce4d23`, `d080fde`), but that pair does not retroactively make the broader Task 2 implementation test-first. This sequencing deviation is recorded rather than treated as full TDD compliance.

## Issues Encountered

No authentication or network gate occurred. Published mode was not invoked; it is the one-time closeout check, not a recurring Plan 02 test.

## User Setup Required

None.

## Next Phase Readiness

Plan 28-03 can now freeze the exact candidate and consume the provider profile, recipe, and receipt controls. It owns the final bounded local workflow and acceptance handoff. The published receipt and final PROF-01/PKG-01 qualification remain pending; no user action is needed for this transition.

## Self-Check: PASSED

The summary exists, and all five task commits are ancestors of the current phase branch.

---
*Phase: 28-qualified-nes-gameplay-package*
*Completed: 2026-11-03*
