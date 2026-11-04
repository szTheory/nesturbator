---
phase: 25-region-selection-and-pal-timing
plan: "05"
subsystem: region-selection
tags: [NES, PAL, NTSC, libretro, C, CTest, acceptance]
requires:
  - phase: 25-03
    provides: Source-derived PAL raster, APU cadence, PCM and public-tone expectations
  - phase: 25-04
    provides: Runner and real libretro region-option lifecycle proof
provides:
  - Truthful core option metadata with real adapter lifecycle and negative-control evidence
  - Owned six-mapper PAL/NTSC frame, sample and pulse-pitch acceptance
  - Final candidate-bound full CI and package receipt with AccuracyCoin fixture closure
  - Explicit review dispositions and public documentation of unverified PAL reset duration
affects: [Phase 25 acceptance, libretro metadata, PAL regional verification]
actuals:
  tokens: 17366.75
  tasks: 3
  commits: 12
plan_head_before: 2b37dc10ced801b42261e2c7bcfa22ce735af858
plan_head_after: 7a2ad0afc873391e4288311418d52e894fe2fffb
tech-stack:
  added: []
  patterns:
    - Owned board-valid fixtures exercise region profiles through the public load/frame API
    - Source-derived integer and rational expectations remain separate from adapter parity evidence
key-files:
  created:
    - .planning/phases/25-region-selection-and-pal-timing/25-ACCEPTANCE.md
    - .planning/phases/25-region-selection-and-pal-timing/25-REVIEW-DISPOSITION.md
  modified:
    - libretro/nesturbator_libretro.info
    - tests/cmake/info_claims.cmake
    - tests/core/test_mapper.c
    - tests/core/test_frame.c
    - include/nesturbator.h
    - README.md
    - docs/api.md
    - docs/testing.md
    - tests/accuracy/baseline-metadata.txt
    - tests/cmake/phase21_hosted_evidence_controls.cmake
    - tests/core/test_synth.c
    - tests/libretro/libretro_host.c
    - tests/runner/hashes.txt
key-decisions:
  - "Advertise core_options=true only after the real adapter lifecycle test demonstrated registration, query and reload behavior."
  - "Use owned source-arithmetic fixtures for all six existing mapper families and retain NTSC guards."
  - "Document PAL reset-release behavior at line 311 as an unverified model; do not invent a duration without a pinned source."
  - "Keep all four inherited fallback rows and both descriptor-less prohibitions flagged and unresolved."
  - "Accept the exact authorized full local workflow on the final code candidate; do not extend work to hosted CI, publication or later phases."
requirements-completed: [REG-01, PAL-01]
coverage:
  - id: D1
    description: "The libretro option declaration agrees with the tested host lifecycle, and metadata false/missing/duplicate controls remain enforced."
    requirement: REG-01
    verification:
      - kind: integration
        ref: "ctest build/dev: libretro.vendored, libretro.info_claims, libretro.info_claims.self_test, libretro.info_support, libretro.no_quirks, docs.links (6/6); docs.links after API revision correction (1/1)"
        status: pass
    human_judgment: false
  - id: D2
    description: "Owned PAL and NTSC timing, sample and pulse-pitch checks cover mapper IDs 0, 1, 2, 3, 4 and 7, with source and hardware limits recorded."
    requirement: PAL-01
    verification:
      - kind: integration
        ref: "focused build and CTest for mapper, cartridge, frame, APU, runner.spectral and libretro.host (10/10)"
        status: pass
    human_judgment: false
  - id: D3
    description: "The final candidate passes full unfiltered local CI and packages the library, libretro adapter and runner."
    requirement: REG-01
    verification:
      - kind: workflow
        ref: "cmake --workflow --preset ci on fc48e4575264422df9a739795bc5286b840b8dc9, tree e328e142f810f5029d59508280d19100a0fee4a8; CTest 712/712 and package success; build/ci/25-05-task3-final-fingerprint-fixed-workflow.log"
        status: pass
    human_judgment: false
duration: 115min
completed: 2026-11-02
status: complete
---

# Phase 25 Plan 05: Region Metadata and Six-Mapper PAL Acceptance Summary

**Truthful libretro option metadata and independent six-mapper PAL/NTSC timing checks passed, followed by full 712-test CI and packaging on the final code candidate.**

## Performance

- **Duration:** 115 min
- **Started:** 2026-11-01T23:48:58Z
- **Completed:** 2026-11-02T01:43:31Z
- **Tasks:** 3 of 3 complete
- **Files modified:** 17 plan-scoped code, documentation, acceptance, review and summary files (plus separate GSD state bookkeeping)

## Accomplishments

- Advertised `core_options=true` after Plan 25-04's real module load/query/reload proof; metadata negative controls still reject false, missing and duplicate claims.
- Added a real public-API PAL/NTSC matrix for NROM, MMC1, UxROM, CNROM, MMC3 and AxROM. Source-derived frame, sample and pulse-pitch expectations distinguish PAL from NTSC and rounded sample accounting.
- Corrected six stale test contracts found by the first full workflow, refreshed the final 56-input build fingerprint after the API/header documentation update, then passed the exact `ci` workflow with all 712 tests and package step.
- Recorded final workflow, campaign report, canonical scoreboard and all three package hashes in [25-ACCEPTANCE.md](25-ACCEPTANCE.md).
- Corrected the API guide's behavior revision. Documented PAL soft-reset register inhibition at pre-render line 311 as an unverified model because pinned evidence does not establish PAL release duration; no unsupported timing or production behavior change was introduced.
- Kept all four inherited fallback rows and both descriptor-less prohibitions flagged, including the reset-duration caveat in the PAL adjacency/reset/reload fallback.

## Task Commits

1. **Task 1: Advertise the exercised option and keep metadata claims enforceable** — `d5bef9a` (`feat`)
2. **Task 2: Six-mapper PAL timing/pitch checks and testing guide evidence** — `9fcfbda` (`test`)
3. **Task 2: Candidate-bound acceptance record** — `e3adc1d` (`docs`)
4. **Partial-plan checkpoint record** — `7640b6a` (`docs`)
5. **Record local CI authority** — `0ace68f` (`docs`)
6. **Repair six focused test/metadata contracts** — `0481ffd` (`fix`)
7. **Record failed full gate and focused repairs** — `a68bed7` (`docs`)
8. **Record durable standing local-CI authority** — `caf1b2d` (`docs`)
9. **Correct API revision and document PAL reset limitation** — `5a17229` (`docs`)
10. **Refresh AccuracyCoin input fingerprint** — `fc48e45` (`fix`)
11. **Record code-review finding dispositions** — `8ef512d` (`docs`)
12. **Record final full-CI acceptance receipt** — `7a2ad0a` (`docs`)

## Final Candidate and Verification

- **Code candidate:** `fc48e4575264422df9a739795bc5286b840b8dc9`; tree `e328e142f810f5029d59508280d19100a0fee4a8`. Later commits contain review and plan records only; no source, test, CMake or package input changed after the verified candidate.
- **Workflow:** `cmake --workflow --preset ci`; configure → build → full unfiltered CTest → package; exit 0, 524.72 seconds total, no timeout under the 1,800-second outer cap. All 712 tests passed in 522.75 seconds with four workers. Campaign and protected-observation fixtures passed within their existing 370- and 760-second CTest caps. Campaign caps remained BOOT 32s, navigation 5s, execution 1,800s, page 15s, collect 10s; no tests were filtered or excluded.
- **Workflow log:** `build/ci/25-05-task3-final-fingerprint-fixed-workflow.log`, SHA256 `27ee7d746c83140d1f43be217f3fa27a97a4c0d22195a6f25b76e9b3326db0d8`.
- **AccuracyCoin:** 22 pages, 146 scored, 126 pass, 20 fail, 0 skip/not-run/timeout. Report SHA256 `f3b5bbf7ddcf4d18deabda6d875b441330dc71c97b8aca5fdcc38bf471f98f6c`; canonical scoreboard SHA256 `9ec5d50d291d2713f5281851f642270498a4e8b0dab273b8b72cd19b66ff111b`; 56-input fingerprint `cf07b0022d450a5bd7e58342aca6b16ed593acd022b65826b24f01061fed1089`.
- **Packages:** library `9e3f4e7cffac1acc6e9304951b0c3cd6d21f5c9fec7ad432c814116f493de165`; libretro `2a656331e587665f216279fc6a32a632b77da028ded09370a1e1a97072cfb3ff`; runner `9d96ce56d473fa1187faa68cfad405641fd60b3d7590c37f3952552f430ab8d1`.
- **Focused checks:** six metadata/docs checks passed 6/6; after the API revision fix, `docs.links` passed 1/1. The six-mapper regional focused selection passed 10/10.

## Code Review Dispositions

The bounded review in [25-REVIEW.md](25-REVIEW.md) found two warnings. The per-finding ledger is [25-REVIEW-DISPOSITION.md](25-REVIEW-DISPOSITION.md):

- **WR-01 — fixed:** `docs/api.md` now reports behavior revision 16 and includes the revision-16 PAL APU entry. The focused documentation links test passed.
- **WR-02 — deferred and disclosed:** source clears PAL reset inhibition at pre-render dot 1 on line 311, but pinned references do not establish that as the PAL hardware release duration. Public documentation calls this an unverified model behavior. The PAL adjacency/reset/reload fallback remains flagged; this does not extend the source-supported Phase 25 timing claim.

## Deviations from Plan

### Auto-fixed issues

**1. [Rule 1 - stale acceptance contracts] Updated six tests/metadata expectations identified by the initial full workflow.**
- **Found during:** Task 3
- **Issue:** Six expectations were stale relative to behavior revision 16 and current rejected-load/cycle-cursor contracts.
- **Fix:** Updated only the affected expected hashes, metadata revision assertions, cycle cursor setup and rejected-load preservation assertion; refreshed only the final input fingerprint when the later header documentation changed a covered input.
- **Files modified:** `tests/runner/hashes.txt`, `tests/cmake/phase21_hosted_evidence_controls.cmake`, `tests/accuracy/baseline-metadata.txt`, `tests/core/test_synth.c`, `tests/libretro/libretro_host.c`
- **Verification:** Focused retests passed and the final full workflow passed 712/712 plus packaging.
- **Committed in:** `0481ffd`, `fc48e45`

**2. [Rule 3 - wrapper output failure] Switched the bounded workflow wrapper to direct file output.**
- **Found during:** Task 3 final verification
- **Issue:** An earlier wrapper that relayed child output through a pipe failed at its output sink; it did not establish a product or CTest failure.
- **Fix:** Preserved the attempt logs and used a minimal process-group wrapper that redirected child stdout/stderr directly to a new ignored log, emitted periodic status only, and enforced 1,800 seconds plus 10 seconds TERM grace.
- **Verification:** Final wrapper reports exit 0, no timeout and no remaining process group; full CTest and package step passed.
- **Recorded in:** `25-ACCEPTANCE.md`; no production change was required for the wrapper correction.

The review warning WR-02 was not auto-fixed in production because available pinned evidence does not determine the correct PAL release duration. Its limitation is explicit in the header/API and acceptance records.

**Total deviations:** 2 execution/test-contract corrections and 1 documented review limitation.
**Impact on plan:** Required CI completed on the final code candidate. No unrelated CI, package, campaign or production behavior changes were made. No CI-efficiency improvement is claimed.

## Issues Encountered

- The first full workflow on `9fcfbda` ran 712 tests and failed six stale assertions (706 passed). All six were diagnosed, corrected and retested. Later, documentation changes altered a fingerprinted header input, so only the 56-input fingerprint metadata required refresh; retained report and scoreboard hashes were rechecked before activation.
- A wrapper output-sink failure was diagnosed separately from the workflow. Logs were retained, no process remained, and the historical RetroArch session-control corpus was preserved.
- The PAL reset-inhibit duration remains unverified against PAL-specific source evidence. This is disclosed as a modeled limitation rather than treated as a proved hardware timing claim.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

Phase 25's source-supported region selection and PAL timing/pitch deliverables have focused and full-workflow evidence. REG-01 and PAL-01 are marked complete. The independent goal-backward verifier passed 23/23 truths with no gaps, behavior-unverified items or UAT; see [25-VERIFICATION.md](25-VERIFICATION.md). Four descriptor-less fallback rows and both prohibitions remain flagged, and PAL reset-release duration remains explicitly unverified. Phase 26/27 and hosted or publication actions were outside this plan and were not started.

---
*Phase: 25-region-selection-and-pal-timing*
*Completed: 2026-11-02*

## Self-Check: PASSED

- Summary file exists at the canonical Plan 25-05 path.
- All twelve recorded plan commits are ancestors of the current branch head.
- `git diff --check` reported no whitespace errors.
