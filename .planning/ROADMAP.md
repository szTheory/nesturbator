# Roadmap: nesturbator

## Milestones

- ✅ **v1 NROM games with sound** — Phases 1–4.2 (shipped 2026-10-09) — [archive](milestones/v1-ROADMAP.md)
- ✅ **v2 Most of the library plays** — Phases 5–10 (shipped 2026-10-11) — [archive](milestones/v2-ROADMAP.md)
- ✅ **v3 Save states and rewind** — Phases 11–15 (shipped 2026-10-13) — [archive](milestones/v3-ROADMAP.md)
- ✅ **v4 Measured NTSC accuracy and robustness** — Phases 16–18 (shipped 2026-10-24) — [archive](milestones/v4-ROADMAP.md)
- ✅ **v5 NMI, VBlank and sprite-0 timing** — Phases 19–21 (shipped 2026-10-30; package publication pending) — [archive](milestones/v5-ROADMAP.md)
- ✅ **v6 Player-ready NES and reproducible feedback** — Phases 22–23 (planning milestone shipped 2026-10-31; no package release) — [archive](milestones/v6-ROADMAP.md)
- 🚧 **v7 Remaining DMA accuracy failures** — Phase 24 (roadmap awaiting approval)

The v5/v6 entries retain their closeout observations. Release PR #56 later
published v0.1.12; current release evidence is reconciled in
[PROJECT.md](PROJECT.md) and [PRODUCT-ROADMAP.md](PRODUCT-ROADMAP.md).

## Overview

v7 corrects the five remaining selected NTSC DMA failures for the declared
RP2A03G target at AccuracyCoin pin `6bee919529289c555803b612b8c9e4f7eaaeeabb`.
Every distinct correction needs an independent bus-cycle explanation and an
owned regression fixture. Existing protected passes, deterministic output,
performance limits and native state continuation are part of the same delivery.
The four approved requirements form one coherent phase at coarse granularity;
plans may separate demonstrated causes after tracing, without presuming that
the five failing rows share a cause.

## Phases

- [ ] **Phase 24: Correct remaining NTSC DMA behavior** — Resolve the five selected DMA failures with causal evidence, protected results and compatible state continuation.

Phase numbering continues from v6's Phase 23. Completed phases remain in the
milestone archives above; no future product proposal becomes execution scope.

## Phase Details

### Phase 24: Correct remaining NTSC DMA behavior

**Goal**: Hosts using the library, runner or libretro adapter receive source-supported NTSC DMA behavior for the five selected failures, with existing passing behavior and deterministic continuation preserved.
**Depends on**: Phase 23 (completed); inherits Phase 21's DMC retry/open-bus repair without treating it as completion of this cluster.
**Requirements**: DMA-01, DMA-02, DMA-03, DMA-04
**Success Criteria** (what must be TRUE):
  1. At the unchanged AccuracyCoin pin and declared RP2A03G target, `DMA Landing on Write`, `DMC DMA + OAM DMA`, `DMC DMA Bus Conflicts`, `Explicit DMA Abort` and `Implicit DMA Abort` each report PASS. Missing, interrupted or aggregate-only results do not establish this outcome. (DMA-01)
  2. For every distinct corrected behavior, a developer can inspect the first causal divergence in cycle-level evidence and run an owned fixture that accepts the corrected behavior and rejects a named wrong model. Hardware references, pinned-test expectations and observed core behavior remain separately identifiable. (DMA-02)
  3. `DMA + $4015 Read`, `Misaligned OAM DMA`, `INC $4014` and every other protected PASS key still pass at the same pin; identical inputs retain identical frame/audio output across supported platforms and the existing performance gates pass without relaxed limits. Any intentional output-baseline change has a source-backed reason. (DMA-03)
  4. When a correction changes in-flight DMA state, saving and loading at production-reachable affected boundaries yields the same subsequent machine behavior and frame/audio output as uninterrupted execution. The state-format and behavior-revision disposition is explicit, including a reasoned no-change disposition where appropriate. (DMA-04)
  5. A host developer can read the README and public-header comments to understand the corrected DMA behavior and any state compatibility consequences, with the owned regression checks registered in the existing CI entrypoint alongside the behavior change. (DMA-02, DMA-04)
**Plans**: 3/4 plans executed

Plans:
**Wave 1**
- [x] 24-01-PLAN.md — Landing-on-write tracer and independent baseline traces for all five rows (DMA-01, DMA-02, DMA-04)

**Wave 2** *(complete)*
- [x] 24-02-PLAN.md — DMC/OAM collision and DMC bus-conflict corrections (DMA-01, DMA-02, DMA-03, DMA-04)

**Wave 3** *(complete)*
- [x] 24-03-PLAN.md — Explicit and implicit abort corrections with native continuation (DMA-01, DMA-02, DMA-03, DMA-04)

**Wave 4** *(runtime authority and acceptance pending)*
- [ ] 24-04-PLAN.md — Regression audit and separately authorized acceptance handoff (DMA-01, DMA-02, DMA-03, DMA-04)
**Canonical refs**: [DMA scope](research/milestone-dma-discovery/SCOPE.md); [preparation index](preparation/README.md); [CPU/APU hardware, especially DMA §2.2](preparation/NES-HARDWARE-CPU-APU.md); [conformance and oracle limits](preparation/CONFORMANCE.md).

### Evidence boundaries within Phase 24

- Localize each selected row before combining fixes. Preserve the current
  recorded FAIL outcomes (`0x26` for landing on write; `0x0a` for the other four)
  as historical starting evidence, not fresh observations.
- Fixtures use the existing bus/core seam and distinguish access kind/address,
  get/put phase, halt/retry, actual sample read, relevant side effects and
  continuation. Include delayed CPU writes, DMC/OAM collision and final-write
  resumption, split internal/external bus effects and explicit/implicit abort
  controls as applicable. Cycle totals alone cannot locate a causal fault.
- Keep implicit-abort chip-revision assumptions explicit. A passing test ROM
  establishes its pinned contract; emulator agreement or matching hashes alone
  does not establish silicon correctness or a specific game's repair.
- Keep implementation and tests owned, integer-only and instance-local. No
  GPL/LGPL emulator source, new dependency, scheduler rewrite, public exact-cycle
  API or unlisted ROM bytes. The three existing deliverables remain the scope.
- Every behavior change includes its owned test and README/public-header
  documentation in the same change. Assess serialized state fields and loader
  compatibility before deciding the format/revision disposition. Preserve the
  current pin, raw-key score contract and existing runtime/performance limits.
- PAL/Dendy, unrelated accuracy clusters, broad playtesting, pin migration and
  general CI/collector cleanup remain outside this milestone. Existing collector
  debt still limits how new source-bound evidence may be qualified; do not label
  a run qualified without addressing its relevant evidence gaps explicitly.

### Verification authorization gate — Phase 24

The interrupted full-CI/AccuracyCoin campaign's completion and result remain
**unknown**. The existing stop is preserved. Milestone, requirements, roadmap,
discussion or plan approval does **not** authorize a full workflow or campaign
rerun, renew a runtime window/budget, or clear historical safety stops.

No tests run during roadmap creation. Later planning must inspect test selection
and configuration without executing the workflow and distinguish independently
authorized bounded fixtures from commands that schedule `accuracycoin.campaign`.
Do not start full CI or a campaign as a discovery probe, automatic verification,
review repair or side effect of a push/PR while that stop is active.

Project acceptance still requires `cmake --workflow --preset ci` and the existing
protected-pass, cross-platform determinism and performance evidence. Do not
disable the campaign, weaken the suite, raise limits or substitute bounded
passes to claim that gate passed. If satisfying the configured project CI gate
would schedule the stopped campaign, **stop Phase 24 execution before that
command** and record the exact command, selected tests, missing evidence and
authorization needed in phase evidence and STATE.md. Keep Phase 24 and affected
requirements incomplete; do not ship, merge or close v7 on partial verification.
The owner acts next only to decide separate runtime authorization at that gate.

## Progress

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 24. Correct remaining NTSC DMA behavior | 3/4 | In Progress | - |

## Coverage

| Requirement | Owning phase | Success criteria |
|-------------|--------------|------------------|
| DMA-01 | Phase 24 | 1 |
| DMA-02 | Phase 24 | 2, 5 |
| DMA-03 | Phase 24 | 3 |
| DMA-04 | Phase 24 | 4, 5 |

All four approved requirements map to exactly one phase. There are no orphaned
or duplicated phase assignments. Mapping is coverage, not completion evidence.

## Backlog disposition

2026-10-13: Backlog 999.2, "Bump AccuracyCoin past 673ef550", is promoted into
[Phase 17](milestones/v4-ROADMAP.md#phase-17-complete-pinned-conformance-baseline),
CONF-01 through CONF-04 and RM-001. No duplicate active backlog remains.

SEED-003 contributes only its approved remaining-DMA accuracy scope in v7.
SEED-006 adds no separate repair without a demonstrated need; the existing
playability/state/board outcomes and recent CI optimizations remain delivered.
No seed files were modified. The three pending property/mutation/signal proposals
were inspected and have no confident requirement match here; they remain unlinked
and deferred. All future direction remains in PRODUCT-ROADMAP.md.

*Last updated: 2026-10-31 after v7 roadmap approval. No tests or campaign run.*
