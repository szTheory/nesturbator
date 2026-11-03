# Project Research Summary

**Project:** nesturbator — v8 PAL region support  
**Domain:** Deterministic NES emulator core, CLI runner, and libretro adapter  
**Researched:** 2026-11-01  
**Confidence:** MEDIUM

## Executive Summary

This milestone extends an existing C17 NES core across its three deliverables: an embeddable library, a headless runner, and a libretro adapter. The approved product contract is specific: resolve single-region NES 2.0 metadata, default ambiguous and legacy images to NTSC with a host preference for multi-region and legacy iNES, refuse Dendy, cover the six existing mapper families, identify state by effective region, and preserve NTSC outputs. PAL must be a complete 2A07/2C07 timing and audio profile, not a frame-rate toggle.

Keep the existing owned C/CMake implementation and add no runtime dependency. Resolve region once at cartridge load, then use an immutable per-instance profile and integer/rational clock and sample accounting; expose the exact PAL frame-rate fraction through the library and convert to libretro doubles only at its API boundary. The main risks are mixing PAL and NTSC clock/APU rules, allowing resolver or host metadata to drift, and overstating uncertain hardware evidence. Protect the NTSC baseline, test all six mapper IDs and all three deliverables, call the 2C07 palette modeled, and describe the 2A07 DMA behavior as the best documented model with its unresolved bus behavior explicit.

## Key Findings

### Recommended Stack

The existing stack already fits this work. Keep the C17 core integer-only, the CMake/CTest targets and `cmake --workflow --preset ci`, the public library API, the runner, libretro adapter, and owned `palgen` path. No emulator, timing, audio, color, command-line, or test framework package is justified. See [STACK.md](STACK.md).

**Core technologies:**

- **C17 and fixed-width integer types:** implement region profiles, clocks, tables, and state identity in owned code while preserving deterministic cross-platform output.
- **CMake 3.25 and existing targets:** build the library, runner, adapter, tools, and tests without another build system or package manager.
- **Existing library/runner/libretro surfaces:** use one resolver and effective region for all hosts; preserve versioned API compatibility and convert rates to floating point only in libretro.
- **Owned C fixtures and CTest:** exercise region policy, timing, state, host parity, all six mappers, and NTSC preservation without adding a test dependency.
- **Owned palette table and `palgen`:** add a deterministic modeled 2C07 display palette at the existing conversion boundary; retain the NTSC table byte-for-byte.

Keep the core output rate at 48 kHz. Represent PAL with exact integer timing and sample remainders; the documented model yields about 959 or 960 samples per PAL frame. The core should expose `322445/6448` Hz, about 50.007 Hz, rather than rounding to 50 Hz.

### Expected Features

“Must have” items are the approved RM-005 contract, not new scope. [FEATURES.md](FEATURES.md) supplies the detailed acceptance landscape.

**Must have (table stakes):**

- One region resolver: honor explicit NES 2.0 NTSC/PAL; default multi-region and legacy iNES to NTSC; allow host NTSC/PAL preference only for those ambiguous cases; refuse Dendy.
- A coherent PAL CPU, PPU, and APU profile, with PAL frame/noise/DMC timing and all six currently supported mapper IDs (0, 1, 2, 3, 4, 7); keep existing NTSC output behavior unchanged.
- Region in machine-state identity: same-region continuation succeeds, cross-region state load fails before applying any state.
- Matching library, runner, and libretro region selection/reporting, including exact native frame-rate metadata and the correct libretro region value.

**Should have (distinguishing details within scope):**

- A deterministic modeled 2C07 palette with PAL red/green emphasis mapping and an explicit color-accuracy caveat.
- The best source-backed 2A07 DMC DMA model, with the evidence tier and unresolved later-cycle bus behavior disclosed.
- Mapper and host parity evidence that makes the selected region and its timing visible in failures.

**Defer (outside this milestone):**

- Dendy, new mapper families, Playstead/libfuturo integration, device/window code, unrelated scene-fault work without a confirmed core cause, and universal hardware or color accuracy claims.
- Broad palette calibration or physical 2A07 DMA claims; the current evidence does not establish either.

### Architecture Approach

Add one load-time resolver and store only the effective region on the instance; derive all other timing values from a static immutable profile. Keep the current CPU, bus, PPU, APU, mapper, audio synthesis, palette-conversion, serializer, runner, and libretro modules shared. The profile changes CPU/bus cadence, PPU dots/raster, APU tables and sample scheduling, but does not create a second PAL core. Keep native pixels and raw emphasis separate from display RGB conversion. The approved contract and this structure are described in [ARCHITECTURE.md](ARCHITECTURE.md).

**Major components:**

1. **Cartridge resolver and profile:** choose region before cartridge replacement or machine reset; single-region NES 2.0 declarations are authoritative and ambiguous/legacy input follows NTSC default plus eligible host preference.
2. **Shared timing core:** apply the selected clocks and raster through existing CPU/bus/PPU/APU seams; retain all six mapper implementations and their CPU-cycle semantics.
3. **PAL DMA path:** model the documented 2A07 opcode-fetch/SYNC halt behavior at the shared CPU/bus seam; keep uncertain later-cycle bus details out of hardware-accuracy claims.
4. **State and host boundaries:** serialize effective region and validate before state application; have runner and libretro obtain selection and rates from the same instance information.
5. **Display palette:** select a modeled PAL table only during RGB conversion, retaining native frame representation and NTSC palette data.

### Critical Pitfalls

1. **Mixing PAL cadence with NTSC clocks or rounding to 50 Hz** — use integer profile values and exact rate fractions; preserve sample remainders and test long-run timing without float clocks.
2. **Changing CPU speed but keeping NTSC APU periods or phase rules** — explicitly select PAL frame/noise/DMC tables and preserve event ordering; protect existing NTSC tables and hashes.
3. **Presenting the 2A07 DMA model as proven silicon behavior** — implement only the selected source-backed observable rule, include a competing-model control, and retain the unresolved no-op-cycle address/behavior limit.
4. **Resolver, override, or host metadata drift** — resolve once at load, test policy through library/runner/adapter, reject Dendy and disallowed overrides, and verify failed loads leave the current instance intact.
5. **Accepting cross-region states or changing NTSC output** — validate region and cartridge before any apply pass; prove cross-region rejection is atomic and keep NTSC frame/audio/palette evidence protected.

Other required checks include PAL PPU OAM refresh, the chart's 239 visible PAL lines against the existing 240-row output contract, all six mapper families (especially MMC3 timing), modeled palette emphasis behavior, rights-cleared/owned fixtures, and accurate host AV metadata. Do not add ROM bytes without the repository's license and manifest requirements. Details and phase map are in [PITFALLS.md](PITFALLS.md).

## Implications for Roadmap

RM-005 is already approved and committed. The following are suggested planning blocks inside that scope; the roadmapper should choose the concrete phase count and maintain the project's rule that each behavior change lands with its automated checks and documentation.

### Phase Block 1: Region resolver and PAL timebase

**Rationale:** Every later PAL behavior depends on one effective region and a coherent profile. Set precedence and profile selection before changing subsystem timing.  
**Delivers:** Load-time shared resolver; NTSC/PAL instance profile; exact native PAL frame rate; PAL CPU/bus/PPU/APU timing, sample scheduling, and the existing six mapper families under both profiles.  
**Addresses:** Region policy, full PAL profile, mapper coverage, NTSC preservation, and exact timing.  
**Avoids:** Resolver divergence, rounded cadence, mixed clock domains, stale APU tables, and treating a line-count change as the whole PPU change.

### Phase Block 2: 2A07 DMA model and state identity

**Rationale:** PAL DMC DMA is a distinct high-risk bus interaction; it depends on the shared scheduler and must have observable evidence. State must also distinguish otherwise identical cartridges running under different clocks.  
**Delivers:** The selected source-backed opcode-fetch/SYNC halt model with a bounded owned bus fixture; effective-region state identity; same-region continuation and atomic cross-region refusal. Decide state-format and behavior-revision changes from the actual serialized meaning.  
**Addresses:** PAL DMA, machine-state compatibility, deterministic continuation.  
**Avoids:** Claiming physical DMA proof, partial state application, and assuming that the already-reserved region byte is correctly populated/validated.

### Phase Block 3: Runner, libretro, and AV reporting

**Rationale:** Once the core resolves and runs the profile, all existing deliverables must expose the same choice without implementing separate policy.  
**Delivers:** Runner preference/diagnostics; load-time libretro preference; active region from `retro_get_region()`; accurate PAL/NTSC AV timing and unchanged default behavior.  
**Uses:** Existing public API, CLI, core options, and adapter lifecycle.  
**Avoids:** Live timing swaps with stale host metadata, rounded FPS, API/config incompatibility, and deliverable-specific region rules.

### Phase Block 4: Modeled 2C07 palette and integrated regression

**Rationale:** Palette selection is downstream of the resolved region and belongs at the existing display-conversion boundary; its confidence limit is different from timing correctness.  
**Delivers:** Deterministic modeled 2C07 table and emphasis mapping, palette/API checks, final cross-deliverable parity, NTSC frame/audio/palette protection, and updated README/public-header comments.  
**Addresses:** PAL display output, NTSC preservation, and full RM-005 evidence.  
**Avoids:** Calling modeled RGB values canonical, changing native pixel/hash meaning, or allowing the PAL table to alter NTSC output.

### Phase Ordering Rationale

- Region resolution and integer timing precede DMA, state, palette, and host reporting because they all consume the same effective profile.
- Keep the 2A07 DMA question in a bounded slice: it has a weaker evidence base than clock/APU tables and should not expand into unrelated DMA investigation.
- State identity and host reporting follow the profile so they report and validate the actual selected machine rather than duplicate policy.
- Palette conversion follows the profile but stays separate from native emulation pixels; it is independently testable and carries a separate accuracy caveat.
- Test all six accepted mapper IDs and preserve NTSC output throughout the work; do not postpone those checks to a broad final “PAL works” claim.

### Research Flags

Phases likely needing deeper research during planning:

- **Phase Block 1:** Resolve how PAL's 239-line picture maps to the fixed 240-row buffer and confirm 2C07 OAM refresh/vblank/pre-render behavior from the pinned preparation references before acceptance is written.
- **Phase Block 2:** Yes. The exact 2A07 halt mechanism and later no-op-cycle bus behavior remain unsettled; planning should bound evidence to the selected documented model and its observable consequences. Do not treat the forum trace report as primary proof.
- **Phase Block 3:** No broad external research is indicated. Inspect current size-tagged API, state compatibility, and libretro option lifecycle; the existing code and official interface docs establish the integration patterns.
- **Phase Block 4:** Limited research only: carry forward the named 2C07 palette model parameters and emphasis mapping, and preserve the caveat that no universal measured RGB reference was established.

### Confidence Assessment

| Area | Confidence | Notes |
|------|------------|-------|
| Stack | HIGH | Existing C17/CMake/API/adapter seams were inspected and already support the no-dependency approach. Hardware details do not change the stack decision. |
| Features | MEDIUM | RM-005 boundaries are explicit; most hardware behavior comes from cross-checked T2 community references, not Nintendo primary specifications. |
| Architecture | MEDIUM | Current code paths and timing seams are directly inspected; PAL constants and behavior rely on community documentation, with open raster and DMA details. |
| Pitfalls | MEDIUM | Timing, API, and integration risks have clear prevention strategies; exact DMA behavior and canonical palette values remain low-confidence. |

**Overall confidence:** MEDIUM.

#### Gaps to Address

- **2A07 DMA later-cycle behavior:** best available model waits for an instruction opcode fetch/SYNC, but exact later no-op-cycle address/behavior and physical mechanism remain unresolved. Keep claims bounded and treat stronger proof as future evidence, not an acceptance prerequisite added silently.
- **2C07 palette accuracy:** the referenced table is a generated model, not a calibrated universal analog output. Record the model parameters and describe rendered RGB as modeled.
- **PAL raster/output row:** pin how the documented 239-line picture is represented in the fixed 240-row output before writing image-based expected results.
- **State version semantics:** current work records a reserved region byte and v7 artifacts report format 6/behavior revision 14. Verify that byte's current read/write validation and make a deliberate format/revision choice; architecture research recommends format 7/revision 15 if serialized interpretation changes, but this is not an already-approved version decision.
- **Override lifecycle and fixtures:** latch region preference at content load so clocks and host metadata cannot change mid-machine; use owned generated fixtures unless a test ROM is confirmed rights-cleared and independently useful.

## Sources

### Primary (HIGH confidence)

- [Official Libretro core-development documentation](https://docs.libretro.com/development/cores/developing-cores/) and the vendored [`libretro.h`](../../../libretro/libretro.h) — AV metadata, region reporting, load lifecycle, and serialization interface.
- [Official CMake C17 and library-target documentation](https://cmake.org/cmake/help/v3.25/prop_tgt/C_STANDARD.html), [`add_library`](https://cmake.org/cmake/help/v3.25/command/add_library.html), [`C_STANDARD_REQUIRED`](https://cmake.org/cmake/help/latest/prop_tgt/C_STANDARD_REQUIRED.html), and [`C_EXTENSIONS`](https://cmake.org/cmake/help/latest/prop_tgt/C_EXTENSIONS.html) — current build language and target model.
- Local [PROJECT.md](../../PROJECT.md), [PRODUCT-ROADMAP.md](../../PRODUCT-ROADMAP.md), current [public API](../../../include/nesturbator.h), and the implementation files enumerated in [ARCHITECTURE.md](ARCHITECTURE.md) — approved RM-005 scope and current owned-code seams.

### Secondary (MEDIUM confidence)

- Project [preparation index](../../preparation/README.md), [CPU/APU notes](../../preparation/NES-HARDWARE-CPU-APU.md), [PPU/cartridge notes](../../preparation/NES-HARDWARE-PPU-CARTRIDGE.md), [architecture notes](../../preparation/ARCHITECTURE.md), and [libretro/runner notes](../../preparation/LIBRETRO-AND-RUNNER.md) — pinned source map, current time model, existing outputs, and documented open questions.
- NESdev [NES 2.0](https://www.nesdev.org/wiki/NES_2.0), [cycle reference chart](https://www.nesdev.org/wiki/Cycle_reference_chart), [CPU variants](https://www.nesdev.org/wiki/CPU_variants), [APU noise](https://www.nesdev.org/wiki/APU_Noise), [APU DMC](https://www.nesdev.org/wiki/APU_DMC), [DMA](https://www.nesdev.org/wiki/DMA), [DPCM conflict](https://www.nesdev.org/wiki/DPCM_conflict), [OAM](https://www.nesdev.org/wiki/OAM), and [PPU palettes](https://www.nesdev.org/wiki/PPU_palettes) — community hardware descriptions, checked 2026-11-01; not direct hardware proof.

### Tertiary (LOW confidence)

- [NESdev PAL DMA behavior discussion](https://forums.nesdev.org/viewtopic.php?p=300930) — reported trace interpretation supporting the SYNC-halt model; raw primary trace evidence is not published in the discussion.

---
*Research completed: 2026-11-01*  
*Ready for roadmap: yes*
