# Roadmap: nesturbator

## Milestones

- ✅ **v1 NROM games with sound** — Phases 1–4.2 (shipped 2026-10-09) — [archive](milestones/v1-ROADMAP.md)
- ✅ **v2 Most of the library plays** — Phases 5–10 (shipped 2026-10-11) — [archive](milestones/v2-ROADMAP.md)
- ✅ **v3 Save states and rewind** — Phases 11–15 (shipped 2026-10-13) — [archive](milestones/v3-ROADMAP.md)
- 🚧 **v4 Measured NTSC accuracy and robustness** — Phases 16–18 (scope approved 2026-10-13; Phase 16 complete)

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

- [x] **Phase 16: Host evidence and roadmap** — Current RetroArch checks reject known wrong behavior, and milestone direction has a recurring operating contract. (completed 2026-10-23)
- [x] **Phase 17: Complete pinned conformance baseline** — Every AccuracyCoin entry is inventoried and each scored test has an honest bounded outcome. (completed 2026-10-24)
- [ ] **Phase 18: Execution and lifecycle robustness** — Accepted generated programs exercise all supported board families with bounded replay, sanitizer evidence and explicit stop behavior.

## Phase Details

Every phase ships a tested result through the existing library, runner or adapter delivery path. Behavior changes carry tests in `cmake --workflow --preset ci`, README and public-header documentation, relevant deterministic/state/parity evidence and the existing CI, hygiene and perf gates. No new dependency is approved. PAL/Dendy, new boards, expansion audio, FDS, broader real-host OS coverage and unnamed accuracy repairs remain outside this milestone.

### Phase 16: Host evidence and roadmap

**Goal**: Host builders can trust the current RetroArch state/progression evidence and released MMC3 description, while milestone starts and closeouts preserve evidence-backed product direction.
**Depends on**: Phase15 and the existing delivered core/libretro/runner.
**Requirements**: EVID-01, EVID-02, EVID-03, EVID-04, DIR-01, DIR-02
**Success Criteria** (what must be TRUE):
  1. The real-host progression check passes the intended state sequence and rejects dropped, doubled and wrong-load sequences; negative controls prove that identical screenshots alone cannot establish exact progress, using distinguishable frames or absolute state anchors.
  2. Network checks record bounded, attributable completion for safely retried requests and fail or restart when a non-idempotent action is unconfirmed; automated controls reject blind FRAMEADVANCE or PAUSE_TOGGLE retries.
  3. Exact content identity/basename selects each fixture, and automated adversarial parent-directory cases cannot redirect that selection.
  4. Released libretro metadata names the implemented MMC3 support truthfully and agrees with the documented supported profiles under an automated consistency check.
  5. The linked product roadmap exposes all four horizons, stable outcome IDs, evidence and promotion triggers; repository instructions and preparation handoff require reconciliation at both milestone boundaries within the invoked step, retaining delivered seed evidence and unresolved debt. The opening record demonstrates the contract, and existing commands or one-time link/structure checks verify its required artifacts without claiming to judge priority choices. A new recurring roadmap validator is optional if upkeep is repeatedly missed.

**Plans:** 13 live (56 physical; 12 complete; Plan 16-08 outstanding). Current process authority:
- [x] 16-01-PLAN.md
- [x] 16-02-PLAN.md
- [x] 16-03-PLAN.md
- [x] 16-04-PLAN.md
- [x] 16-05-PLAN.md
- [x] 16-06-PLAN.md
- [x] 16-09-PLAN.md
- [x] 16-15-PLAN.md
- [x] 16-21-PLAN.md
- [x] 16-30-PLAN.md

[16-RECOVERY.md](phases/16-host-evidence-and-roadmap/16-RECOVERY.md).

**Recovery:** The user requested an end to the repeated plan/execute loop.
The forensic report found62consecutive docs commits and expanding evidence-
transport prerequisites. Those discretionary mechanics are retired explicitly;
all six product requirements, D-01–D-17and actual test/review/release gates stay.
Old56and the full prior roadmap remain in git at227bd6a; all old checkpoints/
build evidence stay untouched.51–54are superseded incomplete, not successes.
Recovery alone inferred no acceptance. Plan56 completed actual current-source
and parent acceptance. Plan07 has now completed exact-head native acceptance,
owner consent and normal v0.1.11 publication; Plan08's downloaded-byte checks
remain pending.

Completed:01–06(host/fixture/metadata/direction),09(timing),15(identity),
21(cleanup-write repair),30(historical helper preparation),56(current source
acceptance),07(release publication). Source candidate2c356d5 has clean
independent review, final-source CR-01 RED/GREEN, focused CI/ASan and
parent649/649/no-skip/four-stage CI plus three fresh packages. Historical
phase verification remains21/22/gaps_found; Plan08 now supplies released-byte
evidence for the independent phase verification.

**Wave 10** — direct current-source acceptance after completed30
- [x] 16-56-PLAN.md — Fix harness source identity/finalizers; prove current
  lock-race correction; run current649/jobs4CI, focusedASan, packages, paired
  source/tests/docs commit, independent review and ordinary parent checks.

**Wave 11** — complete after genuine56source acceptance and configured parent checks
- [x] 16-07-PLAN.md — Fresh exact-head native required/perf/pinned-host CI,
  affirmative exactPR/head merge consent and normal protected release flow.

**Wave 12** — complete after actual normal07publication
- [x] 16-08-PLAN.md — Verify all18downloaded archives, checksums, attestations,
  sixmetadata copies and successful exact-release-commitCI.

Plan16-07 is complete and its actual v0.1.11 publication is observed. Plan16-08
is complete with downloaded-byte, checksum, attestation and six-platform
metadata evidence. Independent phase-level verification passed 22/22 on
2026-10-23; all six requirements and Phase 16 are complete. Final review found
one missing CI registration, corrected and verified within this execution:
650/650 CI tests with packaging, focused ASan 1/1, clean follow-up review.
No new dependency, global workflow/config change, extra host platform or
milestone upkeep was introduced. Phase 17 is next.

### Phase 17: Complete pinned conformance baseline

**Goal**: Users can reproduce a complete, immutable AccuracyCoin inventory and read an honest bounded NTSC baseline without losing previously protected passing results.
**Depends on**: Phase 16 (delivery order and trustworthy host evidence); inventory execution uses the existing runner and core.
**Requirements**: CONF-01, CONF-02, CONF-03, CONF-04
**Success Criteria** (what must be TRUE):
  1. Automated checks validate the immutable pin `6bee919529289c555803b612b8c9e4f7eaaeeabb`, licence/hash, compressed-name decoding and protected-key migration together; the retained comparison identifies the old pin `673ef550db296136d52229961e7d39366116882a` and explains changed meanings.
  2. The runner inventories all 146 scored tests and five separate DRAW entries at that pin; missing/duplicate entries and malformed result tables fail automated validation, and DRAW never enters the score denominator.
  3. A bounded automated run retains raw results and diagnostics and classifies every scored test as pass, fail, skip, not-run or timeout; not-run/timeout records explain their causes and never imply execution or a passing score.
  4. All 17 previously protected passing keys remain mapped and accounted for. Automated gates reject same-pin pass regressions; upstream meaning or denominator changes have an explicit reviewed disposition and preserved comparison evidence.

**Plans**: 4/4 plans complete

Plans:
**Wave 1**
- [x] 17-01-PLAN.md — Token-bearing end-to-end tracer, complete inventory and retained old comparison

**Wave 2** *(blocked on Wave 1 completion)*
- [x] 17-02-PLAN.md — Independent bounded page campaign, honest reports and actual measured full observation

**Wave 3** *(blocked on Wave 2 completion)*
- [x] 17-03-PLAN.md — Protected-main authority, actual protected measurements and source-backed migration enforcement

**Wave 4** *(blocked on Wave 3 completion)*
- [x] 17-04-PLAN.md — Atomic canonical pin activation, durable comparison and measured CI placement

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

**Plans**: 2/4 plans executed

Plans:
**Wave 1**
- [x] 18-01-PLAN.md — Owned execution tracer and matched initial reach measurement.

**Wave 2** *(blocked on Wave 1 completion)*
- [x] 18-02-PLAN.md — Six-family bank/execution qualification and discriminating lifecycle controls.

**Wave 3** *(blocked on Wave 2 completion)*
- [ ] 18-03-PLAN.md — Native/runner loop and JAM contracts plus adapter presentation policy.

**Wave 4** *(blocked on Wave 3 completion)*
- [ ] 18-04-PLAN.md — Measured portable replay, finite sanitizer campaigns and scoped findings evidence.

**UI hint**: no
**Canonical refs**: [ARCHITECTURE.md](preparation/ARCHITECTURE.md), [ENGINEERING.md](preparation/ENGINEERING.md), [CONFORMANCE.md](preparation/CONFORMANCE.md), [LIBRETRO-AND-RUNNER.md](preparation/LIBRETRO-AND-RUNNER.md); current reach and discovery limits in [research/v4-scope/reliability.md](research/v4-scope/reliability.md).

Closeout stays within Phase 18 and the normal milestone lifecycle: retain measured results, unresolved debt and actual release evidence, then refresh product direction under DIR-02. No additional validation-only phase or all-pass accuracy obligation is introduced. If the execution prototype adds no useful reach, record an explicit scope amendment rather than claiming ROB-01 complete.

## Progress

**Execution order**: 16 → 17 → 18. Priority order does not turn an all-pass conformance score into a hardware dependency.

| Phase | Milestone | Plans Complete | Status | Completed |
|-------|-----------|----------------|--------|-----------|
| 16. Host evidence and roadmap | v4 | 13/13 | Complete    | 2026-10-23 |
| 17. Complete pinned conformance baseline | v4 | 4/4 | Complete    | 2026-10-24 |
| 18. Execution and lifecycle robustness | v4 | 2/4 | In Progress | - |

## Backlog disposition

2026-10-13: Backlog 999.2, "Bump AccuracyCoin past 673ef550", is promoted into [Phase 17](#phase-17-complete-pinned-conformance-baseline), CONF-01 through CONF-04 and [RM-001](PRODUCT-ROADMAP.md#rm-001-complete-ntsc-conformance-evidence). Its pin/scoreboard/hash comparison is included there; no duplicate active backlog remains. Other future outcomes remain proposed in PRODUCT-ROADMAP.md.
