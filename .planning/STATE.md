---
gsd_state_version: "1.0"
milestone: v5
milestone_name: NMI, VBlank and sprite-0 timing
current_phase: 21
current_phase_name: Interrupt latency and hijack
current_plan: 9 of 9 complete
status: completed
stopped_at: CI efficiency quick task completed on the phase branch; PR 61 merge remains
last_updated: "2026-10-30T00:49:54Z"
last_activity: 2026-10-29
last_activity_desc: "Completed CI efficiency quick task 261029-sdj"
state_head: d85a058dd7e186180a6c6903f4253ad39b99d365
progress:
  total_phases: 3
  completed_phases: 3
  total_plans: 27
  completed_plans: 27
  percent: 100
---

# Project State

## Project Reference

See [PROJECT.md](PROJECT.md), updated 2026-10-29 after Phase 21, and AGENTS.md.
Core value: Games behave as on the console, identically across platforms, from a small MIT C library.
Current focus: Phase 21 is complete; all 27 v5 plans and all 11 requirements are verified. PR review/merge and milestone review remain.

Plan 21-09 collected and validated actual CI `40929011059/1` and Nightly
`40929010997/1` for candidate `54c7db2`, tested merge `faf1b91` and protected
base `a64c23f`. All 428 effective inputs match candidate, merge, current and
clean source, including actual filesystem modes checked independently. All
16 required artifact ZIPs match API digests. The clean campaign is 119 PASS /
27 FAIL / five DRAW inventory entries, all 146 scored rows terminal, with
all eleven targets and 114 protected starting PASS keys retained on all six
lanes. Complete native hash inventories match and the unchanged +1% performance
gate passes all three ROMs. Native every-stage and actual-adapter replay pass.

Independent verification passed 32/32. Fresh parent CI passed 695/695 in 382.16
seconds and generated library, runner and libretro packages. INT-01, INT-02 and
KEEP-01 are complete. Source format 3/revision 9/ABI 1 remain unchanged. The
config edit and original causal history under
`refs/phase21-history/execution-6c42607` remain intact. Deferred WR-01 and
Phase 20 CR-01/WR-03/WR-04 have run-specific mitigations, not recurring fixes;
physical exact-tie and judgment-tier provenance limits remain explicit.

Phase 21 execution evidence and the shipping note are published through existing
PR #61, ready for review. No merge or further lifecycle command ran. See
`21-EVIDENCE.md`, `21-EXECUTION-EVIDENCE.md` and `21-VERIFICATION.md` in the
Phase 21 directory. Earlier source pins and pending statuses below are historical.

## Current Position

Phase: 21 (Interrupt latency and hijack) — COMPLETE
Current Plan: 9 of 9 complete
Plan: All 9/9 phase plans complete; all 27/27 v5 plans complete
Status: Verified complete; separate ship step and milestone audit/closeout remain
Last activity: 2026-10-29 -- Phase 21 shipped as PR #61

Progress: [██████████] 100%

## Accumulated Context

### Decisions

- [Phase 21 closeout]: Accept only source-bound final hosted attempts; keep candidate, merge, protected base and evidence-only commits distinct. All 428 input modes/blobs and actual filesystem modes match.
- [Phase 21 closeout]: The independent result is 32/32 with all three requirements satisfied. Retain finite-margin integer-timestamp semantics, explicit unverified physical ties and judgment-tier prohibitions without adding a manual sign-off gate.
- [Phase 21 closeout]: Defer recurring source-identity WR-01 with collector maintenance; independent filesystem checks mitigate this accepted run. Preserve Phase 20 CR-01/WR-03/WR-04 and original execution history.

Earlier decisions below retain their original point-in-execution status; pending hosted acceptance was resolved by Plan 21-09 above.

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

- [Phase 21 review] WR-01: recurring identity verification omits actual regular-file worktree mode. Both current checkouts independently match all 428 actual modes/blobs, including an execute-bit/type supplement; future helper repair remains deferred in 21-REVIEW-DISPOSITION.md.

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
| 261029-sdj | Reduce inventory parsing and reuse CTest-validated hash artifacts | 2026-10-29 | dc3451b | [261029-sdj-optimize-no-skip-inventory-parsing-and-r](./quick/261029-sdj-optimize-no-skip-inventory-parsing-and-r/) |

## Delivery

Quick task `261029-sdj` completes the CI cleanup in `dc3451b`. The no-skip
policy takes 1.951 seconds locally versus 8.756 seconds for the original helper
on the same 695-test inventory, with existing and new negative cases retained.
Hosted lanes clear and upload the native hashes already generated and pin-checked
by CTest, preserving all six artifact names and the full equality gate. Final
local CI passes 695/695 in 418.82 seconds and generates all three packages;
archive, hash, policy, hygiene and workflow syntax checks pass. The actual
campaign remains 119 PASS/27 FAIL with its unchanged report digest.

Code and planning records publish together through PR #61; its resulting
automated checks provide hosted qualification. The previous head `d480280`
has successful CI `40960199568/1` and Nightly `40960199553/1`, superseding the
then-pending statuses in the historical debug handoff below. Prior immutable
phase acceptance keeps its original source identity. The config edit and
recorded debt remain unchanged. Planning-only fast paths remain proposed
followup scope; this task changes neither required-check policy nor coverage.

Post-publication debug repaired supervised poll interpreter cost and a TERM race
during operation timing initialization in `f9ae22e`. Fresh local CI passes
695/695 in 423.66 seconds with all packages; hosted CI `40956561386/1` passes
all 18 jobs. Nightly `40956561408/1` Intel session controls pass all three tests,
including the full matrix in 576.46 seconds under the unchanged 900-second cap.
Both required hosted session lanes also pass. The broader Nightly suite-flake
job is still running at this handoff; its other six work jobs succeed. Final
evidence publication changes planning records only and starts new checks.
Observe their results before merging. The previous immutable phase bundle keeps
its original source identity. The config edit and all recorded debt remain
preserved. See `debug/resolved/hosted-session-poll.md` for causal controls,
evidence limits and run identities. No merge or further lifecycle step ran.

Phase 21 is published through existing PR #61 on
`gsd/phase-21-interrupt-latency-and-hijack`, ready for review. Fresh ship-time
`cmake --workflow --preset ci` passed 695/695 in 420.78 seconds and generated
all three packages; raw output is retained at `build/phase21-ship-ci.log`.
Production immutable-bundle validation passed against the shipping checkout.
Independent actual-filesystem mode/raw-blob checks matched all 428 effective
inputs in both current and clean-source checkouts to accepted candidate
`54c7db2` and tested merge `faf1b91`. The final publication changes evidence only.
The PR description now records accepted CI `40929011059/1`, Nightly
`40929010997/1`, all eleven targets, 114 protected PASS keys and six-platform
hash equality. Existing review dispositions and WR-01/Phase 20 collector debt
remain visible. The config edit and original execution history remain intact.
The shipping commit has no CI-skip token, so required checks run on the final
published head. Review and merge remain; no merge or next lifecycle step ran.

The execution checkpoint below predates this ship publication.

Current Phase 21 acceptance is complete for candidate `54c7db2` / tested merge
`faf1b91`, CI `40929011059/1` and Nightly `40929010997/1`. The immutable bundle
is `build/phase21-hosted-evidence/`; the accepted effective digest is
`004e963df11ad77f080e422ea0b87dab5c060117d38ff9b43d330116b974479e`.
Plan 21-09 evidence and independent verification are committed locally. Ship
separately through existing draft PR #61; this execution performs no remote mutation.

Historical Phase 20 delivery follows; its publication and pending states are historical.

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

Merge PR #61 as a separate step after its publication checks are green.
CI cleanup `dc3451b` passes full local qualification; the publication checks
verify hosted policy behavior and six-platform hash reuse. The prior head
`d480280` has successful required CI and Nightly results.
All phase plans, requirements and independent verification are complete.
Preserve the exact config edit,
original causal history, accepted 428-input source, deferred collector findings
and physical-tie limits. After PR review/merge, run the milestone audit and
closeout as separate authorized steps; do not treat verified implementation
as an already shipped milestone.

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
| Phase 21 P09 | 18min | 2 tasks | 2 files |

## Session

**Last session:** 2026-10-30T00:49:54Z
**Stopped at:** CI efficiency quick task complete; PR #61 publication checks and merge remain
**Resume file:** None
