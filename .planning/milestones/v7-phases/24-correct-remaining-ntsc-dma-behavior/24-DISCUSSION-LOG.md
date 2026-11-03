# Phase 24: Correct remaining NTSC DMA behavior - Discussion Log

> **Audit trail only.** Decisions are captured in `24-CONTEXT.md`; this log preserves the alternatives considered.

**Date:** 2026-10-31
**Phase:** 24-correct-remaining-ntsc-dma-behavior
**Areas discussed:** causal localization and fixtures, hardware/test evidence and revision assumptions, save-state compatibility and verification boundary

---

## Causal localization and fixtures

| Option | Description | Selected |
|--------|-------------|----------|
| Row-first | Trace each failure independently; group only after a shared cause is evidenced. Add focused owned fixtures at the existing seam. | ✓ Locked by approved roadmap and scope note |
| Shared recorder up front | Build a common trace helper before knowing whether the rows share a path. | |

**User's choice:** Consider all relevant decision areas and use a coherent recommendation. Existing project approval already fixes the row-first evidence contract; no routine preference question was needed.
**Notes:** A shared recorder remains an engineering option only if traces show a demonstrated need. Keep fixture assertions tied to distinct failure modes and wrong-model controls.

---

## Hardware/test evidence and revision assumptions

| Option | Description | Selected |
|--------|-------------|----------|
| Pinned test as silicon proof | Treat a passing AccuracyCoin result as proof of the exact hardware behavior. | |
| Layer the evidence | Keep hardware references, pinned test expectations, and observed core behavior separate. | ✓ Locked by approved requirements and prior evidence policy |
| Force one universal RP2A03G implicit-abort outcome | Claim one early or late behavior for every RP2A03G. | |
| Keep the test contract and scope claim narrow | The pinned test accepts both early and later implicit-abort outcomes; do not claim one universal revision behavior. | ✓ Recommended and consistent with pinned test contract |

**User's choice:** All current decision areas were considered under the existing RP2A03G target and pinned test contract; no revision-selection question remains for this phase.
**Notes:** The pinned test is a compatibility oracle. Hardware-source evidence and a wrong-model owned fixture still justify each correction. Do not add a selectable revision profile in this scope.

---

## Save-state compatibility and verification boundary

| Option | Description | Selected |
|--------|-------------|----------|
| Keep the current format in every case | Avoid format changes even if serialized fields or their meanings change. | |
| Version only what implementation changes require | Keep the format when schema/meaning is unchanged; bump it when serialized state changes, and update behavior revision when output changes. | ✓ Locked by public header and architecture contract |
| Add an old-format converter | Preserve states across incompatible layouts with a migration reader. | |

**User's choice:** Follow the existing no-converter state contract and decide versions from actual changes; no new compatibility promise was requested.
**Notes:** Preserve the active CI/campaign stop. If project CI schedules the interrupted campaign, execution must stop before the command for separate authorization. This discussion ran no tests or campaign.

---

## The agent's Discretion

Trace order, helper/fixture factoring, exact test file placement, and plan grouping remain research/planning choices within the approved phase boundary.

## Deferred Ideas

- Reproducible owned property tests — previously reviewed; no confident Phase 24 requirement match.
- Scoped nightly mutation testing — previously reviewed; no confident Phase 24 requirement match.
- Deterministic runner signal checks — previously reviewed; no confident Phase 24 requirement match.
