# Phase 04: Sound - Context

**Gathered:** 2026-10-08
**Status:** Planning complete; ready to execute

<domain>
## Phase Boundary

Make NTSC mapper-0 NROM games produce deterministic NES sound through the existing C library, headless runner, and libretro adapter. Deliver SND-01 through SND-04: documented output sequences for the two pulse channels, triangle, noise, and DMC; recorded game-audio hashes and equal libretro samples; separate hashes for the APU level-transition stream and final PCM; the six scoped AccuracyCoin APU passes; and the five-tone spectral check. The phase does not add expansion audio, PAL/Dendy support, audio devices, or a GUI.

</domain>

<decisions>
## Implementation Decisions

### Scope and compatibility
- **D-01:** Treat ROADMAP Phase 04 and SND-01–SND-04 as the complete product scope. Add recorded audio hashes for all three licensed NROM games selected in Phase 03 (Nesteroids, Double Action Blaster Guys, and RHDE: Furniture Fight). Every added ROM or binary fixture must have an allowed licence, provenance, pin, and manifest hash.
- **D-02:** Preserve the public frame API, fractional 48 kHz sample cadence, and the no-cartridge test-card-and-silence behavior. Cartridge audio fills the existing caller-owned mono `int16_t` buffer. The libretro adapter continues to duplicate each mono sample to left and right.
- **D-03:** Keep the core integer-only and deterministic, with all mutable emulation and synthesis state in the instance. Write the APU, tests, and public-header/README documentation in this phase. Prefer existing code and a small owned implementation over another dependency; do not open or copy GPL/LGPL emulator source.

### Audio model and output
- **D-04:** Carry forward DEC.14 and the architecture contract: model the NTSC RP2A03G; mix the two pulse channels, triangle, noise, and DMC with the checked-in exact integer table; use the documented fixed-point band-limited synthesis and NES output filters; output mono signed 16-bit PCM at 48,000 Hz by default. The sample-rate fraction and carried remainder keep 798/799 samples per nominal NTSC frame without drift.
- **D-05:** Preserve both hash layers: hash the ordered `(CPU cycle, mixed level)` transitions before synthesis as the logic-level signal, and hash the final 16-bit PCM separately. `--hash-audio` reports both values as SND-02 requires. PCM changes from synthesis/filter adjustments must not obscure whether the APU's transition stream changed.
- **D-06:** Treat the specified synthesis recipe and quality threshold as locked inputs: integer lookup tables, 16 taps, 32 phases, Q15 rows summing to 32768, and a largest non-harmonic peak below 16 kHz under -80 dB for the five reference tones. Use the pulse periods 100, 40, 12, and 8 plus the triangle period 1 defined by the preparation reference.

### Automated acceptance
- **D-07:** Run recurring, machine-verifiable audio checks through `cmake --workflow --preset ci` on the existing six platform/architecture legs wherever applicable: per-channel output sequences, licensed-game audio hashes, transition and PCM hash stability, libretro sample equality, the six named AccuracyCoin tests, and the spectral threshold. Extend the existing CTest and libretro host seams. Keep the hosted RetroArch smoke test's device-independent setup; do not require a physical audio device or listening session.
- **D-08:** Keep the phase free of owner UAT. The project's existing sample-level, hash, scoreboard, and spectral assertions are the acceptance evidence; audio-device behavior belongs to the frontend and is outside this core's deliverables.

### the agent's Discretion
- Choose internal C structs, file/module boundaries, event scheduling, and bounded test implementation details that satisfy the locked hardware behavior and existing instance/bus/API patterns.
- Use the cited hardware documentation and public test behavior to resolve cycle-level details, including frame-counter timing, length-counter edges, DMC DMA, and register side effects. Do not ask the owner to choose internal architecture or reopen already-settled product contracts.
- Keep generated coefficient and mixer data reproducible and checked in; choose the smallest owned generator or verification mechanism that fits the existing CMake/CTest patterns.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Scope and settled product decisions
- `.planning/ROADMAP.md` §Phase 4 — phase goal, success criteria, requirements, and canonical references.
- `.planning/REQUIREMENTS.md` §Sound — SND-01 through SND-04 acceptance criteria; Out of Scope.
- `.planning/PROJECT.md` — deliverables, deterministic/clean-room/dependency constraints, automation-first verification, and owner hand-off boundary.
- `.planning/preparation/DECISIONS.md` — DEC.03–DEC.05, DEC.12, DEC.14, DEC.20, DEC.22, DEC.24, DEC.28–DEC.29, and DEC.39.
- `.planning/phases/03-a-real-game-in-retroarch/03-CONTEXT.md` — the three licensed NROM games, regression gates, and automation/no-human-UAT preference.
- `.planning/phases/01-a-test-frame-in-retroarch/01-CONTEXT.md` — fixed public API conventions, silent no-cartridge output, and exact audio sample cadence.
- `.planning/phases/02-the-cpu-matches-the-public-vectors/02-CONTEXT.md` — the CPU/bus cycle seam and phase-2 decisions relevant to DMC DMA integration.

### Hardware and audio design
- `.planning/preparation/README.md` — reference index and source-tier conventions.
- `.planning/preparation/NES-HARDWARE-CPU-APU.md` §2.1–2.4, §3–6 — shared timeline and DMA, APU timers/frame counter/length counter/DMC, mixer and filters, integer synthesis recommendation, spectral test, and NTSC profile.
- `.planning/preparation/ARCHITECTURE.md` §2–3, §5, §7–8 — CPU-driven time, APU/mixer/synthesis/output/hash contract, public buffers, and libretro/runner conversion.
- `.planning/preparation/CONFORMANCE.md` “AccuracyCoin in detail,” “ROMs that can be committed or released,” “Scoreboard design,” and “Where each test runs” — APU test names, scoreboard rules, licensed ROM policy, and test-lane placement.
- `.planning/preparation/ENGINEERING.md` — C17 rules, CTest conventions, CI matrix, and dependency/provenance checks.

### Existing implementation and test seams
- `include/nesturbator.h` — stable frame/audio buffer contract and current silent-audio documentation that this phase updates.
- `src/internal.h` — per-instance device state and existing fractional audio remainder.
- `src/frame.c` — sample-count scheduling and current silence output to replace for cartridge runs.
- `src/bus.c` — CPU-cycle bus seam where APU register and timing integration belongs.
- `libretro/libretro.c` — existing mono-to-stereo adapter and batch callback.
- `runner/main.c`, `runner/sha256.c`, `runner/sha256.h` — runner option parsing and owned SHA-256 implementation to extend for audio hashes.
- `tests/core/test_frame.c` — current sample cadence and silence assertions.
- `tests/libretro/libretro_host.c` — callback capture and frame-to-host integration tests.
- `tests/accuracy/scoreboard.txt`, `tests/accuracy/scoreboard-main.txt` — current and protected AccuracyCoin baselines.
- `tests/roms/manifest.txt` and `tests/runner/hashes.txt` — licensed fixture inventory and existing recorded game hashes.

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `src/frame.c` already schedules 798 or 799 samples per nominal frame and carries the fractional remainder; retain that timing while generating PCM.
- `libretro/libretro.c` already advertises the 48 kHz rate, duplicates mono samples to stereo, and submits one batch per frame.
- `runner/sha256.c` is an owned portable SHA-256 implementation; `runner/main.c` has the frame-hash CLI pattern and a caller-owned audio buffer.
- CTest, `tests/check.h`, the libretro callback host, licensed NROM fixtures, and the AccuracyCoin scoreboard provide existing verification patterns without a test-framework dependency.

### Established Patterns
- All mutable machine state lives in `struct nesturbator`; internal symbols use the `nesturbator__` prefix.
- CPU accesses advance one cycle through `src/bus.c`; the APU must share this timeline and the existing DMA/interrupt seams.
- `cmake --workflow --preset ci` is the local and hosted test entrypoint; checks must not rely on network-fetched ROMs or a human listening test.
- Hardware comments cite the matching canonical source, and behavior changes update both README and public-header comments.

### Integration Points
- Add APU state and advancement to the instance/bus and frame execution paths.
- Send generated mono PCM through the existing runner and libretro buffers.
- Extend the runner with the two audio hashes and extend CTest coverage for channel sequences, game hashes, AccuracyCoin, and spectral quality.
- Update CI only where a recurring audio regression check adds value, preserving the existing six-platform required roll-up and hosted RetroArch smoke setup.

</code_context>

<specifics>
## Specific Ideas

- Keep audio proof machine-readable and repeatable: per-channel sequences, stable game output, logic-level and PCM hashes, six AccuracyCoin cases, and a quantified spectral limit.
- Continue the project's preference for a small owned implementation and a flat dependency tree.
- No open product-level gray areas remained after applying the existing phase requirements, preparation decisions, and the owner's standing instruction to accept the documented recommendations.

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within Phase 04 scope.

</deferred>

---

*Phase: 04-Sound*
*Context gathered: 2026-10-08*
