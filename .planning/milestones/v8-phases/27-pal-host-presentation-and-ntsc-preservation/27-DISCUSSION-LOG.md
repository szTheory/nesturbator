# Phase 27: PAL host presentation and NTSC preservation - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in `27-CONTEXT.md` — this log preserves the alternatives considered.

**Date:** 2026-11-02  
**Phase:** 27-PAL host presentation and NTSC preservation  
**Areas discussed:** Runner profile reporting; source-bound AccuracyCoin evidence

---

## Runner profile reporting

| Option | Description | Selected |
|--------|-------------|----------|
| Always emit a profile line | Makes every invocation self-describing, but changes the established stdout contract consumed by scripts and tests. | |
| Add an explicit `--info` report | Prints the resolved region and exact frame/audio rate fractions only when requested; keeps default stdout stable. | ✓ |
| No runner report | Leaves information to the library and adapter, but does not satisfy Phase 27's requirement that the runner expose its selected profile and timing. | |

**User's choice:** The owner asked us to follow the synthesized recommendations. The recommended opt-in `--info` report was selected.

**Notes:** Independent reviews of the library/API, CLI, libretro, and palette boundaries found the public API shape, libretro conversion, modeled 2C07 scope, and NTSC preservation contract already settled by Phase 25, the approved v8 requirements, and existing code. Official libretro documentation confirms that AV information is content-dependent, queried after valid content loads, and should accurately report FPS and audio sampling rate. Keep the runner report based on the resolved instance, not the input preference.

---

## Source-bound AccuracyCoin evidence

| Option | Description | Selected |
|--------|-------------|----------|
| Keep exact source binding and refresh evidence at the final candidate | Preserve all 56 fingerprinted inputs and the historical behavior-14 receipt; defer rebinding until Phase 27 code and validator inputs are frozen, then use a fresh final-candidate campaign report. | ✓ |
| Keep the old receipt and defer Phase 27 | Preserves the historic evidence untouched, but leaves this phase blocked despite a complete current-candidate campaign having already run under full CI. | |
| Exclude inputs or weaken fingerprint checks | Could hide a mismatch, but risks attributing old results to unmeasured inputs and adds general CI/evidence-policy scope. | |

**User's choice:** The owner asked for a cross-role research fan-out and instructed us to synthesize a recommendation and automatically follow it. The recommended exact-source policy was selected.

**Notes:** Three read-only GSD advisor reviews covered evidence integrity, CMake/CTest behavior, and phase scope. The full CI run executed `accuracycoin.campaign` as a CTest fixture and produced a complete report for source `13cd9c698ddb` (behavior 17; fingerprint `c5f4c0d4…/56`; 126 PASS, 20 FAIL, 5 inventory-only DRAW; 22/22 pages; 146/146 terminal rows; zero skips/timeouts). The report and scoreboard hashes match the prior receipt. The candidate is intermediate because later Phase 27 plans change fingerprinted inputs. `accuracycoin_activation.cmake` also expects behavior 14 and is itself fingerprinted; update it before final candidate measurement, then refresh only exact-candidate provenance after all inputs are frozen. Do not change scoreboard results without new evidence. Preserve the old behavior-14 receipt and the separate stop on the older interrupted campaign. General rationale: [SLSA Build Provenance v1.2](https://slsa.dev/spec/v1.2/build-provenance) describes identifying relevant build inputs for verification; its application here is an inference, not a SLSA requirement.

---

## the agent's Discretion

- Choose a stable, parseable `--info` line spelling using current runner conventions; preserve exact fractions as the authoritative values.
- Extend the owned offline palette generator and shared conversion path with no runtime dependency.

## Deferred Ideas

- Deterministic runner signal checks — separate runner testing work.
- Reproducible owned property tests — broader test-infrastructure work, with only phase-specific fixtures in scope.
- Scoped nightly mutation testing — no PAL-specific need was established.
- Calibrated/universal PAL color claims or direct hardware color qualification — outside the approved modeled-palette contract.
