# Phase 9: MMC3 - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-10-10
**Phase:** 09-mmc3
**Areas discussed:** HM mapper 4 ROMs, A12 clock and IRQ model, Blargg nightly oracle, Loader rules and MAP-03

The owner selected all four areas. They asked for a fan-out with an
adversarial pass and one coherent set of recommendations to be followed
without further questions. Four advisor researchers ran in parallel, and the
orchestrator cross-checked them against the code.

---

## HM mapper 4 ROMs

| Option | Description | Selected |
|--------|-------------|----------|
| A. Commit as shipped, assert `0000` only | `$A001` never runs, because no M4 ROM declares PRG-RAM; a vacuous pass | |
| B. A plus a build-time derived W8K copy | Patching the header forces the `$A001` test; nothing extra is committed | ✓ |
| C. B plus a derived S8K copy with the save chain | TKROM-style battery proof; `.sav[0] == 0xB6` proves write protect | ✓ |
| D. Build ROMs with Holy Mapperel's toolchain | Adds ca65, Python and PIL to CI | |

**User's choice:** fan-out recommendation, auto-followed (B + C).
**Notes:** Twins `M4_P128K` and `M4_P128K_CR32K` are skipped.

## A12 clock and IRQ model

| Option | Description | Selected |
|--------|-------------|----------|
| A. Board counts M2 falls between edges via the hook | No core change; integer only | ✓ (index from region-correct `cpu_cycle`, not `tick / 24`) |
| B. Raw tick difference ≥ 72 | Not M2-quantised | |
| C. CPU_CLOCK watch every cycle | Hot-path cost on every board | |
| D. Filter in the PPU | Counts PPU events; breaks the seam contract | |

Revision: one routine, NEC selected by submapper 4, Sharp otherwise (✓).
MC-ACC is not modelled. Tests run at the hook level and through the real
PPU (✓ both).

**Notes:** In the adversarial pass, `src/internal.h` documents `cpu_cycle` as
region-correct, unlike `ticks / 24`. Researcher-flagged: `$A001` power-on is
stored encoded so that zero means enabled.

## Blargg nightly oracle

| Option | Description | Selected |
|--------|-------------|----------|
| A. Test-only helper `tests/mmc3/oracle.c` | `$6000` via the internal read, `$F8` via the public peek; no API change | ✓ |
| B. New runner flag | Widens the public contract | |
| C. Screen decode | Unlicensed font; weaker signal | |

Recording: a separate `tests/mmc3/oracle.txt` (✓), not the AccuracyCoin
scoreboard. Wiring: a separate `mmc3-oracle` nightly job (✓), not folded into
`vectors-full` or `ci`.

## Loader rules and MAP-03

| Option | Description | Selected |
|--------|-------------|----------|
| PRG power of two 32-512 KiB | First draft | |
| PRG power of two 16-512 KiB | Needed by blargg `mmc3_irq_tests` (16 KiB) | ✓ |
| Submappers 0 and 4 only | Rejects 1, 2, 3, 5 and 6-15 | ✓ |
| Runner-local `describe_rejection` | Names four-screen, MMC6 and the mapper with no API change | ✓ |
| New public reason API | More surface for a single message | |

**Notes:** iNES 1 mapper 4 always gets 8 KiB of RAM; the Low G Man gap is
documented.

## Claude's Discretion

- Test file and CTest names, the `$A001` encoding, the M2 index derivation
  pinned by tests, the form of the derived-ROM entries, and message wording
  beyond the fixed strings.

## Deferred Ideas

- MMC6, MC-ACC, T9552, hard-wired mirroring, four-screen, mappers 118, 119
  and 206.
- PAL/Dendy test tick values.
- A public rejection-reason API.
- iNES 1 header repair.
