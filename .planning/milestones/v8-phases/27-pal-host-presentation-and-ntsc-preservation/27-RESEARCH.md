# Phase 27: PAL host presentation and NTSC preservation - Research

**Researched:** 2026-11-02
**Domain:** C emulator host API, PAL display-palette modeling, AV metadata, deterministic regression protection
**Confidence:** MEDIUM

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

### Runner profile reporting
- **D-01:** Add an explicit `--info` report for the runner. It reports the *resolved* region plus the exact frame-rate and sample-rate fractions obtained from the active instance; it must not merely echo the requested `--region` preference.
- **D-02:** Keep the existing stdout output unchanged unless `--info` is requested. This preserves scripts and tests that consume frame, movie, hash, and state lines. The report should use a stable, parseable line; exact fractions are authoritative, and any decimal form is supplementary.

### Shared AV and display contracts
- **D-03:** Keep the library's existing separate `nesturbator_get_region()` and `nesturbator_get_info()` queries. The latter carries exact integer rate fractions. Do not add a duplicate region field or change the public struct layout for this phase.
- **D-04:** Derive libretro's `retro_get_region()` and AV rates from the same loaded library instance. Convert the exact frame-rate fraction to the API's `double` only at the libretro boundary; report 48,000 Hz audio and the loaded game's frame rate. Do not round PAL to 50 Hz or trigger runtime AV reconfiguration for a region already fixed at content load.
- **D-05:** Select the modeled PAL display table through the existing `nesturbator_get_palette(inst, ...)` conversion boundary using the effective region. Keep native index/emphasis pixels canonical for hashes, use the owned offline palette-generation path, map PAL emphasis as specified by the selected model, and preserve the existing NTSC table byte-for-byte. Describe PAL RGB as a named model, not calibrated universal or hardware-exact output.
- **D-06:** Protect the established NTSC frame, audio, palette, callback, and regression outputs across all six mapper IDs. Do not silently re-pin an NTSC baseline; any intentional change needs a separate evidence-based justification.

### the agent's Discretion
- Choose the exact `--info` line spelling and placement using existing runner conventions, while keeping it opt-in and retaining the exact fractions.
- Extend the owned palette-generation and shared host-conversion code with the smallest local change. Keep native pixel meaning and NTSC table bytes fixed, add no dependency, and update README/public-header/API comments with the behavior.
- Use owned or permitted fixtures and existing test helpers for palette mapping, loaded-game AV reporting, cross-deliverable parity, and NTSC regressions. Run the repository's required local workflow during implementation under the standing limits.

### Reviewed Todos (not folded)
- `2026-10-26-make-runner-signal-checks-deterministic.md` — runner signal determinism is unrelated to PAL presentation and AV reporting.
- `2026-10-26-add-reproducible-owned-property-tests.md` — general property-test expansion is outside the approved phase; add only the focused owned fixtures needed for VID-01, AV-01, and NTSC-01.
- `2026-10-26-evaluate-scoped-nightly-mutation-testing.md` — general nightly mutation testing is unrelated to this phase and has no demonstrated PAL-specific need.

### Deferred Ideas (OUT OF SCOPE)

#### Reviewed Todos (not folded)
- Deterministic runner signal checks — separate runner testing work, not required to expose PAL profile timing.
- Reproducible owned property tests — broad test infrastructure proposal; phase-specific fixtures remain in scope.
- Scoped nightly mutation testing — no demonstrated PAL-specific need.
- Calibrated/universal PAL color claims or direct hardware color qualification — excluded by VID-01 and current evidence.
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| VID-01 | A host can obtain a deterministic PAL display palette using the named modeled 2C07 table and emphasis mapping; documentation identifies it as a model, not universally calibrated or hardware-exact color output. [VERIFIED: .planning/REQUIREMENTS.md:19] | Extend the instance-aware palette getter and offline generator; assert every palette/emphasis index, repeatability, PAL emphasis ordering, and unchanged NTSC entries. |
| AV-01 | The library, runner and libretro adapter use the same resolved region; the library and runner expose the selected profile and its timing, and libretro reports the matching NTSC/PAL region and accurate loaded-game frame and audio rates. [VERIFIED: .planning/REQUIREMENTS.md:23] | Query active region and exact info from the same loaded instance; add opt-in runner reporting; test libretro's converted doubles after load/reload. |
| NTSC-01 | Adding PAL leaves existing NTSC behavior and output hashes unchanged across all six supported mapper families, with existing NTSC regression evidence retained and any intentional output change separately justified. [VERIFIED: .planning/REQUIREMENTS.md:28] | Keep existing frame/audio/hash fixtures and NTSC table fixed; run mapper-scoped NTSC controls and do not update baselines to accommodate unexplained changes. |
</phase_requirements>

## Summary

Phase 27 is the final presentation and host-reporting layer over the region profile completed in Phases 25–26. The loader already resolves and stores one effective region per instance; the public API exposes that region separately from exact rational AV rates. Keep this shared source of truth and use it for runner reporting, libretro AV metadata, and display-table selection. [VERIFIED: src/cartridge.c:233-253] [VERIFIED: src/cartridge.c:343-347] [VERIFIED: src/cartridge.c:417-421] [VERIFIED: include/nesturbator.h:265-274] [VERIFIED: include/nesturbator.h:477-480]

The main implementation gap is palette selection: nesturbator_get_palette currently ignores its instance and copies only the NTSC table, while tools/palgen is an owned offline generator and host/convert.c already converts native pixels using a supplied lookup table. Add PAL table generation and select it at the existing getter boundary. Keep native index/emphasis pixels and their hashes unchanged; describe PAL colors as the chosen modeled 2C07 decode. [VERIFIED: src/palette.c:4-17] [VERIFIED: tools/palgen/palgen.c:1-9] [VERIFIED: host/convert.c:8-16] [CITED: https://www.nesdev.org/wiki/PPU_palettes] [CITED: https://www.nesdev.org/wiki/PAL_video]

**Primary recommendation:** implement only the three agreed output seams: opt-in runner profile reporting, libretro region/AV values from its active instance, and an instance-selected modeled PAL table generated by the existing offline tool. Keep the NTSC table and existing native frame/audio/hash baselines fixed. [VERIFIED: 27-CONTEXT.md:16-29] [VERIFIED: .planning/research/milestone-v8-pal/SUMMARY.md:99-104]

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| Resolve content and preference | API / Backend (library loader) | Runner and libretro host | The loader validates the image, resolves its effective region, and stores it on the active instance; both hosts pass preferences into that loader. [VERIFIED: src/cartridge.c:233-253] [VERIFIED: runner/main.c:1091-1099] [VERIFIED: libretro/libretro.c:471-475] |
| Expose timing and region | API / Backend (library API) | Runner and libretro host | The library supplies exact fractions and a distinct region query; libretro converts the same instance's values at its API boundary. [VERIFIED: src/instance.c:49-63] [VERIFIED: libretro/libretro.c:273-297] [VERIFIED: libretro/libretro.c:523-529] |
| Select display RGB | API / Backend (palette getter) | Shared runner/libretro conversion | The getter owns region-specific table selection; shared host conversion maps canonical native pixels through that selected table. [VERIFIED: src/palette.c:4-17] [VERIFIED: host/convert.c:8-16] |
| Preserve NTSC behavior | API / Backend plus all three deliverable tests | CI hash comparison | Existing NTSC frame and audio hashes are recorded for games across the supported mapper families and compared with generated output. [VERIFIED: tests/runner/hashes.txt:1-70] [VERIFIED: tests/CMakeLists.txt:686-705] |

## Project Constraints (from AGENTS.md)

- Keep emulation and test code owned in this repository; libretro.h is the only vendored file, the core links only C memory functions, and the phase stays within the library, runner, and libretro adapter without window/device code. [VERIFIED: AGENTS.md:26-40]
- Use only hardware documentation and test behavior; do not inspect or retain GPL/LGPL emulator source. [VERIFIED: AGENTS.md:45-47]
- Keep the core integer-only and per-instance deterministic. A host-boundary double conversion is already part of the adapter contract. [VERIFIED: AGENTS.md:48-49] [VERIFIED: .planning/preparation/ENGINEERING.md:23]
- Every behavior change requires an owned regression, README and public-header comment updates, and the local cmake --workflow --preset ci workflow. [VERIFIED: AGENTS.md:50-53]
- Prefer small owned helpers and no new dependency unless a demonstrated need cannot be met by existing code, the C standard library, or the platform. [VERIFIED: AGENTS.md:74-82]
- Keep implementation within the approved phase and stop at its boundary. Local workflow authority is capped at 30 minutes with existing tighter limits preserved; hosted workflows, pushes, PRs, merges, and releases are outside this authority. [VERIFIED: AGENTS.md:54-59] [VERIFIED: AGENTS.md:108-120]
- The rolling product-roadmap refresh rule applies to new-milestone and complete-milestone steps, not this phase research artifact. [VERIFIED: AGENTS.md:61-70]

## Standard Stack

### Core

| Library / Tool | Version | Purpose | Why Standard |
|----------------|---------|---------|--------------|
| Owned C implementation | C17 | Extend the palette getter and keep the three existing deliverables. | The project already builds its core and host adapters as C17 and requires owned code. [VERIFIED: .planning/preparation/ENGINEERING.md:13-14] [VERIFIED: AGENTS.md:37-40] |
| CMake + CTest | CMake 3.25 minimum | Build and run existing focused checks and the integrated workflow. | Existing project presets use CTest without a test-framework dependency. [VERIFIED: .planning/preparation/ENGINEERING.md:35-46] [VERIFIED: .planning/preparation/ENGINEERING.md:52-53] |
| Existing tools/palgen and shared host conversion | In-tree | Generate checked-in deterministic palette data offline and convert native frames for hosts. | This is already the project's owned table-generation and conversion path; the generator is host-only. [VERIFIED: tools/palgen/palgen.c:1-9] [VERIFIED: host/convert.c:8-16] |

### Supporting

| Library / Tool | Version | Purpose | When to Use |
|----------------|---------|---------|-------------|
| Libretro API | Vendored libretro.h | Publish loaded-game region and AV timing to the frontend. | Use existing callbacks and structs; do not add runtime reconfiguration for a profile fixed at content load. [VERIFIED: libretro/libretro.h:442-444] [VERIFIED: libretro/libretro.c:273-297] [VERIFIED: 27-CONTEXT.md:20-24] |

### Alternatives Considered

| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| Existing owned palette generator | Add a third-party palette/color package | No new package is needed to generate static table data; a package would add dependency surface without resolving the model's evidence limit. [VERIFIED: .planning/research/milestone-v8-pal/STACK.md:23] [VERIFIED: .planning/research/milestone-v8-pal/STACK.md:40] |
| Existing region and info queries | Add a region field to nesturbator_info | The locked decision keeps the public struct layout and exposes region separately, avoiding duplicate state. [VERIFIED: 27-CONTEXT.md:20-22] |

No external packages are recommended, so the Package Legitimacy Audit is not applicable for this phase. [VERIFIED: 27-CONTEXT.md:23-29]

## Architecture Patterns

### System Architecture

This flow uses the single load-time effective region stored by the library; exact AV queries, runner/libretro reporting, and palette selection then read that instance. [VERIFIED: src/cartridge.c:343-347] [VERIFIED: src/instance.c:49-63] [VERIFIED: libretro/libretro.c:273-297] [VERIFIED: src/palette.c:4-17]

~~~mermaid
flowchart LR
    C[Content + eligible preference] --> L[Shared library loader]
    L --> I[Active instance: effective region]
    I --> Q[region + exact AV fractions]
    Q --> R[Runner info report]
    Q --> A[Libretro AV + region callbacks]
    I --> P[Instance-selected display table]
    T[Owned offline palette generator] --> P
    N[Native index + emphasis frame] --> H[Shared host conversion]
    P --> H
    N --> NH[Native frame/hash path]
~~~

The runner forwards its preference through nesturbator_load_cartridge_region(); after successful load, it can report the instance's resolved region and exact rates returned by nesturbator_get_info(). Existing frame/hash stdout should remain unchanged unless the new info option is present. [VERIFIED: runner/main.c:409-421] [VERIFIED: runner/main.c:1091-1100] [VERIFIED: 27-CONTEXT.md:16-22]

Libretro already reads its preference during retro_load_game(), loads the same core instance, refreshes its palette after successful load, and derives retro_get_system_av_info() and retro_get_region() from that instance. Retain this lifecycle: the frontend queries AV data after valid content load, and the selected profile remains fixed until reload. [VERIFIED: libretro/libretro.c:471-493] [VERIFIED: libretro/libretro.c:273-297] [VERIFIED: libretro/libretro.c:523-529] [CITED: https://docs.libretro.com/development/cores/developing-cores/]

### Recommended Project Structure

Reuse the current owners: include/nesturbator.h for public contracts, src/palette.c for table selection, the existing palette data source and tools/palgen/palgen.c for offline generation, host/convert.c for RGB conversion, runner/main.c for CLI reporting, and libretro/libretro.c for frontend metadata. Keep documentation updates in the README, API guide, runner guide, and testing guide. [VERIFIED: 27-CONTEXT.md:61-69] [VERIFIED: src/palette.c:1-17] [VERIFIED: tools/palgen/palgen.c:1-9] [VERIFIED: host/convert.c:1-16]

### Pattern 1: Read the loaded instance, not the preference

**What:** Resolve region once during load; query region and rational AV rates from the resulting instance. [VERIFIED: src/cartridge.c:233-253] [VERIFIED: src/instance.c:49-63]

**When to use:** Runner info reporting, libretro AV timing and region, or palette table selection. [VERIFIED: 27-CONTEXT.md:16-24]

The public contract currently states:

~~~c
uint32_t width;   /* pixels per row of the video output: 256 */
uint32_t height;  /* rows of the video output: 240 */
uint32_t fps_num; /* NTSC: 39375000 / 655171; PAL: 322445 / 6448 */
uint32_t fps_den;
uint32_t sample_rate_num; /* audio samples per second: 48000 / 1 */
uint32_t sample_rate_den;
~~~

[VERIFIED: include/nesturbator.h:265-274]

The region enum is already separate:

~~~c
enum nesturbator_region {
    NESTURBATOR_REGION_NTSC = 0,
    NESTURBATOR_REGION_PAL = 1
};
~~~

[VERIFIED: include/nesturbator.h:180-184]

At the libretro boundary, keep the existing one-way conversion from rational values to the API's doubles:

~~~c
if (ni.fps_den != 0u) {
    info->timing.fps = (double)ni.fps_num / (double)ni.fps_den;
}
if (ni.sample_rate_den != 0u) {
    info->timing.sample_rate = (double)ni.sample_rate_num / (double)ni.sample_rate_den;
}
~~~

[VERIFIED: libretro/libretro.c:291-296]

The vendored Libretro region constants are:

~~~c
#define RETRO_REGION_NTSC  0
#define RETRO_REGION_PAL   1
~~~

[VERIFIED: libretro/libretro.h:442-444]

### Pattern 2: Keep display conversion downstream of native pixels

**What:** Preserve native palette index plus raw emphasis bits as the hash input; let the selected 512-entry display table supply RGB at the existing getter and conversion boundary. [VERIFIED: include/nesturbator.h:278-295] [VERIFIED: .planning/preparation/NES-HARDWARE-PPU-CARTRIDGE.md:75-76] [VERIFIED: host/convert.c:8-16]

**When to use:** Adding modeled PAL display colors without changing emulated PPU values or the 256×240 buffer contract. [VERIFIED: 27-CONTEXT.md:20-24] [VERIFIED: include/nesturbator.h:265-274]

Name the target in code comments and docs as the NESdev 2C07 wiki palette model; the page identifies a pally v0.24.1 generated model. Reproduce its stated 2C07 phase/colorburst interpretation and PAL red/green emphasis mapping in the owned generator; do not add pally as a build or runtime dependency. [CITED: https://www.nesdev.org/wiki/PPU_palettes] [VERIFIED: .planning/research/milestone-v8-pal/PITFALLS.md:23] [VERIFIED: .planning/preparation/NES-HARDWARE-PPU-CARTRIDGE.md:73-76] [VERIFIED: tools/palgen/palgen.c:1-9]

The project preparation records that PAL swaps the red and green emphasis interpretation and that no single palette suits every title. The named table is deterministic modeled RGB, not a measured universal output or direct hardware proof. [VERIFIED: .planning/preparation/NES-HARDWARE-PPU-CARTRIDGE.md:73-76] [CITED: https://www.nesdev.org/wiki/PPU_palettes] [CITED: https://www.nesdev.org/wiki/PAL_video]

### Anti-Patterns to Avoid

- **Echoing the requested region:** explicit NES 2.0 metadata can determine a different effective region from the host preference; report the loaded instance instead. [VERIFIED: src/cartridge.c:233-253] [VERIFIED: 27-CONTEXT.md:16-22]
- **Rounding PAL to 50 Hz:** the library has an exact PAL fraction and libretro should divide it only at its API boundary. [VERIFIED: include/nesturbator.h:265-274] [VERIFIED: libretro/libretro.c:273-296]
- **Unconditionally reconfiguring libretro AV during load:** the phase locks timing at content load; current AV info is derived from the same loaded instance. [VERIFIED: 27-CONTEXT.md:20-24] [VERIFIED: libretro/libretro.c:471-493]
- **Converting PAL emphasis as NTSC:** project hardware notes record the PAL red/green swap. [VERIFIED: .planning/preparation/NES-HARDWARE-PPU-CARTRIDGE.md:73] [CITED: https://www.nesdev.org/wiki/Colour_emphasis]
- **Changing native pixels to represent display RGB:** native values remain canonical frame-hash input and the RGB path is separate. [VERIFIED: include/nesturbator.h:278-295] [VERIFIED: .planning/preparation/DECISIONS.md:34]
- **Calling the table hardware-exact or universally calibrated:** source support is a generated model and the project explicitly bounds its claim. [VERIFIED: .planning/research/milestone-v8-pal/ARCHITECTURE.md:102] [VERIFIED: .planning/research/milestone-v8-pal/PITFALLS.md:23] [CITED: https://www.nesdev.org/wiki/PPU_palettes]
- **Updating NTSC expected hashes to hide drift:** the phase decision requires established NTSC outputs to remain protected; any intentional change needs separate evidence and justification. [VERIFIED: 27-CONTEXT.md:23-24]

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Region policy | A runner or adapter resolver | nesturbator_load_cartridge_region() and the active instance | One loader already resolves and stores effective region for every deliverable. [VERIFIED: src/cartridge.c:233-253] [VERIFIED: runner/main.c:1091-1099] [VERIFIED: libretro/libretro.c:471-475] |
| Public timing rates | A second rate table in each host | nesturbator_get_info() | It returns exact regional FPS fractions and the fixed 48000 / 1 audio rate; hosts consume the same instance result. [VERIFIED: src/instance.c:49-63] [VERIFIED: include/nesturbator.h:265-274] |
| PAL RGB table | Runtime palette package or duplicate converter | Extend owned tools/palgen; select its table in nesturbator_get_palette() | Static generated data fits the existing lookup API and shared conversion routine without a new runtime dependency. [VERIFIED: tools/palgen/palgen.c:1-9] [VERIFIED: src/palette.c:4-17] [VERIFIED: host/convert.c:8-16] |
| Hash semantics | RGB hashing or palette-dependent native frames | Preserve native frame hashes; check RGB table/conversion separately | The public header calls native pixels canonical for frame hashes and separates display conversion. [VERIFIED: include/nesturbator.h:293-295] |

**Key insight:** region is a property of the loaded instance; timing and display table are downstream reads of that same property. Duplicating region policy or moving palette selection into PPU pixel generation creates extra ways for the three deliverables to disagree. [VERIFIED: src/cartridge.c:343-347] [VERIFIED: src/instance.c:49-63] [VERIFIED: src/palette.c:4-17] [VERIFIED: .planning/preparation/DECISIONS.md:34]

## Focused Verification Contracts

These are plan inputs, not tests run during this research. Nyquist validation is explicitly disabled in project configuration, but the approved phase still needs focused owned regression coverage and the required local workflow. [VERIFIED: .planning/config.json:21-26] [VERIFIED: AGENTS.md:50-53]

| Contract | Existing seam/evidence | Focused addition or assertion |
|----------|------------------------|-------------------------------|
| Library exposes same effective region and exact timing | Header defines separate region and info queries; loader stores region on the instance. [VERIFIED: include/nesturbator.h:265-274] [VERIFIED: include/nesturbator.h:477-480] [VERIFIED: src/cartridge.c:343-347] | For NTSC and PAL loaded content, query both functions and assert exact fractions; leave the info struct layout unchanged. |
| Runner info reports resolved data and preserves stdout compatibility | Runner already parses and forwards --region; ordinary frame/hash output is printed in its existing loop. [VERIFIED: runner/main.c:409-421] [VERIFIED: runner/main.c:1091-1100] [VERIFIED: runner/main.c:1277-1285] | Assert the opt-in record contains resolved region and exact FPS/sample fractions for explicit, ambiguous and legacy fixtures. Assert runs without --info retain exact existing stdout for hash/movie/state consumers. |
| Libretro reports active region and loaded-game AV rates | libretro.host already runs a region/preference matrix and checks direct-library parity, exact native fractions, host FPS tolerance, fixed audio rate, callback count and runner tick parity. [VERIFIED: tests/libretro/libretro_host.c:3866-3879] [VERIFIED: tests/libretro/libretro_host.c:3881-4008] | Keep/check AV and retro_get_region() after PAL and NTSC load, preference reload, unload/no-content, and refused replacement. Ensure it remains the active instance profile rather than a copied preference. |
| PAL palette and emphasis are deterministic | Existing core.palette tests copy rules, representative NTSC colors and invariants; existing host.convert checks 9-bit indexing and pitches. [VERIFIED: tests/core/test_palette.c:36-99] [VERIFIED: tests/host/test_convert.c:29-62] | Extend palette API coverage to every palette entry/emphasis combination for PAL, test PAL red/green mapping and repeated lookup, then test selected-table conversion through the shared host path. Keep native hashes independent from RGB. |
| Owned palette generation stays reproducible | palette.regen regenerates the NTSC table and compares it byte-for-byte with its checked-in source. [VERIFIED: tests/CMakeLists.txt:2022-2029] [VERIFIED: tests/cmake/palette_regen.cmake:1-14] | Keep the existing NTSC generator output and table unchanged; add a separate PAL regeneration comparison. Retain explicit NTSC table-byte protection instead of accepting a changed reference as a new baseline. |
| NTSC frame/audio output remains fixed across all six mappers | tests/runner/hashes.txt contains NTSC PCM, transition, and frame hashes across the existing mapper-game set; CTest regenerates and compares the inventory. [VERIFIED: tests/runner/hashes.txt:1-70] [VERIFIED: tests/CMakeLists.txt:686-705] | Run existing NTSC baseline checks unchanged. Add only any focused mapper-scoped control needed to attribute drift; do not re-pin or expand general campaign/property testing. |
| Regional parity covers the three deliverables | Current host matrix compares library, runner, and adapter profile behavior, including runner ticks and adapter audio for its generated region fixture. [VERIFIED: tests/libretro/libretro_host.c:3881-4008] | Ensure required content covers mapper IDs quoted below under both region controls, with matching resolver result, deterministic native outputs, and adapter RGB converted from the library's selected palette. Existing NTSC checks remain controls. |

The project requirements list these supported mapper IDs verbatim:

> (0, 1, 2, 3, 4 and 7) [VERIFIED: .planning/REQUIREMENTS.md:17]

The public header names the same IDs:

~~~c
/* Cartridge bytes are malformed or outside a supported board profile (mappers 0 NROM, 1 MMC1,
   2 UxROM, 3 CNROM, 4 MMC3 and 7 AxROM). Every rejected image returns this status with
   the instance unchanged. */
~~~

[VERIFIED: include/nesturbator.h:160-163]

Existing NTSC RGB unit and adapter tests pin representative output samples; preserve those assertions and the full current NTSC data table. The selected PAL palette remains a model rather than an independent color-hardware oracle. [VERIFIED: tests/core/test_palette.c:66-89] [VERIFIED: tests/libretro/libretro_host.c:4427-4434] [VERIFIED: src/palette_ntsc.c:1-19] [VERIFIED: .planning/research/milestone-v8-pal/PITFALLS.md:23]

## Common Pitfalls

### Pitfall 1: Reporting requested preference as selected region

**What goes wrong:** Runner output can say PAL even if explicit NES 2.0 metadata fixes the loaded instance to NTSC. [VERIFIED: src/cartridge.c:233-253]

**How to avoid:** Query nesturbator_get_region() after the load succeeds and combine it with nesturbator_get_info() for exact rates. [VERIFIED: 27-CONTEXT.md:16-22] [VERIFIED: include/nesturbator.h:477-480] [VERIFIED: include/nesturbator.h:525-527]

### Pitfall 2: Breaking runner consumers with a new stdout line

**What goes wrong:** Existing tools consume frame, movie, hash, and state lines. [VERIFIED: 27-CONTEXT.md:18]

**How to avoid:** Gate the report behind the explicit option and make its line stable and parseable. Keep exact fractions authoritative. [VERIFIED: 27-CONTEXT.md:16-18]

### Pitfall 3: Treating 50 Hz as the PAL rate

**What goes wrong:** A rounded rate drifts from the exact loaded profile and can desynchronize host pacing. [VERIFIED: include/nesturbator.h:265-274] [CITED: https://docs.libretro.com/development/cores/developing-cores/]

**How to avoid:** Keep 322445/6448 in the library and runner report; divide only for libretro's double field. [VERIFIED: include/nesturbator.h:270-273] [VERIFIED: libretro/libretro.c:291-296]

### Pitfall 4: Calling modeled RGB a hardware measurement

**What goes wrong:** A deterministic palette can be mistaken for universal calibrated 2C07 output despite decoder and display assumptions. [VERIFIED: .planning/research/milestone-v8-pal/ARCHITECTURE.md:102] [VERIFIED: .planning/research/milestone-v8-pal/PITFALLS.md:23]

**How to avoid:** State the chosen NESdev 2C07 model and generation assumptions in generator comments and public docs. Explicitly limit claims to this model. [CITED: https://www.nesdev.org/wiki/PPU_palettes] [VERIFIED: 27-CONTEXT.md:23-24]

### Pitfall 5: Letting PAL work silently alter NTSC

**What goes wrong:** Shared generator or table-selection edits can change NTSC display output or native frame/audio hashes across existing boards. [VERIFIED: src/palette.c:4-17] [VERIFIED: tests/runner/hashes.txt:1-70]

**How to avoid:** Preserve NTSC table bytes, existing palette samples, callback outputs, frame/audio hashes, and the existing six-mapper baseline. Investigate before any baseline edit; an intentional behavior change requires separate evidence and justification. [VERIFIED: 27-CONTEXT.md:23-24] [VERIFIED: tests/core/test_palette.c:66-89] [VERIFIED: tests/CMakeLists.txt:686-705]

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| nesturbator_get_palette ignores the instance and always copies the NTSC table. [VERIFIED: src/palette.c:4-17] | Use its existing instance argument to select the modeled NTSC or PAL display table after region load. [VERIFIED: 27-CONTEXT.md:20-24] | Phase 27 | Native pixels and frame hashes stay unchanged; only RGB display conversion becomes profile-aware. [VERIFIED: include/nesturbator.h:293-295] [VERIFIED: .planning/research/milestone-v8-pal/ARCHITECTURE.md:102] |
| Runner exposes preference input and existing frame/hash output. [VERIFIED: runner/main.c:78-110] | Add the locked opt-in info report from the active instance. [VERIFIED: 27-CONTEXT.md:16-18] | Phase 27 | Existing output remains compatible by default. [VERIFIED: 27-CONTEXT.md:18] |

## Assumptions Log

No unresolved factual assumptions are needed for planning: current interfaces and values were read from source, the host contract was checked against official Libretro documentation, and the palette model has a pinned source recipe plus a complete generated-output oracle. That oracle does not turn the model into direct hardware proof. [VERIFIED: include/nesturbator.h:265-274] [CITED: https://docs.libretro.com/development/cores/developing-cores/] [CITED: https://www.nesdev.org/wiki/PPU_palettes] [CITED: https://www.nesdev.org/wiki/File%3A2C07_wiki.pal]

## Open Questions (RESOLVED)

1. **Exact info-line grammar and placement — RESOLVED:** Plan 27-01 selects `info region=<ntsc|pal> fps=<numerator>/<denominator> sample_rate=48000/1`; emit it only with `--info`, after successful setup and before existing execution output. The exact fractions remain authoritative and no-option stdout remains unchanged. [VERIFIED: 27-01-PLAN.md:78-88]

2. **How much model detail to expose in its name — resolved.**
   - The source identifies the selected output as pally v0.24.1, generated with `python3 pally.py --skip-plot -cld -ppu "2C07" -phd 4 --delay-line-filter -e -o docs/NESDev/2C07_wiki.pal`. It says the RC lowpass time constant was chosen for approximately a -5-degree hue shift per color column and that delay-line comb filtering is used. [CITED: https://www.nesdev.org/wiki/PPU_palettes] [CITED: https://www.nesdev.org/wiki/File%3A2C07_wiki.pal]
   - The published file is a complete 1536-byte RGB oracle for 512 native color/emphasis entries (SHA-256 `cb052e4ef8c738c19c2304804e3851d5afd2ba8bf68ed810cabd69de0fe9e9e5`). Selected first-group RGB values are `$00` = `626262`, `$01` = `002365`, `$06` = `540501`, `$0F` = `000000`, `$16` = `932824`, `$21` = `61A1FB`, `$26` = `E47974`, `$31` = `C4DDFF`, and `$3E` = `000000`. These values were read from the published file; the binary is not added to the repository. [CITED: https://www.nesdev.org/wiki/File%3A2C07_wiki.pal]
   - NESdev documents the PAL colorburst as phase 7, line-alternating V, and the paired hue oscillators; it documents the PAL red/green emphasis swap while blue stays in place. [CITED: https://www.nesdev.org/wiki/PAL_video] [CITED: https://www.nesdev.org/wiki/Colour_emphasis]
   - Recommendation: name it “NESdev 2C07 wiki palette model,” include pally v0.24.1 and the generation recipe in developer-facing comments/docs, and use the published output plus the project’s own tests as independent sample/golden evidence. Keep the accuracy claim explicitly model-based; the published output does not establish a physical 2C07 RGB capture or universal TV calibration. Do not add pally as a dependency or vendor its binary. [VERIFIED: 27-CONTEXT.md:23-29] [CITED: https://www.nesdev.org/wiki/File%3A2C07_wiki.pal]

## Additional palette source clarification

The earlier direct page fetch returned HTTP 403, but the NESdev search-indexed source pages and the linked `2C07_wiki.pal` file were subsequently recovered. The file page states the generator version, complete command, approximate RC hue-shift criterion, delay-line filter, and MIT-0 license for the generator. The raw output file was used only to derive its SHA-256 and representative RGB samples; it remains outside the repository. This closes the planning oracle gap without changing the owner-selected palette model or reading emulator source. [CITED: https://www.nesdev.org/wiki/PPU_palettes] [CITED: https://www.nesdev.org/wiki/PAL_video] [CITED: https://www.nesdev.org/wiki/Colour_emphasis] [CITED: https://www.nesdev.org/wiki/File%3A2C07_wiki.pal]

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|-------------|-----------|---------|----------|
| CMake | Existing configure/build/test workflow | ✓ | 4.4.3 local; project minimum 3.25 | — [VERIFIED: .planning/preparation/ENGINEERING.md:35-46] [VERIFIED: local shell cmake --version] |
| Clang C compiler | Existing C17 build | ✓ | Apple clang 21.0.0 local | Existing supported CMake toolchain selection; no new compiler dependency. [VERIFIED: .planning/preparation/ENGINEERING.md:27-31] [VERIFIED: local shell clang --version] |

No external runtime service or package is needed for this code-only phase. No build or test command was run during this research.

## Security Domain

Security enforcement is explicitly disabled in project configuration ("security_enforcement": false), so the ASVS category matrix is not applicable to this phase. [VERIFIED: .planning/config.json:49]

## Sources

### Primary: project and API contracts

- 27-CONTEXT.md, D-01–D-06 — locked host reporting, API, palette, regression, and scope decisions. [VERIFIED: 27-CONTEXT.md:16-34]
- include/nesturbator.h, src/cartridge.c, src/instance.c — public profile/rate APIs and effective region storage. [VERIFIED: include/nesturbator.h:265-274] [VERIFIED: src/cartridge.c:233-253] [VERIFIED: src/instance.c:49-63]
- src/palette.c, tools/palgen/palgen.c, host/convert.c — current table getter, owned generator, and shared conversion boundary. [VERIFIED: src/palette.c:4-17] [VERIFIED: tools/palgen/palgen.c:1-9] [VERIFIED: host/convert.c:8-16]
- libretro/libretro.c, libretro/libretro.h — active-instance AV and region mapping. [VERIFIED: libretro/libretro.c:273-297] [VERIFIED: libretro/libretro.c:523-529] [VERIFIED: libretro/libretro.h:442-444]
- tests/libretro/libretro_host.c, tests/core/test_palette.c, tests/host/test_convert.c, tests/runner/hashes.txt, tests/CMakeLists.txt — current integration and regression seams. [VERIFIED: tests/libretro/libretro_host.c:3866-4008] [VERIFIED: tests/core/test_palette.c:36-99] [VERIFIED: tests/host/test_convert.c:29-62] [VERIFIED: tests/runner/hashes.txt:1-70] [VERIFIED: tests/CMakeLists.txt:686-705]
- .planning/preparation/README.md, DECISIONS.md, ARCHITECTURE.md, NES-HARDWARE-PPU-CARTRIDGE.md, LIBRETRO-AND-RUNNER.md — indexed design basis and documented source tiers. [VERIFIED: .planning/preparation/README.md:13-40] [VERIFIED: .planning/preparation/DECISIONS.md:34-37] [VERIFIED: .planning/preparation/ARCHITECTURE.md:65-73] [VERIFIED: .planning/preparation/NES-HARDWARE-PPU-CARTRIDGE.md:70-76] [VERIFIED: .planning/preparation/LIBRETRO-AND-RUNNER.md:20-28]

### Secondary: technical documentation

- Official Libretro core development documentation — AV information is queried after valid game load; report accurate FPS/audio rates and respect fixed-rate synchronization. [CITED: https://docs.libretro.com/development/cores/developing-cores/]
- NESdev PPU palettes and PAL video — source for the named generated 2C07 model and PAL composite color assumptions. NESdev is the project's curated T2 community-reference tier, not direct hardware proof. [VERIFIED: .planning/preparation/README.md:38-40] [VERIFIED: .planning/preparation/NES-HARDWARE-PPU-CARTRIDGE.md:131-142] [CITED: https://www.nesdev.org/wiki/PPU_palettes] [CITED: https://www.nesdev.org/wiki/PAL_video]
- NESdev Colour emphasis — PAL/Dendy red-green emphasis swap reference. [VERIFIED: .planning/preparation/NES-HARDWARE-PPU-CARTRIDGE.md:73] [CITED: https://www.nesdev.org/wiki/Colour_emphasis]

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — existing C17/CMake targets, owned generator, and host converter cover the approved work without new packages. [VERIFIED: .planning/preparation/ENGINEERING.md:13-14] [VERIFIED: .planning/preparation/ENGINEERING.md:35-53] [VERIFIED: tools/palgen/palgen.c:1-9]
- Architecture: HIGH — live loader, API, runner, adapter, and conversion paths were opened and traced. [VERIFIED: src/cartridge.c:233-253] [VERIFIED: src/instance.c:49-63] [VERIFIED: runner/main.c:1091-1100] [VERIFIED: libretro/libretro.c:273-297] [VERIFIED: host/convert.c:8-16]
- PAL palette model: MEDIUM — the selected model is documented by NESdev and preparation notes but is synthesized/model-based, not a hardware RGB capture. [VERIFIED: .planning/research/milestone-v8-pal/PITFALLS.md:23] [CITED: https://www.nesdev.org/wiki/PPU_palettes]
- Pitfalls: HIGH for host/NTSC contracts, MEDIUM for palette accuracy limits — the former are directly represented by code/tests; the latter inherits the community source tier. [VERIFIED: tests/runner/hashes.txt:1-70] [VERIFIED: tests/libretro/libretro_host.c:3866-4008] [VERIFIED: .planning/preparation/NES-HARDWARE-PPU-CARTRIDGE.md:73-76]

**Research date:** 2026-11-02
**Valid until:** 2026-12-02, or sooner if the libretro API or selected palette model changes
