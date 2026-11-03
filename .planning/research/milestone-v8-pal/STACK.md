# Technology Stack: v8 PAL Region Support

**Project:** nesturbator  
**Researched:** 2026-11-01  
**Mode:** Feasibility  
**Verdict:** Existing C17, CMake, CTest, runner, and libretro paths are sufficient. PAL support needs no new runtime or build dependency.

## Recommendation

Keep the existing owned C implementation and CMake targets. Express region as an instance-level profile with fixed-width integer timing/audio parameters, and route the one resolved region through the existing library API, runner, and libretro adapter. Keep floating-point conversion at the libretro AV-info boundary, where it already exists; do not add a timing, audio, color, or emulator framework.

The existing core is a static C library with no linked third-party library. CMake requires 3.25, selects C17, requires that standard, and disables compiler extensions. The root project already builds the library, runner, libretro module, `palgen`, and CTest suite. The top-level-only tools and adapters are already separated from embedding consumers. These are adequate boundaries for PAL profile data and behavior changes.

## Recommended Stack

| Technology | Existing version or form | Purpose | Why it fits this scope |
|---|---|---|---|
| C17 and `<stdint.h>` | Existing required language level | Region profile, CPU/PPU/APU timing, mapper behavior, state identity | Integer ratios and fixed-width fields fit the deterministic core and existing cross-platform contract. PAL changes are modeled in owned code; no external emulation library is needed. |
| CMake | Minimum 3.25; existing presets and targets | Build the library, CLI, adapter, palette tool, and tests | Existing `STATIC`, `OBJECT`, and `MODULE` targets already represent the three deliverables and offline tools. No new build system or package manager is warranted. |
| CTest and in-repository C fixtures | Existing | Check region parsing, exact timing/audio, state compatibility, and NTSC preservation | Tests can share the current C test helpers and workflow. No test framework dependency is required. |
| Vendored `libretro.h` | Existing; sole vendored file | Expose a selectable host region and publish PAL frame rate through libretro | Core options and `retro_get_system_av_info()` are part of the existing API. The adapter already computes AV timing from the library’s exact rate fractions. `retro_get_region()` can report the selected NTSC or PAL standard; Dendy remains unsupported. |
| Existing runner and public C API | Existing | Expose region selection to non-libretro hosts and CLI users | The scope needs host override for multi-region and legacy iNES input. Add this through the project’s own API/CLI input rather than introducing a command-line or config library. Preserve the existing versioned-struct and ABI rules when choosing the API shape. |
| Owned palette source and `palgen` tool | Existing source/tool | Carry the modeled 2C07 palette and document its color-accuracy limit | Palette data and any generation changes can remain in the repository. Do not add a palette package; do not claim analog color accuracy beyond the selected model. |

## Region-Specific Stack Guidance

- Resolve a single-region NES 2.0 declaration in the core’s existing cartridge path. Use the agreed NTSC fallback for ambiguous and legacy images; allow the host override only for multi-region and legacy iNES. Keep the policy shared by the library, runner, and adapter so their behavior cannot drift.
- Represent PAL timing with integer clock/divider/frame constants in the core. The preparation notes give PAL frame rate as `322445 / 6448` Hz (about 50.007 Hz); preserve the fraction rather than rounding it to 50 Hz. The current public info structure already expresses frame and audio rates as numerator/denominator pairs, while the adapter converts them to libretro’s `double` fields. Libretro expects fixed AV rates and asks for system AV info after valid content is loaded, so report the selected region’s stable timing there.
- Retain all six accepted mapper IDs (0, 1, 2, 3, 4, 7) in the same mapper interface. Region changes affect shared CPU, PPU, APU, and bus timing; they do not require a mapper library or a separate PAL mapper stack.
- Continue to identify machine state by cartridge and region. The native state format already has a region byte and byte-wise encoding; use the project-owned serializer and its existing compatibility checks. Decide any state-format or behavior-revision change in the implementation phase, not by adding a serializer dependency.
- Model the documented PAL-specific 2A07 DMA behavior in the current CPU/bus path. The project’s hardware notes explicitly leave one halt edge unresolved, so the stack can support the model but cannot establish that hardware behavior by itself. Keep that evidence limit visible and avoid claiming universal DMA accuracy.
- Preserve the NTSC baseline by leaving the default profile and its output contract unchanged. PAL must be selected explicitly by the agreed metadata/override policy; Dendy is rejected.

## Dependency Decision

| Category | Recommendation | Alternative considered | Reason to decline alternative |
|---|---|---|---|
| Timing/audio math | Existing fixed-width integer code | Add a timing or DSP library | No external package supplies hardware truth; integer region constants and existing sample synthesis are sufficient and preserve determinism. |
| Libretro integration | Existing vendored header and callback API | Add libretro-common or another helper package | The project already implements the small adapter surface it needs, and its clean-room rule permits only `libretro.h` as a vendored file. |
| PAL palette | Owned palette table/generator | Add a color-science or palette package | Scope calls for a modeled 2C07 palette with a caveat. A package would increase the dependency surface without resolving which hardware palette is authoritative. |
| Build and tests | Existing CMake/CTest workflows | Introduce a package manager, alternate build system, or test framework | The current path builds all deliverables and cross-platform tests without a framework dependency. |

No package installation or new external source checkout is recommended. The core should continue to link only the C memory functions. The existing `palgen` host tool may retain its current host-side math-library use if needed; that is not a core runtime dependency.

## Alternatives Considered

| Category | Recommended | Alternative | Why not |
|---|---|---|---|
| Core language/runtime | C17 and owned code | Port PAL support through a third-party emulator core | Violates the clean-room/ownership direction, adds unwanted dependency weight, and does not improve source-backed behavior evidence. |
| Region input | One shared resolved region with existing public API, CLI, and libretro option surfaces | Infer region from filenames or bundle a ROM database | Filename heuristics are not the approved metadata policy; a database is unnecessary for single-region NES 2.0 and would add licensing and maintenance concerns. |
| Core pixel/audio path | Existing native pixel and integer PCM outputs, extended for the PAL profile | Add a graphics or audio device library | The host supplies presentation and devices; the core only returns frames and samples. |

## Build and Verification Path

Keep `cmake --workflow --preset ci` as the integrated local verification path, with the existing CMake/CTest targets. PAL fixtures and controls should be written in C and attached to the affected library/runner/libretro targets. Required evidence should cover the approved metadata and override cases, Dendy refusal, all six mapper IDs, NTSC output preservation, same-region state continuation, and cross-region state rejection. This is research guidance; no implementation, test run, or workflow change was performed here.

## Evidence and Confidence

**Stack recommendation confidence: HIGH.** The no-new-dependency decision follows directly from the current project build and API. The research seam classifies the fetched, cross-checked web digests as MEDIUM; they are official interface documentation, and the in-repository build/API provide direct corroboration. Hardware-specific PAL behavior confidence is separate: the project’s preparation notes cite community hardware documentation for PAL clocks and record unresolved 2A07 DMA evidence. That limit affects model correctness, not dependency selection.

| Claim | Confidence | Evidence |
|---|---|---|
| C17/CMake can continue to build the core and deliverables without a new framework | HIGH | Current [CMakeLists.txt](../../../CMakeLists.txt) sets C17, requires it, disables extensions, and declares the existing targets. Official CMake docs describe those standard properties and library target types. |
| The core can keep exact rate fractions and publish them through the adapter | HIGH | Current [public header](../../../include/nesturbator.h) carries rational AV rates; `libretro.c` converts them at the API boundary. Official Libretro documentation says AV info follows valid content load and accurate FPS/sample-rate reporting matters for synchronization. |
| Region-specific hardware values belong in owned core code, not an added library | MEDIUM | The local [CPU/APU hardware notes](../../preparation/NES-HARDWARE-CPU-APU.md) and [PPU/cartridge notes](../../preparation/NES-HARDWARE-PPU-CARTRIDGE.md) describe the PAL timing differences and explicitly flag unresolved DMA behavior. They cite NESdev community references for much of the hardware description. |
| PAL palette can be included as a modeled in-tree table/tool output | MEDIUM | Current `palette_ntsc.c` and `tools/palgen` establish the existing data path. The milestone explicitly accepts a modeled 2C07 palette with a color-accuracy caveat; exact reference color evidence remains a phase-level concern. |

## Sources

- [Official Libretro core development documentation](https://docs.libretro.com/development/cores/developing-cores/) — API lifecycle, AV info reporting, fixed rates, callback model, and canonical `libretro.h` pointer. Primary interface source; accessed 2026-11-01.
- [Official CMake 3.25 `C_STANDARD` documentation](https://cmake.org/cmake/help/v3.25/prop_tgt/C_STANDARD.html) and [`add_library`](https://cmake.org/cmake/help/v3.25/command/add_library.html), plus current [`C_STANDARD_REQUIRED`](https://cmake.org/cmake/help/latest/prop_tgt/C_STANDARD_REQUIRED.html) and [`C_EXTENSIONS`](https://cmake.org/cmake/help/latest/prop_tgt/C_EXTENSIONS.html) documentation — C17 selection and existing target forms. Primary documentation; accessed 2026-11-01.
- Local [project scope](../../PROJECT.md), [CMake build](../../../CMakeLists.txt), [public API](../../../include/nesturbator.h), and [Libretro adapter](../../../libretro/libretro.c) — direct first-party evidence of current stack and boundary behavior.
- Local [preparation index](../../preparation/README.md), [CPU/APU hardware notes](../../preparation/NES-HARDWARE-CPU-APU.md), [PPU/cartridge hardware notes](../../preparation/NES-HARDWARE-PPU-CARTRIDGE.md), and [engineering/build notes](../../preparation/ENGINEERING.md) — project-curated design basis. The hardware files identify NESdev Wiki evidence as T2 and mark the PAL DMA edge unresolved; these are not treated as official vendor specifications.
