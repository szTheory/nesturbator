---
gsd_state_version: "1.0"
milestone: v6
status: v6 complete; v0.1.12 published; CI efficiency follow-up identified
stopped_at: Release PR #56 squash-merged; release workflow and publication succeeded
last_updated: "2026-10-31T15:14:38Z"
last_activity: 2026-10-31
last_activity_desc: Required CI passed; Release PR #56 merged; v0.1.12 published with verified assets
state_head: 94703591ca1d5c1d5adcfb6e2fd51987de4a2558
progress:
  total_phases: 2
  completed_phases: 2
  total_plans: 7
  completed_plans: 7
  percent: 100
milestone_name: Player-ready NES and reproducible feedback
current_phase: null
current_phase_name: null
current_plan: null
---

# Project State

## Project Reference

See [PROJECT.md](PROJECT.md), updated for v6 closeout on 2026-10-31, and AGENTS.md.
Core value: Games behave as on the console, identically across platforms, from a small MIT C library.
Current focus: v6 is archived. Phase 22 passed 7/7 and Phase 23 passed 9/9; all ten requirements and three end-to-end flows are satisfied. The `tech_debt` audit and closeout record are in [v6 audit](milestones/v6-MILESTONE-AUDIT.md) and [v6 closeout](milestones/v6-CLOSEOUT.md). Quick task 261031-ecd fixed the release-version fingerprint mismatch and skips redundant optional Nightly lanes on generated release PRs. Release PR #56 passed required CI, squash-merged, and published v0.1.12. A duplicate-CI efficiency follow-up is recorded in the quick-task handoff.

## Current Position

Phase: Milestone v6 archived
Plan: —
Status: Quick task 261031-ecd and the v0.1.12 release are complete
Last activity: 2026-10-31 — exact-head CI passed; release assets and attestations verified and published

## Next Action

V6 is archived in [MILESTONES.md](MILESTONES.md), with the full
[closeout](milestones/v6-CLOSEOUT.md) and [audit](milestones/v6-MILESTONE-AUDIT.md).
Quick task [261031-ecd](quick/261031-ecd-fix-release-only-accuracycoin-fingerprin/261031-ecd-SUMMARY.md)
passes its focused local checks and the authorized remote release gates. Its
clean-base fingerprint for the release fix is
`78822adfa35ac2b0e2f562cfb0067ae08db7e76c3e6e4027fa63d7bcca400de2` (56
inputs); the current phase branch's `0534...` value must not be used for
Release PR #56.

Release PR #56 passed required CI at exact head
`6bd7f267345f3bc153fc08f301a9bac166ed5812` on base
`45a078d2de1f08e39caf1c5942fcdb57f493d8af`, then auto-squash-merged as
`2338608ab184f040620a30d17756f5e06a42d4e3`. The Release workflow's reusable
CI, ordinary main-push CI, and Nightly all succeeded on the merge commit.
Each CI campaign passed all 693 tests. Release CI verified the exact 18
platform archives, generated and verified attestations, rejected a tampered
archive, and published [v0.1.12](https://github.com/szTheory/nesturbator/releases/tag/v0.1.12)
with 18 archives and `SHA256SUMS`.

The main-push CI repeated the Release workflow's full validation on the same
merge commit, including another AccuracyCoin campaign. This is a concrete
efficiency opportunity: conditionally skip ordinary main-push CI for the
Release Please `chore(main): release ...` commit while retaining the Release
workflow's reusable CI, archive artifacts, and publication gate. Do not skip
main-push Nightly coverage without a separate decision. The evidence and
handoff are in [quick task 261031-ecd](quick/261031-ecd-fix-release-only-accuracycoin-fingerprin/261031-ecd-RESUME.md).

Next command: `$gsd-quick --validate Skip the redundant main-push CI run for Release Please merge commits while preserving the reusable release CI and publication gate.`
It plans, implements, and locally validates the CI efficiency follow-up.
Context may be cleared before running it. The previous authorization covered
the release run; get a fresh owner decision before any push starts another full
AccuracyCoin campaign. The agent continues next after that authorization
boundary.

Earlier Phase 23 document-test timeouts and wall-time jumps remain unexplained;
no timeout was raised or machine restart performed. The configured CI cache can
enable the AccuracyCoin campaign, so do not rerun full CI or campaign tests under
this phase. No collector/provenance refresh, live host/device/private-content
operation or wider runtime window is authorized. Phase 22's six descriptor-less
prohibitions and Phase 23's eight judgment-tier limits remain flagged; source
review is neither owner risk acceptance nor proof of unseen external activity.
No owner signoff is an acceptance prerequisite.

No runtime source, public ABI, save format, ROM pin or hash baseline changed.
Cold replay and native checkpoint routes remain separate; exact-build support
exceeds native loader checks and declared provenance is not authentication.
Native/adapter equality establishes neither host presentation nor hardware
correctness. Commercial reports remain unclassified. PowerShell and other
platforms were not executed locally; no daily progress was touched.

The closeout is on gsd/phase-23-reproduce-and-report-a-scene. Local changes to
AGENTS.md, .planning/config.json and .planning/continue.md remain outside the
milestone archive commits. Collector/platform, host-recovery debt and all
historical safety stops are unchanged. No tests, full CI workflow, campaign,
push or release action was run for closeout.

## Preserved Debt and Boundaries

- Phase 23: intermittent local document CTest timeouts and wall-time jumps remain unexplained; preserve current failures and earlier post-review passes. The configured full CI regression command was interrupted after `accuracycoin.campaign` was scheduled; its completion and result are unknown. Do not rerun it without an owner decision. PowerShell/other-platform execution was not performed.

- Recurring collector Git/filesystem-mode enforcement, ignored-fixture/relative-root handling and duplicate validator debt remain open. Independent accepted-run checks do not fix future collections.
- Historical measurement attribution, README revision advisory, Phase 19 state-save cost/citation/early-stage coverage limits and summary/parser metadata warnings remain visible in the audit.
- macOS nofp Apple-toolchain symbol handling remains platform debt. Accepted hosted Linux results remain distinct.
- Real-campaign minimization remains proven only by controls; acknowledged debug sessions and three testing proposals stay preserved.
- WINDOWS entry 18 now agrees across JSON/table/counters, supported by existing fixed source evidence; no previously refused deviation append was invented. The archived deferred-item text records the original refusal.
- Physical zero-margin CPU/PPU precedence is unmeasured; judgment-tier provenance limits remain flagged. No safety stop, budget, threshold or new scope is waived by closeout.
- Historic Phase 16 safety stops and runtime windows remain unchanged in archived records. This milestone boundary grants no new runtime authority.

### Quick Tasks Completed

| # | Description | Date | Commit | Directory |
|---|-------------|------|--------|-----------|
| 261026-i8z | Reduce repeated CI suites, session controls and vector inventory work | 2026-10-26 | dbbf04f | [261026-i8z-reduce-ci-overhead-using-the-recovered-c](./quick/261026-i8z-reduce-ci-overhead-using-the-recovered-c/) |
| 261029-sdj | Reduce inventory parsing and reuse CTest-validated hash artifacts | 2026-10-29 | dc3451b | [261029-sdj-optimize-no-skip-inventory-parsing-and-r](./quick/261029-sdj-optimize-no-skip-inventory-parsing-and-r/) |
| 261031-ecd | Fix release fingerprint and skip redundant Nightly lanes | 2026-10-31 | 9470359 | [261031-ecd-fix-release-only-accuracycoin-fingerprin](./quick/261031-ecd-fix-release-only-accuracycoin-fingerprin/) |

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

## Session

**Last session:** 2026-10-31T15:14:38Z
**Stopped at:** v0.1.12 published; duplicate main-push CI documented as a follow-up
**Next command:** $gsd-quick --validate Skip the redundant main-push CI run for Release Please merge commits while preserving the reusable release CI and publication gate.
**Resume file:** .planning/quick/261031-ecd-fix-release-only-accuracycoin-fingerprin/261031-ecd-RESUME.md

## Performance Metrics

| Plan | Duration | Tasks | Files |
|------|----------|-------|-------|
| Phase 22 P01 | 13 min | 2 tasks | 5 files |
| Phase 22 P02 | 12min | 2 tasks | 5 files |
| Phase 22 P03 | 9 min | 2 tasks | 3 files |
| Phase 22 P04 | about 20 minutes | 3 tasks | 4 files |
| Phase 23 P01 | 12 min | 2 tasks | 5 files |
| Phase 23 P02 | 15 min | 2 tasks | 6 files |
| Phase 23 P03 | 13 min | 2 tasks | 4 files |

## Decisions

- Phase 22: Keep one canonical API/runner/release/testing/progress guide per topic; retain checked support and the marked install command in README.
- Phase 22: Loaded module path/digest and matching provenance establish build identity; unavailable identity remains unknown.
- Phase 22: Exact-build/content snapshot support differs from actual native loader checks; preserve newest battery independently from snapshots and both backup sets.
- Phase 22: Host controls/storage/containers and native payloads retain separate ownership and evidence limits; unknown compatibility is not recovery acceptance.
- Phase 22: Existing measured native CI evidence repaired registration-driven provenance drift; no assertion or safety stop was weakened. Full details remain in phase artifacts and PROJECT.md.
- [Phase 23]: Keep one report route with explicit unknown values and private-artifact limits; leave raw logs and captures local.
- [Phase 23]: Treat repeated frame and audio hashes as repeatability evidence, and keep gameplay sensitivity controls independent.
- [Phase 23]: Qualify only same-invocation native checkpoints with exact runner, ROM and state digests; keep loader checks distinct from exact-build support.
- [Phase 23]: Keep replay examples in fresh scratch roots with explicit frame/time bounds and treat interrupted or incomplete output as failure.
- [Phase 23]: Route each observation to the least costly evidence source that can answer its claim; matching hashes establish repeatability, not independent correctness.
- [Phase 23]: Keep commercial reports unclassified and retain judgment-only safety findings as flagged-unverified.

## Operator Next Steps

- Consider the documented duplicate main-push CI follow-up. Do not push a workflow change or trigger another full AccuracyCoin campaign without a fresh owner decision.
