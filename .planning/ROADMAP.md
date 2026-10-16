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

**Plans**: 7/11 live plans completed; Plans 01–06 and 09 complete. Plan13 retains partial recorder repair but is incomplete after an unusable single ASan observation; Plan14 current-source acceptance/review and delivery Plans07/08 remain blocked. There are 14 physical plans; Plans 10/11/12 are superseded/incomplete without SUMMARY or completion credit.

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
- [x] 16-09-PLAN.md — Measure and repair journal overhead while preserving readiness timing, semantic and cleanup bounds; run focused/full CI and fresh source review.

**Wave 7** *(gap closure; Plan13 stopped incomplete with partial commits and zero observation allowance)*
- [ ] 16-13-PLAN.md — Trace cleanup RED/GREEN and seven capture/36 trace cases retained; the single ASan observation is unusable because the harness mixes session and capture invocation IDs. No SUMMARY/credit; unchanged resume blocked; bounded amendment required.

**Wave 8** *(blocked on accepted Plan 13 and required gates)*
- [ ] 16-14-PLAN.md — Require current focused/full ci/asan, normal ci/all tests/three packages, independent exact-source clean review and required later gates; any original assertion correction requires compatible current cause and reason-specific RED first.

Superseded history: [16-10-PLAN.md](phases/16-host-evidence-and-roadmap/16-10-PLAN.md) preserves partial timing work/revoked completion and points to 11; [16-11-PLAN.md](phases/16-host-evidence-and-roadmap/16-11-PLAN.md) preserves the consumed passing CI observation and points to 12; [16-12-PLAN.md](phases/16-host-evidence-and-roadmap/16-12-PLAN.md) preserves recorder source/tests and unusable ASan evidence, and points to substantive 13 with remaining current acceptance assigned to 14. All three have status superseded, no success SUMMARY and no credit; installed scanning excludes them.

**Wave 9** *(blocked on genuine Plan 14 acceptance and every required later gate; Plan 13 recorder success alone cannot unlock delivery)*
- [ ] 16-07-PLAN.md — Prepare the checked phase PR, require fresh exact-head native PR CI, obtain exact-head owner merge consent and observe the normal release flow.

**Wave 10** *(depends on the actual normal publication from Plan 07)*
- [ ] 16-08-PLAN.md — Verify downloaded six-platform metadata, all 18 archives/checksums/attestations and exact-release-commit CI.

**Historical Plan 11 planning (2026-10-15):** [Plan 11](phases/16-host-evidence-and-roadmap/16-11-PLAN.md) passed independent review with zero blockers/warnings; all 6 requirements and 17 decisions remain covered. Its 3 tasks form one wave-7 continuation. At planning, the installed index excluded superseded 10, reported 10 live plans with 7 complete, selected only 11 as ready and kept 07/08 blocked; Plan 11's evaluation-scope query resolved with no matching commits. Supersession preserved partial source commits and honest incomplete status without fabricating a summary or replaying completed timing work.

**Historical Plan 11 execution stop (2026-10-15):** [Plan 11's diagnosis record](phases/16-host-evidence-and-roadmap/16-11-INTERRUPTION-REPAIR.md) retains one bounded CI observation that passed in 4.18 seconds without reproducing or explaining the missing-reason assertion. Temporary instrumentation was reverted. No source correction, reason-specific RED/GREEN, ASan, full workflow, review or success summary is claimed. At that historical stop the one-ASan diagnostic allowance was unused, pending a distinct source-bound hypothesis and bounded amendment. The index then marked 11 DAG-ready, but its checkpoint commit without a summary required safe-resume reconciliation; Plan 12 subsequently superseded 11. Plans 07/08 and EVID-04 remained blocked at that stop; Plan 12 below was the next continuation, now superseded by 13/14.

**Scoped debug result (2026-10-15):** [The diagnostic investigation](debug/phase16-interruption.md) could not recover the original assertion-time file/grep/process facts. Two standalone shell checks did not reproduce early wait; no new real-driver, CI or ASan observation ran. The concrete next planning scope is durable assertion-boundary capture with deterministic status 0/1/2, pathname-replacement and event-ownership self-tests, preserving the original assertion and budgets. Cause-specific evidence is required before repair; unavailable historical attribution is not required before planning diagnostic coverage. A substantive capture-first successor must reconcile the installed resume gate, preserve incomplete 11 and carry all repair/acceptance obligations forward before rebinding 07. That debug step created no successor or completion credit; the subsequent Plan 12 below carries its capture-first recommendation.

**Retained failed evidence:** [Plan 10 evidence](phases/16-host-evidence-and-roadmap/16-NATIVE-TIMING-REPAIR.md) preserves the timing correction, historical passing controls/packages and clean review at `1b1589a`, followed by the required normal workflow failure at `5f899ec`: 648/649, `retroarch.session.controls` at 230.07 seconds, `FAIL interruption first failure was not retained`, packaging not reached. Later journal rows and an isolated replay do not establish cause. Plans 11/12 stopped incomplete; active 13/14 now preserve their partial work and remaining acceptance obligations. Earlier native [run 38764197994](https://github.com/szTheory/nesturbator/actions/runs/38764197994) at [PR #50](https://github.com/szTheory/nesturbator/pull/50) head `3de23d4` failed Intel stall at 25,946 ms and ASan delay at 26,269 ms, both against strict less-than-25,000 ms; native causes remain unknown. Only genuine Plan 14 current-source acceptance and required later gates permit 07's fresh native CI before exact-head owner consent and normal publication; 08 verifies actual released bytes. The [historical verifier](phases/16-host-evidence-and-roadmap/16-VERIFICATION.md) remains 21/22 with EVID-04 open. No phase completion, merge consent, publication or Phase 17 advancement is claimed.

**Historical capture-first gap planning (2026-10-15):** [Plan 12](phases/16-host-evidence-and-roadmap/16-12-PLAN.md) adds substantive shared-path/single-grep capture with deterministic status 0/1/2, exact snapshot/metadata, pathname-replacement, recorder-error and event-ownership tests before any driver observation. It supersedes incomplete 11 while preserving all repair/acceptance obligations and the consumed 4.18-second CI pass. At planning exactly one ASan observation remained, outer 60 seconds and attempt/cleanup 30000/1000 ms, asking no-match versus file identity versus grep execution condition without assuming a race. A pass or unusable recording retains coverage and stops incomplete without SUMMARY; only a demonstrated compatible cause, reason-specific regression/correction and every current CI/package/review/post-wave gate can unlock 07. Historical cause stays unproven. Installed index and successor evaluation-scope must both be rechecked after state/planning reconciliation and before dispatch; DAG-ready alone is insufficient. Planning commits use docs(16) or unscoped docs to avoid matching successor execution history. No completion, consent, native acceptance or publication is claimed. Independent checking passed with zero blockers/warnings after one targeted Plan 08 audit-prose revision; requirement coverage is 6/6 and decision coverage 17/17. That planning step routed to gaps-only execution; its initial empty-history proof is superseded by the partial execution below.

**Historical Plan 12 execution stop (2026-10-15):** [The retained repair record](phases/16-host-evidence-and-roadmap/16-12-INTERRUPTION-REPAIR.md) records RED `f61a29f`, recorder/docs source `4bfa3a6`, and one ASan observation exiting 1 in 6.26 seconds after seven synthetic tests and all 44 progression controls. The original interruption grep returned 0 with 242 captured stdout bytes; TERM send 0, wait130, one QUIT and lock release were recorded. Driver trace field8 host_pid becomes `unavailable` after cleanup, but its validator only accepts numeric values there; field9 host_alive already accepts unavailable. This concrete instrumentation mismatch makes the recording unusable and does not diagnose the historical assertion failure. Both CI and ASan allowances are consumed, zero remaining. No compatible cause-specific correction, Task3 CI/packages/review, post-wave acceptance or SUMMARY exists; 07/08 and EVID-04 remain blocked. Installed scope now resolves five Plan12 commits without SUMMARY, so unchanged execution is blocked despite DAG readiness. That stop routed to the bounded planning amendment below. No automatic retry, old allowance reset, completion credit or release action was authorized.

**Historical recorder/current-acceptance gap amendment (2026-10-15):** [Plan 13](phases/16-host-evidence-and-roadmap/16-13-PLAN.md) inherits recorder source `4bfa3a6` and its seven cases, adds deterministic actual writer/validator cleanup RED/GREEN and strict field/order/time/role/invocation checks, then permits one separately justified NEW ASan selector observation (outer 60 seconds, unchanged 30000/1000 budgets) for corrected recorder call-site compatibility. Both old CI/ASan observations remain exhausted; consume the new allowance before launch and stop incomplete on failure/unusable evidence. Recorder completion is a narrow claim and cannot unlock 07. [Plan 14](phases/16-host-evidence-and-roadmap/16-14-PLAN.md) carries current controls, normal CI/packages, fresh independent review and mandatory later gates. It explicitly replaces the diagnostic pass-stop rule with finite current acceptance after a demonstrated recorder correction: historical assertion cause remains unknown, passing gates claim no original assertion repair, and any current correction still requires compatible cause/reason-specific RED first. No semantic/timing/cleanup gate is waived. New scope is substantive; do not fabricate a SUMMARY or bypass safe resume. Recheck both new empty evaluation scopes and installed DAG after planning commit and immediately before dispatch; only 13 is initially ready. Counts are 14 physical, 11 live, 7 complete; no implementation/acceptance/native/release outcome is claimed by planning. Independent plan checking passed with zero blockers/warnings;6/6 requirements,17/17 decisions and all11 live plans/25 tasks pass structure checks. Both command probes contain80 entries with zero findings; path not_applicable is not runtime proof. At that planning handoff the next command was `$gsd-execute-phase 16 --gaps-only`; its partial execution and new stop follow.


**Current Plan13 execution stop (2026-10-15):** [The recorder repair record](phases/16-host-evidence-and-roadmap/16-13-RECORDER-REPAIR.md) preserves extraction69d396d, cleanup-PID RED4c7d814, trace GREEN/docs6454457, prelaunch consumption3dafb18 and unusable outcome1a481a6. Seven capture and36 trace cases pass, including a fresh parent offline rerun. The one new ASan selector exited1 in6555ms: the actual3-row driver trace passed, but the21-row harness ledger attributes13 rows to the owned session and8 to constant `capture`. The actual capture helper resets its invocation ID from the directory basename, producing `harness-event-order-or-identity-invalid`. Original assertion0/stdout242bytes/stderr0, TERM0, wait130, oneQUIT, absent lock and equal snapshots do not diagnose the historical assertion failure. All observation allowances are exhausted/zero. No further source fix, real observation, full workflow/packages/review/post-wave acceptance, Plan14 dispatch or SUMMARY occurred. Installed scope13 has five matching partial commits without SUMMARY, so safe resume blocks unchanged execution despite DAG readiness;14/07/08 remain not ready. Counts stay14physical/11live/7complete. Next: `$gsd-plan-phase 16 --gaps` for substantive shared harness identity repair and deterministic actual capture-boundary coverage, retaining every unfinished acceptance gate; any further allowance requires explicit bounded amendment. No consent, publication, phase completion or Phase17 advancement.

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
| 16. Host evidence and roadmap | v4 | 7/11 | Plans10/11/12 superseded;13 incomplete/unusable observation; amendment required;14/07/08 blocked | - |
| 17. Complete pinned conformance baseline | v4 | 0/TBD | Not started | - |
| 18. Execution and lifecycle robustness | v4 | 0/TBD | Not started | - |

## Backlog disposition

2026-10-13: Backlog 999.2, "Bump AccuracyCoin past 673ef550", is promoted into [Phase 17](#phase-17-complete-pinned-conformance-baseline), CONF-01 through CONF-04 and [RM-001](PRODUCT-ROADMAP.md#rm-001-complete-ntsc-conformance-evidence). Its pin/scoreboard/hash comparison is included there; no duplicate active backlog remains. Other future outcomes remain proposed in PRODUCT-ROADMAP.md.
