---
phase: 28-qualified-nes-gameplay-package
plan: "03"
subsystem: release
tags: [cmake, ctest, package, provenance, ntsc]
requires:
  - phase: 28-qualified-nes-gameplay-package/28-01
    provides: relocated installed consumer and reproducible permitted-gameplay evidence
  - phase: 28-qualified-nes-gameplay-package/28-02
    provides: source-checked NTSC provider profile and fail-closed published receipt gate
provides:
  - Candidate-bound local workflow and archive acceptance for the exact phase branch source
  - Concrete release handoff with exact hosted and published receipt gates
  - Version metadata reconciled to published v0.1.12 without guessing the next version
affects: [28 shipping, package publication, provider integration, milestone closeout]
actuals:
  tokens: 37129
  tasks: 3
  commits: 8
  plan_head_before: 48631ddd75f628b8475f519032facfef18946c8c
  plan_head_after: 4e4cc2f7f99efc6b92c39af78a84ca1b1d052299
  final_candidate_source: 08a0d1ed2cf738c428e81dead9407b9b695a8608
  final_candidate_tree: 51e8787f052cb5658dacff3ab49445b9f0430b33
  final_candidate_inputs: 423
  final_workflow_seconds: 401
tech-stack:
  added: []
  patterns:
    - Frozen input manifests bind local archives and runtime evidence to one source identity
    - Local readiness, hosted CI and published-package acceptance remain separate gates
key-files:
  created:
    - .planning/phases/28-qualified-nes-gameplay-package/28-IDENTITY.json
    - .planning/phases/28-qualified-nes-gameplay-package/28-ACCEPTANCE.md
  modified:
    - .release-please-manifest.json
    - version.txt
    - include/nesturbator.h
    - README.md
    - libretro/nesturbator_libretro.info
    - tests/accuracy/baseline-metadata.txt
    - tests/cmake/provider_receipt.cmake
    - .planning/STATE.md
    - .planning/ROADMAP.md
key-decisions:
  - "Keep v0.1.12 as the observed published baseline and let the configured Release Please flow select the next version."
  - "Treat the final successful local workflow as candidate readiness only; hosted CI, publication and PKG-01 remain pending."
  - "Preserve unrelated user changes and require a clean, ownership-resolved release checkout before publication."
patterns-established:
  - "Freeze source, profile, scenario and build inputs before local package qualification."
  - "Run handoff validation against the frozen manifest and prove absent publication fails for its named reason."
requirements-completed: [INST-01, GAME-01]
coverage:
  - id: D1
    description: "The relocated installed consumer and permitted boot-to-gameplay scenario pass against the final local package."
    requirement: GAME-01
    verification:
      - kind: integration
        ref: "cmake --workflow --preset ci; build/ci/phase28-local-frame-sampling/LastTest.log"
        status: pass
    human_judgment: false
  - id: D2
    description: "Candidate source, package archives, runtime profile and build identity are frozen and locally accepted."
    verification:
      - kind: integration
        ref: ".planning/phases/28-qualified-nes-gameplay-package/28-IDENTITY.json"
        status: pass
    human_judgment: false
  - id: D3
    description: "The handoff keeps exact-candidate CI, resulting-main CI and published receipt gates pending until their exact evidence exists."
    requirement: PKG-01
    verification:
      - kind: other
        ref: "cmake -DSOURCE_DIR=. -DMODE=handoff -DACCEPTANCE=.planning/phases/28-qualified-nes-gameplay-package/28-IDENTITY.json -P tests/cmake/provider_receipt.cmake"
        status: pass
    human_judgment: true
    rationale: "Hosted CI and immutable release archives have not yet been verified; the separate shipping step must complete those gates."
duration: 70min
completed: 2026-11-03
status: complete
---

# Phase 28 Plan 03: Qualified NES gameplay package Summary

**The v0.1.12 baseline is reconciled, final candidate `08a0d1e` passes the bounded local package/gameplay workflow, and the phase verifier preserves hosted and publication gaps.**

## Performance

- **Duration:** 70 min
- **Started:** 2026-11-03T16:49:56Z
- **Completed:** 2026-11-03T18:00:16Z
- **Tasks:** 3
- **Files modified:** 12 plan-owned files, including the summary and state/roadmap records

## Accomplishments

- Aligned the manifest and four package-facing version markers to the observed published v0.1.12 baseline while preserving ABI 1, behavior revision 17 and state format 6.
- After 19 committed review fixes, froze candidate `33628daf29f723637e4e3572f659f5d9c5f0ab47` (tree `75d7901e0499f36fea1a7875ba0d0a8662897b21`) with 423 hashed inputs. The follow-up controller-boundary regression and corrected 238-pixel guide measurement produced candidate `08a0d1ed2cf738c428e81dead9407b9b695a8608` (tree `51e8787f052cb5658dacff3ab49445b9f0430b33`), still with 423 inputs. Its 401-second workflow passed configure, build, all 726 CTest tests and package creation; installed-runtime profile and all three archive inventory checks passed.
- The standard-depth review of the prior candidate was clean across 19 files. A follow-up review of the three changed files on `08a0d1e` was also clean; the review disposition records zero open findings, `28-REVIEW-FIX.md` records earlier fixes, and `28-REVIEW-DELTA.md` records the candidate delta.
- The first post-review workflow exposed two stale AccuracyCoin fingerprint checks. The 56-input digest was recomputed through the owned helper, the baseline metadata was updated in commit `33628da`, the focused fingerprint test passed, and the bounded full workflow then passed all 726 tests. No standalone campaign was run.
- Updated the release handoff with current PR heads, observed `main`, worktree ownership, required exact-head CI and Release Please gates, and the executable final receipt command. MODE=handoff passed and confirmed an absent publication fails at the intended named gate.
- Kept the older interrupted AccuracyCoin run stopped and unknown. The registered current `accuracycoin.campaign` fixture passed in the final workflow; no legacy collector or campaign entrypoint was invoked.

## Task Commits

Each task was committed atomically; Task 2 required focused follow-up commits as its post-run collector findings were corrected.

1. **Task 1: Reconcile observed release lineage before freezing package metadata** — `78800c1` (fix)
2. **Task 2: Freeze exact candidate inputs and run the bounded current-source acceptance once** — `a607ff0`, `11afb61`, `cc6c368`, `dc27ab7`, `32e7ca3` (feat/fix)
3. **Task 3: Preserve the shipping boundary and make the published receipt the final acceptance gate** — `b0ee076` (fix), `4e4cc2f` (docs)

Post-plan review and candidate-qualification fixes were committed through `7682882`; the final AccuracyCoin input-fingerprint update is `33628da`. See [28-REVIEW-FIX.md](28-REVIEW-FIX.md) for each reviewed finding and its exact source commit.

**Plan metadata:** recorded in the separate final docs commit after this summary and state update.

## Files Created/Modified

- `.release-please-manifest.json`, `version.txt`, `include/nesturbator.h`, `README.md`, `libretro/nesturbator_libretro.info` — restore the observed published baseline markers.
- `tests/accuracy/baseline-metadata.txt` — refresh the 56-input AccuracyCoin source fingerprint after Plan 28-02 changed the test registration.
- `tests/cmake/provider_receipt.cmake` — implement local/handoff identity validation, record build/runtime identity, and robustly match CMake's line-wrapped missing-publication diagnostic.
- `tests/cmake/provider_profile.cmake`, `tests/cmake/provider_receipt.cmake`, `tests/cmake/qualified_consumer.cmake`, `tests/consumer/main.c` — close code-review findings with owned mutation controls and bounded consumer checks; exact changes are recorded in `28-REVIEW-FIX.md`.
- `.planning/phases/28-qualified-nes-gameplay-package/28-IDENTITY.json` — bind the accepted source, build, profile, scenario, local report and archive inventory.
- `.planning/phases/28-qualified-nes-gameplay-package/28-ACCEPTANCE.md` — separate local proof from hosted/publication gates and document the release handoff.
- `.planning/STATE.md`, `.planning/ROADMAP.md` — record the completed execution plan and the pending publication boundary.

## Decisions Made

- Keep v0.1.12 as the latest observed release baseline; Release Please determines the next version after merge.
- Do not promote local archive evidence to candidate CI, main CI or publication acceptance.
- Preserve dirty user-owned planning and coordination material. The current phase checkout is not suitable for publishing; the unrelated linked quick-task worktree remains untouched.
- Keep PKG-01 and the version-bound published receipt pending until the normal shipping path verifies the actual release artifacts.
- Keep the historic interrupted campaign stopped and unknown. The final workflow's registered `accuracycoin.campaign` fixture is current-run evidence only.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Corrected CTest result parsing for current log formatting**
- **Found during:** Task 2
- **Issue:** The post-run collector did not reliably parse the current CTest log line formatting after the workflow itself passed.
- **Fix:** Parse normalized CTest result strings and use the actual current status lines.
- **Files modified:** `tests/cmake/provider_receipt.cmake`
- **Verification:** The final 726-test workflow passed and its required tests were present in the frozen evidence.
- **Committed in:** `cc6c368`

**2. [Rule 1 - Bug] Initialize parent objects in the candidate identity**
- **Found during:** Task 2
- **Issue:** The first accepted workflow result could not be serialized into the intended nested JSON identity sections.
- **Fix:** Initialize the identity's qualification, source, inputs, scenario, contract, compatibility, build, acceptance and gate objects before setting fields.
- **Files modified:** `tests/cmake/provider_receipt.cmake`
- **Verification:** Candidate construction and the final local acceptance completed with all required identity fields.
- **Committed in:** `dc27ab7`

**3. [Rule 1 - Missing identity] Capture actual compiler and platform facts**
- **Found during:** Task 2
- **Issue:** The first populated identity omitted compiler and host/target facts needed to reproduce the accepted build.
- **Fix:** Read the configured compiler and system identity from CMake-generated files and capture the CTest preset controls.
- **Files modified:** `tests/cmake/provider_receipt.cmake`
- **Verification:** Final identity records AppleClang 21.0.0.21000101, CMake/CTest 4.4.3, Darwin/arm64, Release/Ninja and four CTest jobs.
- **Committed in:** `32e7ca3`

**4. [Rule 1 - Bug] Match the wrapped missing-publication diagnostic**
- **Found during:** Task 3
- **Issue:** The planned handoff control failed because CMake wrapped its expected missing-receipt message across lines, while the checker matched one contiguous string.
- **Fix:** Normalize whitespace before checking the named diagnostic, then refreeze and rerun local acceptance on the corrected checker source.
- **Files modified:** `tests/cmake/provider_receipt.cmake`
- **Verification:** At that execution stage, the `b0ee076` source's 450-second workflow passed; MODE=handoff then passed and confirmed the absent-receipt gate reason. Later code-review fixes and the post-review metadata refresh superseded that candidate; `33628da` then passed its own 486-second workflow, and the final coverage/documentation follow-up candidate `08a0d1e` passed its 401-second workflow.
- **Committed in:** `b0ee076`

**5. [Rule 3 - Blocking issue] Refresh the AccuracyCoin input fingerprint**
- **Found during:** Task 2
- **Issue:** Plan 28-02 changed `tests/CMakeLists.txt`, making the earlier 55-input fingerprint stale.
- **Fix:** Recompute the manifest for 56 inputs and update the stored build-input SHA-256.
- **Files modified:** `tests/accuracy/baseline-metadata.txt`
- **Verification:** `accuracycoin.input_fingerprint` passed; the current campaign fixture also passed in the final workflow.
- **Committed in:** `11afb61`

**6. [Rule 3 - Blocking issue] Refresh the AccuracyCoin input fingerprint after review fixes**
- **Found during:** final candidate acceptance
- **Issue:** The review fixes changed two tracked AccuracyCoin build inputs, so the 56-input baseline digest no longer matched the current source. The full workflow failed only `accuracycoin.input_fingerprint` and `accuracycoin.activation`; 724 other tests passed.
- **Fix:** Recompute the digest through `accuracycoin_compute_input_fingerprint`, update only `build_inputs_sha256`, and preserve the historical campaign report and its source identity.
- **Files modified:** `tests/accuracy/baseline-metadata.txt`
- **Verification:** Focused `accuracycoin.input_fingerprint` passed; the subsequent bounded workflow passed all 726 tests and packaging, including the registered current campaign fixture. No separate collector/campaign entrypoint was run.
- **Committed in:** `33628da`

Earlier full workflows reached successful build/test/package stages but exposed post-run receipt collection defects; those results were not reused as final qualification. Each defect was diagnosed and fixed before the next required refreeze. The final recorded local run is bound to `08a0d1e`. Phase verification returned `gaps_found` (14/17): the frame-boundary test and pixel-count discrepancy are closed, while exact-candidate/main hosted CI and the published receipt binding PKG-01/PROF-01 remain open. GSD routes `gaps_found` to a closure plan, and `gsd-ship` preflight requires verification status `passed`; the plan's earlier direct-to-ship handoff therefore needs reconciliation before shipping.

**Total deviations:** 5 auto-fixed (4 Rule 1, 1 Rule 3)
**Impact on plan:** Collector fixes were necessary to make final identity and handoff evidence accurate. No emulation or public API scope was added.

## Issues Encountered

- The candidate has no hosted PR or run. At handoff `origin/main` was `b25146c6efaac41c58f60555f59f0fe79a90ca68`; PR #63 was open at head `d54e21898fbf7bb3984fd2d33aa563932b464581`. PR #51 was open at `814825fbb81086de17f0fb6f4df0f556f5619ebc` and its reported base SHA differed from the observed main head. Both require triage and reconciliation during shipping.
- The current phase checkout contains unrelated user edits. They remain preserved and excluded from the accepted implementation manifest; publication must use a clean, ownership-resolved release checkout.
- The hosted candidate, resulting-main and published-package gates remain pending by design.

## User Setup Required

None - no external service configuration is required for local qualification. The owner proceeds through the existing shipping command.

## Next Phase Readiness

Plan 28-03 execution is complete and locally ready at candidate `08a0d1e`. Phase verification is `gaps_found` (14/17), so Phase 28 is not complete and `$gsd-ship 28` is blocked by its verification preflight. The phase plan's earlier next-step instruction points to shipping, while the GSD verifier routes the remaining exact-CI and published-receipt truths through gap planning; project instructions also require discussion before planning. Next actor is the owner, running `$gsd-discuss-phase 28` to settle that scope before any gap plan. Context can be cleared; STATE.md, this summary, the verification report and acceptance handoff retain the evidence and route.

---
*Phase: 28-qualified-nes-gameplay-package*
*Completed: 2026-11-03*

## Self-Check: PASSED

- Summary file exists.
- All eight plan task commits are ancestors of the phase branch head.
- `git diff --check` passed.
