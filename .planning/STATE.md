---
gsd_state_version: "1.0"
milestone: v5
milestone_name: NMI, VBlank and sprite-0 timing
current_phase: 20
current_phase_name: NMI line and VBlank flag
status: verifying
stopped_at: Completed 20-06-PLAN.md
last_updated: "2026-10-27T01:40:18.946Z"
last_activity: 2026-10-26
last_activity_desc: Plan 20-06 local execution completed; awaiting parent verification gates
state_head: 5ad07e9997e7858fdb18f1272e2c800cc387e5f0
progress:
  total_phases: 3
  completed_phases: 1
  total_plans: 12
  completed_plans: 12
  percent: 33
---

# Project State

## Project Reference

See [PROJECT.md](PROJECT.md), updated 2026-10-25 after Phase 19, and AGENTS.md.
Core value: Games behave as on the console, identically across platforms, from a small MIT C library.
Current focus: v5 NMI, VBlank and sprite-0 timing (RM-004 in [PRODUCT-ROADMAP.md](PRODUCT-ROADMAP.md)); Phase 20 NMI line and VBlank flag.

## Current Position

Phase: 20 (NMI line and VBlank flag) — EXECUTING
Plan: 6 of 6
Status: Local plans complete; hosted evidence and phase verification gates pending
Last activity: 2026-10-26 — Plan 20-06 local execution completed; awaiting parent verification gates

Progress: [███░░░░░░░] 33%

## Accumulated Context

### Decisions

- [Phase 20 oracle research]: The owner selected independent hardware/source research. A published Visual2A03 die-network probe now establishes six finite-margin sampling/retention outcomes; it separates internal phi2 from widened M2 and preserves the input/request stage distinction. The exact simultaneous PPU/CPU edge remains unverified, and no acceptance rule was relaxed. See [20-NMI-ORACLE-RESEARCH.md](phases/20-nmi-line-and-vblank-flag/20-NMI-ORACLE-RESEARCH.md).
- v5 build order follows research/SUMMARY.md: JAM attribution first; each phase writes its owned reproducers before its fix so main stays green; sprite-0 in its own phase before the NMI model; NMI line and edge latch before the poll; shared interrupt entry and hijack last.
- KEEP-02 and KEEP-03 sit in Phase 20 (hash moves, state format 2); KEEP-01 sits in Phase 21 (shared interrupt entry touches CPU vectors and the MMC3 IRQ). Every phase keeps the protected PASS set.
- [Phase 19]: JAM attribution: frame_and_sample returns nesturbator_status; ACCURACYCOIN_ACTIVE_RUNNING/BUDGET/JAM three-state classifier; execution-stage JAM is fail/0x03/guest-jam with exit 0
- [Phase 19]: 19-02: collector resolves the attributed JAM row by full key and classifies it with ACCURACYCOIN_ACTIVE_JAM; earlier-stage JAM writes no report and exits 1
- [Phase 19]: 19-03: STOP_JAM shows a never-written blank buffer for black so the cached picture survives; cache tag = frame_number + state-CRC timeline generation (D-16 amendment)
- [Phase 19]: D-10 amendment applied: cases 2, 4, 8, 9 and all-Y=$FF are guards; 1, 3, 5, 6, 7 and $FD are reproducers run through expect_red until their fix lands
- [Phase 19]: 19-05: page-19 driver case uses case-local execution cap 300; campaign caps unchanged; state/* hash rows re-pinned for behaviour revision 7
- [Phase 19]: 19-06: AccuracyCoin baseline re-recorded from measured full campaign (108 pass/38 fail/0 timeout); matched research prediction
- [Phase 20 planning]: Six sequential plans cover reproducers, split status/line capture with coherent state format 2/revision 8, NMI polling, exact race-state continuation, host replay and final evidence. Independent plan check passed after isolating four new test oracles; all four requirements and 24 decisions are assigned.
- [Phase 20]: Keep live PPU, bus, and guest observations separate from source-assembly table comparator checks.
- [Phase 20]: Equal integer timestamps use PPU transition, CPU sample, then bus side effect; exact physical tie precedence remains unclaimed.
- [Phase 20]: The bus captures a rising derived NMI line at cycle close; later line deassertion does not revoke the request.
- [Phase 20]: PPUSTATUS decoding requires the PPU address range as well as register bits 010, including for DMC parked reads.
- [Phase 20]: NMI instruction polls use a separately serialized cpu.nmi_polled latch distinct from the captured cpu.nmi_pending request; late captures remain until the next poll.
- [Phase 20]: Plan 20-03 keeps one global timestamp model and no per-table offset; the exact physical tie remains unproven.
- [Phase 20]: Exact NMI cutpoints use declared constructed origins and complete production cpu_step calls; no partial-instruction state API is introduced.
- [Phase 20]: Captured NMI history remains serializable after VBlank status clears; a CRC-correct omitted-history control is caught by the guest-marker oracle.
- [Phase 20]: Plan 20-05: public host frame boundaries validate NMI guest history separately from exact instruction-boundary race evidence.
- [Phase 20]: Plan 20-05: runner audio hashes compare the identical resumed suffix against uninterrupted execution.
- [Phase 20]: Plan 20-05: KEEP-02 and KEEP-03 remain pending; local host replay does not establish six-platform or later hosted evidence.
- [Phase 20]: Plan 20-06 gates each full AccuracyCoin NMI target key independently of aggregate totals.
- [Phase 20]: The 108-key protected PASS authority stays separate from the six promoted NMI targets.
- [Phase 20]: KEEP-02/03 remain pending hosted exact-head evidence; local cutpoint and host replay claims stay bounded.

### Blockers/Concerns

- [Phase 20 execution]: Plans 20-02 through 20-05 are complete under amended D-10. Plan 20-03 revalidated all six target rows at default alignment under the single global model policy; Plan 20-04 reached enabled set/clear and captured-before-poll states by complete production instructions and replayed them through public save/load; Plan 20-05 added local frame-boundary replay across core, runner, and libretro. The exact physical paired-chip tie remains an evidence limit. Plan 20-06 remains pending under its normal prerequisites.
- `tests/accuracy/measurements.md` records the Plan 20-03 source-sequence results, Plan 20-04's constructed/executed origins, all 20 bounded Control 8/9 candidates, and the production sampler endpoint projections. No alignment shift or per-table offset was selected.
- KEEP-02 and KEEP-03 remain pending for cross-platform hash confirmation and hosted evidence. Plan 20-05 supplies local host replay; it does not resolve physical tie precedence or satisfy later hosted evidence. Plan 20-04 found no required instruction-boundary cutpoint that needs clarification.
- The pause handoff records three Nightly vectors-full fetch failures on 2026-10-26 (curl 28 / bad pack header from the upstream 65x02 download). This was external to Phase 19 and not a required PR check. Quick task 261026-i8z preserved the cold-fetch policy; no fresh hosted recheck was run.

### Pending Todos

- [2026-10-26] [testing] Add reproducible owned property tests — [todo file](.planning/todos/pending/2026-10-26-add-reproducible-owned-property-tests.md)
- [2026-10-26] [testing] Evaluate scoped nightly mutation testing — [todo file](.planning/todos/pending/2026-10-26-evaluate-scoped-nightly-mutation-testing.md)
- [2026-10-26] [testing] Make runner signal checks deterministic — [todo file](.planning/todos/pending/2026-10-26-make-runner-signal-checks-deterministic.md)

### Quick Tasks Completed

| # | Description | Date | Commit | Directory |
|---|-------------|------|--------|-----------|
| 261026-i8z | Reduce repeated CI suites, session controls and vector inventory work | 2026-10-26 | dbbf04f | [261026-i8z-reduce-ci-overhead-using-the-recovered-c](./quick/261026-i8z-reduce-ci-overhead-using-the-recovered-c/) |

## Delivery

Phase 16 is on main and released as v0.1.11. Phases 17 and 18 merged as PR #55 (exact-head CI 40150921041 green; Nightly 40150888217 success); PR #54 was folded into it. Release PR #56 (0.1.12) was open at the v5 opening; record its publication in MILESTONES.md once observed. Phase 19 squash-merged as PR #57 on 2026-10-26 (`ab9c2e6`), after the CI repairs documented in `debug/resolved/pr57-ci-failures.md`. Branch `gsd/phase-20-nmi-line-and-vblank-flag` was created and pushed from that main. Phase 20 context, research, six plans and a passed plan check are recorded. Plans 20-01 through 20-05 are complete; Phase 20 remains in progress with final evidence pending. Plan 20-05 proves local host frame-boundary replay only; KEEP-02/03 still need six-platform and hosted evidence.

## Deferred Items

Items acknowledged and deferred at milestone close, most recent first:

| Category | Item | Status | Deferred At | Milestone |
|----------|------|--------|-------------|-----------|
| debug_sessions | phase16-cleanup-fixture | verifying | 2026-10-24 | v4 |
| debug_sessions | phase16-interruption | investigating | 2026-10-24 | v4 |
| deferred_items | 16-host-evidence-and-roadmap/deferred-items.md: local macOS nofp reports Apple-toolchain stack_chk_fail/stack_chk_guard symbols | acknowledged | 2026-10-24 | v4 |

Seven seed acknowledgements from earlier closes remain suppressed by their own `audit_acknowledged` markers.

## Preserved Debt

- Nightly run 39993108154: vectors-full failed (not a Release gate).
- [Phase 19] Code review advisories open in 19-REVIEW-DISPOSITION.md: sprite-0 hit set only inside compose_pixel (held by NULL-video rejection), single jam_state_crc may blank an earlier jammed state under run-ahead, per-frame save_state cost in the adapter.
- [Phase 19] Navigation- and start-stage JAM branches share the boot-stage code but only boot is constructed in a test.
- [Phase 19] 19-06 baseline-metadata records a local dirty-tree run (source_dirty=1); a clean hosted run should confirm it.
- Real-campaign minimization is proven only by controls.
- Historical metadata warnings (16-05 command strings as paths; three Phase 17 references) do not change any verdict.
- Preexisting `.planning/config.json` edits remain unstaged.

## Next Action

Plan 20-06 local execution is complete; Phase 20 remains in progress for the parent verification gates. The exact-six target gate, final campaign provenance, 108-key protected PASS comparison, native hash inventory, local CI, and ASan all pass. KEEP-02/03 remain pending exact-head six-platform hash/protected PASS and hosted evidence; the Nightly vectors-full fetch debt and physical exact-tie limit remain open. No hosted result or full Phase 20 completion is claimed.

## Session

Last session: 2026-10-27T01:40:18.906Z
Stopped at: Completed 20-06-PLAN.md
Resume file: None

## Performance Metrics

| Plan | Duration | Tasks | Files |
|------|----------|-------|-------|
| Phase 17 P01 | 54min | 3 tasks | 15 files |
| Phase 17 P02 | 67min | 3 tasks | 12 files |
| Phase 17 P03 | 42min | 2 tasks | 15 files |
| Phase 17 P04 | 7h22m | 2 tasks | 38 files |
| Phase 18 P01 | 59min | 2 tasks | 12 files |
| Phase 18 P02 | 60min | 3 tasks | 13 files |
| Phase 18 P03 | 20min | 2 tasks | 13 files |
| Phase 18 P4 | 3h20m | 3 tasks | 20 files |
| Phase 19 P01 | 35 min | 3 tasks | 6 files |
| Phase 19 P02 | 40 min | 3 tasks | 5 files |
| Phase 19 P03 | 75 min | 3 tasks | 6 files |
| Phase 19 P04 | 50 min | 2 tasks | 2 files |
| Phase 19 P05 | 40min | 3 tasks | 9 files |
| Phase 19 P06 | 40 min | 3 tasks | 10 files |
| Phase 20 P01 | 32min | 2 tasks | 6 files |
| Phase 20 P02 | 90min | 3 tasks | 24 files |
| Phase 20 P03 | 83min | 3 tasks | 25 files |
| Phase 20 P04 | 31min | 3 tasks | 4 files |
| Phase 20 P5 | 32min | 3 tasks | 12 files |
| Phase 20 P6 | 26min | 3 tasks | 7 files |
