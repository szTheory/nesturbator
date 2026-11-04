# Requirements Archive: v7 Remaining DMA accuracy failures

**Archived:** 2026-11-01
**Status:** SHIPPED

For current requirements, see `.planning/REQUIREMENTS.md`.

---

# Requirements: nesturbator

**Milestone:** v7 — Remaining DMA accuracy failures
**Defined:** 2026-10-31
**Core Value:** Games behave as they do on the console, identically on every platform, from a small MIT-licensed C library that any host can embed.

## v1 Requirements

Requirements for the committed v7 scope.

### NTSC DMA accuracy

- [x] **DMA-01**: At the existing AccuracyCoin pin `6bee919529289c555803b612b8c9e4f7eaaeeabb`, the core passes `DMA Landing on Write`, `DMC DMA + OAM DMA`, `DMC DMA Bus Conflicts`, `Explicit DMA Abort`, and `Implicit DMA Abort` for the declared RP2A03G target.
- [x] **DMA-02**: Every distinct corrected behavior has source-backed cycle-level evidence and an owned fixture with a wrong-model control; hardware behavior, test expectations, and observed core behavior are recorded separately.

### Regression and compatibility

- [x] **DMA-03**: `DMA + $4015 Read`, `Misaligned OAM DMA`, `INC $4014`, and every other protected PASS key remain passing at the same pin, with deterministic output and existing performance limits preserved.
- [x] **DMA-04**: If a fix changes in-flight DMA state, native save/load resumes identically and the state-format/behavior-revision impact is explicit; README and public-header comments describe the changed behavior.

## Future Requirements

- PAL and Dendy timing profiles, other AccuracyCoin failure clusters, a broader accuracy sweep, and further game/host playtesting remain future proposals.
- AccuracyCoin pin migration is a separate change and is not part of v7.

## Out of Scope

| Feature | Reason |
|---------|--------|
| PAL/Dendy timing, unrelated CPU/PPU accuracy clusters, sprite evaluation/OAM corruption, and broader commercial playtesting | v7 is limited to the five selected NTSC DMA failures. |
| AccuracyCoin pin migration or a universal chip-revision claim | The existing pin and declared RP2A03G target are retained; revision-dependent behavior must stay explicit. |
| General scheduler rewrite, new dependency, public exact-cycle API, or unrelated collector/CI cleanup | The scope note identifies no demonstrated need for these changes. |
| Rerunning the previously interrupted full CI/AccuracyCoin campaign without separate authorization | The existing campaign stop remains active and is not changed by milestone or requirement approval. |

## Evidence and safety notes

The [DMA scope note](../research/milestone-dma-discovery/SCOPE.md) records the per-row evidence needs, primary test/source references, and clean-room boundaries. It does not establish one shared cause among the five failures. The three passing DMA rows and all other protected PASS keys are regression guards, not scope to modify.

The prior full-CI/campaign run was interrupted and its outcome is unknown. This milestone opening did not rerun it or claim a pass. The current stop remains until separately authorized; a requirement approval does not supply that authorization.

## Traceability

| Requirement | Phase | Status |
|-------------|-------|--------|
| DMA-01 | Phase 24 | Complete |
| DMA-02 | Phase 24 | Complete |
| DMA-03 | Phase 24 | Complete |
| DMA-04 | Phase 24 | Complete |

**Coverage:**
- v1 requirements: 4 total
- Mapped to phases: 4 — each assigned to Phase 24 exactly once
- Unmapped: 0

All four requirements are verified by the Phase 24 report and its source-bound
acceptance receipt. The owner's automation direction replaces the redundant
human-only checkpoint; existing stops and action authority remain unchanged.

---
*Requirements defined: 2026-10-31*
*Last updated: 2026-11-01 after passing Phase 24 verification; no new campaign run.*
