---
gsd_state_version: "1.0"
milestone: v5
milestone_name: NMI, VBlank and sprite-0 timing
current_phase: 21
current_phase_name: Interrupt latency and hijack
status: planning
stopped_at: Phase 20 complete, ready to plan Phase 21
last_updated: "2026-10-28T19:38:22.603Z"
last_activity: 2026-10-28
last_activity_desc: "Phase 20 refreshed evidence shipped as PR #59"
state_head: ad98012802bea9b158ef3c077421eaeb9b34491a
progress:
  total_phases: 3
  completed_phases: 2
  total_plans: 18
  completed_plans: 18
  percent: 67
---

# Project State

## Project Reference

See [PROJECT.md](PROJECT.md), updated 2026-10-28 after Phase 20, and AGENTS.md.
Core value: Games behave as on the console, identically across platforms, from a small MIT C library.
Current focus: v5 NMI, VBlank and sprite-0 timing (RM-004 in [PRODUCT-ROADMAP.md](PRODUCT-ROADMAP.md)); Phase 21 interrupt latency and hijack. Phase 20's repaired source is independently verified complete. The refreshed verification/evidence and shipping note are published through open, ready-for-review PR #59; merge remains conditional on the final head's required CI.

Nightly follow-up and re-verification: the native clock/hash helpers and Windows
binary-fixture repair have fresh source-bound acceptance. CI `40748417351/1`
passed 18/18 jobs and Nightly `40740261760/1` passed all seven work jobs, including
Intel session controls. Current effective source is
`87ee34cf7b5ba45d87e748ab69b69f929a7fe7be96c02ed044bf9b1394f93202`
over 418 files. Production collection and current-checkout validation passed;
all 418 mode/blob entries match the candidate and both hosted trees. Fresh local
CI passed 688/688 in 434.48 seconds and packaged all three deliverables.
Independent verification passed 32/32 with a fresh 95-file fingerprint.
The prior 417-input acceptance remains historical. See
20-REVERIFICATION-EVIDENCE.md in the Phase 20 directory. Phase 21 has not started.

## Current Position

Phase: 21 — Interrupt latency and hijack
Plan: Not started
Status: Ready to plan
Last activity: 2026-10-28 -- Phase 20 refreshed evidence shipped as PR #59

Progress: [███████░░░] 67%

## Accumulated Context

### Decisions

- Phase 20 completes with 32/32 verified truths and all four assigned requirements satisfied. All 12 plan summaries, original gap closures and the source-bound accepted hosted bundle are preserved in the phase directory.
- Captured NMI history and polled NMI work remain separate serialized latches. Exact race states use complete production instructions and label constructed origins separately from executed endpoints.
- The finite-margin circuit evidence constrains one global integer-timestamp policy; no physical paired-chip exact-tie winner or per-table alignment shift is claimed. Full decision history remains in 20-CONTEXT.md, summaries and measurements.
- Current candidate `7848cc8`, tested merge `716e8d4`, protected base `ab9c2e6` and evidence-only commits remain separate. All 418 effective inputs and Git modes/blob IDs match. Earlier candidate `fe70d61` / tested merge `ef6d9e5` retain their historical 417-input evidence; source-changing repairs require fresh hosted evidence.
- Phase 21 retains its planned IRQ/poll/shared-entry/hijack scope. Phase 20 collector hardening is explicit debt, not silently added scope or an acceptance waiver.

### Blockers/Concerns

- [Phase 20 delivery] PR #59 is the merge vehicle on `gsd/phase-20-nmi-line-and-vblank-flag`, ready for review with refreshed evidence published by `$gsd-ship 20`. Merge only after the final published head's required CI passes. Begin Phase 21 source changes after delivery.
- [Phase 20 review debt] CR-01: recurring collector enforcement omits Git file modes. The current accepted run has an independent 418/418 mode/blob match against all three hosted trees; future enforcement repair is deferred.
- [Phase 20 review debt] WR-03: source scoreboard lookup recursively scans ignored build fixtures and relative SOURCE_REF fails. Collection used an absolute clean-equivalent checkout and current-checkout validation; direct tracked-path handling remains deferred.
- [Phase 20 review debt] WR-04: duplicate identical Nightly validator definition remains deferred cleanup. See 20-REVIEW-DISPOSITION.md for all dispositions.
- [Evidence limit] Exact physical paired-chip tie precedence remains unverified under amended D-10. The judgment-tier clean-room prohibition remains explicitly flagged; hygiene passes and no contamination was found. No manual sign-off gate is introduced.

### Pending Todos

- [2026-10-26] [testing] Add reproducible owned property tests — [todo file](.planning/todos/pending/2026-10-26-add-reproducible-owned-property-tests.md)
- [2026-10-26] [testing] Evaluate scoped nightly mutation testing — [todo file](.planning/todos/pending/2026-10-26-evaluate-scoped-nightly-mutation-testing.md)
- [2026-10-26] [testing] Make runner signal checks deterministic — [todo file](.planning/todos/pending/2026-10-26-make-runner-signal-checks-deterministic.md)

### Quick Tasks Completed

| # | Description | Date | Commit | Directory |
|---|-------------|------|--------|-----------|
| 261026-i8z | Reduce repeated CI suites, session controls and vector inventory work | 2026-10-26 | dbbf04f | [261026-i8z-reduce-ci-overhead-using-the-recovered-c](./quick/261026-i8z-reduce-ci-overhead-using-the-recovered-c/) |

## Delivery

Current acceptance refresh: candidate `7848cc8980e8e96a1fc6f5ec223a9783fb7ef19d`,
tested merge `716e8d49c80602e3524b9348881ddf171fe33a2f`, protected base `ab9c2e6`.
CI `40748417351/1` and Nightly `40740261760/1` passed production content
collection, including the repaired host-session helpers. The new immutable bundle
is `build/phase20-refresh-hosted-evidence/`; the previous bundle is preserved.
CI `40740261875/2` was green but its mixed-attempt artifacts were correctly
rejected by the collector. A full rerun was requested then cancelled because it
also retains old artifacts. Updating the PR's factual verification note started
the accepted fresh CI run on the unchanged source. No source, deadline or
acceptance rule changed. The refreshed review, verification and evidence are
published through the existing PR #59. Fresh ship-time local CI passed
688/688 in 438.85 seconds and produced all three packages; production bundle
revalidation passed and all 418 mode/blob entries match the accepted candidate
and publication head. The shipping note uses no CI-skip token, so required CI
runs on the final published head. The PR description now records current
418-input acceptance instead of the superseded pending refresh. Existing review
and debt dispositions are retained; no external review command is configured.
No merge occurred and no next lifecycle command ran.

Historical initial delivery:

Phase 19 squash-merged as PR #57 (`ab9c2e6`). Phase 20 uses existing PR
#59 on `gsd/phase-20-nmi-line-and-vblank-flag`. Candidate
`fe70d619d6a3cf7b80ed1867ff9faf4c7c9053b1`, actual tested merge
`ef6d9e5ec2c94ddfd431c1d7162f620722a145df`, and protected base
`ab9c2e6eb822c06f0dfb0f08a0ca42ec541358be` have accepted CI
`40700458574/1` and Nightly `40700458462/1` content. Effective digest
`a08da046cedeecbdd28227cc53d54e3d3e7dff2a93ee88aab9ead67da47bf61c`
(417 files) is unchanged by evidence closeout. Independent re-verification
passed 32/32 and fresh local CI passed 687/687 with all three packages.
The earlier execution performed no remote mutation. The 2026-10-28 ship step
publishes the evidence closeout and shipping note to PR #59 and marks it ready
for review. Fresh local CI passed 687/687, all three packages were produced,
the immutable hosted bundle revalidated, and 417/417 effective mode/blob
entries match the accepted candidate. Required CI runs on the final shipping
head without a skip token. Existing phase review and recorded dispositions
are retained; no external review command is configured. No merge or next
lifecycle command is part of this ship step.
See 20-DELIVERY.md for earlier publication and failed attempts; 20-EVIDENCE.md
and 20-10-SUMMARY.md for accepted immutable artifact and source identities.

## Deferred Items

Items acknowledged and deferred at milestone close, most recent first:

| Category | Item | Status | Deferred At | Milestone |
|----------|------|--------|-------------|-----------|
| debug_sessions | phase16-cleanup-fixture | verifying | 2026-10-24 | v4 |
| debug_sessions | phase16-interruption | investigating | 2026-10-24 | v4 |
| deferred_items | 16-host-evidence-and-roadmap/deferred-items.md: local macOS nofp reports Apple-toolchain stack_chk_fail/stack_chk_guard symbols | acknowledged | 2026-10-24 | v4 |

Seven seed acknowledgements from earlier closes remain suppressed by their own `audit_acknowledged` markers.

## Preserved Debt

- Historical Nightly run 39993108154 failed vectors-full (not a Release gate). Current-source cold fetch/full vectors and MMC3 are confirmed successful by the accepted Phase 20 Nightly artifacts; historical failures remain recorded.
- [Phase 19] Code review advisories open in 19-REVIEW-DISPOSITION.md: sprite-0 hit set only inside compose_pixel (held by NULL-video rejection), single jam_state_crc may blank an earlier jammed state under run-ahead, per-frame save_state cost in the adapter.
- [Phase 19] Navigation- and start-stage JAM branches share the boot-stage code but only boot is constructed in a test.
- [Phase 19] Historical 19-06 baseline metadata remains a truthful local dirty-tree run. Phase 20 now separately records clean hosted confirmation of the current campaign and protected starting PASS set.
- Real-campaign minimization is proven only by controls.
- Historical metadata warnings (16-05 command strings as paths; three Phase 17 references) do not change any verdict.
- Preexisting `.planning/config.json` edits remain unstaged.
- [Phase 20] CR-01/WR-03/WR-04 collector maintenance findings remain deferred with current-run mitigations/evidence in 20-REVIEW-DISPOSITION.md.
- [Metadata warning] phase.complete flags 20-12-SUMMARY.md's shortened `availability/availability.txt` pointer as absent at repository root. The actual retained scratch file exists at `build/phase20-hosted-evidence/availability/availability.txt`; this path-parser warning changes no verdict.

## Next Action

Review PR #59 and merge after its final published head's required CI passes.
Phase 20 is verified complete on the repaired 418-input source; all four assigned
requirements are complete. Phase 21 has no CONTEXT.md yet: after delivery,
`$gsd-discuss-phase 21` gathers its interrupt-latency/hijack approach.
No next lifecycle command ran here. The preexisting local config edit remains
unchanged and outside the execution commits.

## Performance Metrics

| Plan | Duration | Tasks | Files |
|------|----------|-------|-------|
| Phase 20 P11 | 69min | 2 tasks | 11 files |
| Phase 20 P12 | 232min | 2 tasks | 12 files |
| Phase 20 P10 | 15min | 2 tasks | 2 evidence files |

## Session

**Last session:** 2026-10-28T19:14:28Z
**Stopped at:** Phase 20 complete, ready to plan Phase 21
**Resume file:** None
