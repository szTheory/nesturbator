# Phase 27: PAL host presentation and NTSC preservation - Context

**Gathered:** 2026-11-02  
**Status:** Ready for planning

<domain>
## Phase Boundary

Finish PAL host presentation across the existing C library, headless runner, and libretro adapter. The three deliverables must agree on the content's resolved region and timing; libretro reports matching region and loaded-game AV rates; hosts can obtain deterministic RGB from the named modeled 2C07 palette and its emphasis mapping; and NTSC behavior and outputs remain unchanged across mapper IDs 0, 1, 2, 3, 4, and 7. Keep the 256×240 native output and existing adapter-conversion contract. Do not add Dendy, boards, consumer integrations, calibrated/universal color claims, new dependencies, or general testing/collector work.

</domain>

<decisions>
## Implementation Decisions

### Runner profile reporting
- **D-01:** Add an explicit `--info` report for the runner. It reports the *resolved* region plus the exact frame-rate and sample-rate fractions obtained from the active instance; it must not merely echo the requested `--region` preference.
- **D-02:** Keep the existing stdout output unchanged unless `--info` is requested. This preserves scripts and tests that consume frame, movie, hash, and state lines. The report should use a stable, parseable line; exact fractions are authoritative, and any decimal form is supplementary.

### Shared AV and display contracts
- **D-03:** Keep the library's existing separate `nesturbator_get_region()` and `nesturbator_get_info()` queries. The latter carries exact integer rate fractions. Do not add a duplicate region field or change the public struct layout for this phase.
- **D-04:** Derive libretro's `retro_get_region()` and AV rates from the same loaded library instance. Convert the exact frame-rate fraction to the API's `double` only at the libretro boundary; report 48,000 Hz audio and the loaded game's frame rate. Do not round PAL to 50 Hz or trigger runtime AV reconfiguration for a region already fixed at content load.
- **D-05:** Select the modeled PAL display table through the existing `nesturbator_get_palette(inst, ...)` conversion boundary using the effective region. Keep native index/emphasis pixels canonical for hashes, use the owned offline palette-generation path, map PAL emphasis as specified by the selected model, and preserve the existing NTSC table byte-for-byte. Describe PAL RGB as a named model, not calibrated universal or hardware-exact output.
- **D-06:** Protect the established NTSC frame, audio, palette, callback, and regression outputs across all six mapper IDs. Do not silently re-pin an NTSC baseline; any intentional change needs a separate evidence-based justification.

### Source-bound AccuracyCoin evidence
- **D-07:** Keep the existing 56-input AccuracyCoin fingerprint and exact-source activation gate strict. Do not exclude runner/header/test inputs or weaken the fingerprint to make the historical receipt apply to changed code.
- **D-08:** Preserve the checked-in behavior-14 receipt as historical evidence and keep the older interrupted campaign stopped and unknown. The full Phase 27 CI run on `13cd9c698ddb` did execute its configured AccuracyCoin fixture and produced a complete 126 PASS / 20 FAIL report with the same report and scoreboard hashes as the existing receipt; treat that capture as intermediate because Plans 27-02 through 27-04 change fingerprinted inputs.
- **D-09:** Rebind source-specific acceptance only after all Phase 27 fingerprinted code and validation inputs are final. Update the activation check to the final behavior revision before capturing the candidate. Refresh source revision, input fingerprint, behavior revision, runtime metadata, and artifact reference from that exact candidate report. Keep scoreboard outcomes and hashes unchanged unless the fresh report proves an output change; any changed result is a regression to investigate, not a value to silently re-pin.
- **D-10:** Plan 27-04 owns final source-evidence refresh and exact-candidate verification. After all fingerprinted code and validation inputs are frozen, update the activation check to the final behavior revision, run only the `accuracycoin.campaign` CTest fixture to capture the exact candidate report, and refresh source-bound provenance fields without changing scoreboard expectations unless that report proves an output change. Then run the bounded full workflow on the unchanged input set; it reruns the campaign fixture and validates the captured evidence. Diagnose any failure before an in-window retry. This is a new current-candidate run, not a restart or relabeling of the older stopped run.

### the agent's Discretion
- Choose the exact `--info` line spelling and placement using existing runner conventions, while keeping it opt-in and retaining the exact fractions.
- Extend the owned palette-generation and shared host-conversion code with the smallest local change. Keep native pixel meaning and NTSC table bytes fixed, add no dependency, and update README/public-header/API comments with the behavior.
- Use owned or permitted fixtures and existing test helpers for palette mapping, loaded-game AV reporting, cross-deliverable parity, and NTSC regressions. Run the repository's required local workflow during implementation under the standing limits.
- Choose the smallest plan adjustment that defers final AccuracyCoin source-identity binding until all fingerprinted Phase 27 inputs are frozen; do not broaden the work into general CI/fingerprint redesign.

### Reviewed Todos (not folded)
- `2026-10-26-make-runner-signal-checks-deterministic.md` — runner signal determinism is unrelated to PAL presentation and AV reporting.
- `2026-10-26-add-reproducible-owned-property-tests.md` — general property-test expansion is outside the approved phase; add only the focused owned fixtures needed by VID-01, AV-01, and NTSC-01.
- `2026-10-26-evaluate-scoped-nightly-mutation-testing.md` — general nightly mutation testing is unrelated to this phase and has no demonstrated PAL-specific need.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Approved scope and prior decisions
- `.planning/ROADMAP.md` §Phase 27 and §Execution and evidence boundaries — approved goal, success criteria, cross-deliverable scope, NTSC guards, and explicit stops.
- `.planning/REQUIREMENTS.md` §PAL emulation and video, §Host and machine-state behavior, and §NTSC compatibility — locked VID-01, AV-01, and NTSC-01 outcomes and evidence limits.
- `.planning/PROJECT.md` — v8 target, three deliverables, deterministic and clean-room constraints.
- `AGENTS.md` — phase workflow, no-new-dependency preference, documentation/test obligations, local verification authority, and handoff rules.
- `.planning/phases/25-region-selection-and-pal-timing/25-CONTEXT.md` — region policy, exact rational rates, fixed output contract, and prior host/API decisions.
- `.planning/phases/26-pal-dma-and-regional-state-continuation/26-CONTEXT.md` — completed region-specific state behavior and Phase 27 boundary.
- `.planning/VERIFICATION-POLICY.md` — exact-source evidence, bounded local acceptance, reuse, and retry rules.

### PAL model and source map
- `.planning/research/milestone-v8-pal/SUMMARY.md` §Phase Block 4 — modeled palette, cross-deliverable regression scope, and color-confidence limits.
- `.planning/research/milestone-v8-pal/ARCHITECTURE.md` — palette conversion boundary, exact region timing, adapter integration, and open evidence limits.
- `.planning/research/milestone-v8-pal/STACK.md` — existing tools/APIs and rationale for no runtime palette dependency.
- `.planning/preparation/README.md` — source index and reference conventions.
- `.planning/preparation/DECISIONS.md` DEC.13 and DEC.16 — native pixels remain hash input, color conversion stays outside the core, and public ABI is versioned separately.
- `.planning/preparation/ARCHITECTURE.md` §§4, 7 — native/display output contract and public API conventions.
- `.planning/preparation/NES-HARDWARE-PPU-CARTRIDGE.md` §§4, 8 — palette/emphasis evidence, modeled PAL color limits, and region timing.
- `.planning/preparation/LIBRETRO-AND-RUNNER.md` — runner output conventions and libretro AV/region contract, with pinned source references.

### Existing API, adapters, and tests
- `include/nesturbator.h` — public region/rate queries, native pixel contract, and palette getter.
- `src/palette.c` and `src/palette_ntsc.c` — current instance-aware display-table API and NTSC table that must remain byte-identical.
- `tools/palgen/palgen.c` — owned deterministic offline palette generator.
- `host/convert.c` — shared native-pixel-to-RGB conversion used by the runner and libretro adapter.
- `runner/main.c` and `docs/runner.md` — stable CLI output contracts and region preference path.
- `libretro/libretro.c` and `libretro/libretro.h` — active-region and loaded-game AV reporting contracts.
- `README.md` and `docs/api.md` — user and host-author documentation to update.
- `docs/testing.md` — current native, conversion, adapter, and PAL/NTSC evidence descriptions.
- `tests/cmake/accuracycoin_input_fingerprint.cmake` and `tests/cmake/accuracycoin_input_fingerprint_controls.cmake` — complete 56-input source identity and stale-identity control.
- `tests/cmake/accuracycoin_activation.cmake` — behavior-revision, report, and scoreboard activation contract.
- `tests/accuracy/baseline-metadata.txt` and `tests/accuracy/measurements.md` — exact-candidate source provenance and historical measured-result ledger; preserve the existing receipt when adding a new candidate record.
- `https://slsa.dev/spec/v1.2/build-provenance` — primary build-provenance reference: relevant build inputs are identified for verification. Applying that general guidance to this campaign is a project-specific inference, not a SLSA requirement.

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `nesturbator_get_region()` returns the active region; `nesturbator_get_info()` returns exact FPS and sample-rate fractions. They are already the shared source for host behavior.
- `nesturbator_get_palette(inst, ...)` returns a 512-entry lookup indexed by native palette entry plus emphasis. Its instance parameter is already reserved for region-specific display tables; it currently copies only the NTSC table.
- `tools/palgen` is an in-tree, offline host tool with deterministic output; `host/convert.c` is shared by runner dumps and libretro frames.
- Libretro already derives `retro_get_system_av_info()` from the active library instance and maps that instance to `retro_get_region()`; existing host tests exercise the adapter boundary.

### Established Patterns
- The core stores exact integer fractions; floating-point conversion belongs only at the libretro API boundary.
- Native frame hashes use the 256×240 native pixels before RGB conversion. NTSC output and lookup data are protected baselines.
- Runner stdout is consumed by hashes, movie replay, state output, and tests. A new informational line therefore stays behind an explicit option.
- The project uses owned tests and generated/permitted fixtures, updates README and public-header comments with behavior changes, and keeps runtime dependencies unchanged.
- The AccuracyCoin fingerprint includes core, runner, public-header, CMake, and selected campaign-validation inputs. With `NESTURBATOR_ACCURACYCOIN_FULL=ON`, `accuracycoin.campaign` is a CTest fixture required by activation, so the bounded local full workflow generates a report for its current candidate before the activation comparison.
- The Plan 27-01 capture is source-bound to `13cd9c698ddb` / fingerprint `c5f4c0d4…` over 56 inputs, behavior revision 17, with 22/22 pages and 146/146 scored rows complete. Its scores and report hash match the prior receipt, but later phase plans change the fingerprinted set, so it is not the final Phase 27 receipt.

### Integration Points
- `runner/main.c` queries the loaded profile after successful cartridge resolution and can report it through the opt-in information path.
- `libretro/libretro.c` refreshes the lookup table on content load and supplies AV/region callbacks from the same instance.
- Palette selection belongs at `nesturbator_get_palette()` and is consumed by `host/convert.c`; it must not alter PPU native pixel generation.
- Existing native palette, host conversion, libretro host, six-mapper, and NTSC regression tests provide extension points.

</code_context>

<specifics>
## Specific Ideas

The owner asked for recommendations backed by technical and stakeholder review, prefers a smaller dependency tree, and explicitly approved following the synthesized recommendation. Keep the CLI profile report opt-in and exact; use the existing instance-aware palette API and the named modeled 2C07 table with its documented caveat. For the fingerprint gate, keep exact source identity and bind only a final Phase 27 candidate; do not change the historical scoreboard or treat the intermediate Plan 27-01 capture as phase acceptance.

</specifics>

<deferred>
## Deferred Ideas

### Reviewed Todos (not folded)
- Deterministic runner signal checks — separate runner testing work, not required to expose PAL profile timing.
- Reproducible owned property tests — broad test infrastructure proposal; phase-specific fixtures remain in scope.
- Scoped nightly mutation testing — no demonstrated PAL-specific need.
- Calibrated/universal PAL color claims or direct hardware color qualification — excluded by VID-01 and current evidence.
- Changing AccuracyCoin fingerprint semantics or broad campaign/collector cleanup — separate CI/evidence-policy scope; do not fold it into Phase 27.

</deferred>

---

*Phase: 27-pal-host-presentation-and-ntsc-preservation*
*Context gathered: 2026-11-02*
