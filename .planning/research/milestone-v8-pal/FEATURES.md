# Feature Landscape: v8 PAL Region Support

**Domain:** Deterministic NES emulator core, headless runner, and libretro adapter  
**Researched:** 2026-11-01  
**Research mode:** Ecosystem and milestone scope  
**Confidence:** MEDIUM overall. The milestone contract is explicit; libretro behavior is from official documentation, while NES hardware details rely mainly on NESdev's curated community references (T2) and one forum trace report (T4), not Nintendo-published specifications.

## Table Stakes

Features expected for a PAL mode to be useful through each of the project's three deliverables. “PAL support” must select a coherent machine profile, not merely alter how often frames are presented.

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| One explicit region-resolution contract | The same cartridge must not run differently in the library, runner, and libretro adapter. | Medium | NES 2.0 byte 12 values 0 and 1 identify NTSC and PAL; value 2 means multi-region; value 3 identifies Dendy. Resolve declared single-region metadata directly. Resolve multi-region and legacy iNES content to NTSC in `auto` mode; let the host explicitly select NTSC or PAL for those ambiguous/legacy cases. Refuse Dendy with a clear unsupported-region result; never treat it as PAL. NESdev documents these encodings and describes “multiple” as content intended to self-adjust to NTSC or PAL, but it is a community-maintained specification (T2), not Nintendo primary material. [NES 2.0](https://www.nesdev.org/wiki/NES_2.0) |
| Full PAL CPU, PPU, and APU profile | Correct video speed and music pitch depend on the clocks and chip tables, not a frontend speed setting. | High | Model the 2A07's ÷16 CPU clock and PAL timing, and the 2C07's five master clocks per PPU dot and 106,392-dot frame. PAL runs at approximately 50.007 frames/s in the project's exact timing model. Use PAL frame-counter, noise, and DMC period tables; a fixed 48 kHz output can stay fixed while the emulated source clock and per-frame sample count change. The PAL CPU clock is about 1.662607 MHz versus about 1.789773 MHz for NTSC, which affects tone pitch. [Cycle reference chart](https://www.nesdev.org/wiki/Cycle_reference_chart), [CPU variants](https://www.nesdev.org/wiki/CPU_variants), and [APU register reference](https://www.nesdev.org/wiki/APU_registers) are T2 community references; project-pinned calculations and source notes are in [NES hardware: CPU, bus, DMA and APU](../../preparation/NES-HARDWARE-CPU-APU.md) and [NES hardware: PPU, video, cartridges and file formats](../../preparation/NES-HARDWARE-PPU-CARTRIDGE.md). |
| Existing mapper coverage under either region | A PAL timing profile should apply to games already supported by the product. | High | Keep the six existing mapper IDs: **0 NROM, 1 MMC1, 2 UxROM, 3 CNROM, 4 MMC3, and 7 AxROM**. Exercise each family with PAL timing; preserve mapper-specific timing, including MMC3's observation of PPU A12. This is coverage expansion for the current board set, not mapper expansion. The six-family boundary is the approved RM-005 scope in [PRODUCT-ROADMAP.md](../../PRODUCT-ROADMAP.md). |
| Source-backed 2A07 DMC DMA model with a stated limit | PAL samples can collide with CPU/PPU/controller reads differently from the NTSC 2A03. | High | The best available model is that 2A07 DMC DMA waits for an instruction's first opcode-fetch cycle (the `SYNC` cycle), avoiding the ordinary side-effect-read collisions seen on 2A03. A 2025 NESdev report says direct traces support this rule. The accessible report does not publish the raw traces as a primary artifact, and the behavior of later no-operation cycles, including the exact address driven on the bus, remains unresolved. Implement only the evidence-backed behavior and keep that remaining uncertainty visible; do not claim hardware sign-off. The PAL rule is described by NESdev's T2 [DPCM conflict](https://www.nesdev.org/wiki/DPCM_conflict) and [DMA](https://www.nesdev.org/wiki/DMA) references; the trace interpretation is T4 [forum evidence](https://forums.nesdev.org/viewtopic.php?p=300930). See the open question already recorded in the project [CPU/APU preparation](../../preparation/NES-HARDWARE-CPU-APU.md). |
| Region-aware state identity | Replaying a saved CPU/APU/PPU timeline with a different clock profile is not valid continuation. | Medium | Store the resolved region/model in machine-state identity. Same-region states must resume; cross-region states must be rejected before any state is applied. Keep the libretro serialized size fixed for a loaded game, as required by its API; version state format/revision as needed by the resulting state change. Libretro does not prescribe region identity in states; this is the project's deterministic compatibility contract. [libretro.h](https://raw.githubusercontent.com/libretro/RetroArch/master/libretro-common/include/libretro.h) defines the fixed-size serialization constraints and success/failure result for unserialize; see project [architecture preparation](../../preparation/ARCHITECTURE.md) and [libretro/runner preparation](../../preparation/LIBRETRO-AND-RUNNER.md). |
| Accurate host AV metadata for the selected profile | Frontends use the reported frame rate and audio sample rate to pace video, mix audio, and record synchronized output. | Medium | After a successful load, report `retro_system_av_info` with the actual output geometry/aspect ratio, PAL frame rate, and configured output sample rate. `retro_get_region()` must return libretro's PAL value (1) or NTSC value (0). The core-development guide stresses accurate FPS and sample-rate reporting; the canonical header defines geometry and timing fields. If an active runtime option changes the whole AV profile, use `SET_SYSTEM_AV_INFO` sparingly because the frontend may reinitialize AV drivers; a geometry-only change uses `SET_GEOMETRY`. [Libretro Core Development Overview](https://docs.libretro.com/development/cores/developing-cores/) and the [canonical libretro.h](https://raw.githubusercontent.com/libretro/RetroArch/master/libretro-common/include/libretro.h) are official T1 sources. |
| Preserve existing NTSC behavior | PAL additions must not silently change the current product's default experience or established deterministic results. | High | Keep the NTSC clock/profile, frame/audio output, and existing NTSC defaults unchanged. Guard it with the existing NTSC evidence and compare hashes so the region work does not introduce incidental output changes. This is explicit in RM-005 and [PROJECT.md](../../PROJECT.md). |

## Distinguishing Details

| Feature | Value Proposition | Complexity | Notes |
|---------|-------------------|------------|-------|
| A shared resolved-region value across all deliverables | A host can select or inspect one region decision and get consistent timing, output, and state behavior everywhere. | Medium | The C API, CLI, and libretro option/input should call the same resolver. Keep “auto” behavior visible: NES 2.0 single-region values select their declared profile; multi-region and legacy iNES default to NTSC and allow an explicit NTSC or PAL choice. Do not infer region from filenames, market assumptions, or a bundled checksum database. |
| A modeled 2C07 palette with an honest accuracy caveat | PAL content gets a deliberate PAL color interpretation instead of reusing the NTSC conversion by accident. | Medium | The 2C07 uses a different colorburst reference and emphasis-channel ordering. Use a documented, deterministic palette model, but describe it as modeled: rendered RGB values depend on the selected decode assumptions and display chain, and the source evidence does not establish one universally exact palette. The palette source is NESdev's T2 [PPU palettes](https://www.nesdev.org/wiki/PPU_palettes), cross-checked against the project [PPU preparation](../../preparation/NES-HARDWARE-PPU-CARTRIDGE.md). |
| Machine-readable regional reporting | Frontend users and automation can tell what timing a loaded game actually uses. | Low–Medium | The libretro interface exposes `retro_get_region()` as NTSC/PAL and `retro_get_system_av_info()` for timing and geometry; the runner/library should expose the same resolved region through their existing contracts. Report the actual PAL rate derived from the emulated clock (approximately 50.007 Hz), rather than rounding it to 50.0 when the core knows the more exact value. This follows the T1 libretro API and the project's integer-fraction timing policy. |
| Explicitly bounded PAL DMA confidence | The feature can improve PAL gameplay while leaving the remaining hardware question auditable. | Medium | Keep a named residual: the first-cycle halt model has wiki-level support and a reported physical trace basis, but the bus address during later no-operation cycles is not settled. A focused owned fixture can protect the chosen model and expose wrong alternatives; it cannot convert the T4 report into universal silicon proof. |

## Anti-Features

| Anti-Feature | Why Avoid | What to Do Instead |
|--------------|-----------|-------------------|
| Dendy support or silently treating Dendy as PAL | Dendy uses a distinct CPU divider and video/VBlank arrangement. Reusing PAL would falsely claim behavior the project has explicitly excluded. | Reject NES 2.0 timing value 3 as unsupported, with a clear error. Keep separate Dendy work proposed until its own evidence and demand justify scope. |
| Guessing region for legacy content | Older metadata and territory labels are not a reliable basis for changing the approved default, and filename heuristics would make behavior unpredictable. | Default legacy iNES to NTSC in auto mode, and allow an explicit host override. Apply the same documented default to NES 2.0 “multiple” metadata. |
| A second PAL/low-accuracy core or frame-rate-only PAL toggle | Two emulation paths multiply bugs; changing cadence alone leaves the CPU/APU pitch and hardware timing wrong. | Extend the single deterministic machine profile with PAL clocks and tables while keeping NTSC as the unchanged default. |
| Unqualified “accurate PAL colors” or “fully accurate PAL” claims | Current 2C07 palette references model a composite decode; they do not establish a universal display calibration. 2A07 bus behavior also has a documented evidence gap. | Call the palette modeled and state the DMA limit. Scope acceptance to specific evidence and the six supported mappers. |
| ROM/checksum database, commercial ROM bundle, or emulator-source reuse | These conflict with the existing licensing, clean-room, and repository hygiene rules and are unnecessary for header-driven resolution. | Use NES 2.0 metadata, deterministic NTSC fallback, and host overrides. Implement from hardware references and owned tests. |
| New mappers, Dendy, Playstead/libfuturo integration, or unrelated scene fixes in this milestone | These have separate acceptance triggers or no confirmed core diagnosis and would broaden a bounded regional milestone. | Keep them deferred per the approved [product roadmap](../../PRODUCT-ROADMAP.md). |
| New third-party dependency for region handling or color tables | The resolver and small fixed table fit the existing C implementation and ownership constraints. | Use existing modules, standard C, and a checked-in deterministic table where needed. |

## Feature Dependencies

```text
NES 2.0 metadata + host override policy
  → one resolved NTSC/PAL profile (legacy and multi-region default to NTSC)
  → CPU/PPU/APU timing and PAL-specific channel tables
  → mapper timing for IDs 0, 1, 2, 3, 4, and 7
  → library and runner frames/audio
  → libretro PAL/NTSC region and AV metadata

Resolved profile → state identity → same-region continuation / cross-region refusal
Resolved PPU model → 2C07 palette table (modeled, with caveat)
NTSC preservation checks → protects all existing NTSC output behavior
```

## MVP Recommendation

Prioritize:

1. **Region resolution and refusal:** define the NES 2.0 and legacy defaults, host overrides for multi-region/legacy inputs, and early Dendy refusal.
2. **PAL profile through the existing six mappers:** implement the 2A07/2C07 clocks, PAL APU tables and a source-backed, explicitly bounded DMC DMA model; keep NTSC output unchanged.
3. **Persistence and host integration:** serialize resolved region identity, support same-region load and reject cross-region load, and report the selected profile through library/runner/libretro including libretro AV timing.
4. **Modeled 2C07 color output:** add the deterministic PAL palette alongside acceptance that names its model and accuracy limit.

Defer additional mapper families, Dendy, Playstead/libfuturo, broad visual accuracy work, and universal color or hardware-accuracy claims. A region resolver and timing profile are prerequisites for all later PAL-specific host reporting and state tests.

## Sources and Confidence

| Source | Tier | Confidence | Supports |
|--------|------|------------|----------|
| [Libretro core-development documentation](https://docs.libretro.com/development/cores/developing-cores/) and [canonical libretro.h](https://raw.githubusercontent.com/libretro/RetroArch/master/libretro-common/include/libretro.h) | T1: official docs/API | MEDIUM (websearch source, cross-checked) | AV info fields, timing accuracy expectation, region enum, runtime AV update behavior, serialization contract |
| [NES 2.0](https://www.nesdev.org/wiki/NES_2.0) | T2: curated community reference | MEDIUM (cross-checked with the project's hardware preparation) | Header byte 12 timing values and meaning of multi-region |
| [CPU variants](https://www.nesdev.org/wiki/CPU_variants), [cycle reference chart](https://www.nesdev.org/wiki/Cycle_reference_chart), [APU registers](https://www.nesdev.org/wiki/APU_registers), [DMA](https://www.nesdev.org/wiki/DMA), [DPCM conflict](https://www.nesdev.org/wiki/DPCM_conflict), and [PPU palettes](https://www.nesdev.org/wiki/PPU_palettes) | T2: curated community reference | MEDIUM; silicon details require hardware-level follow-up | PAL clocks, APU tables, DMA behavior, and modeled 2C07 color behavior |
| [PAL DMA behavior thread](https://forums.nesdev.org/viewtopic.php?p=300930) | T4: forum report | MEDIUM as a lead, not primary proof | Reported traces suggesting 2A07 DMA waits for an opcode-fetch/SYNC cycle; raw trace evidence is not published in the thread |
| [NES hardware CPU/APU preparation](../../preparation/NES-HARDWARE-CPU-APU.md), [NES hardware PPU/cartridge preparation](../../preparation/NES-HARDWARE-PPU-CARTRIDGE.md), [architecture preparation](../../preparation/ARCHITECTURE.md), and approved [v8 product roadmap](../../PRODUCT-ROADMAP.md) | T1/T2 project records; external sources retain their own tiers | HIGH for approved feature boundaries; MEDIUM for cited hardware details | Existing time model, six mapper IDs, output contracts, prior open questions, and the milestone's approved scope |

The 2A07 DMA rule is the main evidence gap for phase-specific research. Treat the opcode-fetch halt as the best available model, retain the unexplained no-operation-cycle bus behavior as an explicit limit, and do not present the forum report as a direct primary artifact. NESdev is useful and frequently maintained, but it remains T2 community documentation under the project's own source-tier definitions in [preparation/README.md](../../preparation/README.md).
