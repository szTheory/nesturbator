---
gsd_state_version: "1.0"
milestone: v5
milestone_name: NMI, VBlank and sprite-0 timing
status: completed
current_phase: null
current_plan: null
stopped_at: Positive Mario 3, Contra and Ninja Gaiden Deck gameplay reports recorded
last_updated: "2026-10-30T19:45:32Z"
last_activity: 2026-10-30
last_activity_desc: Owner reports Ninja Gaiden plays great; positive MMC1, MMC3 and UxROM smoke signal
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
Current focus: collect ordinary-play feedback through the working Steam/ES-DE collection route; next milestone remains proposed.

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

The owner relaunched Mario 3 from the Nesturbator collection and confirmed the
controller works. A read-only live check found the intended core and corrected
player-one configuration. The game was left running. No immediate user action
is required. The owner subsequently reported good Mario 3 input timing, visuals
and audio, low perceived latency and no observed issues. The owner then reported
Contra (USA) plays well with similar feedback, then reported Ninja Gaiden
(displayed as **Shadow Warriors**) plays great. These are positive MMC3, UxROM
and MMC1 gameplay smoke reports. If the owner continues, The Legend of Zelda
(USA, Rev A) is the next suggested voluntary title, with battery-backed progress
to assess later. No Zelda gameplay or persistence result is claimed. The launcher
includes handheld fallback, but unplug/reconnect and fallback remain untested.

The Deck is in Gaming Mode and ES-DE is open through its repaired existing Steam
shortcut. Open Collections → Nesturbator to use seven existing supported NTSC
entries, each with Nesturbator selected and other cores still switchable. No main
ROM files were changed or added. Normal battery storage is reused; snapshots
are core-specific within normal state storage. The owner saw commercial Mario 3
on the TV and confirmed physical Start after the corrected SDL2 PowerA profile.
The subsequent collection launch used the correct core but Start did not work. Steam virtual-device presentation differs from the direct desktop run; a context-specific controller resolver was applied, and the owner confirmed working input after relaunching from the collection. Handheld fallback is configured but untested. Mario 3, Contra and Ninja Gaiden have positive user-reported gameplay smoke evidence; instrumented latency, sustained/full-game coverage, clean frontend return and progress roundtrip remain open.

Two Mac Mario reports were useful smoke signal; mild sluggishness/audio tearing
remain unattributed. The owner explicitly deferred a full game sweep and higher
confidence performance testing. TV visibility and ES-DE startup were repaired
from observed host evidence, with local rollback backups. No core code, ABI,
state format or emulation behavior changed. Details are in the
[baseline](playtesting/2026-10-30-nes-baseline.md).

Read [PII-free lessons](playtesting/LESSONS.md) before milestone scoping or another
emulator's host setup. Owner direction now prefers this real-library daily-play
loop as a fast source of feedback before deeper Playstead integration. Device
addresses, account identifiers, personal paths, raw media/logs/captures and save
bytes remain outside Git in private operational records.

Separate planning continuation:

`$gsd-new-milestone`

Resume the pending player-ready scope decision using the
[research synthesis](research/milestone-v6-discovery/SYNTHESIS.md) and refreshed
[product direction](PRODUCT-ROADMAP.md). Immediate remains uncommitted; no next
milestone requirements, phases or version were approved. Keep existing
state/host evidence credited, DMA as the following accuracy candidate, PAL as the
regional alternative, and actual Playstead acceptance separate from a handoff.
Context may be cleared; read [continue.md](continue.md) to resume.

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

**Last session:** 2026-10-30T19:45:32Z
**Stopped at:** Mario 3, Contra and Ninja Gaiden Deck smoke reports positive; optional Zelda next, no setup work pending. Planning continuation: `$gsd-new-milestone`
**Resume file:** `.planning/continue.md`
