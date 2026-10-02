---
title: "Architecture: time, components, public API, state and mappers"
summary: "The core's shape: one timeline in half-master-clock ticks, a CPU that drives the bus, a per-dot PPU, integer audio, page-table mappers and an opaque-instance C API."
read_when: "Before designing a core subsystem, the public header, the libretro adapter or the runner."
updated: 2026-10-02
---

# Architecture

## Why this file exists
The hardware maps say what the console does; this file says how the core is put together so that it can do it. Each choice names its strongest alternative and the observation that would change it. Register-level detail stays in the hardware maps, which are cited by file and section.

## 1. Shape
- Three build targets: the core library, the headless runner and the libretro adapter. The runner and the adapter are thin clients of one public header.
- The core is a function of its inputs: configuration, cartridge bytes, battery RAM, controller state per frame, and the sequence of calls. It has no globals, files, clock, threads or floating point, and it links only the C memory functions.
- One accurate core. There is no second, faster, less accurate mode: two cores mean two sets of bugs and two sets of hashes.
- A layout that fits: `include/` (one public header), `src/` (cpu, bus, ppu, apu, synth, cart, state, and `mappers/` with one file per board family), `runner/`, `libretro/`, `tests/`.

## 2. Time
- **Unit.** One tick is half a master-clock period. NTSC has 24 ticks per CPU cycle and 8 per PPU dot; PAL 32 and 10; Dendy 30 and 10. It is the coarsest unit in which every documented clock edge is an integer: on NTSC, M2 is low for 9 ticks and high for 15 ([NES-HARDWARE-CPU-APU.md](NES-HARDWARE-CPU-APU.md), section 2.1).
- **One timeline.** Each instance has one 64-bit tick counter. Every device records how far it has run.
- **The CPU drives.** A CPU cycle is one bus call. Inside it, time advances to M2 rise and the PPU catches up, then to M2 fall and the PPU catches up again, and the access completes. That reproduces the one documented sub-cycle case: a `$2002` read takes the vertical-blank flag at M2 rise and the sprite flags at M2 fall.
- **APU and CPU-clocked mapper counters** advance once per CPU cycle, with a get/put phase bit.
- **PPU catch-up.** The PPU exposes `run_until(tick)`. The first implementation catches up at both M2 edges of every cycle (eager). A lazy schedule, where the PPU falls behind until a register access, a mapper-visible event or the end of the frame, is a later option, accepted only if frame hashes are identical to the eager schedule across the whole test set.
- **Profile.** The power-on CPU/PPU alignment (four on NTSC) and the APU get/put phase (two) are fields of a machine profile with fixed defaults, next to console type, chip revision and RAM fill.

Recommendation: the model above, which matches the shape of the half-cycle designs that score 136 or more on AccuracyCoin ([NES-ECOSYSTEM.md](NES-ECOSYSTEM.md), section 2). Strongest alternative: stepping every master clock, as TriCNES does, the only permissive design at 140. A test that passes only with whole-master-clock M2 edges, or a C measurement that puts fixed stepping within about 20% of this model's speed, would change the choice.

Published accuracy-class emulators report 250 to 400 frames per second on desktop CPUs. The number that decides between eager and lazy catch-up is nanoseconds per dot from the first PPU phase, on an Apple Silicon Mac and on a hosted CI runner.

## 3. CPU and bus
- **Instruction-level core.** Each instruction is straight-line code that issues one bus call per cycle, dummy reads and writes included. All 256 opcodes are implemented.
- **DMA lives in the read call.** When RDY is low on a read, the call spends the halt, alignment, get and put cycles, repeating the parked read with its side effects, and then performs the read. State this needs: the parked address, the get/put phase, two address sources for register decode, separate internal and external data-bus latches (`$4015` reads use the internal one), and the console type for controller clocking ([NES-HARDWARE-CPU-APU.md](NES-HARDWARE-CPU-APU.md), section 2.2).
- **Mid-instruction stops.** A "halted during this read" flag covers the four store opcodes whose result changes when RDY falls. No source gives another reason to stop inside an instruction, so run calls stop at instruction boundaries. Alternative: a per-cycle state machine. A need to pause on an exact cycle through the public API, such as a cycle-stepping debugger, would change this.
- **Interrupts.** NMI (edge) and IRQ (level) are sampled in the second half of every cycle and polled in an instruction's last cycle, with the documented exceptions for branches, flag-changing instructions and vector hijacking ([NES-HARDWARE-CPU-APU.md](NES-HARDWARE-CPU-APU.md), section 2.3).
- **Profile fields.** The constant used by the two unstable opcodes ANE and LXA, the RAM fill at power-on and the chip revision are profile fields. The public CPU vectors fit only the constant $EE, and an open report says test ROMs need $FF, so one fixed value may not pass both ([NES-HARDWARE-CPU-APU.md](NES-HARDWARE-CPU-APU.md), sections 7 and 8).

## 4. PPU
- One dot per step, with two ordered half-dot phases. Pattern and nametable fetches are real bus accesses that the cartridge sees. The background shift registers, the external address latch and both OAM address counters are literal state.
- The hardest public tests, AccuracyCoin pages 19 to 22, probe exactly that state, and a scanline or tile-batch renderer lacks it ([NES-HARDWARE-PPU-CARTRIDGE.md](NES-HARDWARE-PPU-CARTRIDGE.md), section 2).
- Alternative: no half-dot phases, which is enough for the classic suites. A page 20 to 22 failure traced to ordering inside a dot decides.
- **Output.** Each pixel is 16 bits: bits 0 to 5 are the palette entry after greyscale and backdrop handling, bits 6 to 8 the emphasis bits. Each frame carries an uncropped 256 by 240 buffer, the PPU model, the ticks elapsed and the colour-subcarrier phase at line 0 dot 0. Hashes are taken over these values, so no palette choice enters a hash. A helper outside the deterministic core converts through a 512-entry table to XRGB8888 for adapters ([NES-HARDWARE-PPU-CARTRIDGE.md](NES-HARDWARE-PPU-CARTRIDGE.md), section 4).

## 5. APU and audio
- Channel timers, the frame counter and the length-counter edge rules follow [NES-HARDWARE-CPU-APU.md](NES-HARDWARE-CPU-APU.md), section 4.
- **Mixer.** Integer tables generated offline and checked in. An exact 16 by 16 by 128 table for triangle, noise and DMC avoids the 4% error of the common 203-entry approximation.
- **Synthesis.** On each change of the mixed level, the delta times an interpolated kernel row is added to an integer buffer; a running sum is the output; three first-order fixed-point filters follow. The kernel is a checked-in integer array: 16 taps, 32 phases, Q15, each row summing to exactly 32768 so the running sum cannot drift ([NES-HARDWARE-CPU-APU.md](NES-HARDWARE-CPU-APU.md), section 5). A Python prototype of this measured its worst non-harmonic component at -87 dB.
- **Output.** Mono signed 16-bit samples at a rate given as a ratio at creation, 48000 Hz by default. On NTSC that is exactly 352 samples per 13125 CPU cycles.
- **Two hashes.** The stream of (cycle, level) transitions before synthesis is the primary one: it changes only when APU logic changes. The PCM hash is secondary and changes with any kernel or filter adjustment.

## 6. Cartridge and mappers
- **Loader.** iNES and NES 2.0 detection follows [NES-HARDWARE-PPU-CARTRIDGE.md](NES-HARDWARE-PPU-CARTRIDGE.md), section 6. Declared sizes are compared with the file length before anything is allocated; NES 2.0 exponent sizes can describe more than 64 bits.
- **Header corrections** arrive as an optional struct from the host. No header database is bundled, because none publishes a licence ([NES-HARDWARE-PPU-CARTRIDGE.md](NES-HARDWARE-PPU-CARTRIDGE.md), section 7).
- **Banking.** 1 KiB page tables on both buses. A mapper is one module per board family. It declares which events it watches: register writes with bus conflicts, a CPU-cycle clock, the PPU address bus including A12, nametable override, its IRQ line, an expansion-audio tap, non-volatile RAM. A board that watches nothing costs nothing in the PPU loop.
- Page-table pointers are rebuilt from bank registers after a state load and are never serialised.
- **Order.** NROM first, because AccuracyCoin runs on it; then by library share ([NES-HARDWARE-PPU-CARTRIDGE.md](NES-HARDWARE-PPU-CARTRIDGE.md), section 5).

## 7. Public API
The header is designed in the first phase. These principles are shared with the sibling Neo Geo core, so one future host can treat every core the same way:
- An opaque instance created from a configuration struct with an allocator hook. Allocation happens at create and at load, never while running.
- Size-tagged structs, fixed-width fields, status codes, a version call. The ABI, the state format, the behaviour revision and the battery-RAM layout are versioned separately.
- Load copies the ROM bytes, because libretro's content pointer is only valid during the load call.
- Run one frame, or run a bounded number of ticks and get back a stop reason and the elapsed time. With no cartridge loaded, a frame is a fixed test pattern, so a build or a release can be checked without any ROM.
- Controller state is passed per frame. A slot for polling at latch time is left for later.
- Video goes into a caller-owned buffer of native 16-bit pixels with frame metadata; audio into a caller-owned buffer with a returned sample count. Frame and sample rates are reported as integer fractions (NTSC 39375000/655171 frames per second).
- Memory: a side-effect-free peek over the CPU address space, and regions (RAM, PRG-RAM, CHR-RAM, nametables, OAM, palette) whose pointers stay fixed from load to unload.
- State: fixed size per loaded cartridge and independent of options, canonical byte order, no pointers, validated in full before any of it is applied.
- Battery RAM: raw bytes compatible with existing `.sav` and `.srm` files, with a dirty generation counter the host acknowledges.
- Tracing is a compile-time option; a release build contains no trace code.

## 8. Adapters
- **libretro adapter.** Converts fractions to doubles, pixels through the palette table and mono to stereo; exposes memory descriptors; reports the fixed state size; ships a `.info` file that declares its save-state level ([LIBRETRO-AND-RUNNER.md](LIBRETRO-AND-RUNNER.md), sections 1, 2 and 4).
- **Runner.** No window. It replays an input movie, prints frame and audio hashes, dumps a frame image, runs test ROMs and vector files, writes battery saves by temp file and rename into a given directory, and exits with distinct status codes ([LIBRETRO-AND-RUNNER.md](LIBRETRO-AND-RUNNER.md), section 5).

## 9. Not in the design
A second fast core; floating point in the core; threads; window, audio-device or input-device code; a bundled header database; anything taken from copyleft emulators.

## Open questions
- Eager or lazy PPU catch-up: nanoseconds per dot from the first PPU phase settles it.
- Whether any public test needs ordering inside a dot: the AccuracyCoin source comments for pages 20 to 22 settle it.
- The `$4017` write delay, where two wiki pages disagree: AccuracyCoin's frame-counter IRQ result codes settle it.
- Whether the public API ever needs an exact-cycle stop: a debugger requirement from the future host would settle it.

## Sources
| ID | Tier | Source | Pin or access date | Supports |
|---|---|---|---|---|
| ARCH.01 | T2 | [NES-HARDWARE-CPU-APU.md](NES-HARDWARE-CPU-APU.md) and its Sources table | 2026-10-02 | Time, CPU, DMA, APU, synthesis |
| ARCH.02 | T2 | [NES-HARDWARE-PPU-CARTRIDGE.md](NES-HARDWARE-PPU-CARTRIDGE.md) and its Sources table | 2026-10-02 | PPU, output, mappers, loader |
| ARCH.03 | T2 | [NES-ECOSYSTEM.md](NES-ECOSYSTEM.md) and its Sources table | 2026-10-02 | Timing models and speed of existing emulators |
| ARCH.04 | T2 | [LIBRETRO-AND-RUNNER.md](LIBRETRO-AND-RUNNER.md) and its Sources table | 2026-10-02 | Adapter and runner needs |
| ARCH.05 | T3 | Technical design review held during preparation (unpublished) | 2026-10-02 | API principles, hashing of the pre-synthesis audio stream |
