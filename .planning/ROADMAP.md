# Roadmap: nesturbator

## Milestones

- ✅ **v1 NROM games with sound** — Phases 1–4.2 (shipped 2026-10-09) — [archive](milestones/v1-ROADMAP.md)
- ✅ **v2 Most of the library plays** — Phases 5–10 (shipped 2026-10-11) — [archive](milestones/v2-ROADMAP.md)
- ✅ **v3 Save states and rewind** — Phases 11–15 (shipped 2026-10-13) — [archive](milestones/v3-ROADMAP.md)
- 🚧 **v4 Measured NTSC accuracy and robustness** — Phases 16–18 (scope approved 2026-10-13; not started)

## Phases

<details>
<summary>✅ v1 NROM games with sound (Phases 1–4.2) — SHIPPED 2026-10-09</summary>

- [x] Phase 1: A test frame in RetroArch (12/12 plans) — completed 2026-10-03
- [x] Phase 2: The CPU matches the public vectors (11/11 plans) — completed 2026-10-06
- [x] Phase 3: A real game in RetroArch (15/15 plans) — completed 2026-10-09
- [x] Phase 4: Sound (4/4 plans) — completed 2026-10-09
- [x] Phase 4.1: Close gap: GAME-01 — initialize accepted iNES trainers (1/1 plan, INSERTED) — completed 2026-10-09
- [x] Phase 4.2: Close gap: SND-01 — deliver single-sample libretro audio (1/1 plan, INSERTED) — completed 2026-10-09

</details>

<details>
<summary>✅ v2 Most of the library plays (Phases 5–10) — SHIPPED 2026-10-11</summary>

- [x] Phase 5: Tune-up and v1 debt (8/8 plans) — completed 2026-10-10
- [x] Phase 6: Mapper seam and PPU fetch pipeline (2/2 plans) — completed 2026-10-10
- [x] Phase 7: UxROM, CNROM and AxROM (4/4 plans) — completed 2026-10-10
- [x] Phase 8: MMC1 and battery saves (8/8 plans) — completed 2026-10-10
- [x] Phase 9: MMC3 (12/12 plans) — completed 2026-10-10
- [x] Phase 10: Close-out and boot-to-play (7/7 plans) — completed 2026-10-11

</details>

<details>
<summary>✅ v3 Save states and rewind (Phases 11–15) — SHIPPED 2026-10-13</summary>

- [x] Phase 11: Tune-up and state readiness (4/4 plans) — completed 2026-10-11
- [x] Phase 12: Save states in the library (10/10 plans) — completed 2026-10-12
- [x] Phase 13: Rewind and run-ahead in RetroArch (8/8 plans) — completed 2026-10-12
- [x] Phase 14: Runner process contract (8/8 plans) — completed 2026-10-12
- [x] Phase 15: Speed and close-out (8/8 plans) — completed 2026-10-13

</details>

### v4 — Measured NTSC accuracy and robustness

Establish trustworthy current-host evidence, a complete pinned NTSC conformance inventory and bounded execution robustness evidence. These measurements inform the next accuracy or regional milestone; they do not promise an all-pass score. Planning milestone v4 is distinct from package release v0.1.10. Future direction and promotion triggers remain in [PRODUCT-ROADMAP.md](PRODUCT-ROADMAP.md).

- [ ] **Phase 16: Host evidence and roadmap** — Current RetroArch checks reject known wrong behavior, and milestone direction has a recurring operating contract.
- [ ] **Phase 17: Complete pinned conformance baseline** — Every AccuracyCoin entry is inventoried and each scored test has an honest bounded outcome.
- [ ] **Phase 18: Execution and lifecycle robustness** — Accepted generated programs exercise all supported board families with bounded replay, sanitizer evidence and explicit stop behavior.

## Phase Details

Every phase ships a tested result through the existing library, runner or adapter delivery path. Behavior changes carry tests in `cmake --workflow --preset ci`, README and public-header documentation, relevant deterministic/state/parity evidence and the existing CI, hygiene and perf gates. No new dependency is approved. PAL/Dendy, new boards, expansion audio, FDS, broader real-host OS coverage and unnamed accuracy repairs remain outside this milestone.

### Phase 16: Host evidence and roadmap

**Goal**: Host builders can trust the current RetroArch state/progression evidence and released MMC3 description, while milestone starts and closeouts preserve evidence-backed product direction.
**Depends on**: Phase 15 (shipped v3 core, state, host and CI contracts)
**Requirements**: EVID-01, EVID-02, EVID-03, EVID-04, DIR-01, DIR-02
**Success Criteria** (what must be TRUE):
  1. The real-host progression check passes the intended state sequence and rejects dropped, doubled and wrong-load sequences; negative controls prove that identical screenshots alone cannot establish exact progress, using distinguishable frames or absolute state anchors.
  2. Network checks record bounded, attributable completion for safely retried requests and fail or restart when a non-idempotent action is unconfirmed; automated controls reject blind FRAMEADVANCE or PAUSE_TOGGLE retries.
  3. Exact content identity/basename selects each fixture, and automated adversarial parent-directory cases cannot redirect that selection.
  4. Released libretro metadata names the implemented MMC3 support truthfully and agrees with the documented supported profiles under an automated consistency check.
  5. The linked product roadmap exposes all four horizons, stable outcome IDs, evidence and promotion triggers; repository instructions and preparation handoff require reconciliation at both milestone boundaries within the invoked step, retaining delivered seed evidence and unresolved debt. The opening record demonstrates the contract, and existing commands or one-time link/structure checks verify its required artifacts without claiming to judge priority choices. A new recurring roadmap validator is optional if upkeep is repeatedly missed.

**Plans**: 8 plans; 6 executed, 2 gap-closure plans pending

Plans:
**Wave 1**
- [x] 16-01-PLAN.md — Qualify the generated marker and native save origins on the pinned host.
- [x] 16-06-PLAN.md — Verify installed rolling direction and actual v4 opening evidence once.

**Wave 2** *(blocked on Wave 1 completion)*
- [x] 16-02-PLAN.md — Share an exact progression verdict with independent semantic controls.

**Wave 3** *(blocked on Wave 2 completion)*
- [x] 16-03-PLAN.md — Bound one-shot commands, attribute artifacts and require real-host progression.

**Wave 4** *(blocked on Wave 3 completion)*
- [x] 16-04-PLAN.md — Select exact fixture names and authenticate actual supplied bytes.

**Wave 5** *(blocked on Wave 4 completion)*
- [x] 16-05-PLAN.md — Protect truthful MMC3 descriptions through staged and packaged metadata.

**Wave 6** *(gap closure; depends on completed Plans 05/06)*
- [ ] 16-07-PLAN.md — Prepare the checked phase PR, obtain exact-head merge consent and observe the normal release flow.

**Wave 7** *(depends on the actual normal publication from Plan 07)*
- [ ] 16-08-PLAN.md — Verify downloaded six-platform metadata, checksums, attestations and exact-release-commit CI.

**Verification (2026-10-15):** 6/8 plans implemented; [required CI](https://github.com/szTheory/nesturbator/actions/runs/38676328559) passed on source `841434b`, and source review is clean. [Phase verification](phases/16-host-evidence-and-roadmap/16-VERIFICATION.md) verifies 21/22 truths. EVID-04 is the sole gap: actual published v0.1.10 metadata omits MMC3, while current source, staging and packages pass. Gap Plans 07/08 explicitly prepare delivery, gate the concrete phase PR merge on owner consent and collect actual published-byte evidence through the existing release pipeline. Phase 16 remains pending until that evidence passes independent verification. Next: `$gsd-execute-phase 16 --gaps-only`.

**UI hint**: no
**Canonical refs**: [LIBRETRO-AND-RUNNER.md](preparation/LIBRETRO-AND-RUNNER.md), [ENGINEERING.md](preparation/ENGINEERING.md), [GSD-HANDOFF.md](preparation/GSD-HANDOFF.md); current scope evidence in [research/SUMMARY.md](research/SUMMARY.md).

### Phase 17: Complete pinned conformance baseline

**Goal**: Users can reproduce a complete, immutable AccuracyCoin inventory and read an honest bounded NTSC baseline without losing previously protected passing results.
**Depends on**: Phase 16 (delivery order and trustworthy host evidence); inventory execution uses the existing runner and core.
**Requirements**: CONF-01, CONF-02, CONF-03, CONF-04
**Success Criteria** (what must be TRUE):
  1. Automated checks validate the immutable pin `6bee919529289c555803b612b8c9e4f7eaaeeabb`, licence/hash, compressed-name decoding and protected-key migration together; the retained comparison identifies the old pin `673ef550db296136d52229961e7d39366116882a` and explains changed meanings.
  2. The runner inventories all 146 scored tests and five separate DRAW entries at that pin; missing/duplicate entries and malformed result tables fail automated validation, and DRAW never enters the score denominator.
  3. A bounded automated run retains raw results and diagnostics and classifies every scored test as pass, fail, skip, not-run or timeout; not-run/timeout records explain their causes and never imply execution or a passing score.
  4. All 17 previously protected passing keys remain mapped and accounted for. Automated gates reject same-pin pass regressions; upstream meaning or denominator changes have an explicit reviewed disposition and preserved comparison evidence.

**Plans**: TBD
**UI hint**: no
**Canonical refs**: [CONFORMANCE.md](preparation/CONFORMANCE.md), [LIBRETRO-AND-RUNNER.md](preparation/LIBRETRO-AND-RUNNER.md), [ENGINEERING.md](preparation/ENGINEERING.md); pin/encoding updates in [research/v4-scope/accuracy.md](research/v4-scope/accuracy.md) supersede the older preparation pin for this phase.

### Phase 18: Execution and lifecycle robustness

**Goal**: Host builders have reproducible, bounded evidence that accepted programs execute across all six supported board families and preserve the documented lifecycle, deterministic continuation and halt contracts.
**Depends on**: Phase 16 and the shipped core/state/host contracts. Phase 17 comes first by priority, but a passing conformance score is not a technical prerequisite.
**Requirements**: ROB-01, ROB-02, ROB-03, ROB-04
**Success Criteria** (what must be TRUE):
  1. A short measured prototype demonstrates useful additional program-execution reach beyond existing loader/state fuzzing. The owned fuzz/replay target runs bounded accepted profiles with generated program/CHR bytes and controlled input on mappers 0, 1, 2, 3, 4 and 7; measurements show every family reaches execution.
  2. Automated replay exercises reset, save/load and unload/reload, rejects cross-instance interference and compares deterministic continuations against the existing API contracts.
  3. Normal CI replays a fixed permitted/generated corpus, while finite nightly sanitizer campaigns retain deterministic crash/timeout reproducers under measured time, memory and execution budgets selected from throughput/coverage evidence. Findings are minimized; a zero-finding campaign is reported only as finite evidence.
  4. Automated guest-loop and JAM cases demonstrate the documented progress/stop outcomes, including the adapter's observable halt policy. Every reproduced library/host contract violation has a minimized regression and correction or an explicit scope amendment; unexpected major repair/architecture scope is never silently absorbed or waived.

**Plans**: TBD
**UI hint**: no
**Canonical refs**: [ARCHITECTURE.md](preparation/ARCHITECTURE.md), [ENGINEERING.md](preparation/ENGINEERING.md), [CONFORMANCE.md](preparation/CONFORMANCE.md), [LIBRETRO-AND-RUNNER.md](preparation/LIBRETRO-AND-RUNNER.md); current reach and discovery limits in [research/v4-scope/reliability.md](research/v4-scope/reliability.md).

Closeout stays within Phase 18 and the normal milestone lifecycle: retain measured results, unresolved debt and actual release evidence, then refresh product direction under DIR-02. No additional validation-only phase or all-pass accuracy obligation is introduced. If the execution prototype adds no useful reach, record an explicit scope amendment rather than claiming ROB-01 complete.

## Progress

**Execution order**: 16 → 17 → 18. Priority order does not turn an all-pass conformance score into a hardware dependency.

| Phase | Milestone | Plans Complete | Status | Completed |
|-------|-----------|----------------|--------|-----------|
| 16. Host evidence and roadmap | v4 | 6/8 | Gap closure planned | - |
| 17. Complete pinned conformance baseline | v4 | 0/TBD | Not started | - |
| 18. Execution and lifecycle robustness | v4 | 0/TBD | Not started | - |

## Backlog disposition

2026-10-13: Backlog 999.2, "Bump AccuracyCoin past 673ef550", is promoted into [Phase 17](#phase-17-complete-pinned-conformance-baseline), CONF-01 through CONF-04 and [RM-001](PRODUCT-ROADMAP.md#rm-001-complete-ntsc-conformance-evidence). Its pin/scoreboard/hash comparison is included there; no duplicate active backlog remains. Other future outcomes remain proposed in PRODUCT-ROADMAP.md.
