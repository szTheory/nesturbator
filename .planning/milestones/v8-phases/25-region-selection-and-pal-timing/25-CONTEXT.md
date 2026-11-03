# Phase 25: Region selection and PAL timing - Context

**Gathered:** 2026-11-01  
**Status:** Ready for planning

<domain>
## Phase Boundary

Make supported PAL content run at its intended CPU, PPU, and APU cadence and pitch through the existing C library, headless runner, and libretro adapter. Resolve one effective NTSC or PAL profile under the approved metadata and host-preference policy. Cover mapper IDs 0, 1, 2, 3, 4, and 7, retain the existing 256×240 output contract, and protect existing NTSC behavior. Do not add Dendy, boards, consumer integrations, or Phase 26/27 work.

</domain>

<decisions>
## Implementation Decisions

### Host region preference
- **D-01:** The host supplies an `Auto`, `NTSC`, or `PAL` preference for each content load. The host owns persistence, including any per-game setting; the core does not add a preference store.
- **D-02:** Explicit single-region NES 2.0 metadata is authoritative. For multi-region NES 2.0 and legacy iNES, `Auto` resolves to NTSC and an explicit host preference may select NTSC or PAL. Dendy is refused; a host preference cannot treat it as PAL.
- **D-03:** Resolve the effective region during content load and hold it fixed until unload/reload. A changed host option applies to the next load, so emulation clocks and host-reported region cannot diverge during a session.
- Keep selection policy consistent across the library, runner, and libretro adapter through the existing core seams. Planning should choose the smallest API representation that fits the current size-tagged C API and adapter lifecycle; do not add a dependency or a separate region policy in each deliverable.

### PAL output and timing
- **D-04:** Preserve the public 256×240 frame-buffer contract for PAL in both the library and libretro adapter. Do not change callers to 256×239.
- **D-05:** Keep PAL's internal raster/vblank timing separate from output-buffer height. Before defining visual expectations, use pinned permitted sources and/or an owned fixture to establish how the documented 239-line PAL picture is represented at row 239. Do not guess or claim hardware verification.
- **D-06:** Use the exact integer/rational PAL cadence and retain deterministic frame/sample accounting; do not round the frame rate to 50 Hz. Select the matching PAL CPU/PPU/APU cadence and frequency tables across all six currently supported mapper families.
- Protect established NTSC frame, audio, palette, and mapper behavior throughout the phase. Phase 25 must not absorb the 2A07 DMA model or regional state continuation (Phase 26), nor PAL palette/host-presentation completion (Phase 27).

### the agent's Discretion
- Choose concrete API field names, option plumbing, and implementation slices using existing C/library/runner/libretro patterns while preserving D-01 through D-06 and the existing load-failure guarantees.
- Use only owned or permitted fixtures, preserve the clean-room and integer-only rules, and keep dependencies unchanged.

### Reviewed Todos (not folded)
- `2026-10-26-add-reproducible-owned-property-tests.md` — generic owned property-test proposal; it is not required to establish the approved PAL timing behavior.
- `2026-10-26-evaluate-scoped-nightly-mutation-testing.md` — generic mutation-testing proposal; no PAL-specific need was established.
- `2026-10-26-make-runner-signal-checks-deterministic.md` — runner signal determinism proposal; it is not part of region selection or PAL timing.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Approved scope and project constraints
- `.planning/ROADMAP.md` §Phase 25 — phase goal, requirements, success criteria, phase boundaries, and verification stops.
- `.planning/REQUIREMENTS.md` §Region selection, §PAL emulation and video, §Evidence and safety notes — locked REG-01/PAL-01 behavior and fixture/accuracy limits.
- `.planning/PROJECT.md` — core architecture, deterministic/clean-room constraints, and three deliverables.
- `AGENTS.md` — workflow, compatibility, verification, and handoff rules.

### PAL timing and raster evidence
- `.planning/research/milestone-v8-pal/SUMMARY.md` — PAL model recommendations, implementation risks, confidence limits, and open raster question.
- `.planning/research/milestone-v8-pal/ARCHITECTURE.md` — shared profile and load-time resolution approach; fixed output and PAL raster implications.
- `.planning/preparation/ARCHITECTURE.md` — integer tick unit and regional clock model.
- `.planning/preparation/NES-HARDWARE-CPU-APU.md` — regional clocks, frame/noise/DMC tables, and evidence limits.
- `.planning/preparation/NES-HARDWARE-PPU-CARTRIDGE.md` — cartridge region metadata, PAL raster/frame timing, and output/pixel conventions.
- `.planning/preparation/LIBRETRO-AND-RUNNER.md` — existing host timing/region APIs and libretro option lifecycle guidance.

### Existing public and implementation seams
- `include/nesturbator.h` — current cartridge-load API, fixed 256×240 output and timing info contracts.
- `src/cartridge.c` — header validation and cartridge-load replacement boundary.
- `src/frame.c` — frame target and deterministic sample accounting.
- `src/apu.c` — NTSC APU frequency tables and event scheduling.
- `src/ppu.c` — current visible rows, raster, vblank, and frame sequencing.
- `runner/main.c` — runner content-load and reporting path.
- `libretro/libretro.c` — adapter load, geometry, AV, and region-reporting path.
- `libretro/libretro.h` — vendored libretro API contracts.
- `README.md` — user-facing behavior documentation that must accompany behavior changes.

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `nesturbator_load_cartridge()` is the existing library content boundary; cartridge validation occurs before replacing the active cartridge.
- `nesturbator_info` already reports frame geometry, frame rate, and sample rate; the adapter derives its AV values from library information.
- The libretro adapter currently advertises no core options and reports NTSC unconditionally; region preference/reporting and final AV behavior must use the shared resolved profile within the approved phases.
- The current mapper implementations already share the CPU/bus timing path, so PAL profile timing should cover those six boards without adding mapper families.

### Established Patterns
- The core is integer-only and per-instance. Regional values and timing remainders belong to the machine instance, not global state.
- Native output is physical visible-row order, rows 0–239, at 256 pixels wide. Libretro base/max geometry is also fixed at 256×240.
- The current loader's validation-before-replacement behavior is an existing failure guarantee to preserve if the load interface changes.
- NTSC timing and APU tables are currently fixed; the project uses owned regression tests and updates README/public-header comments with behavior changes.

### Integration Points
- Region resolution joins the library cartridge-load path, then informs frame scheduling, CPU/PPU/APU timing, runner behavior, and libretro AV/region reporting.
- Existing contracts and implementation are inspected in `include/nesturbator.h`, `src/cartridge.c`, `src/frame.c`, `src/apu.c`, `src/ppu.c`, `runner/main.c`, and `libretro/libretro.c`.

</code_context>

<specifics>
## Specific Ideas

Use a host-owned per-content `Auto/NTSC/PAL` preference, resolve it once at load, and keep the public PAL frame output at 256×240. Keep the final-row treatment evidence-driven. No new consumer integration or dependency is requested.

</specifics>

<deferred>
## Deferred Ideas

- The PAL row-239 visual semantics are an in-phase research/evidence task, not a preselected pixel behavior.
- Dendy, additional boards, libfuturo/Playstead integration, 2A07 DMA, regional state continuation, palette/presentation completion, and confirmed scene-specific fixes remain outside Phase 25 under the approved v8 phase boundaries.
- The three reviewed testing todos remain deferred and were not folded into this phase.

</deferred>

---

*Phase: 25-region-selection-and-pal-timing*
*Context gathered: 2026-11-01*
