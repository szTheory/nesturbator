# Requirements: nesturbator

**Defined:** 2026-10-25
**Core Value:** Games behave as they do on the console, identically on every platform, from a small MIT-licensed C library that any host can embed.

## v5 Requirements

Milestone v5 "NMI, VBlank and sprite-0 timing" (RM-004 in [PRODUCT-ROADMAP.md](PRODUCT-ROADMAP.md)). AccuracyCoin rows are at pin `6bee919` and run by `cmake --workflow --preset ci`. Research: [research/SUMMARY.md](research/SUMMARY.md).

### Frame interrupts arrive when the console's would

- [x] **NMI-01**: A game that reads `$2002` around the start of VBlank sees the console's flag value and gets or loses the NMI as the console does; the `$2002 flag timing`, `NMI Timing` and `NMI Suppression` rows pass.
- [x] **NMI-02**: A game that enables or disables NMI around the start or end of VBlank gets NMIs exactly when the console would, including the extra NMI Bases Loaded II relies on; the `NMI Control`, `NMI at VBlank end` and `NMI disabled at VBlank` rows pass.

### CPU interrupts start on the right instruction

- [x] **INT-01**: An IRQ is taken on the same instruction as on the console after CLI, SEI, PLP, RTI and branches; the `Interrupt flag latency` row passes.
- [x] **INT-02**: An NMI arriving during BRK or an IRQ takes over its vector as on the console; the `NMI Overlap BRK` and `NMI Overlap IRQ` rows pass.

### Status-bar splits land on the right line

- [x] **SPR-01**: Sprite-0 hit is set under the console's conditions and on its dot; the `Sprite 0 Hit behavior` row passes.
- [x] **SPR-02**: The first sprite evaluated acts as sprite 0 wherever it sits in OAM, and the row finishes within the page's frame cap; the `Arbitrary Sprite zero` row passes.

### Everything that works today keeps working

- [x] **KEEP-01**: Every AccuracyCoin row that passes today still passes on every CI lane, and the CPU vectors, blargg MMC3, Holy Mapperel and boot-to-play checks stay green.
- [x] **KEEP-02**: A frame or audio hash that moves is re-pinned in the same change with its reason, and stays identical on all six platforms.
- [x] **KEEP-03**: A save state taken at any frame, including during the VBlank race or an interrupt sequence, loads back to the same run under rewind and run-ahead; a state from an earlier format is refused with the existing version error.

### A JAM is reported as a JAM

- [x] **JAM-01**: An AccuracyCoin run that hits a guest JAM reports a JAM rather than a generic frame failure.
- [x] **JAM-02**: `libretro.runahead` shows that run-ahead across a JAM gives the same picture and silence as running without run-ahead.

## Future Requirements

- DMA timing: the eight DMA rows (DMC/OAM DMA conflicts and aborts, `$4015` reads) — preferred next accuracy scope.
- PAL timing and pitch (RM-005).
- A nightly fetch-only blargg check (`ppu_vbl_nmi`, `cpu_interrupts_v2`) copying the MMC3 lane.

## Out of Scope

| Feature | Reason |
|---------|--------|
| Raising the 1800-frame AccuracyCoin cap without a measured reason | `tests/accuracy/budgets.txt` already rejects it; re-measure only after rows pass |
| Special cases for single rows or titles | One NMI-line and interrupt-poll model fixes the cluster; patches break neighbouring rows |
| A newer AccuracyCoin pin | Scores across pins are not comparable |
| Committing blargg ROMs | No published licence |
| Other failing clusters (DMA, OAM, `$2007`, open bus) | Each is scoped separately in PRODUCT-ROADMAP.md |

## Traceability

| Requirement | Phase | Status |
|-------------|-------|--------|
| NMI-01 | Phase 20 | Complete |
| NMI-02 | Phase 20 | Complete |
| INT-01 | Phase 21 | Complete |
| INT-02 | Phase 21 | Complete |
| SPR-01 | Phase 19 | Complete |
| SPR-02 | Phase 19 | Complete |
| KEEP-01 | Phase 21 | Complete |
| KEEP-02 | Phase 20 | Complete |
| KEEP-03 | Phase 20 | Complete |
| JAM-01 | Phase 19 | Complete |
| JAM-02 | Phase 19 | Complete |

**Coverage:**
- v5 requirements: 11 total
- Mapped to phases: 11
- Unmapped: 0

---
*Requirements defined: 2026-10-25*
*Last updated: 2026-10-29 after Phase 21 verification; all v5 requirements complete*
