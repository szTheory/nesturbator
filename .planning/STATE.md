---
gsd_state_version: "1.0"
milestone: v6
milestone_name: Player-ready NES and reproducible feedback
current_phase: 23
current_phase_name: Reproduce and report a scene
status: Phase 23 paused at regression gate
stopped_at: Phase 23 review refresh committed; configured full-CI regression command interrupted
last_updated: "2026-10-31T13:11:54Z"
last_activity: 2026-10-31
last_activity_desc: Phase 23 review refreshed; full-CI regression command interrupted at the preserved campaign boundary; verifier not run
state_head: c72a47d80ad7a86d871537ac2f05b356f5c1655d
progress:
  total_phases: 2
  completed_phases: 2
  total_plans: 7
  completed_plans: 7
  percent: 100
current_plan: null
---

# Project State

## Project Reference

See [PROJECT.md](PROJECT.md), updated 2026-10-31 after Phase 22 verification refresh, and AGENTS.md.
Core value: Games behave as on the console, identically across platforms, from a small MIT C library.
Current focus: All v6 implementation plans are complete. Phase 22 verification is current and passed. Phase 23 review and disposition are refreshed and committed; its verification remains stale because the regression gate did not complete.

## Current Position

Phase: 23 (Reproduce and report a scene) — PAUSED AT REGRESSION GATE
Plan: No implementation plans remain; all three summaries already exist.
Status: Review refreshed and committed; regression gate interrupted; verifier not run.
Last activity: 2026-10-31 — full configured CI command started and was interrupted at the campaign boundary.

## Next Action

Phase 22's refreshed [verification](phases/22-install-and-protect-progress/22-VERIFICATION.md)
still passes. Phase 23's refreshed [review](phases/23-reproduce-and-report-a-scene/23-REVIEW.md)
and [disposition](phases/23-reproduce-and-report-a-scene/23-REVIEW-DISPOSITION.md)
cover the three shared files and found no new issues. Both artifacts are committed.

The Phase 23 regression gate resolved `workflow.test_command` to
`cmake --workflow --preset ci`. That full workflow started despite the preserved
instruction to exclude the AccuracyCoin campaign. CTest output reached roughly
608/706 tests, and `accuracycoin.campaign` had been scheduled. I interrupted
CTest. It left two child processes that I identified; one had exited before I
sent SIGINT, and the other exited after the signal. A follow-up process check
found none from this run. The interrupted run did not leave a usable final CTest
log, so whether that campaign test completed and its result are unknown. No
Phase 23 verifier ran and its fingerprint remains stale. Do not treat the
interrupted run as a regression pass or a verification.

The next actor is the owner, to decide whether to continue Phase 23 verification
using only the bounded CTest selections already documented in
[Phase 22 refresh evidence](phases/22-install-and-protect-progress/22-REFRESH-EVIDENCE.md),
with `-FA accuracycoin_full_report`, after reviewing the uncertain campaign
activity. If continuing, the next OpenGSD command is:

`$gsd-execute-phase 23`

Before running it, set `workflow.test_command` to the bounded CTest selection for
that run; do not use the configured full CI workflow. Read [continue.md](continue.md)
first. Context may be cleared before the owner resumes. Milestone audit remains
separate and must wait for a passing Phase 23 refresh. No new implementation
phase, audit, closeout, publication, push or PR was started.

The bounded Phase 22 tests previously passed within their recorded limits; those
results are historical and do not make the interrupted regression gate pass.
Earlier Phase 23 document timeouts and wall-time jumps remain unresolved. No
timeout was raised or machine restart performed.

The configured CI cache enables a full AccuracyCoin campaign. Scoped CTests
excluded its required fixture with -FA accuracycoin_full_report. No new campaign,
collector/provenance refresh, live host/device/private-content operation or wider
runtime window is authorized. Phase 22's six descriptor-less prohibitions and
Phase 23's eight judgment-tier limits remain flagged; source review is neither
owner risk acceptance nor proof of unseen external activity. A default verifier
request for six owner signoffs was removed under the explicit project rule that
checks do not wait on people and owner signoff cannot become an acceptance
prerequisite. The final report records this instruction precedence and all limits.

No runtime source, public ABI, save format, ROM pin or hash baseline changed.
Cold replay and native checkpoint routes remain separate; exact-build support
exceeds native loader checks and declared provenance is not authentication.
Native/adapter equality establishes neither host presentation nor hardware
correctness. Commercial reports remain unclassified. PowerShell and other
platforms were not executed locally; no daily progress was touched.

Work remains on gsd/phase-23-reproduce-and-report-a-scene. Owner edits to AGENTS.md
and .planning/config.json remain unstaged and uncommitted. Collector/platform,
host-recovery debt and all historical safety stops are unchanged.

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

**Last session:** 2026-10-31T13:11:54Z
**Stopped at:** Phase 23 regression gate interrupted; shared-file verification remains stale
**Next command:** $gsd-execute-phase 23 (only after choosing the bounded CTest command)
**Resume file:** .planning/continue.md

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
