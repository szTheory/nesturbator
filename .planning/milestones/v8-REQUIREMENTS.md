# Requirements Archive: v8 PAL region support

**Archived:** 2026-11-03
**Status:** SHIPPED

This is the archived v8 acceptance baseline. Requirements for a future
milestone will be created during `$gsd-new-milestone`.

---

# Requirements: nesturbator

**Milestone:** v8 — PAL region support
**Defined:** 2026-11-01
**Core Value:** Games behave as they do on the console, identically on every platform, from a small MIT-licensed C library that any host can embed.

## v1 Requirements

Requirements for the committed v8 scope.

### Region selection

- [x] **REG-01**: A host can run supported content with one resolved NTSC or PAL profile: explicit single-region NES 2.0 metadata takes precedence; multi-region and legacy iNES content default to NTSC but accept an explicit host preference; Dendy content is refused. A host preference cannot contradict explicit single-region metadata.

### PAL emulation and video

- [x] **PAL-01**: A player can run PAL content on each supported mapper family (0, 1, 2, 3, 4 and 7) with PAL CPU/PPU/APU cadence and frequency tables, producing independently checked PAL frame timing and audio pitch.
- [x] **DMA-01**: PAL DMC DMA follows the best source-backed 2A07 model in an owned or permitted fixture with a control that distinguishes the wrong model; documentation states that later no-op-cycle bus behavior and direct hardware proof remain unresolved.
- [x] **VID-01**: A host can obtain a deterministic PAL display palette using the named modeled 2C07 table and emphasis mapping; documentation identifies it as a model, not universally calibrated or hardware-exact color output.

### Host and machine-state behavior

- [x] **AV-01**: The library, runner and libretro adapter use the same resolved region; the library and runner expose the selected profile and its timing, and libretro reports the matching NTSC/PAL region and accurate loaded-game frame and audio rates.
- [x] **STATE-01**: A machine state resumes under the same resolved region and is rejected before any state is applied when the loaded machine's region differs; format, behavior-revision and ABI consequences are documented according to the actual serialized and public-contract changes.

### NTSC compatibility

- [x] **NTSC-01**: Adding PAL leaves existing NTSC behavior and output hashes unchanged across all six supported mapper families, with existing NTSC regression evidence retained and any intentional output change separately justified.

## Future Requirements

- A distinct Dendy profile with its own timing and evidence contract.
- Additional mapper families, expansion audio, FDS, and extra peripherals.
- Hardware-qualified 2A07 DMA behavior, calibrated 2C07 color claims, or a universal PAL accuracy claim.
- Actual libfuturo or Playstead integration and adopter-specific release qualification.
- A reported scene/core fix only after bounded native/source triage establishes a core fault and an independent reproducer.

## Out of Scope

| Feature | Reason |
|---------|--------|
| Dendy emulation or treating Dendy as PAL | Dendy is a separate profile and remains refused until its own evidence and demand justify scope. |
| New mapper families or broader Famicom hardware | v8 covers only the six existing mapper families; other boards and devices have separate triggers. |
| libfuturo/Playstead integration, a new host, window, audio-device or input-device code | These are consumer/host responsibilities; v8 extends the existing library, runner and libretro deliverables. |
| Commercial ROMs, named-game playability claims, ROM databases or bundled game bytes | Use owned or permitted fixtures under the repository manifest and hygiene rules. |
| Canonical/universal palette claims or proven 2A07 DMA hardware behavior | Current references support a modeled palette and a bounded DMA model, with the documented evidence limits retained. |
| Broad scene-fault work, general CI/collector cleanup, a new dependency or an all-pass AccuracyCoin target | These are unneeded for this regional outcome or lack a demonstrated trigger. |
| Treating AccuracyCoin as PAL evidence or running an unbounded campaign | The previous interrupted result remains unknown and is not inherited as evidence. The owner's standing authority covers bounded local-CI workflows and their configured AccuracyCoin tests; that NTSC campaign still does not prove PAL behavior. |

## Evidence and safety notes

The [v8 PAL research synthesis](../research/milestone-v8-pal/SUMMARY.md) and
regional hardware references record source tiers and open questions. Use owned
or permitted PAL fixtures; do not treat the NTSC AccuracyCoin score as PAL
acceptance. The 2C07 output is a named model, and the 2A07 DMA rule is the best
source-backed model rather than a claim of physical verification.

All behavior changes must include owned regression tests and update the README
and public-header comments in the same change, then run
`cmake --workflow --preset ci` under the standing local verification authority.
Keep clean-room, MIT, integer-only, per-instance determinism, and the existing
three deliverables. The owner's standing local-CI authority covers local CMake
workflows and configured campaigns. This requirements scope does not authorize
hosted workflows, pushes, PRs, merges, releases or device tests.

## Traceability

Each approved requirement maps to exactly one owning v8 phase. Supplemental
validation evidence may be recorded without changing that ownership. NTSC
regression protection applies throughout implementation; Phase 27 owns its final
acceptance.

| Requirement | Phase | Status |
|-------------|-------|--------|
| REG-01 | Phase 25 | Complete |
| PAL-01 | Phase 25 | Complete |
| DMA-01 | Phase 26 | Complete |
| VID-01 | Phase 27 | Complete |
| AV-01 | Phase 27 | Complete |
| STATE-01 | Phase 26 | Complete |
| NTSC-01 | Phase 27 | Complete |

Phase 26 remains the sole owning phase for STATE-01. Phase 27.1 provides
supplemental validation closure for the audited PAL five-step endpoint and
regional frame-counter validation cases, as documented in the [Phase 27.1
verification](v8-phases/27.1-close-state-01-regional-state-validation-gaps/27.1-VERIFICATION.md).
This supplemental evidence does not add a requirement or a second owner.

**Coverage:**
- v1 requirements: 7 total
- Mapped to phases: 7
- Unmapped: 0

---
*Requirements defined: 2026-11-01*
*Last updated: 2026-11-03 after traceability reconciliation for Phase 27.1 supplemental STATE-01 validation.*
