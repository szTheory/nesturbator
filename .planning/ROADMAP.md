# Roadmap: nesturbator

## Milestones

- ✅ **v1 NROM games with sound** — Phases 1–4.2 (shipped 2026-10-09) — [archive](milestones/v1-ROADMAP.md)
- ✅ **v2 Most of the library plays** — Phases 5–10 (shipped 2026-10-11) — [archive](milestones/v2-ROADMAP.md)
- ✅ **v3 Save states and rewind** — Phases 11–15 (shipped 2026-10-13) — [archive](milestones/v3-ROADMAP.md)
- ✅ **v4 Measured NTSC accuracy and robustness** — Phases 16–18 (shipped 2026-10-24) — [archive](milestones/v4-ROADMAP.md)
- ✅ **v5 NMI, VBlank and sprite-0 timing** — Phases 19–21 (shipped 2026-10-30; package publication pending) — [archive](milestones/v5-ROADMAP.md)
- **v6 Player-ready NES and reproducible feedback** — Phases 22–23 (approved 2026-10-30; Phase 22 complete)

## v6: Player-ready NES and reproducible feedback

**Status:** Approved execution contract. The owner approved the ten requirements
and two-phase roadmap with “yes i approve” on 2026-10-30. Milestone opening is
complete; Phase 22 discussion and planning are complete with independent plan
verification passed. All four Phase 22 plans are implemented and committed;
review corrections and independent phase verification passed. Phase 23 research
and independent plan verification are complete. Plan 23-01 is implemented and
verified, and plan 23-02 is implemented with focused checks passing; plan 23-03
remains pending.

**Goal:** A player can install an identified core, protect daily progress and
supply a precise scene report that a developer can reproduce with existing facilities.

**Direction:** RM-014 in [PRODUCT-ROADMAP.md](PRODUCT-ROADMAP.md).
**Requirements:** [REQUIREMENTS.md](REQUIREMENTS.md), 10 approved requirements.
**Research:** Reuse [existing discovery](research/milestone-v6-discovery/SYNTHESIS.md)
and the [playtest postmortem](playtesting/POSTMORTEM.md); no fresh generic fan-out.

## Phases

- [x] **Phase 22: Install and protect progress** — Make the existing core straightforward to install and document how to preserve progress, while keeping executable developer contracts intact. (completed 2026-10-30)
- [ ] **Phase 23: Reproduce and report a scene** — Provide a bounded, private-content-safe report/replay route that distinguishes repeatability from correctness.

### Phase 22: Install and protect progress

**Goal:** A player can find accurate installation/control guidance and preserve
progress across an identified core upgrade or rollback.
**Depends on:** v5 complete; no additional owner play or release publication is required.
**Requirements:** READY-01, READY-02, READY-03, READY-04, READY-05, READY-06.

**Success criteria:**
1. The README leads directly to current supported-content limits, installation,
   controls, progress and troubleshooting; detailed API/runner/release material
   is reachable through working topic-guide links, with no contradictory opening.
2. The documented archive selection/install path matches actual packaged targets
   and a permitted no-ROM/test fixture, and explains how to identify the loaded
   core. Published package capabilities and merged-source revisions are distinct.
3. Host-control guidance covers the real frontend launch and separate active
   player ports. It credits existing automated/core and daily-play evidence with
   its limits rather than requiring a new device or commercial play session.
4. A documented scratch-only backup/upgrade/refused-load/rollback walkthrough
   preserves newer battery progress and distinguishes core snapshots, host
   containers, auto-loading and version policy; existing state tests or a targeted
   independent check establish the claimed refusal/unchanged-state behavior.
5. The maintained install/archive, API/status/state, runner-options/stop/exit and
   info/release checks pass after relocation, and their relevant negative
   controls still reject missing or contradictory contracts. No assertion is
   dropped merely to shorten the README.

**Plans:** 4/4 plans complete in 4 sequential waves; independent plan verification passed.

**Wave 1**

- [x] 22-01-PLAN.md — Canonical API guide with relocation and link controls

**Wave 2** *(blocked on Wave 1 completion)*

- [x] 22-02-PLAN.md — Runner help/options/stop/exit contract migration

**Wave 3** *(blocked on Wave 2 completion)*

- [x] 22-03-PLAN.md — Release and engineering/testing guide migration

**Wave 4** *(blocked on Wave 3 completion)*

- [x] 22-04-PLAN.md — Player install, identity, controls and safe progress

[Research](phases/22-install-and-protect-progress/22-RESEARCH.md),
[source audit](phases/22-install-and-protect-progress/22-SOURCE-AUDIT.md) and
[plan verification](phases/22-install-and-protect-progress/22-PLAN-CHECK.md).

### Phase 23: Reproduce and report a scene

**Goal:** A developer can repeat a precisely reported scene without disturbing
daily progress and choose the next evidence source without overclaiming a diagnosis.
**Depends on:** Phase 22 (identity, storage and version policy).
**Requirements:** REPRO-01, REPRO-02, REPRO-03, REPRO-04.

**Success criteria:**
1. A public report template/example identifies scene, access, content/core/host
   identities, inputs, expected/actual behavior, repeatability and untried items,
   with clear directions to keep commercial media and private identifiers local.
2. A documented permitted-fixture recipe reproduces frame/audio output from
   cold boot and a supported checkpoint using existing facilities. Repeated runs
   agree and an independently chosen wrong-input/continuation control is detected;
   existing unique controls are reused rather than reimplemented.
3. Examples state finite frame and outer-time bounds, use isolated scratch
   storage, and clearly refuse or report unsupported identity/container cases.
   Native payloads are not silently treated as RetroArch disk states. Daily
   progress and private artifacts are absent from public verification inputs/output.
4. The triage guide separates native pixels/audio, adapter conversion, host
   presentation and original game behavior, states what an independent hardware
   oracle would need, and records the current commercial reports as unclassified.
   Repeated hashes alone are never labeled a correctness or scene-fix result.

**Plans:** 2/3 plans executed in 3 sequential waves; independent plan verification passed. Use a small owned helper only after confirming the missing facility.

**Wave 1**

- [x] 23-01-PLAN.md — Sanitized report and bounded repeated cold-movie tracer

**Wave 2** *(depends on Wave 1)*

- [x] 23-02-PLAN.md — Neutral native checkpoint, identity preflight and isolated storage

**Wave 3** *(depends on Wave 2)*

- [ ] 23-03-PLAN.md — Layer-specific triage, independent oracle and complete guide verification

[Research](phases/23-reproduce-and-report-a-scene/23-RESEARCH.md) and
[plan verification](phases/23-reproduce-and-report-a-scene/23-PLAN-CHECK.md).

## Acceptance and scope limits

- Library, runner and libretro remain the only deliverables. No new GUI,
  general frontend, shared replay framework or incidental dependency.
- Existing docs/packaging/state/input coverage and 22 title reports are credited.
  No fresh commercial sweep, required daily-device operation or owner sign-off.
  Public examples work with permitted fixtures and available public facilities.
- This milestone does not add overscan, a state converter, core accuracy fixes,
  regions/mappers or actual Playstead integration. Following scene accuracy is
  conditional on a confirmed core fault; otherwise DMA remains next.
- Changes to runtime behavior need owned regressions, same-change README/header
  comments and `cmake --workflow --preset ci`. Documentation-only relocation
  uses relevant document/package checks. Preserve deterministic hashes, protected
  PASS keys, existing no-skip and performance gates.
- Collector mode/lookup debt, platform debt, historical attribution/provenance
  limits and runtime stops remain disclosed. New qualified collection requires
  its recurring prerequisites to be fixed under bounded approved scope first;
  this roadmap grants no new live-testing window or campaign retry.

## Progress

| Phase | Plans Complete | Status | Completed |
|---|---|---|---|
| 22. Install and protect progress | 4/4 | Complete    | 2026-10-30 |
| 23. Reproduce and report a scene | 2/3 | In Progress | — |

**Requirement coverage:** 10/10 mapped exactly once; 0 unmapped.
**Next action:** `$gsd-execute-phase 23` — finish Phase 23 plan 23-03. Context
may be cleared first; read STATE.md. Plans 23-01 and 23-02 are complete; the
phase remains in progress.

## Backlog disposition

2026-10-13: Backlog 999.2, "Bump AccuracyCoin past 673ef550", is promoted into
[Phase 17](milestones/v4-ROADMAP.md#phase-17-complete-pinned-conformance-baseline),
CONF-01 through CONF-04 and RM-001. No duplicate active backlog remains.

SEED-006 is selected only for the demonstrated docs/contract delta; the existing
playability/state/board outcomes and recent CI optimizations remain delivered.
No seed files were modified. The three pending property/mutation/signal proposals
were inspected and have no confident requirement match here; they remain unlinked
and deferred. All future direction remains in PRODUCT-ROADMAP.md.

*Last updated: 2026-10-31 after Phase 23 plan 23-02 execution.*
