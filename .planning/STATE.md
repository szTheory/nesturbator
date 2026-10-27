---
gsd_state_version: "1.0"
milestone: v5
milestone_name: NMI, VBlank and sprite-0 timing
current_phase: 20
current_phase_name: NMI line and VBlank flag
status: awaiting_hosted_results
stopped_at: PR 59 replacement e6e83a3 published; CI 40514060876 and Nightly 40513950000 active
last_updated: "2026-10-27T13:41:24Z"
last_activity: 2026-10-27
last_activity_desc: Session dependencies and cold-start cleanup repaired; performance gate restored without raising caps; final local verification recorded in delivery ledger
state_head: e6e83a31e413e12abe9dee142e3c0f7143d63eb4
progress:
  total_phases: 3
  completed_phases: 1
  total_plans: 16
  completed_plans: 15
  percent: 33
---

# Project State

## Project Reference

See [PROJECT.md](PROJECT.md), updated 2026-10-25 after Phase 19, and AGENTS.md.
Core value: Games behave as on the console, identically across platforms, from a small MIT C library.
Current focus: v5 NMI, VBlank and sprite-0 timing (RM-004 in [PRODUCT-ROADMAP.md](PRODUCT-ROADMAP.md)); Phase 20 NMI line and VBlank flag.

## Current Position

Phase: 20 (NMI line and VBlank flag) — AWAITING HOSTED RESULTS
Plan: 9 of 10 complete; 20-10 has zero completed tasks
Status: Draft PR #59 replacement e6e83a3 published; CI 40514060876 and Nightly 40513950000 active; all local gates passed; 20-10 and all requirements remain pending
Last activity: 2026-10-27 — Delivery authorized; formatting hook repaired without bypass; remeasured campaign and full CI passed 684/684

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
- [Phase 20 gap planning]: Plans 20-07 through 20-10 run sequentially in waves 7–10: changing-sprite decoder witness and WR-01/02 corrections; equal-tick and combined empty-PCM state witnesses; local hosted collector/retention preparation; then real hosted collection. Independent review passed after separating preparation from collection so 20-09 closes normally and 20-10 checks external delivery before any task commit. All four requirements and 24 decisions remain covered; no implementation or hosted result is claimed by planning.
- [Phase 20]: Keep live PPU, bus, and guest observations separate from source-assembly table comparator checks.
- [Phase 20]: Plan 20-03 supersedes Plan 20-02's initial tie policy: equal integer timestamps use bus side effect, detector capture, then the tied PPU transition; CPU polling reads captured history at phi2 open. Exact physical tie precedence remains unclaimed.
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
- [Phase 20]: Plan 20-07: split $2002 samples remain distinct; dot-130/131 witnesses prove the late sprite flag through both production decoder aliases.
- [Phase 20]: Keep all four Phase 20 requirements pending for independent re-verification; local dirty campaign, hosted evidence debt, and physical exact-tie limit remain explicit.
- [Phase 20]: Keep full artifact keys as identity when frame and state observations share ticks; leave schema and native hash pins unchanged.
- [Phase 20]: Serialize APU sample phase and synthesizer/filter history independently of transient PCM staging.
- [Phase 20]: Treat synthetic inventory and internal frame-zero results as local evidence; keep hosted requirements pending.
- [Phase 20]: Fixture provenance cannot satisfy hosted acceptance; collection requires fresh run and attempt metadata plus source-content verification.
- [Phase 20]: Bind pull-request candidate head and protected base to the actual tested synthetic merge commit, with an independent full effective-source digest.
- [Phase 20]: Keep all four Phase 20 requirements pending until real hosted evidence passes; local preparation alone does not complete the phase.

### Blockers/Concerns

- [Phase 20 execution]: Plans 20-01 through 20-09 are locally executed and summarized under amended D-10. Plan 20-03 revalidated all six target rows at default alignment under the single global model policy; Plan 20-04 reached enabled set/clear and captured-before-poll states by complete production instructions and replayed them through public save/load; Plan 20-05 added local frame-boundary replay across core, runner, and libretro; Plan 20-06 recorded the measured campaign; Plan 20-07 added the changing-sprite M2 sample witness and fixed WR-01/02 locally; Plan 20-08 added the equal-tick inventory and combined frame-zero audio-history witnesses; Plan 20-09 completed the locally tested collector and artifact retention. The exact physical paired-chip tie remains an evidence limit. Only 20-10 remains, pending separately authorized hosted delivery.
- [Plan 20-10 checkpoint]: Read-only checks found no `build/phase20-hosted-evidence/delivery.json`, CI/Nightly runs on the phase branch, or phase PR. Its first-task external precondition is unmet (`pending_delivery`, blocking-human availability gate); no collection, artifact mutation, task commit or SUMMARY was made. This is a missing delivery fact, not a manual test or sign-off request.
- [Execution handoff]: Corrected 20-10's future command examples to use the completed collector's checkout-directory argument `SOURCE_REF=.` instead of the planning placeholder `HEAD`. Only planning prose changed; no collection or 20-10 task was executed and the effective-source digest remains unchanged.
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

Phase 16 is on main and released as v0.1.11. Phases 17 and 18 merged as PR #55 (exact-head CI 40150921041 green; Nightly 40150888217 success); PR #54 was folded into it. Release PR #56 (0.1.12) was open at the v5 opening; record its publication in MILESTONES.md once observed. Phase 19 squash-merged as PR #57 on 2026-10-26 (`ab9c2e6`), after the CI repairs documented in `debug/resolved/pr57-ci-failures.md`. Branch `gsd/phase-20-nmi-line-and-vblank-flag` was created and pushed from that main. Plans 20-01 through 20-09 are locally executed and summarized. Plan 20-09's final local CI passed 684/684 and packaged the library, runner, and libretro outputs; the final four-witness recheck passed 4/4. Its causal source commit is `392b8f5`, effective digest `684085be7b2ecccc55f30eb885387a111aecfb622950da3184a84bce1d489c43` over 415 files; later planning-only commits remain separate from the eventual delivered candidate. Hosted exact-head evidence remains pending; Phase 20 and all four requirement IDs remain open for independent phase-level re-verification. The original gap-execution step did not push, create a PR, dispatch workflows, or claim hosted results. The subsequent unblocking request authorized PR #59 delivery and its evidence-driven repairs; see 20-DELIVERY.md for superseding source identities and actual results.

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
- [Phase 20] Plan 20-07 locally resolved review warnings WR-01/02 and the changing-sprite-status M2 decoder gap. Its dirty-source local campaign preserves all six target passes and the 108-key protected PASS set; hosted authority remains separate.
- [Phase 20] Remaining verification gap: exact-head hosted evidence and protected-PASS retention. Plan 20-08 locally resolved equal-tick inventory adjacency and combined frame-zero/empty-PCM/nonzero-audio-history witnesses. See 20-VERIFICATION.md and 20-EVIDENCE.md. The clean-room prohibition remains visibly flagged without an invented manual sign-off gate; hygiene passed and no contamination was found.

## Next Action

Monitor draft PR #59 candidate `e6e83a31e413e12abe9dee142e3c0f7143d63eb4`: CI `40514060876` and Nightly `40513950000`, both attempt 1. The actual PR merge snapshot is `11533e456d41f740751c8353f947d65017aae692`. Delivery JSON and 20-DELIVERY.md record these selected identities; earlier runs are superseded. The effective source digest is `de6d4355222063e019c7a3a94e30a404b52041f3d994659d445f12b4a23318ab` over 415 files. Local CI 684/684, ASan/UBSan 676/676, MMC3 13/13 and the real per-ROM performance gate passed without cap changes. Once required hosted jobs and artifacts are complete and available, run `$gsd-execute-phase 20 --gaps-only` to resume only 20-10 and independent phase verification. This planning-only handoff stays local to avoid restarting CI. Keep requirements open, preserve unrelated config edits and debt, and do not merge or advance to Phase 21 before actual hosted gates and re-verification pass.
