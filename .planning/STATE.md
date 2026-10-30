---
gsd_state_version: "1.0"
milestone: v5
milestone_name: NMI, VBlank and sprite-0 timing
status: completed
current_phase: null
current_plan: null
stopped_at: Optional queue and mini postmortem recorded; roadmap refreshed; next milestone scope remains unapproved
last_updated: "2026-10-30T22:56:37Z"
last_activity: 2026-10-30
last_activity_desc: Synthesized playtest lessons, diminishing returns, optional remaining games and conditional future milestone sequence
progress:
  total_phases: 3
  completed_phases: 3
  total_plans: 27
  completed_plans: 27
  percent: 100
---

# Project State

## Project Reference

See [PROJECT.md](PROJECT.md), updated 2026-10-30 after v5 closeout, and AGENTS.md.
Core value: Games behave as on the console, identically across platforms, from a small MIT C library.
Current focus: choose the next milestone from the completed gameplay/research checkpoint; further owner play is optional.

## Current Position

v5 completes three phases, 27 plans, 68 tasks and eleven requirements. All three
phase reports were current and passed before archival; the milestone audit has
`tech_debt` and no blocking gaps. Closeout is an explicit acknowledgement
override: six newly acknowledged records and ten prior acknowledgements remain
disclosed. Their artifact verdicts were not promoted to resolved.

The [closeout](milestones/v5-CLOSEOUT.md), [audit](milestones/v5-MILESTONE-AUDIT.md),
[integration report](milestones/v5-INTEGRATION-AUDIT.md) and
[direction snapshot](milestones/v5-PRODUCT-ROADMAP.md) preserve detailed evidence.
Historical state and refresh prose remain in [v5-STATE.md](milestones/v5-STATE.md).
The canonical campaign is 119 PASS / 27 FAIL, 146 terminal scored rows plus five
unscored DRAW entries; all eleven selected targets pass. ABI 1 / state format 3 /
behavior revision 9 remains the current contract.

PR #61 merged. Package release PR #56 (0.1.12) remains open; publication is not
claimed. Closeout commits are local on `gsd/phase-21-v5-closeout`, based on merged
main and the completed audit. No tag or remote mutation is part of closeout.
The pre-existing AGENTS.md and config edits remain unstaged and preserved.

## Next Action

The owner asked for an updated play list, a diminishing-returns assessment and a
mini postmortem with future milestones. The [postmortem](playtesting/POSTMORTEM.md)
and [optional queue](playtesting/DECK-QUEUE.md) now capture those recommendations.
Feedback covers 22 of 26 prepared Deck titles across all six supported mapper
IDs. No further gameplay is required to plan. Optional next games are Castlevania
and Journey to Silius, then Street Gangs/River City Ransom and Mario 1; existing
positive scene checks should not be reassigned as missing.

Prefer **v6 candidate: Player-ready NES and reproducible feedback (RM-014)**:
short truthful player docs, identified install/core, controls/progress guidance,
state-version backup/rollback policy and a bounded reproduction recipe using
existing runner/host facilities. Optional overscan presentation remains a scope
choice. No new general frontend or mandatory commercial sweep is proposed.

New proposed RM-015 is conditional scene-driven accuracy: native/source-based
localization must establish a core fault before a fix/regression milestone is
promoted ahead of DMA. Otherwise the five remaining DMA failures stay the next
measured accuracy candidate, with three former candidates already protected.
Actual Playstead acceptance is separately consumer-owned. PAL remains the
regional alternative; Punch-Out!! now supplies named MMC2 interest without
approving a mapper. All four horizons and audit debt are reconciled in
[PRODUCT-ROADMAP.md](PRODUCT-ROADMAP.md), direction revision 7.

The next actor is the owner selecting milestone scope. The agent can use the
retained scene evidence after that choice; no more play is a prerequisite.
Root causes of the reported seams, slowdown and music interruption remain
unresolved. Regular-VSync-on/adaptive-off and corrected ports were read back on
Shatterhand relaunch without a confirmed visual fix. Four private recipes replay
identically, but repeatability is not correctness and their native checkpoints
are not qualified RetroArch files. Read [automation](playtesting/AUTOMATION.md),
[scene findings](playtesting/SCENE-PLAYLIST.md) and [controls](playtesting/CONTROLS.md)
before another operational test. No native fix, public helper or CI fixture was
implemented here; private content/media/states remain ignored.

`$gsd-new-milestone`

This resumes scope/version/seed selection and approval before creating the next
requirements and execution roadmap. v5 remains completed; no new milestone
version, requirements or phases were approved. Context may be cleared first;
read [continue.md](continue.md). Preserve the owner's unrelated unstaged config
and AGENTS.md edits, existing runtime stops and all closeout evidence limits.

## Preserved Debt and Boundaries

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

**Last session:** 2026-10-30T22:56:37Z
**Stopped at:** Research/playtest checkpoint synthesized and optional queue refreshed; awaiting next milestone scope selection. Next: `$gsd-new-milestone`
**Resume file:** `.planning/continue.md`
