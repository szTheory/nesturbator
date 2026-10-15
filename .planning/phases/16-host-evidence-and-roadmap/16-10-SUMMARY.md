---
phase: 16-host-evidence-and-roadmap
plan: "10"
subsystem: testing
tags: [readiness, timing, ctest, frame-index, host-evidence]

requires:
  - phase: 16
    provides: "Readiness supervision, real native-prefix validation, and complete session controls from Plan 09"
provides:
  - "Failure-visible timing diagnostics and bounded removal of overwritten prefix replay"
  - "CI and ASan coverage for both readiness cases, full controls, and progression regressions"
  - "Source-bound timing evidence, independent clean review, and a safe Plan 07 handoff"
affects: [16-07, 16-08, EVID-01, EVID-02, EVID-04]

actuals:
  tokens: 7496
  tasks: 3
  commits: 7
commits: 7
plan_head_before: 4bc40c92e569d8821bc7ab9e47ab5a78b976763c
plan_head_after: 10694f7da7baed83e3d61dca3f687213dbecb183

tech-stack:
  added: []
  patterns:
    - "Emit phase timing evidence before a bound assertion while preserving failure status"
    - "Share a reporter regression with the registered readiness integration path"

key-files:
  created:
    - .planning/phases/16-host-evidence-and-roadmap/16-NATIVE-TIMING-REPAIR.md
    - .planning/phases/16-host-evidence-and-roadmap/16-NATIVE-REVIEW.md
    - .planning/phases/16-host-evidence-and-roadmap/16-NATIVE-REVIEW-DISPOSITION.md
  modified:
    - tests/retroarch/test_state_session.sh
    - tests/retroarch/frame_index.c
    - README.md
    - include/nesturbator.h
    - .planning/STATE.md
    - .planning/ROADMAP.md

key-decisions:
  - "Skip only the later-target pre-load replay; retain cold rendering for target images at the native anchor because frame pixels are not serialized."
  - "Treat the local replay reduction as bounded evidence, not a demonstrated overall readiness speedup or explanation of the hosted failures."
  - "Run the synthetic timing-report selftest through the existing registered readiness control without changing CTest registration or timeouts."

requirements-completed: [EVID-02]

duration: 67min
completed: 2026-10-15
status: complete
---

# Phase 16 Plan 10: Native readiness timing repair summary

**Measured and removed the overwritten later-prefix replay while retaining required anchor renders, visible failure timings, full controls, and the separate native acceptance gate.**

## Performance

- **Duration:** 67 min
- **Started:** 2026-10-15T17:12:56Z
- **Completed:** 2026-10-15T18:19:27Z
- **Tasks:** 3
- **Files modified or created:** 10, including plan evidence, this summary, and state/roadmap updates

## Accomplishments

- Preserved the two distinct hosted REDs: Intel `readiness-response-stall` at 25,946 ms and ASan `readiness-delayed` at 26,269 ms, both against the unchanged strict `<25,000 ms` full-driver bound. Their platform details and causes remain unknown.
- Measured three independent prefix processes per case. The targeted later-frame verdict replayed 150 frames that were overwritten by loading its already validated native anchor. The final correction skips only that replay; two target-150 renderings and all per-boundary validation and verdicts remain intact.
- Made timing reports visible before the existing duration assertion and wired the synthetic over-bound regression into registered `retroarch.session.readiness`. The registered control then continues through both real prefixes.
- Passed the post-review CI selection 10/10, ASan session readiness and full controls 2/2, and the post-review normal CI workflow 649/649 with all three packages generated.
- Obtained a fresh independent review bound to final source commit `1b1589a49f735195b9dbf89a238dc0518777335c`; it is clean with zero findings and disposition `open: 0`.

## Task Commits

1. **Task 1: Attribute and repair a delayed/stalled full-prefix path** — `0b60c88` (reporter RED), `00f61ce` (failure-visible reporter), `262f580` (prefix replay RED), `c1db852` (bounded replay correction and docs).
2. **Task 2: Protect native failure cases and surrounding controls** — no additional source change; the focused/full CI and ASan controls are recorded in the verification below.
3. **Task 3: Record full CI, package evidence, and independent review** — `1b1589a` (review-driven coverage correction and docs), `2ca8485` (timing evidence and fresh review), `10694f7` (RED evidence addendum).

The seven commits above are measured from the persisted plan ledger at summary write. The final GSD metadata commit is made after the summary self-check.

## Files Created/Modified

- `tests/retroarch/frame_index.c` — retain required anchor-image renders while skipping only later-target replay work overwritten by loading the validated native anchor.
- `tests/retroarch/test_state_session.sh` — report measured fields before the unchanged bound assertion and run its synthetic report regression on the registered readiness path.
- `README.md`, `include/nesturbator.h` — describe the measured cost, its limits, and the registered reporter regression.
- `16-NATIVE-TIMING-REPAIR.md` — native failure identities, local comparison, raw evidence, final checks, clean source review, and Plan 07/08 handoff.
- `16-NATIVE-REVIEW.md`, `16-NATIVE-REVIEW-DISPOSITION.md` — independent final-source review and resolution of its one warning.
- `.planning/STATE.md`, `.planning/ROADMAP.md` — record Plan 10 completion while keeping Plan 07/08, publication, and EVID-04 open.

## TDD Evidence

- **Reporter RED:** The registered target failed on missing synthetic over-bound timing fields. The machine record was accepted as `RED_EVIDENCE_OK`; semantic assessment confirmed that `retroarch.session.readiness` executed the intended report assertion, with no fixture or collection failure. The final readiness run passes the same fixed-data report checks, including the expected nonzero synthetic bound result, before running both real prefixes.
- **Prefix RED:** `retroarch.progression.controls` reached a real `PREFIX_OK`, then failed the new work assertion: 450 pre-load frames across targets 150, 150, and 151 instead of the intended zero for the later target. The classifier returned `RED_EVIDENCE_OK`. An unconditional skip failed the native image-identity control because the framebuffer is not serialized; the corrected test requires 300 anchor-image frames and passes.
- **GREEN:** The corrected progression control passed. The focused readiness regression and both later CI/ASan selections also passed with unchanged timing windows, sends, identities, and reaping checks.

## Verification

- `ctest --preset ci -R '^retroarch.session.readiness$' --no-tests=error --verbose --output-on-failure` — 1/1 passed in 19.14 seconds. The reporter regression ran before the real delayed and stalled prefixes; driver plus cleanup measured 7,457/10,667 ms.
- `ctest --preset ci -R '^retroarch.(session.(readiness|controls)|marker[.]|progression[.])' --no-tests=error --output-on-failure` — 10/10 passed in 245.25 seconds. Raw log SHA-256: `c90ab2254a5f1917dad3f1e877b52e503091c11c37174337f45dcc6564aedf9e`.
- `ctest --preset asan -R '^retroarch.session.(readiness|controls)$' --no-tests=error --verbose --output-on-failure` — 2/2 passed in 304.88 seconds, with no sanitizer diagnostic. The full controls reached `VERDICT_OK`. Raw log SHA-256: `40cf25ba4c377788b85e477ea7fd04d5ceba4ab27219398e50a27bf388be6dd6`.
- `cmake --workflow --preset ci` — configure/build passed, all 649 tests passed in 263.37 seconds, and CPack generated library, libretro, and runner archives. Raw log SHA-256: `a5709b1df1d551defc7884e9f7676e4cd13a650a8c38c21bbfd525dea78da35d`.
- Final package SHA-256 values: library `5c1a1250575ce179f5f60ee651acb8adf851706003573b4589eb920e1f2ea5d0`; libretro `80af4083d7db24ee77c0c7800fc5d1e1b32b5dd508a55fb0ca2e14aebfb87249`; runner `ac7da4bc265dd72bd6b56358381124a81a6cf704f47925e0ffe3b0423f7af613`.
- `git diff --check 4bc40c92e569d8821bc7ab9e47ab5a78b976763c..HEAD` passed. Source review binding was machine-checked against the final commit, `status: clean`, `open: 0`, and all five scoped review paths. The preserved `.planning/config.json` remains byte-identical to its execution baseline.

The complete commands, native identities, raw-output paths, and all comparison caveats are in [16-NATIVE-TIMING-REPAIR.md](16-NATIVE-TIMING-REPAIR.md). Local passes do not establish that the Intel or hosted ASan timing failures are fixed.

## Decisions Made

- Keep the target-150 cold render because the framebuffer is absent from serialized state; skip only the target-151 pre-load replay that the later state load overwrites.
- Keep native subinterval and root-cause details marked unknown; the candidate's single local comparison does not show an overall readiness improvement.
- Preserve Plan 07 as the fresh exact-head native CI and owner-consent gate, and Plan 08 as actual publication-byte verification. EVID-04 and phase verification remain open.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Retained framebuffer rendering at the native anchor.**
- **Found during:** Task 1
- **Issue:** An initial unconditional replay removal made target-150 image validation fail with `IDENTITY`; framebuffer pixels are not restored by the native state load.
- **Fix:** Limit the skip to `target_frame > anchor_frame`; retain both target-150 renders and require exactly 300 pre-load frames across the three-boundary prefix.
- **Files modified:** `tests/retroarch/frame_index.c`, `README.md`, `include/nesturbator.h`
- **Verification:** The progression RED/GREEN, CI/ASan controls, full workflow, and fresh source review passed.
- **Committed in:** `c1db852`

**2. [Rule 2 - Missing critical coverage] Run the reporter regression in registered CI.**
- **Found during:** Task 3 independent review
- **Issue:** The synthetic report selftest was opt-in, so normal readiness CTest did not protect the failure-reporting contract.
- **Fix:** Run the existing selftest when `STATE_SESSION_CONTROL=readiness`, then continue through the same real prefixes; keep CTest registration and deadlines unchanged.
- **Files modified:** `tests/retroarch/test_state_session.sh`, `README.md`, `include/nesturbator.h`
- **Verification:** Focused readiness, combined CI controls, combined ASan controls, normal workflow, and a fresh clean review at the final source commit.
- **Committed in:** `1b1589a`

**Total deviations:** 2 auto-fixed (1 Rule 1, 1 Rule 2). No timing bounds, test registration, production budgets, or delivery gates were widened.

## User Setup Required

None for this local step.

## Next Phase Readiness

Plan 07 may safely resume at Task 1 after this local evidence and review. Reconcile preparation `30a009838b741a429863e7cb5143526fbaad8ed7`, the pushed PR #50 head `3de23d4aed5d9696dc4020db3b8ef5e8205300fb`, local failure handoff `9b37c9a`, and the local Plan 10 commits. Reuse PR #50, capture all final intended commits before pushing, and require fresh native pull-request CI on that exact head; stop if it fails. No merge consent was requested or granted. Plan 08 still owns the normal publication flow and all 18 archive/checksum/attestation checks. EVID-04 and Phase 16 remain incomplete; the historical 21/22 verifier result is unchanged.

---
*Phase: 16-host-evidence-and-roadmap*
*Completed: 2026-10-15*

## Self-Check: PASSED

The summary, timing evidence, and both review artifacts exist; all seven task commits are ancestors of the current branch. The scoped added-line stub scan, deletion check, and `git diff --check` passed. `.planning/config.json` remains byte-identical to its execution baseline.
