# Roadmap: nesturbator

## Milestones

- ✅ **v1 NROM games with sound** — Phases 1–4.2 (shipped 2026-10-09) — [archive](v1-ROADMAP.md)
- ✅ **v2 Most of the library plays** — Phases 5–10 (shipped 2026-10-11) — [archive](v2-ROADMAP.md)
- ✅ **v3 Save states and rewind** — Phases 11–15 (shipped 2026-10-13) — [archive](v3-ROADMAP.md)
- ✅ **v4 Measured NTSC accuracy and robustness** — Phases 16–18 (shipped 2026-10-24) — [archive](v4-ROADMAP.md)
- ✅ **v5 NMI, VBlank and sprite-0 timing** — Phases 19–21 (shipped 2026-10-30; package publication later completed) — [archive](v5-ROADMAP.md)
- ✅ **v6 Player-ready NES and reproducible feedback** — Phases 22–23 (planning milestone shipped 2026-10-31; no package release) — [archive](v6-ROADMAP.md)
- ✅ **v7 Remaining DMA accuracy failures** — Phase 24 (planning milestone shipped 2026-11-01; no package release) — [archive](v7-ROADMAP.md)
- ✅ **v8 PAL region support** — Phases 25–27.1 (planning milestone shipped 2026-11-03; no package release or tag)

The v5/v6 closeout entries retain their original publication observations.
Release PR #56 later published v0.1.12; package publication remains distinct
from planning milestone completion. Current package evidence is summarized in
[PROJECT.md](../PROJECT.md) and [v8 product-roadmap snapshot](v8-PRODUCT-ROADMAP.md).

## Backlog disposition

2026-10-13: Backlog 999.2, “Bump AccuracyCoin past 673ef550,” was promoted into
[Phase 17](v4-ROADMAP.md#phase-17-complete-pinned-conformance-baseline)
and CONF-01 through CONF-04. No duplicate active backlog remains.

## v8 — PAL region support

PAL content runs at intended timing and pitch through the existing library,
headless runner and libretro adapter. The seven approved requirements cover
region selection, all six existing mapper families, PAL timing and DMA,
modeled display colors, host AV reporting, region-specific state identity and
NTSC preservation. The approved roadmap began with three phases at the
configured coarse granularity. Phase 27.1 was inserted after the v8 audit
found two STATE-01 validation gaps. Phases 25–27.1 and their goal-backward
verification are complete. The [milestone audit](v8-MILESTONE-AUDIT.md) has
no blocking gaps and records non-blocking evidence debt. The verified
[closeout](v8-CLOSEOUT.md) preserves the remaining limits and next-step
direction. Phase candidate receipts and verification remain in the archived
phase artifacts.

## Phases

- [x] **Phase 25: Region selection and PAL timing** - Resolve one region and run the existing boards with PAL CPU, PPU and APU cadence. (completed 2026-11-01)
- [x] **Phase 26: PAL DMA and regional state continuation** - Apply the bounded 2A07 DMA model and make region part of safe state continuation. (completed 2026-11-02)
- [x] **Phase 27: PAL host presentation and NTSC preservation** - Expose matching AV behavior and modeled colors across the deliverables, with unchanged NTSC output. (completed 2026-11-02)
- [x] **Phase 27.1: Close STATE-01 regional state validation gaps** - Validate the PAL five-step endpoint and regional frame counters before applying state. (completed 2026-11-02)

## Phase Details

### Phase 25: Region selection and PAL timing

**Goal**: A library user can select supported content under the agreed region policy and run PAL content at its intended frame timing and audio pitch on every existing mapper family.
**Depends on**: Phase 24 (completed v7 baseline)
**Requirements**: REG-01, PAL-01
**Success Criteria** (what must be TRUE):
  1. A host loading explicit single-region NES 2.0 content gets its declared NTSC or PAL profile; a contradictory host preference cannot change that declaration.
  2. A host loading multi-region or legacy iNES content gets NTSC by default and can request PAL; Dendy content is refused.
  3. Owned or permitted PAL content on mapper IDs 0, 1, 2, 3, 4 and 7 produces independently checked PAL frame timing and audio pitch, with the matching CPU/PPU/APU cadence and PAL frequency tables.
  4. A caller advancing many PAL frames observes deterministic frame and sample accounting at the exact PAL cadence, without drift from rounding the frame rate to 50 Hz.

**Plans**: 5/5 complete in 5 waves; the final candidate passed full local CI and packaging, and the phase verifier passed 23/23 truths
**Wave 1**
- [x] 25-01-PLAN.md — Add shared load-time region resolution and record the pinned PAL raster/APU evidence

**Wave 2** *(blocked on Wave 1 completion)*
- [x] 25-02-PLAN.md — Add PAL bus timing and exact frame/sample accounting

**Wave 3** *(blocked on Wave 2 completion)*
- [x] 25-03-PLAN.md — Add PAL raster, APU cadence, frequency tables and deterministic audio

**Wave 4** *(blocked on Wave 3 completion)*
- [x] 25-04-PLAN.md — Pass region preferences through the runner and libretro adapter

**Wave 5** *(blocked on Wave 4 completion)*
- [x] 25-05-PLAN.md — Verify six-mapper PAL behavior, option metadata and bounded acceptance evidence

### Phase 26: PAL DMA and regional state continuation

**Goal**: A user can run PAL DMC DMA under the documented 2A07 model and resume a machine state only under its matching resolved region.
**Depends on**: Phase 25
**Requirements**: DMA-01, STATE-01
**Success Criteria** (what must be TRUE):
  1. A PAL DMC DMA fixture exhibits the selected source-backed 2A07 halt behavior, and a competing wrong-model control produces a distinguishable result.
  2. A user reading the DMA contract can identify the modeled behavior and the unresolved later no-op-cycle bus behavior and direct-hardware-proof limit; passing the fixture does not imply physical verification.
  3. Saving and restoring under the same resolved region reproduces the uninterrupted frame, audio and machine-state continuation, including PAL timing/sample remainders and in-flight DMA where applicable.
  4. Loading a state from the opposite region is rejected before any state is applied, leaving the loaded machine's subsequent behavior unchanged.
  5. A host author can determine supported state compatibility and the actual format, behavior-revision and ABI consequences from the documented public contract; version changes follow actual serialized and public-contract changes.

**Plans**: 2/2 plans complete in 2 waves

Plans:
**Wave 1**
- [x] 26-01-PLAN.md — Implement the bounded PAL opcode-fetch DMA model with a same-timeline wrong-model control and public evidence contract

**Wave 2** *(blocked on Wave 1 completion)*
- [x] 26-02-PLAN.md — Preserve regional state continuation, atomic cross-region refusal and fixed adapter state size, then record bounded local acceptance

### Phase 27: PAL host presentation and NTSC preservation

**Goal**: A host receives consistent PAL selection, timing and modeled display colors through all three deliverables while existing NTSC behavior and output remain protected.
**Depends on**: Phase 26
**Requirements**: VID-01, AV-01, NTSC-01
**Success Criteria** (what must be TRUE):
  1. The library, runner and libretro adapter resolve the same content and eligible preference to the same region; the library and runner expose that region and its timing.
  2. Libretro reports the matching NTSC/PAL region and accurate loaded-game frame and audio rates, including the exact PAL cadence rather than a rounded 50 Hz assumption.
  3. A host obtains deterministic PAL RGB output from a named modeled 2C07 palette and its emphasis mapping, and the documentation clearly limits color claims to that model rather than universal calibration or hardware-exact output.
  4. Existing NTSC frame, audio and palette outputs and regression behavior remain unchanged on mapper IDs 0, 1, 2, 3, 4 and 7; any proposed intentional output change is separately justified instead of silently re-pinning a baseline.
  5. Owned or permitted content exercised through the library, runner and adapter retains matching regional behavior and deterministic outputs under the existing native-output and adapter-conversion contracts.

**Plans**: 4/4 plans complete across 4 waves

Plans:
**Wave 1**
- [x] 27-01-PLAN.md — Expose the resolved region and exact AV rates through the runner, library and adapter

**Wave 2** *(blocked on Wave 1 completion)*
- [x] 27-02-PLAN.md — Add the modeled 2C07 PAL palette to the instance-selected palette getter

**Wave 3** *(blocked on Wave 2 completion)*
- [x] 27-03-PLAN.md — Verify every modeled palette entry and regeneration while protecting NTSC output

**Wave 4** *(blocked on Wave 3 completion)*
- [x] 27-04-PLAN.md — Complete cross-deliverable PAL acceptance and record scoped review evidence

## Execution and evidence boundaries

- NTSC protection applies in every phase. Phase 27 owns the final NTSC requirement mapping; earlier phases cannot postpone their behavior-change regression checks.
- Each behavior change includes an owned regression and matching README/public-header comments, verified through `cmake --workflow --preset ci` during implementation. Existing local authority, resource caps and no-retry rules apply; roadmap creation runs no build or test.
- Use integer-only, per-instance deterministic owned code and owned or permitted fixtures under the ROM manifest and hygiene rules. Keep the three deliverables and add no dependency.
- Planning used the [PAL research synthesis](../research/milestone-v8-pal/SUMMARY.md) and its linked preparation sources. Phase 25 resolved the documented PAL raster/OAM behavior and the 239-line picture within the existing 240-row output contract before image expectations were fixed. Phase 26 bounded the source-backed DMA model; Phase 27 preserved the named palette model and emphasis caveat.
- Existing state format 6, behavior revision 14 and ABI 1 are the v7 baseline, not a predetermined v8 version decision. Apply the approved compatibility policy to actual changes.
- Do not add Dendy support, new boards, Playstead/libfuturo integration, unconfirmed scene repairs, commercial-game or universal-accuracy claims, or general collector cleanup. NTSC AccuracyCoin evidence does not establish PAL acceptance.
- The older interrupted full-CI/AccuracyCoin outcome remains unknown and its restart stop remains active. No hosted action, push, PR, merge, release, device test or campaign restart was authorized by this roadmap. Preserve the [v7 audit debt](v7-MILESTONE-AUDIT.md) and source-bound historical evidence.

## Progress

**Execution order:** Phase 25 → Phase 26 → Phase 27 → Phase 27.1. All four phases and their goal-backward verification are complete. The v8 audit and verified closeout are archived with this roadmap.

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 25. Region selection and PAL timing | 5/5 | Complete    | 2026-11-01 |
| 26. PAL DMA and regional state continuation | 2/2 | Complete    | 2026-11-02 |
| 27. PAL host presentation and NTSC preservation | 4/4 | Complete    | 2026-11-02 |
| 27.1. Close STATE-01 regional state validation gaps | 1/1 | Complete    | 2026-11-02 |

## Coverage and next step

The seven approved requirements retain their original traceability in
[v8-REQUIREMENTS.md](v8-REQUIREMENTS.md#traceability); inserted Phase 27.1 closes two
additional audited STATE-01 validation gaps. Phases 25, 26, 27, and 27.1 and
their goal-backward verification are complete; candidate-bound acceptance
records are linked from each phase directory. Phase 27's final receipt and
five-criterion verification are in
[27-ACCEPTANCE.md](v8-phases/27-pal-host-presentation-and-ntsc-preservation/27-ACCEPTANCE.md)
and
[27-VERIFICATION.md](v8-phases/27-pal-host-presentation-and-ntsc-preservation/27-VERIFICATION.md).
The next command after closeout is `$gsd-new-milestone`, which selects and
defines fresh scope from the archived product direction. Context can be cleared
first after reviewing [STATE.md](../STATE.md) and the [product roadmap](../PRODUCT-ROADMAP.md).

### Phase 27.1: Close STATE-01 regional state validation gaps (INSERTED)

**Goal:** A caller can restore valid modeled PAL five-step endpoint states and reachable regional frame continuations, while inconsistent frame counters are refused atomically before they can overflow or stretch PAL execution.
**Requirements**: STATE-01
**Depends on:** Phase 27
**Success Criteria** (what must be TRUE):
  1. PAL five-step states accept frame cycles 37281, 37282 and 41565; the endpoint round-trips and continues, 41566 is invalid, and NTSC five-step/PAL four-step still reject 37282.
  2. Both regions reject frame counters ahead of the validated regional tick quotient; PAL also rejects a lag greater than one frame. CRC-correct invalid states return the existing invalid-state status without changing the instance or save generation.
  3. Ordinary PAL instruction overshoot, a reachable PAL JAM/reset one-frame lag and valid NTSC format-6 states continue correctly; NTSC receives no PAL-style lower-lag constraint.
  4. The README and public header document these exact bounds and evidence limits with state format 6, behavior revision 17 and ABI 1 unchanged; the final candidate passes the existing full local CI workflow within its 30-minute bound. DMA scope and the older stopped/unknown campaign remain unchanged.

**Plans:** 1/1 plans complete in 1 wave

Plans:
**Wave 1**
- [x] 27.1-01-PLAN.md — Close both shared-loader validation gaps with exact public-loader regressions, reachable continuation controls and bounded local acceptance
