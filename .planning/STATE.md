---
gsd_state_version: "1.0"
milestone: v9
milestone_name: LibFuturo NES gameplay handoff
current_phase: 28
current_phase_name: Qualified NES gameplay package
status: gaps_found
stopped_at: "Phase 28 gap plan 28-04 created and verified; next: $gsd-execute-phase 28 --gaps-only"
last_updated: "2026-11-03T22:13:23.234Z"
last_activity: 2026-11-03
last_activity_desc: "Phase 28 gap plan 28-04 created and independently verified; next: $gsd-execute-phase 28 --gaps-only."
state_head: a7b27d525e2ec6450c75e9b72642ae846223b90e
progress:
  total_phases: 1
  completed_phases: 0
  total_plans: 4
  completed_plans: 3
  percent: 0
---

# Project State

## Project Reference

See [PROJECT.md](PROJECT.md), updated during the v9 opening on 2026-11-03, and AGENTS.md.
**Core value:** Games behave as on the console, identically across platforms, from a small MIT C library.
**Current focus:** Phase 28 — Qualified NES gameplay package

## Current Position

Phase: 28 (Qualified NES gameplay package) — READY TO EXECUTE
Plan: 4 plans; 3 of 4 execution plans complete
Status: Candidate `08a0d1e` passed final local package/gameplay acceptance, the review delta is clean, and handoff validation passed. The phase verifier reports 14/17 truths verified; hosted CI and the published receipt binding PROF-01/PKG-01 remain pending.
Last activity: 2026-11-03 — Phase 28 gap plan 28-04 created and independently verified; next: $gsd-execute-phase 28 --gaps-only.


## Next Action

Plans 28-01 through 28-03 remain complete. The independently reviewed [28-04 gap plan](phases/28-qualified-nes-gameplay-package/28-04-PLAN.md) covers the remaining exact-candidate/resulting-main CI and published-receipt evidence. Its post-plan coverage check passed for all four requirements and decisions D-01 through D-09. Planning did not run builds, tests, hosted checks, or release actions. Preserve the accepted candidate identity, unrelated local edits, and the older interrupted CI/AccuracyCoin campaign stop. Next command: `$gsd-execute-phase 28 --gaps-only`, which executes only Plan 28-04. Context can be cleared before running it.

Next actor: The owner runs `$gsd-execute-phase 28 --gaps-only`. No action is needed before that command; context can be cleared.

## Accumulated Context

### Decisions

- v9 owner-approved direction: publish an exact current package/profile and a reproducible permitted NROM gameplay scenario for LibFuturo to consume. Keep the actual LibFuturo host and play-session outcome consumer-owned; no API change is assumed without a demonstrated gap.
- At each milestone boundary, recheck LibFuturo's current direction and evidence. If no high-value Nesturbator migration gap remains, weave relevant consumer checks into general accuracy, mapper, region, or release work rather than rediscovering the route or creating a standalone milestone.
- v9 approved roadmap: one Phase 28 owns PKG-01, INST-01, PROF-01 and GAME-01 because source/package identity binds the installation, profile and gameplay evidence. Package publication is an explicit final gate, not a separate feature phase or a deferred handoff claim.
- v8 requirement scope and Phases 25–27 are approved; the phases cover all seven requirements once with no added hardware, integration, dependency or campaign scope.
- Keep the 2C07 palette explicitly modeled and the 2A07 DMA rule source-backed; unresolved later no-op-cycle bus behavior and direct hardware proof are not silently converted into acceptance claims. Use owned/permitted fixtures and preserve NTSC outputs.
- v7 covers only the five selected failing NTSC DMA rows at the existing pin and declared RP2A03G target; the three passing DMA rows and every other protected PASS key remain guards.
- One phase owns all four requirements: independently justified corrections, owned wrong-model fixtures, regression preservation and state/documentation consequences belong together. Shared causes require traces.
- Preserve integer-only instance state, clean-room sources and the library/runner/libretro deliverables. Review format/revision impact if in-flight fields change; tests and README/public-header comments accompany each behavior change.
- Existing cold replay and native checkpoint routes remain separate; exact-build support differs from loader checks. Declared provenance is not authentication; repeatability and native/adapter equality do not establish host or hardware correctness.
- [Phase 25]: Explicit NES 2.0 region metadata is authoritative; preferences select only ambiguous and legacy images, and Dendy is refused.
- [Phase 25]: The additive region-aware API preserves the old Auto loader and the existing info struct prefix.
- [Phase 25]: Behavior revision advances to 15 while ABI 1 and native state format 6 remain unchanged; PAL state continuation stays in Phase 26.
- [Phase 25]: PAL row and APU edge expectations remain pinned source-derived models, not direct hardware proof.
- [Phase 25]: Regional bus and frame clocks use accessors derived from the loaded effective region.
- [Phase 25]: PAL samples use exact 640/709379 per-tick accounting with a per-instance remainder; no-content output remains NTSC.
- [Phase 25]: Keep PAL native rows distinct from the 312-line raster and analog picture claims.
- [Phase 25]: Select PAL APU tables/events from the loaded effective region and preserve the tested NTSC path.
- [Phase 25]: Carry PAL sample phase and bounded generated overshoot across nominal public-frame drains.
- [Phase 25]: Advance behavior revision to 16 for changed PAL signal behavior; retain ABI 1 and state format 6.
- [Phase 25]: Keep all region precedence in the shared loader; runner and libretro forward preferences only.
- [Phase 25]: Read the libretro preference on non-NULL content load and keep the active profile fixed until reload.
- [Phase 25]: Report the loaded effective region through existing AV fraction conversion while retaining 256×240 geometry.
- [Phase 25]: Advertise core_options=true only after the real Plan 25-04 module option lifecycle proof.
- [Phase 25]: Bind six-mapper PAL/NTSC acceptance to source arithmetic and retain explicit hardware-evidence limits.
- [Phase 25]: Keep all four descriptor-less fallback rows and both prohibitions flagged until separately resolved; a passing workflow does not convert them into hardware proof or blanket permission.
- [Phase 25, historical and superseded]: Focused fixes did not satisfy the initial failed full workflow and the initial one-run authority had been spent. Later durable standing authority covered the bounded retry, which passed on `fc48e45`.
- [Phase 25]: Advertise core_options only after a real adapter lifecycle proof and enforceable metadata controls.
- [Phase 25]: Use owned source-arithmetic acceptance across all six mapper families and retain NTSC guards.
- [Phase 25]: Label PAL reset release at line 311 as an unverified model because pinned evidence does not establish its duration.
- [Phase 25]: Keep all four descriptor-less fallback rows and both prohibitions flagged after the final local workflow passes.
- [Phase 25]: The bounded local full workflow is covered by standing authority; hosted CI and later phases remain outside scope.
- [Phase 26]: PAL ordinary DMC arbitration uses the documented SYNC interpretation for the bounded 2A07 model; owned tests distinguish it from the any-read control, without claiming hardware proof or later no-op-cycle behavior.
- [Phase 26]: Native states record effective region in existing format-6 header byte 23; region mismatch is rejected before application. Behavior revision is 17; ABI 1 and format 6 remain unchanged.
- [Phase 26]: Same-region PAL state continuation and atomic cross-region refusal are covered by native and libretro tests. All 12 checked revision-16 NTSC states load and advance; this does not establish universal build or historical behavior compatibility.
- [Phase 26]: Generated prompts to acknowledge already locked DMA-01 and STATE-01 limits were superseded by evidence review; UAT is complete while both prohibitions remain unresolved. Reopen only if evidence, claims, required action, or authority changes.
- [Phase 26, historical candidate]: The bounded local workflow on `90d7173` passed 714 tests and packaging; the later current-source workflow on `42b092f` passed 722 tests and packaging and is recorded in the current [verification report](milestones/v8-phases/26-pal-dma-and-regional-state-continuation/26-VERIFICATION.md). Unresolved DMA/STATE fallback rows and both transparency prohibitions remain flagged.
- [Phase 27]: Runner --info reports the loaded region and exact frame/sample fractions; default stdout remains unchanged.
- [Phase 27]: Library region and info queries remain separate; libretro AV and region derive from the same loaded instance.
- [Phase 27]: The intermediate AccuracyCoin capture remains historical to its candidate; final source binding waits for frozen Phase 27 inputs.
- [Phase 28]: After the preferred Nesteroids candidate failed its source-to-ROM identity gate, qualify only the independently rebuilt DABG image with source, linker-map, and instruction evidence.
- [Phase 28]: Keep the gameplay qualification scoped to the upstream installed NTSC package; it is not LibFuturo host, hardware, or publication evidence.
- [Phase 28]: Keep source/toolchain reconstruction one-time and local; recurring install.gameplay uses the permitted manifest ROM and source-backed facts.
- [Phase 28]: The NTSC profile remains candidate-only until the exact published package and scenario pass the live receipt gate.
- [Phase 28]: Receipt CI records bind candidate, main, and release-source evidence to the expected source SHA, workflow, and event.
- [Phase 28]: Candidate 33628daf29f723637e4e3572f659f5d9c5f0ab47 was locally accepted with a 423-input frozen manifest and a passing 486-second workflow; it was superseded by 08a0d1ed2cf738c428e81dead9407b9b695a8608 after the final coverage and documentation correction below.
- [Phase 28]: Candidate 08a0d1ed2cf738c428e81dead9407b9b695a8608 supersedes the prior local candidate after adding the planned public-frame boundary input regression, correcting the guide's measured 238-pixel count and updating the profile digest. Its 401-second bounded workflow passed all 726 CTest tests, installed-runtime profile, three archive checks, and MODE=handoff; a clean review delta covers the three changed files.
- [Phase 28]: Final verification reports 14/17 truths verified. The test and pixel-count gaps are closed; exact-candidate/main hosted CI and the published receipt binding PKG-01/PROF-01 remain open. The verifier routes `gaps_found` to `$gsd-plan-phase 28 --gaps`, while the phase plan anticipated `$gsd-ship 28`; ship preflight refuses until verification passes. Discuss the scope before planning, per project instructions.
- [Phase 28]: The post-review AccuracyCoin fingerprint is 9461d786ad93fd0047fc297c1a501627902304ae187ef1f7c35245586a65e44d for 56 inputs; its focused check passed before the final full workflow. The older interrupted campaign remains stopped and unknown.
- [Phase 28]: Keep v0.1.12 as the observed baseline and let Release Please derive the next version from merged Conventional Commit history.
- [Phase 28]: Preserve unrelated user changes in the phase checkout and use a clean, ownership-resolved release checkout for shipping.

### Pending Todos

The three deferred property/mutation/signal proposals remain outside v8; their records are preserved below. No new todo scope is approved.

### Blockers/Concerns

- **Abort/state policy settled:** the approved countdown remains format6. The follow-through correction is behavior14 with no new serialized field/meaning or ABI change. Apply the approved compatibility policy without a new prompt for each implementation detail.

- **Plan24-02 bus-latch decision resolved:** owner approved cpu_data/external open_bus, format5 and behavior12 before mutation; implemented/verified. The later approved abort change is format6 / behavior13.

- **Plan 24-01 native-state checkpoint resolved:** owner approved the explicit silence byte, continuously clocked bits, format 4 and behavior revision 10. Later compatibility follow-through follows the owner’s standing policy; only a materially new consequence outside it needs another decision.

- **Phase24 authority and evidence:** Canonical report and shared completion gate pass; all requirements/phase complete. The three old human-only labels have explicit superseding dispositions under owner-directed automation. Existing execution/publication boundaries remain unchanged; historical authority is not inferred from CI success.
- Phase 23 document-test timeouts/wall-time jumps, five heuristic edges and Phase 22/23 judgment-tier prohibitions remain unresolved/flagged. No new timeout or restart is justified. Windows/PowerShell, host recovery and cross-build compatibility remain unqualified; commercial reports remain unclassified and daily progress untouched.
- Collector Git/filesystem-mode, ignored-fixture/relative-root and duplicate-validator debt remain open. Historical attribution/revision advisories, Phase 19 cost/citation/early-stage coverage, real-campaign minimization controls and macOS nofp limits remain in the [v5 audit](milestones/v5-MILESTONE-AUDIT.md) and [product ledger](PRODUCT-ROADMAP.md#unresolved-debt-and-delivered-tune-up). New evidence cannot inherit old-run qualification.
- WINDOWS entry 18 was reconciled from fixed existing evidence; the original refused append remains historical. Physical exact-timestamp precedence is unmeasured. Historic Phase 16 safety stops, runtime windows and thresholds remain unchanged; no closeout or approval waives them.
- Plan 25-05's first full workflow failed at CTest (706/712); all six stale test/metadata contracts were diagnosed and fixed. The final bounded local workflow passed 712/712 and packaging on `fc48e45` under standing local-CI authority and existing per-test/fixture limits. The older interrupted run remains unknown and is not relabeled by the final run.
- Phase 26's reported DMA model is based on the cited trace and authors' interpretation, not direct hardware proof. Later held no-op-cycle addresses/register effects and other unsettled PAL abort/reload behavior remain unknown. The final local workflow and state compatibility evidence do not resolve those flags or qualify universal cross-build behavior.
- Phase 27's intermediate Plan 27-01 report remains distinct from final evidence. Plan 27-04 bound the final report to the frozen behavior-17 candidate; the 56-input fingerprint stayed unchanged and the bounded full workflow passed. Preserve the behavior-14 historical receipt and keep the older interrupted campaign stopped/unknown; see [27-ACCEPTANCE.md](milestones/v8-phases/27-pal-host-presentation-and-ntsc-preservation/27-ACCEPTANCE.md).
- Phase 25's two prior PAL state-loader findings are fixed by Phase 27.1's endpoint bound and pre-apply frame-counter validation. The current Phase 27.1 verification covers both closures; Phase 26 did not cover these exact cases. See the [Phase 27.1 verification](milestones/v8-phases/27.1-close-state-01-regional-state-validation-gaps/27.1-VERIFICATION.md) and the refreshed [Phase 25 review disposition](milestones/v8-phases/25-region-selection-and-pal-timing/25-REVIEW-DISPOSITION.md).
- Phase 28 Plan 28-01 was interrupted before its summary, then completed by preserving its four task commits and current work, rerunning both planned checks, and reconciling the measured install.gameplay time to 2.06 seconds. See [28-01-SUMMARY.md](phases/28-qualified-nes-gameplay-package/28-01-SUMMARY.md). Final phase verification exists at [28-VERIFICATION.md](phases/28-qualified-nes-gameplay-package/28-VERIFICATION.md) with status `gaps_found`; the published receipt does not exist, so hosted CI and PROF-01/PKG-01 remain incomplete.

### Roadmap Evolution

- Phase 27.1 inserted after Phase 27: Close STATE-01 regional state validation gaps (URGENT)

## Delivery and Carried Evidence

v7 is archived with its roadmap, requirements, audit, product-roadmap snapshot and Phase 24 artifacts under `milestones/`. The four quick-task folders remain in `.planning/quick/` because they have no milestone ownership.
v6 is archived: both phases, all ten requirements and three flows passed; its [audit](milestones/v6-MILESTONE-AUDIT.md) and [closeout](milestones/v6-CLOSEOUT.md) retain non-blocking debt. Release PR #56 later published v0.1.12 with 18 archives and SHA256SUMS; milestone completion and package publication remain separate.
Quick tasks 261026-i8z, 261029-sdj and 261031-ecd remain delivered. [261031-fq6](quick/261031-fq6-skip-the-redundant-main-push-ci-run-for-/261031-fq6-SUMMARY.md) merged; PR/main-push CI passed, while its exact generated Release Please main-push skip event remains unobserved. No user action is needed for that completed task.
Pre-existing local edits to AGENTS.md, config, continue and quick-task records remain outside this roadmap's scope; they were not reverted.

## Deferred Items

Items acknowledged and deferred at milestone close, most recent first:

| Category | Item | Status | Deferred At | Milestone |
|----------|------|--------|-------------|-----------|
| debug_sessions | entry-review-inhibition | verifying | 2026-10-30 | v5 |
| todos | 2026-10-26-add-reproducible-owned-property-tests.md | presence-only | 2026-10-30 | v5 |
| todos | 2026-10-26-evaluate-scoped-nightly-mutation-testing.md | presence-only | 2026-10-30 | v5 |
| todos | 2026-10-26-make-runner-signal-checks-deterministic.md | presence-only | 2026-10-30 | v5 |
| deferred_items | 21/deferred-items.md: `.planning/WINDOWS.md` could not accept the prior plan's deviation entry: its frontmatter counts 18 entries, while the authoritative fenced JSON and table contain 17 records (table row 18 has no JSON entry). `gsd_run windows append` refused the update. The pre-existing ledger consistency issue remains deferred to its owning planning-health work. | acknowledged | 2026-10-30 | v5 |
| deferred_items | 21/deferred-items.md: macOS arm64 `nofp` undefined-symbol allowlist  - **Reproduced at:** final candidate `1d27a0f12542a235d8d79b9c97591168fbd77d8e`. - **Command:** `cmake --workflow --preset nofp` on macOS arm64. - **Result:** `abi.float_scan`, `abi.float_scan.selftest`, `abi.global_symbols`, and `abi.float_fixture` passed. `abi.undefined_symbols` rejected the Apple stack-protector references `stack_chk_fail` and `stack_chk_guard`. - **Scope:** the configured hosted `nofp` matrix uses Linux x64 and Linux arm64. This local macOS failure does not establish either hosted result. Keep the allowlist gate intact and require the actual Linux CI results before treating the `nofp` requirement as verified.  The initial local `nofp` run also reported the decimal `2.3` in the state validation comment. That source citation was changed to integer-only section wording in `1d27a0f`; the final source scan passes. The scanner and its hard gate were not weakened. | acknowledged | 2026-10-30 | v5 |
| debug_sessions | phase16-cleanup-fixture | verifying | 2026-10-24 | v4 |
| debug_sessions | phase16-interruption | investigating | 2026-10-24 | v4 |
| deferred_items | 16-host-evidence-and-roadmap/deferred-items.md: local macOS nofp reports Apple-toolchain stack_chk_fail/stack_chk_guard symbols | acknowledged | 2026-10-24 | v4 |

Seven seed acknowledgements from earlier closes remain suppressed by their own `audit_acknowledged` markers.

## Performance Metrics

Phase 27.1 closes the two STATE-01 validation cases later identified by the v8 audit and passes 7/7 verification truths; its candidate-bound full workflow passed all 722 tests and created all three packages. See its acceptance, verification, and summary reports. Phase 26 verification was refreshed on 2026-11-03 and passes 11/11 truths; the current-head regression gate passed 722/722 tests and packaged all three deliverables. CTest took 434.77 seconds, including the 254.66-second `retroarch.session.controls` check. The implementation, build, and test inputs remain identical to the accepted source candidate. The DMA and STATE fallback rows and transparency prohibitions remain unresolved; direct hardware proof and later no-op-cycle behavior are not claimed. Phase 25 and historical v7 receipts remain source-bound as recorded below and in their artifacts.
**Per-Plan Metrics:**

| Plan | Duration | Tasks | Files |
|------|----------|-------|-------|
| Phase 25 P01 | 12min | 3 tasks | 9 files |
| Phase 25 P02 | 12min | 2 tasks | 8 files |
| Phase 25 P03 | 16min | 3 tasks | 11 files |
| Phase 25 P04 | 9min | 2 tasks | 7 files |
| Phase 25 P05 earlier partial run | 51min | 2 tasks; gate attempted | 12 files |
| Phase 25 P05 | 115min | 3 tasks | 17 files |
| Phase 27 P01 | 5 min (task-commit span) | 2 tasks | 7 files |
| Phase 27 P02 | 11min | 2 tasks | 7 files |
| Phase 27 P03 | 5min | 2 tasks | 7 files |
| Phase 27 P04 | 42min | 3 tasks | 11 files |
| Phase 28 P01 | 53min | 2 tasks | 7 files |
| Phase 28 P02 | 49min | 2 tasks | 9 files |
| Phase 28 P03 | 70min | 3 tasks | 12 files |

## Session Continuity

**Last session:** 2026-11-03T22:13:23.217Z
**Stopped at:** Phase 28 gap plan 28-04 created and verified; next: $gsd-execute-phase 28 --gaps-only
**Next actor:** The owner runs `$gsd-execute-phase 28 --gaps-only` to execute the Phase 28 gap-closure plan.
**Resume file:** .planning/phases/28-qualified-nes-gameplay-package/28-04-PLAN.md
**Context:** Plans 28-01 through 28-03 are complete and summarized. Candidate `08a0d1e` passed 726-test local acceptance and a clean review delta. The verifier closes the frame-boundary and pixel discrepancy findings but leaves hosted CI and the published PROF-01/PKG-01 binding pending. `$gsd-ship 28` is blocked until verification passes. Preserve unrelated user changes and the stopped campaign.

## Operator Next Steps

- Run `$gsd-discuss-phase 28` to resolve how the exact-head/main CI and published-receipt gaps fit the phase plan before planning closure. This follows the GSD verifier route; the earlier `$gsd-ship 28` handoff is blocked because ship requires a `passed` verification status. Context can be cleared before running it. Preserve unrelated user work and do not rerun the completed execute step.
