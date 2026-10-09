# Phase 04: Sound - Research

**Researched:** 2026-10-08
**Domain:** NTSC RP2A03G APU emulation and deterministic fixed-point synthesis
**Confidence:** MEDIUM

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions
- **D-01:** Treat ROADMAP Phase 04 and SND-01–SND-04 as the complete product scope. Add recorded audio hashes for all three licensed NROM games selected in Phase 03 (Nesteroids, Double Action Blaster Guys, and RHDE: Furniture Fight). Every added ROM or binary fixture must have an allowed licence, provenance, pin, and manifest hash.
- **D-02:** Preserve the public frame API, fractional 48 kHz sample cadence, and the no-cartridge test-card-and-silence behavior. Cartridge audio fills the existing caller-owned mono `int16_t` buffer. The libretro adapter continues to duplicate each mono sample to left and right.
- **D-03:** Keep the core integer-only and deterministic, with all mutable emulation and synthesis state in the instance. Write the APU, tests, and public-header/README documentation in this phase. Prefer existing code and a small owned implementation over another dependency; do not open or copy GPL/LGPL emulator source.
- **D-04:** Carry forward DEC.14 and the architecture contract: model the NTSC RP2A03G; mix the two pulse channels, triangle, noise, and DMC with the checked-in exact integer table; use the documented fixed-point band-limited synthesis and NES output filters; output mono signed 16-bit PCM at 48,000 Hz by default. The sample-rate fraction and carried remainder keep 798/799 samples per nominal NTSC frame without drift.
- **D-05:** Preserve both hash layers: hash the ordered `(CPU cycle, mixed level)` transitions before synthesis as the logic-level signal, and hash the final 16-bit PCM separately. `--hash-audio` reports both values as SND-02 requires. PCM changes from synthesis/filter adjustments must not obscure whether the APU's transition stream changed.
- **D-06:** Treat the specified synthesis recipe and quality threshold as locked inputs: integer lookup tables, 16 taps, 32 phases, Q15 rows summing to 32768, and a largest non-harmonic peak below 16 kHz under -80 dB for the five reference tones. Use the pulse periods 100, 40, 12, and 8 plus the triangle period 1 defined by the preparation reference.
- **D-07:** Run recurring, machine-verifiable audio checks through `cmake --workflow --preset ci` on the existing six platform/architecture legs wherever applicable: per-channel output sequences, licensed-game audio hashes, transition and PCM hash stability, libretro sample equality, the six named AccuracyCoin tests, and the spectral threshold. Extend the existing CTest and libretro host seams. Keep the hosted RetroArch smoke test's device-independent setup; do not require a physical audio device or listening session.
- **D-08:** Keep the phase free of owner UAT. The project's existing sample-level, hash, scoreboard, and spectral assertions are the acceptance evidence; audio-device behavior belongs to the frontend and is outside this core's deliverables.

### the agent's Discretion
- Choose internal C structs, file/module boundaries, event scheduling, and bounded test implementation details that satisfy the locked hardware behavior and existing instance/bus/API patterns.
- Use the cited hardware documentation and public test behavior to resolve cycle-level details, including frame-counter timing, length-counter edges, DMC DMA, and register side effects. Do not ask the owner to choose internal architecture or reopen already-settled product contracts.
- Keep generated coefficient and mixer data reproducible and checked in; choose the smallest owned generator or verification mechanism that fits the existing CMake/CTest patterns.

### Deferred Ideas (OUT OF SCOPE)
None — discussion stayed within Phase 04 scope.

[VERIFIED: .planning/phases/04-sound/04-CONTEXT.md:17-33,107]
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| SND-01 | A test steps each APU channel (two pulse, triangle, noise, DMC) through its documented output sequence; the runner's audio for the committed open-licence games matches recorded hashes, and the libretro test program receives equal samples. | Use channel documentation, existing frame cadence, licensed fixtures, runner hash inventory, and the libretro host callback. [VERIFIED: .planning/REQUIREMENTS.md:36] |
| SND-02 | nesturbator-run --hash-audio prints two hashes, one of the level-transition stream and one of the 16-bit samples, and both are identical on every platform. | Reuse runner SHA-256; add canonical serialization and a private transition observer without altering the public frame API. [VERIFIED: .planning/REQUIREMENTS.md:37] |
| SND-03 | The scoreboard shows AccuracyCoin's length counter and frame counter and DMC tests passing: Length Counter, Length Table, Frame Counter IRQ, Frame Counter 4-step, Frame Counter 5-step and Delta Modulation Channel. | The pinned AccuracyCoin page 14 contains these six tests; extend page execution and scoreboard comparison. [VERIFIED: .planning/REQUIREMENTS.md:38; .planning/preparation/CONFORMANCE.md:28-32] |
| SND-04 | A test in the ci preset renders five reference tones and shows that the synthesiser's largest non-harmonic peak below 16 kHz is under -80 dB. | Preparation defines five periods, a 32768-sample window, and spectral checks. [VERIFIED: .planning/REQUIREMENTS.md:39; .planning/preparation/NES-HARDWARE-CPU-APU.md:78-82] |
</phase_requirements>

## Project Constraints (from AGENTS.md)

- Own all emulation and test code; libretro.h is the only vendored file; the core library links only C memory functions. [VERIFIED: AGENTS.md:14-15] Exact rule: “All emulation and test code is written here. `libretro.h` is the only vendored file. The core library links only the C memory functions.”
- Keep deliverables to the C library, headless runner, and libretro adapter; no window, audio-device, or input-device code. [VERIFIED: AGENTS.md:16-17] Exact rule: “Library, runner, libretro adapter. No window, audio-device or input-device code.”
- ROM/binary fixtures must be manifest-listed; avoid personal paths/identities; use the noreply commit identity and hygiene checks. [VERIFIED: AGENTS.md:18-21] Exact rule: “No ROM or BIOS bytes except test ROMs listed in `tests/roms/manifest.txt`. No personal paths, email addresses or names. Commits use the GitHub noreply identity. `scripts/hygiene.sh` checks this on every commit and push.”
- Implement from hardware documents and test behavior; do not open or copy GPL/LGPL emulator sources. [VERIFIED: AGENTS.md:22-24] Exact rule: “Implement from hardware documentation and from test behaviour. Do not open GPL or LGPL emulator source, and keep no copy of it in the workspace; reference emulators run only as released binaries.”
- Keep the core integer-only, instance-owned, and deterministic across platforms. [VERIFIED: AGENTS.md:25-26] Exact rule: “Integer-only. All state lives in the instance. The same inputs give the same frame and audio hashes on every platform.”
- Every behavior change needs a test, README/public-header updates, and automated `cmake --workflow --preset ci` evidence. [VERIFIED: AGENTS.md:27-30] Exact rule: “Every behaviour change lands with a test run by `cmake --workflow --preset ci`, and the README and the public header's comments are updated in the same change. Checks are automated; none waits on a person.”
- Follow one GSD step per command, work on a phase branch, and use a PR with a Conventional Commit title. [VERIFIED: AGENTS.md:31-34] Exact rule: “Run one GSD step per command, then stop and report what finished and what comes next. Inside a step, proceed without asking for routine confirmations. Work on a phase branch; merge by pull request with a Conventional Commit title.”
- Use small modules, fixed-width integers, sourced hardware comments, and the remove/reuse/standard-library/platform/minimum-new-code ladder. [VERIFIED: AGENTS.md:38-41] Exact rule: “Small modules, plain control flow, fixed-width integer types. A hardware comment says what the hardware does and cites its source. To decide how to do something, follow the ladder at ponytail.dev: remove the need, reuse what is here, use the standard library, use the platform, and only then write the minimum new code.”
- Omit Validation Architecture because config explicitly says `"nyquist_validation": false`; omit Security Domain because it explicitly says `"security_enforcement": false`. [VERIFIED: .planning/config.json:21-25,49]

## Summary

Build sound on the existing CPU-driven clock and caller-owned frame buffer. The core is C17; bus calls advance a shared tick timeline; the instance already owns a fractional audio remainder; and the runner already contains a portable SHA-256 implementation. Add instance-owned APU state, bus register routing, channel/frame/DMC behavior, transition-to-PCM synthesis, then tests at existing runner, libretro, CTest, and scoreboard seams. [VERIFIED: CMakeLists.txt:10-14,51-61; src/internal.h:101-112; src/bus.c:16-26; runner/sha256.h:15-20]

Use the locked NTSC RP2A03G model and exact integer mixer/synthesis recipe. Implement the channel and register layer first; integrate DMC DMA and IRQ polling with CPU bus reads; then add mixer/synthesis, dual hashes, game audio hashes, exact libretro comparison, AccuracyCoin results, and spectral verification. The main risk is cycle ordering across APU clocks, CPU reads, DMC stalls, and IRQ polling. [VERIFIED: .planning/phases/04-sound/04-CONTEXT.md:22-28; .planning/preparation/NES-HARDWARE-CPU-APU.md:34-47,69-82,109-111; CITED: https://www.nesdev.org/wiki/NES_APU; https://www.nesdev.org/wiki/APU_DMC]

**Primary recommendation:** Add small owned APU and synthesis modules, advance them from the existing CPU-cycle bus timeline, keep mutable state per instance, and use checked-in integer tables. Keep the public mono frame API; add only a private runner-facing transition observer for the logic hash. [ASSUMED: proposed modules and observer; VERIFIED: .planning/phases/04-sound/04-CONTEXT.md:75-91; src/internal.h:101-112; include/nesturbator.h:172-189]

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| APU channels, register behavior, frame/length counters | API / Backend | — | Deterministic machine behavior shares CPU-cycle time. [VERIFIED: .planning/preparation/ARCHITECTURE.md:21-25,44-49] |
| DMC fetches, stalls, and IRQ sources | API / Backend | CPU / Bus | DMC halts/repeats CPU reads and IRQ is sampled in the CPU path. [VERIFIED: .planning/preparation/NES-HARDWARE-CPU-APU.md:34-47; src/internal.h:39-45] |
| Mixer, transition stream, filters, PCM synthesis | API / Backend | Runner diagnostic seam | The core owns deterministic synthesis state; runner consumes pre-synthesis events for its logic hash. [VERIFIED: .planning/preparation/ARCHITECTURE.md:44-49; ASSUMED: private seam] |
| Frame audio delivery | API / Backend | libretro adapter | Core fills mono caller memory; libretro duplicates samples and submits one batch callback. [VERIFIED: include/nesturbator.h:172-181; libretro/libretro.c:188-208] |
| CLI hashes and game inventory | Runner / host | API / Backend | Reuse runner SHA and explicit byte serialization; current public frame struct exposes PCM only. [VERIFIED: runner/sha256.h:15-20; runner/main.c:94-112; include/nesturbator.h:172-189] |
| AccuracyCoin and spectral regressions | Test / CI | API / Backend | Existing CTest has AccuracyCoin page tests and generated-data reproducibility patterns. [VERIFIED: tests/CMakeLists.txt:100-113,377-384] |

## Standard Stack

### Core

| Library / Tool | Version | Purpose | Why Standard |
|----------------|---------|---------|--------------|
| C | C17 | APU, synthesis, tests, adapters | Existing project standard, with extensions disabled. Exact declarations: “set(CMAKE_C_STANDARD 17)” and “set(CMAKE_C_EXTENSIONS OFF)”. [VERIFIED: CMakeLists.txt:12-14] |
| CMake / CTest | CMake minimum 3.25; local CMake 4.4.3 | Build and run regression checks | Exact minimum declaration: “cmake_minimum_required(VERSION 3.25)”. [VERIFIED: CMakeLists.txt:1; local output: “cmake version 4.4.3”, 2026-10-08] |
| Existing runner SHA-256 | Repository implementation | Hash transitions and PCM | Avoid a new dependency; explicit byte serialization can reuse the current portable implementation. [VERIFIED: runner/sha256.h:15-20; runner/main.c:94-112] |
| Checked-in integer tables | Project-owned generated C data | Mixer and band-limited kernel | Keeps runtime integer-only; repository already checks generated data byte-for-byte. [VERIFIED: .planning/preparation/ARCHITECTURE.md:46-49; tests/CMakeLists.txt:377-384] |

### Supporting

| Tool / Fixture | Version | Purpose | When to Use |
|----------------|---------|---------|-------------|
| AccuracyCoin ROM | Pin 673ef550db296136d52229961e7d39366116882a | Length, frame counter, DMC regression | Scoreboard the exact six SND-03 cases. Exact manifest row: “tests/roms/accuracycoin.nes https://github.com/100thCoin/AccuracyCoin/tree/673ef550db296136d52229961e7d39366116882a 673ef550db296136d52229961e7d39366116882a MIT 4fe8c2bc9abc6f4d418da47b73f62cba89fcacd950fae763097a1681d650e839”. [VERIFIED: tests/roms/manifest.txt:18; .planning/preparation/CONFORMANCE.md:28-32,131] |
| Licensed NROM games | Existing manifest pins | Recorded game audio hashes | Use Nesteroids, DABG, and RHDE; no added ROM is needed. [VERIFIED: .planning/phases/04-sound/04-CONTEXT.md:17; tests/roms/manifest.txt:18-20] |
| Libretro callback host | Existing test binary | Compare exact emitted audio without device access | Extend capture from nonzero/count checks to sample-by-sample comparison. [VERIFIED: libretro/libretro.c:188-208; tests/libretro/libretro_host.c:191-209] |

### Alternatives Considered

| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| Checked-in fixed-point coefficients | Runtime floating-point coefficient generation | Runtime floating math is incompatible with deterministic integer-only core requirements. [VERIFIED: AGENTS.md:25-26; .planning/phases/04-sound/04-CONTEXT.md:19,24] |
| Small owned APU/synthesis modules | External DSP or emulator package | Adds dependencies and conflicts with the own-code/clean-room constraints. [VERIFIED: AGENTS.md:14-24; .planning/phases/04-sound/04-CONTEXT.md:19] |

**Installation:** None. Do not add external packages for this phase. [VERIFIED: AGENTS.md:14-15; CMakeLists.txt:51-61]

**Package legitimacy audit:** Not applicable; no external package is recommended. [VERIFIED: .planning/phases/04-sound/04-CONTEXT.md:19]

## Architecture Patterns

### System Architecture Diagram

    ROM / CPU bus access
             |
             v
    CPU bus cycle -----> PPU catch-up
             |
             +--------> APU register routing and per-cycle clocks
             |                   |
             |                   +--> pulse / triangle / noise / DMC
             |                   +--> frame / length / envelope / sweep
             |                   +--> frame IRQ + DMC IRQ -> CPU IRQ poll
             |                   +--> DMC fetch -> CPU read halt and DMA
             v
    mixed-level changes -> ordered (CPU cycle, level) transitions
             |
             v
    fixed-point event synthesis -> NES filters -> mono int16 PCM
             |                                  |
             v                                  +--> caller-owned frame audio
    runner SHA-256 (logic stream)                         |
             +---------- runner SHA-256 (PCM)             +--> libretro stereo batch

Clock, public buffer, PPU catch-up, and adapter boundaries are current contracts. APU/synthesis nodes and the transition observer are proposed. [VERIFIED: .planning/preparation/ARCHITECTURE.md:19-35,44-49,71-73; src/bus.c:16-26; include/nesturbator.h:172-189; libretro/libretro.c:188-208; ASSUMED: new modules/observer]

### Recommended Project Structure

    src/
    ├── apu.c             # proposed: registers, clocks, channel and frame state
    ├── audio.c           # proposed: mixer, event kernel, filters, PCM
    ├── audio_tables.c    # proposed: checked-in mixer/kernel arrays
    ├── internal.h        # add per-instance APU and synthesis state
    ├── bus.c             # decode APU I/O and advance on CPU-cycle seam
    ├── frame.c           # preserve cadence and silent no-cartridge path
    └── cartridge.c       # reset APU/synthesis on load/unload
    runner/main.c         # proposed: add --hash-audio and stream/PCM hashing

New module names are suggestions, not existing paths. Existing integration points are in the context and source tree. [ASSUMED: new names; VERIFIED: .planning/phases/04-sound/04-CONTEXT.md:58-91; CMakeLists.txt:51-61]

### Pattern 1: CPU-cycle-owned APU advancement

**What:** Advance APU at the existing ordered CPU bus-cycle boundary. Decode writes there; keep frame and DMC IRQ source flags separate and derive the CPU IRQ line; integrate DMC DMA in the read-cycle path so repeated reads preserve side effects. [VERIFIED: .planning/preparation/ARCHITECTURE.md:21-35; .planning/preparation/NES-HARDWARE-CPU-APU.md:34-47,49-51; src/bus.c:16-26,79-101]

**When:** Every register operation, channel timer tick, frame sequencer edge, IRQ update, and DMC fetch. [CITED: https://www.nesdev.org/wiki/NES_APU; https://www.nesdev.org/wiki/APU_DMC]

**Pseudocode:**

    on_cpu_bus_cycle(instance, address, operation):
        advance_shared_timeline(instance)
        clock_apu_units(instance)
        if operation is read and DMC requests a fetch:
            halt, align, fetch, and repeat the parked bus read
        perform_bus_operation(instance)
        update_aggregate_irq(instance)

The exact subcycle order must follow the existing CPU/bus model and the pinned tests; channel descriptions alone do not settle it. [VERIFIED: .planning/preparation/NES-HARDWARE-CPU-APU.md:34-47,69-73,109-111]

### Pattern 2: Channel mixer and synthesis

Keep channel timers/sequencers independent, convert outputs through the locked integer mixer table, and emit a transition only when the mixed output changes. Feed deltas into the fixed-point kernel and filters to create PCM. Separate channel behavior tests from direct synthesizer tests so failures identify the responsible layer. [VERIFIED: .planning/preparation/NES-HARDWARE-CPU-APU.md:69-82; .planning/phases/04-sound/04-CONTEXT.md:22-28]

### Pattern 3: Canonical dual hashes

Hash pre-synthesis transitions and post-filter PCM independently. Serialize fields byte-by-byte in a defined order, following the existing frame hash's host-independent pattern. Current frame API exposes PCM, not transition events; use a private runner-facing event sink, not an unbounded log or public API change. The private seam and tuple serialization are planning decisions. [VERIFIED: .planning/phases/04-sound/04-CONTEXT.md:22-24; include/nesturbator.h:172-189; runner/main.c:94-112; runner/sha256.h:15-20; ASSUMED: private sink]

### Anti-Patterns to Avoid

- Do not advance APU from a separate frame loop; use CPU bus-cycle time to preserve write phase and DMA ordering. [VERIFIED: .planning/preparation/ARCHITECTURE.md:21-35]
- Do not linearly sum channels or use runtime floating point; use the locked exact integer mixer and synthesis. [VERIFIED: .planning/preparation/NES-HARDWARE-CPU-APU.md:74-82; .planning/phases/04-sound/04-CONTEXT.md:19,22-24]
- Do not route $4017 reads and writes identically: current reads select controller port 2, while its write behavior must reach the frame counter. [VERIFIED: src/bus.c:24-48,79-101; CITED: https://www.nesdev.org/wiki/APU_Frame_Counter]
- Do not store mutable channel/filter state in globals; state belongs to the instance. [VERIFIED: AGENTS.md:25-26; .planning/preparation/ENGINEERING.md:18-25]
- Do not add device, GUI, or owner listening requirements. [VERIFIED: AGENTS.md:16-17; .planning/phases/04-sound/04-CONTEXT.md:26-28]

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Channel/register behavior | A guessed simplified beeper | Hardware documentation and public test behavior | Timer, frame, length, bus, and DMA details affect real games. [CITED: https://www.nesdev.org/wiki/NES_APU; VERIFIED: .planning/preparation/NES-HARDWARE-CPU-APU.md:56-75] |
| Mixer | Linear summing or approximate common table | Locked exact 16 × 16 × 128 integer table | Preparation identifies the common table as approximate; the exact table is chosen. [VERIFIED: .planning/preparation/NES-HARDWARE-CPU-APU.md:74; .planning/phases/04-sound/04-CONTEXT.md:22] |
| Kernel coefficients | Runtime float sinc/window calculation | Checked-in integer tables and deterministic regeneration check | Prevents runtime floating point and table differences between platforms. [VERIFIED: .planning/preparation/NES-HARDWARE-CPU-APU.md:78-82; tests/CMakeLists.txt:377-384] |
| Hashing | Third-party digest or native-endian struct hash | Existing SHA-256 and explicit byte serialization | Reuses portable implementation and avoids host layout differences. [VERIFIED: runner/sha256.h:15-20; runner/main.c:94-112] |
| Host audio | Device opening/resampling in core | Caller-owned PCM and existing libretro batch callback | Device handling is outside project deliverables. [VERIFIED: AGENTS.md:16-17; libretro/libretro.c:188-208; CITED: https://docs.libretro.com/development/cores/developing-cores/]

**Key insight:** Audio must follow the shared bus timeline. Frame-rate sound effects cannot reliably satisfy DMC DMA, frame IRQ, transition hashes, and stable PCM together. [VERIFIED: .planning/preparation/ARCHITECTURE.md:21-35,44-49; .planning/preparation/NES-HARDWARE-CPU-APU.md:34-47,69-82]

## Common Pitfalls

1. **Cycle phase:** Frame counter, register writes, and DMC requests can be early/late if the APU clock is detached from bus cycles. Keep APU phase in instance state and let the pinned tests settle disputed edges. [VERIFIED: .planning/preparation/NES-HARDWARE-CPU-APU.md:69-73,109-111; CITED: https://www.nesdev.org/wiki/APU_Frame_Counter]
2. **DMC DMA:** A working sample reader can still perturb CPU timing, open bus, controller reads, or OAM DMA. Integrate fetch stalls at CPU read cycles and verify with Delta Modulation Channel plus DMA-related cases. [VERIFIED: .planning/preparation/NES-HARDWARE-CPU-APU.md:34-40,49-51,61-65; CITED: https://www.nesdev.org/wiki/APU_DMC; https://www.nesdev.org/wiki/DMA]
3. **IRQ aggregation:** Frame and DMC IRQ flags have separate rules. Keep source flags independent and recompute the CPU IRQ line each relevant cycle; do not let clearing one source clear the other. [VERIFIED: .planning/preparation/NES-HARDWARE-CPU-APU.md:42-47,65; CITED: https://www.nesdev.org/wiki/APU_DMC; https://www.nesdev.org/wiki/APU_Frame_Counter]
4. **$4017 direction alias:** Bus reads currently use $4017 for controller port 2; writes need separate frame-counter decode. Keep direction-sensitive decode and controller regressions. [VERIFIED: src/bus.c:24-48,79-101; CITED: https://www.nesdev.org/wiki/APU_Frame_Counter]
5. **Channel gating:** Correct sequencer values alone are insufficient; length/envelope/sweep/linear gates and timer limits affect outputs. Test channel sequences and gated output states. [CITED: https://www.nesdev.org/wiki/APU_Pulse; https://www.nesdev.org/wiki/APU_Triangle; https://www.nesdev.org/wiki/APU_Noise]
6. **Table/synth drift:** Point-sampled rows, phase scarcity, non-exact row sums, or float generation can fail spectral/cross-platform checks. Preserve 16 taps, 32 phases, Q15 row sum 32768 and add table regeneration verification. [VERIFIED: .planning/phases/04-sound/04-CONTEXT.md:24; .planning/preparation/NES-HARDWARE-CPU-APU.md:78-82]
7. **Frame boundary state:** CPU instruction residual ticks, sample remainder, filter state, and synthesis history must not cause dropped samples or reset artifacts at each frame. Retain state across frames and preserve silent no-cartridge behavior. [VERIFIED: include/nesturbator.h:172-189; src/frame.c:26-70; src/cartridge.c:122-181; .planning/phases/04-sound/04-CONTEXT.md:18,22]
8. **AccuracyCoin scope:** Page 14 has nine cases: six named APU cases plus register activation and two controller cases. The conformance note states: “14 APU tests (Length Counter, Length Table, Frame Counter IRQ, Frame Counter 4-step, Frame Counter 5-step, Delta Modulation Channel, APU Register Activation, Controller Strobing, Controller Clocking).” Ensure scoreboard explicitly proves all six SND-03 cases; prefer named selection or make extra page cases advisory. [VERIFIED: .planning/preparation/CONFORMANCE.md:28-32; .planning/phases/04-sound/04-CONTEXT.md:27]

## Code Examples

Pseudocode only; names are illustrative. [ASSUMED]

### Register routing

    write_bus(address, value):
        advance_cpu_bus_cycle()
        if address_is_apu_write_register(address):
            apu_write(instance, address, value)
        else:
            preserve_existing_bus_decode(address, value)

    read_bus(address):
        advance_cpu_bus_cycle()
        if address_is_apu_status(address):
            return apu_read_with_bus_side_effects(instance, address)
        return preserve_existing_bus_read(address)

Current bus code already separates direction-sensitive controller and OAM DMA operations, which the APU route must preserve. [VERIFIED: src/bus.c:24-48,79-101]

### State ownership

    struct instance:
        cpu
        bus
        ppu
        apu_state
        synthesis_state
        cartridge
        profile
        sample_fraction

The proposed fields extend the current instance, which owns CPU, bus, PPU, cartridge, profile, and audio remainder. [VERIFIED: src/internal.h:101-112; ASSUMED: new fields]

### Hash encoding

    for each transition in time order:
        hash(cycle bytes in canonical order)
        hash(mixed-level bytes in canonical order)
    for each PCM sample in output order:
        hash(low byte)
        hash(high byte)

The two hash streams are locked; field widths and byte order should be explicitly selected and documented. [VERIFIED: .planning/phases/04-sound/04-CONTEXT.md:22-24; runner/main.c:94-112; ASSUMED: encoding detail]

## State of the Art

| Previous approach | Phase 04 approach | Impact |
|-------------------|------------------|--------|
| Approximate nonlinear mix table | Exact integer pulse/TND lookup table | Avoid approximation error and preserve deterministic output. [VERIFIED: .planning/preparation/NES-HARDWARE-CPU-APU.md:74; .planning/phases/04-sound/04-CONTEXT.md:22] |
| Point-sampled or naive resampling | Fixed-point band-limited event synthesis with filters | Meet the spectral threshold without float in the core. [CITED: https://ccrma.stanford.edu/~stilti/papers/blit.pdf; VERIFIED: .planning/preparation/NES-HARDWARE-CPU-APU.md:78-82; .planning/phases/04-sound/04-CONTEXT.md:24] |
| No cartridge audio path | Deterministic mono PCM with libretro stereo duplication | Keep sound generation in core and playback responsibility in host. [VERIFIED: .planning/phases/04-sound/04-CONTEXT.md:18,23; include/nesturbator.h:172-181; libretro/libretro.c:188-208] |

**Outdated for this project:** Runtime floating-point synthesis and approximate mixer tables conflict with the locked deterministic integer model. Opening GPL/LGPL emulator source conflicts with the clean-room rule. [VERIFIED: .planning/phases/04-sound/04-CONTEXT.md:19,22-24; AGENTS.md:22-26]

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | [ASSUMED] Separate apu, synthesis, and generated-table modules are a maintainable implementation split. | Architecture Patterns | Internal refactor only. |
| A2 | [ASSUMED] A private transition observer is the cleanest runner seam without public ABI changes. | Architecture Patterns | Choose another bounded internal event interface. |
| A3 | [ASSUMED] Transition fields and PCM should be explicitly serialized in canonical byte order. | Architecture Patterns | Hashes can disagree across endianness if implicit. |
| A4 | [ASSUMED] Named-test selection is preferable to requiring all nine AccuracyCoin page-14 cases. | Common Pitfalls | Team may choose full-page scoreboard results. |
| A5 | [ASSUMED] Existing toolchain and project-owned code suffice; no new package is needed. | Standard Stack | A small owned spectral checker may be needed if no suitable test analyzer exists. |

## Resolved Questions

1. **Private transition seam — resolved:** Use a private streaming observer/sink that receives each `(cycle, mixed_level)` transition as emitted and updates the transition SHA-256 without buffering an unbounded event list. Serialize cycle as unsigned 64-bit little-endian and mixed level as signed 32-bit two's-complement little-endian; serialize each PCM sample as signed 16-bit low byte then high byte. Add known-answer cases for empty, one event, equal-cycle ordering, and PCM bytes. This preserves the public frame ABI and follows the selected private streaming seam in Plans 04-01 and 04-04. [VERIFIED: .planning/phases/04-sound/04-CONTEXT.md:22-24; include/nesturbator.h:172-189; runner/sha256.h:15-20]
2. **AccuracyCoin page 14 — resolved:** Add page 14 as an accepted runner page in `runner/main.c`, register the invocation in `tests/CMakeLists.txt`, and use the existing `tests/accuracy/test_scoreboard.c` against the current protected-main and committed local scoreboard baselines. The test must assert all six exact SND-03 names: Length Counter, Length Table, Frame Counter IRQ, Frame Counter 4-step, Frame Counter 5-step, and Delta Modulation Channel. The other three page results remain non-required for SND-03. [VERIFIED: .planning/preparation/CONFORMANCE.md:28-32; runner/main.c; tests/CMakeLists.txt; tests/accuracy/test_scoreboard.c]
3. **Spectral measurement — resolved:** The test-only analyzer discards 4096 output samples for filter warm-up, applies a periodic Hann window to the next 32768 samples, computes a deterministic fixed-point DFT from owned integer coefficients, masks ±1 bin around each integer harmonic of the fundamental, normalizes magnitudes to the fundamental bin, and selects the maximum remaining bin below 16 kHz. These analyzer details are implementation choices; the locked hardware acceptance remains the five periods and the -80 dB non-harmonic peak threshold. No dependency or floating point enters the shipped core. [VERIFIED: .planning/preparation/NES-HARDWARE-CPU-APU.md:78-82; .planning/phases/04-sound/04-CONTEXT.md:24]
4. **Disputed hardware details — resolved:** For this NTSC RP2A03G phase, use the direct power-up measurement: the noise LFSR is `$0000` at power and its first clock shifts in a 1. Record a test for that behavior and note that the APU_Noise overview's statement that the LFSR loads 1 describes the later operational initialization model, not the measured power-on state. For `$4017`, model the parity-dependent 3/4 CPU-cycle frame-counter reset phase from APU_Frame_Counter; NES_APU's 2/3-cycle wording refers to the visible sequence/effect edge. Preserve parity/cycle assertions using all six page-14 cases. The 2A07/PAL DMA concern is out of scope; the existing CPU-cycle bus seam and DMC cycle tests settle M2-high without master-clock subcycle modeling. [CITED: https://www.nesdev.org/wiki/CPU_power_up_state; https://www.nesdev.org/wiki/APU_Noise; https://www.nesdev.org/wiki/APU_Frame_Counter; https://www.nesdev.org/wiki/NES_APU; VERIFIED: .planning/preparation/NES-HARDWARE-CPU-APU.md:109-111]

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|-------------|-----------|---------|----------|
| CMake | Build and CI workflow | ✓ | 4.4.3 | — |
| Ninja | Local build workflow | ✓ | 1.13.2 | Configured CMake generator in CI |
| C compiler | Core and tests | ✓ | Apple clang 21.0.0 | Existing hosted platform/compiler legs |
| Audio device | Nothing; checks use frame buffers and host callback capture | Not required | — | Libretro test callback |

Versions observed from local version probes on 2026-10-08. No package or network-fetched ROM is required for the implementation. [VERIFIED: local command availability/version probes, 2026-10-08; .planning/phases/04-sound/04-CONTEXT.md:26-28]

## Security Domain

Omitted from design because security enforcement is explicitly disabled in project configuration. [VERIFIED: .planning/config.json:49]

## Sources

### Primary sources
- [NESdev APU overview](https://www.nesdev.org/wiki/NES_APU) — channel set, registers and APU behavior. [CITED: https://www.nesdev.org/wiki/NES_APU]
- [NESdev pulse](https://www.nesdev.org/wiki/APU_Pulse), [triangle](https://www.nesdev.org/wiki/APU_Triangle), and [noise](https://www.nesdev.org/wiki/APU_Noise) pages — channel timers, sequences, and gates. [CITED: https://www.nesdev.org/wiki/APU_Pulse; https://www.nesdev.org/wiki/APU_Triangle; https://www.nesdev.org/wiki/APU_Noise]
- [NESdev DMC](https://www.nesdev.org/wiki/APU_DMC) and [DMA](https://www.nesdev.org/wiki/DMA) — memory fetch and bus behavior. [CITED: https://www.nesdev.org/wiki/APU_DMC; https://www.nesdev.org/wiki/DMA]
- [NESdev frame counter](https://www.nesdev.org/wiki/APU_Frame_Counter) and [mixer](https://www.nesdev.org/wiki/APU_Mixer) — sequencing, IRQ, nonlinear mix, and output filters. [CITED: https://www.nesdev.org/wiki/APU_Frame_Counter; https://www.nesdev.org/wiki/APU_Mixer]
- [AccuracyCoin README at pinned commit](https://raw.githubusercontent.com/100thCoin/AccuracyCoin/673ef550db296136d52229961e7d39366116882a/README.md) — public test suite. [CITED: https://raw.githubusercontent.com/100thCoin/AccuracyCoin/673ef550db296136d52229961e7d39366116882a/README.md; VERIFIED: .planning/preparation/CONFORMANCE.md:28-32,131]
- [Libretro core development docs](https://docs.libretro.com/development/cores/developing-cores/) — frame/audio callback contract. [CITED: https://docs.libretro.com/development/cores/developing-cores/]
- [Stilson and Smith, band-limited impulse trains](https://ccrma.stanford.edu/~stilti/papers/blit.pdf) — windowed-sinc synthesis foundation. [CITED: https://ccrma.stanford.edu/~stilti/papers/blit.pdf]

### In-repository sources
- Phase decisions and seams: .planning/phases/04-sound/04-CONTEXT.md:17-33,72-91. [VERIFIED]
- Hardware model, synthesis, and open disputes: .planning/preparation/NES-HARDWARE-CPU-APU.md:34-47,69-82,109-150. [VERIFIED]
- Clock, DMA, audio, API, adapters: .planning/preparation/ARCHITECTURE.md:19-35,44-49,71-76. [VERIFIED]
- AccuracyCoin fixture policy and page map: .planning/preparation/CONFORMANCE.md:22-36,74-84,98-104. [VERIFIED]
- Current instance, bus, frame and public buffer: src/internal.h:39-55,101-112; src/bus.c:16-48,79-101; src/frame.c:26-70; include/nesturbator.h:172-189. [VERIFIED]
- Runner hashing, libretro adapter, and CTest: runner/main.c:94-112,189-203,606-654; runner/sha256.h:15-20; libretro/libretro.c:188-208; tests/libretro/libretro_host.c:191-209; tests/CMakeLists.txt:87-113,377-384. [VERIFIED]

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — confirmed in CMake settings and existing runner/test implementation. [VERIFIED: CMakeLists.txt:1-14,51-61; runner/sha256.h:15-20]
- Architecture: MEDIUM — inspected existing seams; private transition interface remains a recommendation. [VERIFIED: src/bus.c:16-26; src/frame.c:26-70; include/nesturbator.h:172-189; ASSUMED: new interface]
- Hardware behavior: MEDIUM — based on hardware docs and pinned public tests; two timing/power-on items are explicitly disputed in preparation notes. [CITED: https://www.nesdev.org/wiki/NES_APU; VERIFIED: .planning/preparation/NES-HARDWARE-CPU-APU.md:109-111; .planning/preparation/CONFORMANCE.md:22-32]
- Pitfalls: MEDIUM — grounded in documented timing cases and current bus/API seams. [VERIFIED: .planning/preparation/NES-HARDWARE-CPU-APU.md:34-47,56-82]

**Research date:** 2026-10-08
**Valid until:** 2026-11-07 for hardware behavior; check current package/tool information if dependencies are later proposed.

