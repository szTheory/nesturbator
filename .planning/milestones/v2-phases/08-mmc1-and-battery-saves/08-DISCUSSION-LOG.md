# Phase 8: MMC1 and battery saves - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-10-10
**Phase:** 08-mmc1-and-battery-saves
**Areas discussed:** MMC1 ROMs and save oracle, RAM sizing and loader rows, Public save API shape, Write stamp and RetroArch proof

**User's choice:** all four areas. The owner asked for a fan-out with an
adversarial pass for each decision point, followed by one coherent set of
recommendations without further questions. They prefer "another copy-paste
is better than another dependency". Four advisor researchers reported, and
the orchestrator checked the claims against the code and the Holy Mapperel
source.

---

## MMC1 ROMs and save oracle

| Option | Description | Selected |
|--------|-------------|----------|
| Five ROMs; SXROM round-trip read from the PRG RAM row (`+ BATTERY`) plus a `.sav` signature check | Proves the save was loaded, not only written | ✓ |
| Re-run and require `0000` again, plus the `.sav` check | The code is `0000` with or without a save, so the check is vacuous | |

**Notes:** Seven twins and shapes that add nothing are left out (Phase 7 D-02). SNROM, SOROM, SXROM bank order and iNES 1 sizing are covered by synthetic tests.

## RAM sizing and loader rows

| Option | Description | Selected |
|--------|-------------|----------|
| Size from the header, laid out work first then NVRAM, strict NES 2.0, iNES 1 per SAVE-01 | Matches the wiki's SOROM chip order and keeps `.sav` sizes compatible | ✓ |
| Wiki's blanket rule: 32 KiB for every iNES 1 MMC1 game, all of it saved | Every save becomes 32 KiB and SAVE-01 is contradicted | |

**Notes:** Corrects the research draft's `[ nvram | work ]` order. Submapper 5 is accepted only at 32 KiB; 1-4, 6, 7-15 and mapper 155 are rejected.

## Public save API shape

| Option | Description | Selected |
|--------|-------------|----------|
| `get_memory(inst, kind, uint8_t **, size_t *)` with a status, SAVE_RAM only, and a monotonic write-count generation | Fits the `peek_cpu_ram` style; no aliasing trap | ✓ |
| A libretro-style data/size pair | No error channel, `void *` | |
| Ship SYSTEM_RAM now | An untested, frozen promise | |
| A generation reset at load, or counting value changes only | ABA risk, and a compare on a hot path | |

**Notes:** Runner: `.sav` stem naming, a temp file plus rename or MoveFileExA, flush only when the generation and a memcmp both differ, exit status 4 for a wrong size, and `--save-interval 0` is a usage error.

## Write stamp and RetroArch proof

| Option | Description | Selected |
|--------|-------------|----------|
| Fix WR-01 and WR-03 first; MMC1 uses `stamp == last_write + 1` | Moves no hash, because no board reads the stamp | ✓ |
| Fix only the wording and use differences | Leaves an off-by-one trap for Phase 9 | |
| Two RetroArch sessions on the HM battery ROM, each frame equal to the runner's and the two frames different, plus `.srm` size and signature | Cannot pass vacuously | ✓ |
| `.srm` bytes only | Cannot tell "loaded" from "not loaded" | |

**Notes:** The orchestrator reconciled the ROM choice: the RetroArch researcher suggested an S8K ROM, and SXROM was chosen so that the runner and RetroArch share one ROM and one oracle.

## Claude's Discretion

- Test and CTest names.
- The zero-means-$0C control encoding.
- How the bus detects a write into the span.
- The wording of runner messages.
- How the decoder's row-6 interface works.

## Deferred Ideas

- SYSTEM_RAM memory kind.
- SZROM, MMC1A (mapper 155), 2ME and the deprecated submappers.
- iNES 1 SOROM heuristics.
- Measuring RetroArch with a wrong-sized `.srm`.
- A save acknowledgement (SEED-002) and signal flushing (STATE-02).
- Phase 7 WR-01 and WR-02, still open.
