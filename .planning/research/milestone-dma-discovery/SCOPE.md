# Remaining DMA accuracy failures: upstream scope

Date: 2026-10-31
Status: research recommendation for the next milestone; requirements and phases are not approved yet.

## Recommendation

Scope one NTSC DMA-accuracy milestone around the five failing rows in the current AccuracyCoin scoreboard, using the existing pin and current RP2A03G target. Keep the three already-passing DMA rows and every other protected PASS row as regression guards. Investigate and fix only behaviors supported by an independent bus-cycle explanation. The five failures are related but do not yet establish one shared cause.

The current scoreboard records `DMA Landing on Write` as FAIL `0x26`; `DMC DMA + OAM DMA`, `DMC DMA Bus Conflicts`, `Explicit DMA Abort`, and `Implicit DMA Abort` are FAIL `0x0a`. The candidate rows are in [scoreboard.txt](../../../tests/accuracy/scoreboard.txt). The preserved neighboring DMA passes are `DMA + $4015 Read`, `Misaligned OAM DMA`, and `INC $4014`.

## What each candidate needs to establish

| Candidate | Test behavior and evidence needed |
|---|---|
| DMA Landing on Write | The pinned test covers DMC load timing, halt attempts meeting CPU writes, retries, and resulting DMA duration. Trace request timing separately from the successful halt and actual sample read; test relevant get/put phases and one/two-write delays. |
| DMC DMA + OAM DMA | Verify the DMC/OAM collision and OAM resumption across the transfer, including the final writes where the added duration differs. Record the actual get/put accesses and complete CPU-cycle count. |
| DMC DMA Bus Conflicts | Observe CPU internal register selection, external sample-read address/data, internal and external bus latches, frame-counter IRQ acknowledgement, controller clocks and returned open-bus bits separately. The final test code alone cannot locate the fault. |
| Explicit DMA Abort | Exercise disabling DMC playback around a scheduled reload and distinguish an abort from a halt deferred by CPU writes. Check the expected one-cycle aborted transfer window. |
| Implicit DMA Abort | Exercise an empty-buffer, one-byte non-looping sample and a looping control around reload timing. Keep sample length, write-cycle delays and CPU revision assumptions explicit. |

The pinned [AccuracyCoin README](https://github.com/100thCoin/AccuracyCoin/blob/6bee919529289c555803b612b8c9e4f7eaaeeabb/README.md) and [assembly](https://github.com/100thCoin/AccuracyCoin/blob/6bee919529289c555803b612b8c9e4f7eaaeeabb/AccuracyCoin.asm) define what these tests assert. They are primary evidence for the test oracle, not by themselves proof of silicon behavior. The [NESdev DMA reference](https://www.nesdev.org/wiki/DMA) is the project's working timing specification: halts succeed on CPU reads, DMC and OAM get/put phases interact, DMC wins colliding gets, and abort outcomes depend on stop timing. It is a curated community source rather than transistor-level proof. The project preparation map points to the circuit analysis and public tests in [NES-HARDWARE-CPU-APU.md](../../preparation/NES-HARDWARE-CPU-APU.md), and the broader test limitations are in [CONFORMANCE.md](../../preparation/CONFORMANCE.md).

Phase 21's DMC retry/open-bus repair is a prerequisite, not completion of this cluster. Its [audit](../../milestones/v5-phases/21-interrupt-latency-and-hijack/21-DMC-PHASE-AUDIT.md) distinguishes request/retry timing from actual sampling and warns that cycle totals alone are insufficient. Do not infer the cause of any remaining row from that earlier repair or the aggregate campaign score.

## Recommended evidence and implementation shape

1. For each candidate, preserve its current pinned status and identify the first source-bound divergence. Keep test-source expectations, hardware references, and observed implementation behavior as separate evidence.
2. Add small owned C fixtures at the existing bus/core seam. Assert the bus address, read/write kind, APU get/put phase, halt/retry point, data access, side effects, and resulting continuation. Include a wrong-model control so the fixture demonstrates that it catches the bug.
3. Group implementation work only after the traces establish shared causes. Keep behavior in the current bus/APU modules; no new dependency, general scheduler rewrite, public exact-cycle API, ROM-pin change, or broad accuracy sweep is justified by this scope.
4. Preserve deterministic output, all protected PASS keys, and existing runtime budgets. If a change affects serialized in-flight state, test production-reachable save/load continuation and explicitly assess state-format/revision implications.
5. Update behavior documentation with the owned tests. The repository's tested-and-documented rule also requires reviewing the README and public-header comments for each behavior change. Do not claim a broader gameplay or hardware result from the AccuracyCoin campaign alone.

The best product connection is DMC/controller-read corruption during sample playback. NESdev documents a Mario game's input bug in this area; treat it as a lead, since the project has no matching title report and the source does not show that one of these exact failures causes it. Do not turn it into a manual play requirement without a scene-first, verified access plan. Affected-title linkage remains desirable where evidence is available, not a reason to invent a connection.

## Boundaries

- Keep PAL/Dendy timing, sprite evaluation/OAM corruption, unrelated CPU/PPU accuracy, broad commercial playtesting, AccuracyCoin pin migration, and general CI/collector cleanup out of this milestone.
- Keep all ROM bytes governed by `tests/roms/manifest.txt`; do not check in downloaded test binaries or raw private gameplay material.
- Preserve the clean-room rule: no GPL/LGPL emulator source. Released reference binaries may be used only within existing authorization.
- The existing campaign stop and runtime/evidence boundaries remain in force. This research did not run tests or campaigns and authorizes no new campaign.

## Adversarial notes

- AccuracyCoin's design target is RP2A03G, while implicit-abort behavior can differ by chip revision. Keep the chosen profile and accepted test outcomes explicit; do not force a universal chip claim.
- A recent [switch-level 2A03 report](https://tinymachines.ai/docs/nes/n3-report) corroborates a two-cycle OAM delay for an interior DMC collision but reports a sample-address anomaly and leaves it open pending logic-analyzer evidence. It is useful for a probe, not a final hardware oracle.
- NESdev's [game-bug notes](https://www.nesdev.org/wiki/Game_bugs) identify a player-visible DPCM/controller interaction. This supports the area’s relevance while leaving this project's exact fault-to-title mapping unverified.
- Avoid adding a dependency. Existing C tests, source references, and bounded fixtures can express the required behavior; local duplication is preferable to a larger timing abstraction unless implementation evidence demonstrates otherwise.

## Project-state reconciliation for formal milestone opening

The current [PRODUCT-ROADMAP.md](../../PRODUCT-ROADMAP.md) still says release PR #56 is open and v0.1.11 is latest. The current [STATE.md](../../STATE.md) and [261031-ecd summary](../../quick/261031-ecd-fix-release-only-accuracycoin-fingerprin/261031-ecd-SUMMARY.md) say v0.1.12 was published. Reconcile this evidence in the formal new-milestone step before refreshing Immediate/Near/Mid/Long; preserve the unrelated historical audit and safety stops. The product roadmap already ranks the five DMA failures as the default measured accuracy candidate, with RM-015 remaining conditional on a confirmed visual/core fault.

No code, requirements, roadmap, product-roadmap direction, tests, or campaign were changed in this scope review. Existing working-tree edits were left intact.
