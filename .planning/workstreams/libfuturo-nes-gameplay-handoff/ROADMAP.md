# Roadmap: nesturbator

## Milestones

- ✅ **v1 NROM games with sound** — Phases 1–4.2 (shipped 2026-10-09) — [archive](milestones/v1-ROADMAP.md)
- ✅ **v2 Most of the library plays** — Phases 5–10 (shipped 2026-10-11) — [archive](milestones/v2-ROADMAP.md)
- ✅ **v3 Save states and rewind** — Phases 11–15 (shipped 2026-10-13) — [archive](milestones/v3-ROADMAP.md)
- ✅ **v4 Measured NTSC accuracy and robustness** — Phases 16–18 (shipped 2026-10-24) — [archive](milestones/v4-ROADMAP.md)
- ✅ **v5 NMI, VBlank and sprite-0 timing** — Phases 19–21 (planning milestone shipped 2026-10-30; package publication later completed) — [archive](milestones/v5-ROADMAP.md)
- ✅ **v6 Player-ready NES and reproducible feedback** — Phases 22–23 (planning milestone shipped 2026-10-31; no package release) — [archive](milestones/v6-ROADMAP.md)
- ✅ **v7 Remaining DMA accuracy failures** — Phase 24 (planning milestone shipped 2026-11-01; no package release) — [archive](milestones/v7-ROADMAP.md)
- ✅ **v8 PAL region support** — Phases 25–27.1 (planning milestone shipped 2026-11-03; no package release or tag) — [archive](milestones/v8-ROADMAP.md)
- 🚧 **v9 LibFuturo NES gameplay handoff** — Phase 28 (requirements and roadmap approved 2026-11-03)

Planning milestone completion remains separate from package publication. Release PR #56 later published v0.1.12; v0.1.12 remains the latest package until v9's release is observed. Milestone tags are disabled. See [PROJECT.md](PROJECT.md) and [PRODUCT-ROADMAP.md](PRODUCT-ROADMAP.md) for current delivery and future direction.

## v9 — LibFuturo NES gameplay handoff

An integrator should receive one current, identified Nesturbator release, a truthful initial NTSC provider profile, and a reproducible licensed NROM gameplay scenario through the installed public C package. These outcomes share an exact source/package identity, so one phase owns the full handoff. Splitting package publication from its profile and gameplay proof would create an unnecessary second candidate and requalification boundary. Actual LibFuturo installed-consumer, native-host and game-session acceptance remain LibFuturo-owned.

The owner approved the four [requirements](REQUIREMENTS.md) and single-phase roadmap on 2026-11-03. PR64/65 and failed recoveries remain historical and non-qualifying. D42 completed actual-base NTSC source in19–21, then22 failed its sole local workflow. D44 plans25/26 succeeded locally, then27 failed its sole required candidate CI;28 remains historically unstarted and blocked, with future scheduling superseded by32. The owner explicitly approved D45's two-control remedy and separate fresh route from exact `d8155bee6ea7dfd59aa3ff2ad61e60d1ed26dfa3`. New29–32 cover bounded controls/route support, one freeze/local workflow, exact candidate/main CI and normal successful release/published-byte gates. Four sequential concerns share one new candidate; no old D39/D42/D44/MMC2 retry or historical campaign/capture restart is authorized.

## Phases

- [ ] **Phase 28: Qualified NES gameplay package** — Deliver a released current package with relocated-consumer proof, a version-bound NTSC profile, and one reproducible interactive game scenario.

## Phase Details

### Phase 28: Qualified NES gameplay package

**Goal**: A LibFuturo integrator can install an identified current Nesturbator release, replay one permitted NTSC game into interactive play through its public C API, and use a provider profile bound to that exact package.
**Depends on**: v8 archived source and existing package, public API and permitted-ROM facilities; no LibFuturo host completion is required.
**Requirements**: PKG-01, INST-01, PROF-01, GAME-01
**Success Criteria** (what must be TRUE):
  1. An integrator can obtain the published library, runner and libretro artifacts, verify their checksums, and identify one consistent accepted source, package version, ABI, behavior revision and state format. A locally generated archive or release candidate alone does not satisfy publication.
  2. A clean C consumer can configure, build and run using only a relocated install prefix, load permitted content, advance frames with input, and receive video and audio; a refused replacement load leaves the active game usable. Its result does not depend on headers or libraries from the source or build tree.
  3. The manifest-permitted DABG NTSC NROM follows a bounded, reproducible input recipe from boot to a declared gameplay transition with visible response and non-silent audio. Its source-mapped progress predicate establishes interactive progress; a title picture, input activity, audio or a frame hash alone cannot satisfy it.
  4. An integrator can use the version-bound profile to determine the tested package/source, selected NTSC content and region limits, load/refusal policy, controller sampling, pixel interpretation, audio format/cadence/buffer bounds and lifecycle. Battery-backed game memory and machine snapshots are described separately, and capabilities without current consumer proof remain explicitly unqualified.
**Plans**:26 active records:22 outcomes (11 successful,11 halted) and4 unstarted D45 successors. Physical canonical plans32;06/14/15/23/24/28 are scheduler-superseded.25/26 succeeded;27 remains terminally halted and28's unmet historical prerequisite remains unchanged. Only29 is initially ready after independent plan check and exact checkout preparation;30–32 depend on successful new predecessors. D45 approval is recorded; planning consumes no acceptance allowance and closes no requirement.

Plans:
**Wave 1**
- [x] 28-01-PLAN.md — Relocated installed consumer and bounded gameplay oracle (2 tasks)

**Wave 2** *(blocked on Wave 1 completion)*
- [x] 28-02-PLAN.md — NTSC provider profile and offline receipt checks (2 tasks)

**Wave 3** *(blocked on Wave 2 completion)*
- [x] 28-03-PLAN.md — Release lineage, candidate acceptance and shipping handoff (3 tasks)

**Wave 4** *(blocked on Wave 3 completion)*
- [x] 28-04-PLAN.md — Completed bus-only candidate recovery and verification fixes (3 tasks)

**Wave 5** *(blocked on Wave 4 completion)*
- [x] 28-05-PLAN.md — Completed DABG rights/notice implementation and historical local acceptance (2 tasks)

**Wave 6** *(blocked on Wave 5 completion)*
- [x] 28-07-PLAN.md — Halted at profile NO-GO; preserved and never retried
- [x] 28-08-PLAN.md — Halted before the probe: 28/426 accepted qualification inputs differ on the active branch
- [x] 28-09-PLAN.md — Reconcile all input differences and DABG rights; freeze the complete source baseline

**Wave 7** *(successors from the reconciled 28-09 source/rights baseline)*
- [x] 28-18-PLAN.md — One D-39 source-attribution diagnostic and at most one evidence-supported correction, local workflow and exact-head required CI
- [x] 28-10-PLAN.md — Halted at the pre-edit feasibility gate; required raw ACT outputs were not retained, so no probe or production edit ran
- [x] 28-11-PLAN.md — Task 1 synthetic helper/controls complete; authorized Task 2 stopped at pinned-tool preflight before ACT launch; its one-shot root is spent; no candidate or release eligibility ([summary](phases/28-qualified-nes-gameplay-package/28-11-SUMMARY.md))
- [x] 28-12-PLAN.md — Halted after one capture-2 run: verified raw evidence, host-visible artifacts, all three unchanged workload gates fail ([summary](phases/28-qualified-nes-gameplay-package/28-12-SUMMARY.md))
- [x] 28-13-PLAN.md — Manually halted at the safe-resume gate with Task 1 partial; no verified selector/driver contract, source freeze, capture-3 or feasibility decision ([summary](phases/28-qualified-nes-gameplay-package/28-13-SUMMARY.md))
- [x] 28-16-PLAN.md — Halted after its frozen baseline was rejected for missing `role: baseline` and its one-shot baseline-cost stage failed; preserve the freeze and failed ledger, with no retry ([summary](phases/28-qualified-nes-gameplay-package/28-16-SUMMARY.md))
- [x] 28-17-PLAN.md — D-34 recovery ended halted after the sole capture-3 attempt; recovery verified the original record unchanged, but all three workloads fail +1% and lack source-line evidence

**Wave 8 — superseded history**
- 28-06-PLAN.md — Superseded by D-42 plans 28-19 through 28-24; its successful-28-18 prerequisite remains permanently unmet and historical.

**Reset wave 1 — fresh D-42 root**
- [x] 28-19-PLAN.md — Isolated exact-base relocated DABG consumer and source-mapped boundary controls

**Reset wave 2**
- [x] 28-20-PLAN.md — Truthful base-API NTSC profile, source/runtime validation and limits

**Reset wave 3**
- [x] 28-21-PLAN.md — Pinned DABG rights/notice and portable receipt with one-shot controls

**Reset wave 4**
- [x] 28-22-PLAN.md — Halted: sole local workflow failed protected AccuracyCoin fingerprint; no retry

**Reset wave 5 — superseded history**
- 28-23-PLAN.md — Unstarted/blocked by terminal22; future scheduling superseded by28-27

**Reset wave 6 — superseded history**
- 28-24-PLAN.md — Unstarted/blocked; future scheduling superseded by28-28

**D44 wave 1 — completed history**
- [x] 28-25-PLAN.md — Exact historical/current binding, mutation controls and minimum fresh receipt/driver support

**D44 wave 2 — depends on successful25**
- [x] 28-26-PLAN.md — One clean source freeze and sole bounded local full workflow

**D44 wave 3 — depends on successful26**
- [x] 28-27-PLAN.md — Sole exact-candidate/main required CI and normal protected release lineage

**D44 wave 4 — scheduler-superseded history**
- 28-28-PLAN.md — Unstarted and historically blocked by terminal27; future handoff scheduling superseded by32, never treated as success

**D45 wave 1 — sole initially ready successor after independent plan check/checkout preparation**
- [ ] 28-29-PLAN.md — Two persisted-byte/scope control fixes, measured15second headroom and minimum new route/driver support

**D45 wave 2 — depends on successful29**
- [ ] 28-30-PLAN.md — One new source freeze and sole bounded local full qualification

**D45 wave 3 — depends on successful30**
- [ ] 28-31-PLAN.md — One exact-candidate/main required CI and normal successful release lineage

**D45 wave 4 — depends on successful31**
- [ ] 28-32-PLAN.md — Actual published-byte/native/rights receipt and complete provider handoff

**Superseded plans:** 28-06/14/15/23/24/28 retain historical instructions/evidence; none is active work. The new root has no successful-halted-evidence dependency. See [D45 source audit and execution map](phases/28-qualified-nes-gameplay-package/28-D45-SOURCE-AUDIT.md), historical [D44 audit](phases/28-qualified-nes-gameplay-package/28-D44-SOURCE-AUDIT.md) and [reset audit](phases/28-qualified-nes-gameplay-package/28-RESET-SOURCE-AUDIT.md).

**Cross-cutting constraints:** <!-- gsd:generated -->
- D-37/D-38 retire the APU table attempt and preserve spent capture-3 and untouched candidate-only capture-4. D21 and old28-14/15 remain superseded. D39/D42/D44 are terminal; only D45's explicitly approved separate fresh control-maintenance route is authorized now. Historical diagnostic/route constraints below remain preserved history.
- D-39 is limited to the exact base, tested merge and PR head in 28-18-PLAN.md; one optimized `-g` diagnostic set only because required artifact 13372440822 has no source-file line mappings, using three unchanged workloads, serial execution, four workers and a 900-second ceiling. Any missing/unstable attribution or modeled row above +1% is terminal NO-GO.
- Only a conservative source-level GO for all three unchanged workloads permits at most one behavior-preserving correction, one local `cmake --workflow --preset ci`, and one exact-head required CI run. No retry, threshold change, capture-root use, campaign restart or workflow edit is authorized by D-39.
- The 2026-11-09 CI timing review found an approximately 11m17s critical path, with macOS Intel ci-build at approximately 10m50s; no safe material workflow optimization was evident. Workflows stay unchanged within D-39.
- D-42 explicitly supersedes the old 28-06 scheduler route without changing 28-18's failed outcome. The exact-base reset keeps the existing hosted/release gates but binds its own fresh identity and actual base API. D-42 authorizes no src/build-flag changes, capture/APU work, baseline edits, second candidate or retry after any failed gate.
- LibFuturo Phase 9 remains blocked pending Nesturbator's eligible published package/source/build identity, version-bound NTSC profile, licensed scenario and use/notice terms, exact-candidate and resulting-main CI, plus LibFuturo's relocated installed-consumer proof. Nesturbator owns package/profile/scenario evidence; LibFuturo owns its installed consumer, NES host and gameplay acceptance. No Playstead acceptance is established.

Plans18/22/27 remain terminally halted; none satisfies a successful prerequisite. D45's discussion/approval is complete and fresh29–32 are planned. After independent plan check and scoped planning ownership resolution, the orchestrator exclusively prepares/binds exactly build/phase28-d45-candidate on agent-phase28-d45-controls from the approved base before unfiltered execution. No diagnostic/candidate/capture/workflow/hosted/release/campaign allowance was consumed in planning.

### Phase 28 evidence and release boundary

Discuss Phase 28 must settle the exact game access route and independent basis for its progress predicate, the current LibFuturo handoff contract, and the package-line reconciliation before planning implementation. Reuse the existing installed-package and gameplay facilities where they detect the required boundary; add only the missing owned checks with useful recurring detection. Prefer a small local helper to another dependency.

The first route is NTSC-specific. D-42 explicitly resets the provider candidate to the base's ordinary loader, which lacks region preference/query APIs and reports ABI 1, behavior revision 9 and state format 3. Describe only its actual metadata/API shape; no newer regional preservation, PAL/Dendy qualification or snapshot compatibility is claimed. The handoff does not establish LibFuturo host playback, Playstead, physical-device pacing, hardware accuracy, snapshots or broad mapper/title compatibility.

Freeze and qualify the final source/package/profile/scenario identity before publication. Publication is Phase 28's final acceptance gate and is checked again at milestone closeout under [VERIFICATION-POLICY.md](VERIFICATION-POLICY.md): bounded accepted local workflow, related PR triage, exact-candidate required CI, green resulting main CI, clean project-owned release worktrees with unrelated work preserved, and verified released artifacts. Normal release mechanics may use their own workflow commands; PKG-01 remains pending until the published-package receipt exists. Keep the older interrupted CI/AccuracyCoin campaign stopped and unknown. Historical v8 results do not qualify this release candidate.

See the [coordination context](../docs/coordination/milestone-context-2026-11-03.md), [playability seed](seeds/SEED-261009-zs2-single-game-playability-check.md), [asset policy](../ASSET_POLICY.md) and [automation direction](playtesting/AUTOMATION.md) for evidence limits. A concrete core fault discovered during qualification needs independently supported, bounded scope; it does not automatically promote standalone scene-accuracy or mapper work.

## Progress

D45 discussion/approval is complete and29–32 are written; independent plan check and exact checkout preparation precede execution.26 active records have22 outcomes and4 incomplete successors;29 alone is initially ready and28 is scheduler-superseded historical blocked work. Preserve all terminal D42/D44 failures and historical baseline/results/ROM/provenance; the fresh separate allowance is explicit. Phase28/publication remain open; all old campaign/capture/APU/MMC2 stops and consumer ownership remain.

| Phase | Plans with Outcome | Status | Completed |
|-------|-------------------|--------|-----------|
| 28. Qualified NES gameplay package | 22/26 outcomes | PLANNING: D45 plans written, independent check then fresh route | — |

## Backlog disposition

2026-10-13: Backlog 999.2, “Bump AccuracyCoin past 673ef55,” was promoted into [Phase 17](milestones/v4-ROADMAP.md#phase-17-complete-pinned-conformance-baseline) and CONF-01 through CONF-04. No duplicate active backlog remains.

Current disposition: D45 explicitly approves the [two-control remedy and separate fresh route](phases/28-qualified-nes-gameplay-package/28-CONTROL-REMEDY-REVIEW.md#exact-proposed-authority). The assistant completes independent plan check, scoped planning ownership and exact checkout preparation, then continues authorized execution automatically. No user acknowledgment is needed. D42's local failure and D44's candidate CI failure stay terminal. Fresh-context entry: `$gsd-autonomous --ws libfuturo-nes-gameplay-handoff --from 28`; context may be cleared after selected STATE and D45 resume are current.

### Follow-up discussion disposition — 2026-11-10

The bounded control-remedy review and independent conceptual check are complete and the owner explicitly approved D45. Fresh29–32 implement that exact proposal; only future scheduling28 is superseded. D44/D42 approvals and executed failures remain historical/spent. Reuse completed discussion, avoid routine acknowledgments and dispatch only from checked current readiness. Planning consumed no run/campaign/remote allowance and closed no requirement.

## D45 actual disposition — handoff stopped

29 completed with final controls and independent code review.30's sole freeze
and complete local workflow passed, then its sole handoff call failed missing
required narrative checklist text.30 is halted,31/32 blocked. No hosted
admission/publication/new requirement completion. No actual gate was retried.
Read-only review supports the [exact prepared evidence-only exception](phases/28-qualified-nes-gameplay-package/28-D45-HANDOFF-RECOVERY-REVIEW.md), pending explicit owner approval; no new candidate/full workflow proposed.
