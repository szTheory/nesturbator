---
gsd_state_version: "1.0"
milestone: v5
milestone_name: NMI, VBlank and sprite-0 timing
current_phase: 21
current_phase_name: Interrupt latency and hijack
current_plan: 9 of 9
status: executing
stopped_at: Completed 21-08 post-review source qualification; 21-09 pending hosted availability
last_updated: "2026-10-29T16:46:49.565Z"
last_activity: 2026-10-29
last_activity_desc: Plan 21-08 local acceptance contract passed; final CI 695/695, ASAN 686/686, vectors-full 258/258, MMC3 13/13; hosted acceptance remains pending external availability
state_head: 43d260a914ba76a130809524fba53ad58aca1298
progress:
  total_phases: 3
  completed_phases: 2
  total_plans: 27
  completed_plans: 26
  percent: 67
---

# Project State

## Project Reference

See [PROJECT.md](PROJECT.md), updated 2026-10-28 after Phase 20, and AGENTS.md.
Core value: Games behave as on the console, identically across platforms, from a small MIT C library.
Current focus: v5 NMI, VBlank and sprite-0 timing (RM-004). Plans 21-01 through 21-08 are locally complete. The final source-bound campaign remains 119 PASS / 27 FAIL / 5 DRAW across 146 terminal rows; all eleven target rows and all 114 protected starting PASS keys remain covered. Plan 21-08's final local gates passed: CI 695/695, ASAN 686/686, vectors-full 258/258, and MMC3 13/13. The bounded DMC/open-bus prerequisite is resolved. Plan 21-09 is next but remains pending separately delivered hosted CI/Nightly availability and actual PR/tested-merge facts. Hosted evidence and all three phase requirements remain pending. See 21-04-SUMMARY.md, 21-04-CHECKPOINT.md, 21-06-SUMMARY.md, 21-07-SUMMARY.md, and 21-08-SUMMARY.md. Earlier Phase 20 notes remain historical.

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
20-REVERIFICATION-EVIDENCE.md in the Phase 20 directory. Phase 21 execution
began with Plan 21-01; its summary records 262/262 focused tests and 689/689
full CI tests, with all three packages generated.

## Current Position

Phase: 21 (Interrupt latency and hijack) — IN PROGRESS
Current Plan: 9 of 9
Plan: 21-08 complete; 8/9 complete; Plan 21-09 is next, pending external hosted evidence
Status: Local final CI, ASAN, full vectors, and MMC3 checks passed; hosted/all-lane phase acceptance remains pending
Last activity: 2026-10-29 — Phase 21 Plan 08 local preparation passed its final configured gates

Progress: [███████░░░] 67%

## Accumulated Context

### Decisions

- [Phase 21 approved continuation, 2026-10-29]: The owner approved bounded investigation and repair of the traced DMC/open-bus prerequisite at `$4013`, preserving the prepared shared-entry implementation, one green causal commit and every acceptance gate. A general DMA cluster and remote delivery remain outside this approval. The final source-bound campaign and local CI now pass; hosted/all-lane evidence remains pending.

- [Workflow preference]: Mechanical planning file/task-count limits are advisory. Keep justified coherent work together, document the reason and proceed without routine approval interruptions. This does not waive product scope, compatibility, correctness, measured performance/CI gates or the one-step/merge boundaries. Phase 21 Plans 02/04 are explicitly approved at 36/35 files, including 17 generated state seeds each.

- Phase 20 completes with 32/32 verified truths and all four assigned requirements satisfied. All 12 plan summaries, original gap closures and the source-bound accepted hosted bundle are preserved in the phase directory.
- Captured NMI history and polled NMI work remain separate serialized latches. Exact race states use complete production instructions and label constructed origins separately from executed endpoints.
- The finite-margin circuit evidence constrains one global integer-timestamp policy; no physical paired-chip exact-tie winner or per-table alignment shift is claimed. Full decision history remains in 20-CONTEXT.md, summaries and measurements.
- Current candidate `7848cc8`, tested merge `716e8d4`, protected base `ab9c2e6` and evidence-only commits remain separate. All 418 effective inputs and Git modes/blob IDs match. Earlier candidate `fe70d61` / tested merge `ef6d9e5` retain their historical 417-input evidence; source-changing repairs require fresh hosted evidence.
- Phase 21 retains its planned IRQ/poll/shared-entry/hijack scope. Phase 20 collector hardening is explicit debt, not silently added scope or an acceptance waiver.
- [Phase 21]: Adopt poll-time IRQ acceptance and shared internally resumable BRK/IRQ/NMI entry; preserve public frame semantics; plan state format 3 and behavior revision 9.
- [Phase 21]: Live GitHub metadata and fetched origin/main confirm PR #59 merged at a64c23fd3ca7caed7e3ad102ff1a9beaa29fa562; Phase 20 delivery blocker is resolved. Earlier open-PR/next-action delivery prose is historical.
- [Phase 21]: Separate production APU source proof from clearly labeled flat-bus IRQ schedule evidence.
- [Phase 21]: Retain exact-count expected-red poll defect checks until their causal implementation plan removes each wrapper.
- [Phase 21]: Keep AccuracyCoin counts and report hash fixed; update only the build-input digest changed by test registration.
- [Phase 21 Plan 21-01]: Cover both legal branch polls with a flat-bus assertion/acknowledgment window matrix and a production APU/DMC E source/acknowledgment guest; use the C4-low flat-bus expected-red control to discriminate lost first-poll acceptance until the poll fix lands in Plan 21-02.
- [Phase 21]: Keep the approved format 3 / behaviour revision 9 change with unchanged ABI and refuse native formats 1 and 2.
- [Phase 21]: Record the measured test-E latency divergence. The row now passes in Plan 21-04's local final campaign; INT-01 remains pending phase-level hosted/all-lane acceptance.
- [Phase 21]: Keep KEEP-01 pending until protected passes are confirmed across every CI lane.
- [Phase 21]: Treat the T5-phi1 after-C4/before-C5 mapping as a finite-margin inference; exact physical tie precedence remains unmeasured.
- [Phase 21]: Use guest-captured complete paired BRK/IRQ pushed-status tables with a single common exact or +1 alignment per pair.
- [Phase 21]: Plan 21-04 completed after the owner-approved bounded DMC/open-bus repair; all eleven local targets, all 114 protected keys and full local CI pass, while hosted/all-lane acceptance remains pending.
- [Phase 21]: Use source-reachable production entry transitions as legal siblings for state rejection controls.
- [Phase 21]: Keep INT-02 and KEEP-01 pending until hosted and all-lane acceptance is complete.
- [Phase 21]: Keep constructed-origin cutpoint evidence separate from ordinary cpu_step initiation evidence.
- [Phase 21]: Leave runtime code unchanged because Plan 21-04 already supplies the exercised entry behavior.
- [Phase 21]: Keep constructed entry-cutpoint labels separate from ordinary public frame observations.
- [Phase 21]: Reuse the existing libretro.runahead integration target; no CMake registration or AccuracyCoin fingerprint change was needed.
- [Phase 21]: Phase 21 Plan 08 selects the all-eleven target set, 114-key protected authority, revision 9, and current library version through reused collector helpers while keeping Phase 20 defaults intact.
- [Phase 21]: CR-01, WR-03, and WR-04 remain deferred; source identity uses a separate tracked Git mode/blob manifest and clean-equivalent checkout/current-checkout revalidation for this local run.
- [Phase 21]: Plan 21-09 remains pending actual separately delivered CI and Nightly run/attempt, PR, tested-merge, and artifact facts; local controls and packages do not count as hosted acceptance.
- [Phase 21]: Post-review CR-02 uses the existing nmi_pending byte value 2 for an inhibited NMI candidate; candidate 43d260a is locally qualified, while hosted Plan 21-09 and INT-01/INT-02/KEEP-01 remain pending.

### Blockers/Concerns

- [Phase 21 execution] Plan 21-04's historical pre-repair failure (`$4013` returned `$40`; latency was `0x03/3`; overlap rows were not-run) is resolved by the owner-approved bounded DMC retry/controller open-bus repair. Final local campaign evidence is 119 PASS / 27 FAIL / 5 DRAW over 146 terminal rows, with all 114 protected starting PASS keys retained and all eleven targets passing. Plan 21-06 full CI passed 693/693. Hosted/all-lane acceptance remains assigned to later plans. See 21-04-SUMMARY.md, 21-04-CHECKPOINT.md, and 21-06-SUMMARY.md.

- [Phase 20 review debt] CR-01: recurring collector enforcement omits Git file modes. The current accepted run has an independent 418/418 mode/blob match against all three hosted trees; future enforcement repair is deferred.
- [Phase 20 review debt] WR-03: source scoreboard lookup recursively scans ignored build fixtures and relative SOURCE_REF fails. Collection used an absolute clean-equivalent checkout and current-checkout validation; direct tracked-path handling remains deferred.
- [Phase 20 review debt] WR-04: duplicate identical Nightly validator definition remains deferred cleanup. See 20-REVIEW-DISPOSITION.md for all dispositions.
- [Evidence limit] Exact physical paired-chip tie precedence remains unverified under amended D-10. The judgment-tier clean-room prohibition remains explicitly flagged; hygiene passes and no contamination was found. No manual sign-off gate is introduced.
- [Historical Phase 21-02 evidence, superseded by Plan 21-04]: AccuracyCoin latency was `0x03/3` before the DMC/open-bus prerequisite repair. Final local latency now passes; phase-level INT-01 remains open for hosted/all-lane acceptance.

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

Plan 21-09 is next but its external precondition is not met: completed current-candidate CI/Nightly run and attempt identities, actual PR/tested-merge facts, and immutable artifacts are absent. Keep INT-01, INT-02, and KEEP-01 pending. Preserve the unrelated `.planning/config.json` edit and all collector debt. Do not start remote delivery or another lifecycle step from this closeout.

## Performance Metrics

| Plan | Duration | Tasks | Files |
|------|----------|-------|-------|
| Phase 20 P11 | 69min | 2 tasks | 11 files |
| Phase 20 P12 | 232min | 2 tasks | 12 files |
| Phase 20 P10 | 15min | 2 tasks | 2 evidence files |
| Phase 21 P01 | 82min | 2 tasks | 11 files |
| Phase 21 P02 | 62min | 3 tasks | 21 files |
| Phase 21 P03 | 40m | 2 tasks | 6 files |
| Phase 21 P04 | 21m | 3 tasks | 24 files |
| Phase 21 P05 | 25m | 2 tasks | 4 files |
| Phase 21 P06 | 29m | 2 tasks | 5 files |
| Phase 21 P07 | 24m | 2 tasks | 3 files |
| Phase 21 P08 | 84min | 3 tasks | 13 files |

## Session

**Last session:** 2026-10-29T16:46:49.286Z
**Stopped at:** Completed 21-08 post-review source qualification; 21-09 pending hosted availability
**Resume file:** .planning/phases/21-interrupt-latency-and-hijack/21-09-PLAN.md
