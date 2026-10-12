# Phase 12: Save states in the library - Research

**Researched:** 2026-10-11
**Domain:** deterministic C17 serialization of a whole emulator instance (state visitor, canonical little-endian format, CRC-32, all-or-nothing load), plus runner test tooling and six-platform hash pins
**Confidence:** HIGH. Every finding below comes from reading the current tree or from a tool run in this session (a probe program over 10 ROMs, timing runs, a CRC micro-benchmark). No external source was needed and no emulator source was opened (clean-room rule).

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

### Function signatures and buffer contract (STATE-01)

- **D-01: Declarations.** — **Reversibility:** one-way. These signatures enter the public header, which only grows, and hosts compile against them.
  ```c
  size_t nesturbator_state_size(const nesturbator *inst);
  nesturbator_status nesturbator_save_state(const nesturbator *inst, void *buf, size_t capacity);
  nesturbator_status nesturbator_load_state(nesturbator *inst, const void *buf, size_t size);
  ```
  - `state_size` returns 0 for a NULL instance or when no cartridge is loaded. Otherwise it stays constant from a successful cartridge load until unload. It never changes the instance.
  - `save_state` takes a `const` instance, so the type itself says the save does not change it.
  - The functions take a plain byte blob, not a versioned struct, so there is no size tag. The magic, the format version and the cartridge identity are carried inside the blob.

- **D-02: Save accepts any capacity at or above the state size; load needs exactly the state size.**
  - Save writes exactly `nesturbator_state_size(inst)` bytes. It leaves any bytes beyond that untouched and never allocates.
  - Save writes bytes only when it returns `NESTURBATOR_OK`. On any error the buffer is untouched.
  - A NULL `buf` is never a size query; use `nesturbator_state_size` for that.
  - On load, any length other than the state size is refused, so truncated or padded input cannot pass.
  - This matches the libretro split: `retro_serialize` gets a buffer at least the state size, and `retro_unserialize` must match exactly. The Phase 13 fallback that accepts a zero-filled tail stays in the adapter, never in the library.

### Rejection status codes (STATE-04)

- **D-03: Five new statuses, one for each STATE-04 class, appended after `NESTURBATOR_ERR_CARTRIDGE = 7`.** — **Reversibility:** one-way. Enum values are fixed and only appended, and hosts branch on them.

  | Value | Name | Meaning |
  |---|---|---|
  | 8 | `NESTURBATOR_ERR_STATE_FORMAT` | Not a state this library reads: bad magic, or another format version. |
  | 9 | `NESTURBATOR_ERR_STATE_CARTRIDGE` | No cartridge is loaded, or the state's mapper, submapper, region or ROM CRC-32 differs from the loaded cartridge. |
  | 10 | `NESTURBATOR_ERR_STATE_SIZE` | The length is wrong: shorter than the header, unequal to `nesturbator_state_size`, or unequal to the header's total size. Covers truncation at any length. |
  | 11 | `NESTURBATOR_ERR_STATE_CHECKSUM` | The state CRC-32 does not match. |
  | 12 | `NESTURBATOR_ERR_STATE_INVALID` | The CRC matches, but a field is out of range, an enum value is unknown, or a reserved byte is non-zero. |

  - Research suggested three coarse classes. The owner's STATE-04 already requires distinct codes for each case, and five codes still form a small, closed set. Each code tells the host a different thing:
    - FORMAT: another version.
    - CARTRIDGE: the wrong game.
    - SIZE: the transfer or file was cut short.
    - CHECKSUM: the bytes were damaged.
    - INVALID: the state came from a bug or an attacker.
  - The enum does not grow with the format. A new field's range check returns INVALID.
  - `NESTURBATOR_ERR_CARTRIDGE` (7) is not reused, because it means a ROM image was rejected.

- **D-04: Load check order.** Each refusal leaves the instance byte-identical.
  1. A NULL `inst` or NULL `buf` gives `ERR_ARGUMENT`.
  2. A `size` smaller than the fixed header gives `ERR_STATE_SIZE`.
  3. A bad magic or another format version gives `ERR_STATE_FORMAT`.
  4. No cartridge loaded, or a mismatched mapper, submapper, region or ROM CRC, gives `ERR_STATE_CARTRIDGE`. Identity is checked before the full length, because the size depends on the cartridge, so another game's state reports "wrong game" and not "wrong length".
  5. A `size` different from `nesturbator_state_size(inst)`, or a header total size different from `size`, gives `ERR_STATE_SIZE`.
  6. A wrong CRC-32 gives `ERR_STATE_CHECKSUM`.
  7. Range, enum and reserved-zero validation gives `ERR_STATE_INVALID`.
  8. Apply. VALIDATE covers exactly the bytes APPLY reads, so APPLY cannot fail.
  - A flipped byte reports the first check it breaks. A flip in the magic or version gives FORMAT, a flip in an identity field gives CARTRIDGE, and a flip in the size field gives SIZE. A flip anywhere else, including the behaviour revision, gives CHECKSUM.
  - The flip-every-byte test asserts the expected class for each offset. The CRC-recomputed sweep asserts INVALID.
  - The header documents this order with the existing "Checks in order:" idiom.

- **D-05: Save check order.**
  1. A NULL `inst` or NULL `buf` gives `ERR_ARGUMENT`.
  2. No cartridge gives `ERR_STATE_CARTRIDGE`.
  3. A `capacity` smaller than the state size gives `ERR_BUFFER_TOO_SMALL`, the same meaning it has in `run_frame`.

- **D-06: JAM.** Save and load never return `NESTURBATOR_STOP_JAM`.
  - The `jammed` latch (`src/internal.h:48`) is part of the state and is validated as 0 or 1.
  - Saving a jammed instance succeeds.
  - Loading a clean state clears the jam, and loading a jammed state restores it, so the next `run_frame` returns `STOP_JAM` again.
  - A test covers both directions.

- **D-07: Runner mapping.** Every `ERR_STATE_*` status exits 5. stderr carries one line naming the class, for example `nesturbator-run: cannot load state <path> (wrong game)`.
  - A state file the runner cannot read (missing, or a directory) is an I/O failure and exits 1.
  - The README has a status table for the new codes, and a test checks that table against the enum, the same way the exit-code table is checked.

### Frame numbers and time after a load (STATE-03, RUN-01)

- **D-08: A state carries `frame_number` and `ticks`, and load restores them.** — **Reversibility:** costly. Changing this later changes every state hash and the runner's frame-index meaning.
  - A state is a point on one timeline, so save, load and continue equals the uninterrupted run, including the tick count STATE-03 compares.
  - The header comments for both fields change from "since create" to "since power-on, restored by a state load".
  - The frame buffer and audio are not in the state. The first frame after a load must equal the uninterrupted run's next frame, and the fork tests assert this.

- **D-09: Runner frame indices stay absolute, counted from power-on.** — **Reversibility:** one-way. This is part of the runner's published command line.
  - After `--load-state` restores frame K:
    - `--frames N` is the last frame, so the runner runs frames K+1 to N.
    - `--hash-frame N` and `--dump-frame N:FILE` need K < N <= `--frames`, because frame K's picture is not in the state.
    - `--state-at-frame N:FILE` and `--hash-state N` need K <= N <= `--frames`. N = K saves or hashes the state just loaded, which gives a cheap check that a load and a save round-trip.
    - An index out of range exits 2, and the message names K.
  - Without a load, K is 0, so `--state-at-frame 0:FILE` saves the power-on state, matching "including frame 0" in STATE-03.
  - The README says frame indices are absolute and that `--frames` is an end frame, not a count of more frames.

- **D-10: Order of operations.**
  1. Parse and syntax-check the options (exit 2).
  2. Create the instance and load the ROM.
  3. Load the battery save.
  4. Apply `--load-state` (exit 5 if rejected, exit 1 on an I/O failure).
  5. Range-check the frame indices against K (exit 2).
  6. Run the frame loop.
  - State outputs for frame N are written after frame N runs. A state at K is taken before the loop.

- **D-11: Repetition.**
  - `--state-at-frame` and `--hash-state` may each repeat, like `--dump-frame`. The same N given twice to one flag exits 2.
  - `--load-state` may be given once.
  - State files are written only to the given path, through `nesturbator_run_file_replace` (Phase 11 D-01).
  - `--hash-state` prints the SHA-256 of the state bytes in the existing `hashes.txt` line style.

- **D-12: `--movie` together with `--load-state` exits 2** with `--movie cannot be combined with --load-state`. A movie indexed from frame 1 would silently go out of sync. Refusing now leaves STATE-06 free to define the meaning later.

### Format version policy (STATE-02)

- **D-13: A public `NESTURBATOR_STATE_FORMAT_VERSION 1` sits next to `NESTURBATOR_BEHAVIOUR_REVISION` in `include/nesturbator.h`.** — **Reversibility:** one-way. Once a release ships version 1, released state files and host rewind policies depend on it.
  - The header carries an exact version, and any mismatch is refused with `ERR_STATE_FORMAT`. There are no tagged or skippable sections and no converter, since conversion is out of scope.
  - Before the first release that contains states (Phases 12 to 15), the version stays 1 and layout changes simply regenerate the pins.
  - After that release, any change to the state bytes bumps the version in the same commit: a field added, removed, reordered or resized, or a field's meaning changed.
  - A fix that changes only emulation output bumps `NESTURBATOR_BEHAVIOUR_REVISION`, which is recorded in the state but not enforced. A fix that adds a field bumps both.

- **D-14: Semver mapping.**
  - A format bump does not change `NESTURBATOR_ABI_VERSION` and is not a breaking change on its own.
  - It ships under a `feat:` title, and the changelog body says older states are refused.
  - `!` or a BREAKING footer is used only when the C API changes.

- **D-15: The CI guard reuses the existing pin file. No new tool is added.**
  - `tests/runner/hashes.txt` gains a `state_format <N>` line, the state hashes, and the state size for each board.
  - A ci test requires the `state_format` line to equal the macro.
  - A layout change breaks the size or hash pins. Re-pinning them makes the author look at the `state_format` line in the same edit, and after the first release a version mismatch fails the test.
  - The remaining gap is someone editing both the pins and the line without bumping the macro. That gap is accepted.

- **D-16: Header evolution points.**
  - The header has a fixed size and carries a header-size field. Its reserved bytes are written as zero and validated as zero (D-04 step 7).
  - The state CRC-32 covers every byte except the CRC field itself.
  - The magic is 8 bytes and includes a non-text byte, as PNG does, so text-mode mangling is caught.

- **D-17: README "Save states" wording.**
  - A state is an exact snapshot for the same format version, ROM and board, and it is byte-identical on every platform.
  - Another format version is refused, never converted. Hosts should keep rewind buffers in memory and expect user states to need re-creating after a release that bumps the format.
  - The behaviour revision is recorded, not enforced, so after an emulation fix an old state loads but may diverge.
  - A table lists each format version and its first release.
  - Rewinding past an in-game save reverts the save RAM (STATE-05).

### Claude's Discretion
- The exact header field order and widths, the magic bytes, the number of reserved bytes, and the section layout after the header. The research suggests a 64-byte header with nine sections. Keep the header 64 bytes or smaller and every field explicitly little-endian.
- The stderr wording for each rejection class, as long as each class has its own word.
- Test file names, and how the fork-test matrix is split across executables.
- Whether `nesturbator_version` (or the struct it fills) also reports the format version. Add it only if appending is cheap under the ABI rules.

### Deferred Ideas (OUT OF SCOPE)
- A movie that starts from a state (STATE-06). `--movie` with `--load-state` is refused until then (D-12).
- A hygiene check that fails when state pins change without the format macro changing (D-15). Add it only if drift actually happens.
- Accepting a zero-filled tail in `retro_unserialize` if RetroArch pads buffers. That belongs to Phase 13 and the adapter only.

Out of scope for the phase as a whole (CONTEXT domain): libretro serialize and the `.info` claims (Phase 13), the state-loader fuzz target (Phase 13), signal handling and the full exit table (Phase 14), benchmarks (Phase 15), movies that start from a state (STATE-06).
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| STATE-01 | `state_size`/`save_state`/`load_state`; fixed size from load to unload, 0 with no cartridge; no allocation; save does not change the instance; statuses appended; README "Save states" | Sections 2 (inventory), 4 (size), 7 (header/README edits), Pitfalls P3, P4, P15; Validation rows V1-V3 |
| STATE-02 | Canonical LE bytes, header (magic, version, total size, behaviour revision, mapper, submapper, region, ROM CRC-32, state CRC-32), byte-identical on six platforms, pins in `hashes.txt` | Section 5 (format), Section 8 (hash pins), Pitfalls P9, P17; Validation rows V4, V9 |
| STATE-03 | Save/load/continue equals uninterrupted run on six boards at several frames incl. 0, into a pre-filled instance; compares frame hash, audio hash, ticks, final state bytes; save-every-frame changes no pin | Section 1 (boundary spike), Section 2 (inventory), Pitfalls P1, P13, P16; Validation rows V5, V6 |
| STATE-04 | All-or-nothing load; distinct codes for other game/board, other version, truncation, flipped byte, recomputed CRC with out-of-range fields; ASan/UBSan | Section 3 (ranges), Pitfalls P5-P8; Validation rows V7, V8 |
| STATE-05 | Battery RAM in state; generation bumps only when battery bytes change; README states rewind reverts save RAM | Section 6 (generation); Validation row V10 |
| RUN-01 | `--state-at-frame N:FILE`, `--load-state FILE`, `--hash-state N`; rejected state exits 5; files written only to the given path | Section 9 (runner); Pitfalls P10-P12; Validation rows V11, V12 |
</phase_requirements>

## Project Constraints (from CLAUDE.md)

Extracted from the repository's `CLAUDE.md` and `.claude/CLAUDE.md`. Treat as locked, same authority as CONTEXT decisions.

- **Own code, clean room.** All emulation and test code is written here; `libretro.h` is the only vendored file; do not open GPL or LGPL emulator source. The core library links only the C memory functions (enforced by `tests/cmake/undefined_symbols.cmake`: allowed `memcpy memmove memset memcmp malloc free` plus compiler helpers).
- **Three deliverables only.** Library, runner, libretro adapter. No window, audio-device or input-device code.
- **Clean tree.** No ROM bytes except `tests/roms/manifest.txt` entries; no personal paths, emails or names; `*.state` is git-ignored and `scripts/hygiene.sh` rejects it by name (`ROM_NAME='\.(nes|fds|unf|unif|nsf|nsfe|sav|srm|state)$'`), so tests write states only into build directories. Hash pins only, no committed state file (D-15, SUMMARY).
- **Deterministic core.** Integer-only, all state in the instance, same frame and audio hashes on every platform.
- **Tested and documented together.** Every behaviour change lands with a test run by `cmake --workflow --preset ci`, and the README and the public header comments are updated in the same change. Checks are automated; none waits on a person.
- **One GSD step at a time**, phase branch, PR with a Conventional Commit title (`feat:` for the state work per D-14).
- **Style.** C17, no extensions, small modules, plain control flow, fixed-width types; a hardware comment cites its source; follow the ponytail ladder (remove, reuse, standard library, platform, minimum new code).
- **Build discipline found in the tree** (not in CLAUDE.md, but enforced by tests): presets compile with `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Wstrict-prototypes -Wmissing-prototypes -Wvla` and warnings as errors (`CMakeLists.txt` `nesturbator_warnings`); every `tests/cmake/*.cmake` starts with `cmake_minimum_required(VERSION 3.25)` (`script_policy.cmake`); no registered test may be able to skip (`skip_policy.cmake`); the core defines no writable data symbol (`global_symbols.cmake`), so a const lookup table of integers is fine but a mutable static is not.

## Summary

The milestone research (SUMMARY.md, ARCHITECTURE.md) is sound and the CONTEXT decisions refine it. Grounding it in the current code gives one decisive answer to the research flag and a short list of corrections the planner must apply.

**The spike result.** At the point `nesturbator_run_frame` returns, the CPU is always at an instruction boundary. The loop at `src/frame.c:55-57` calls `nesturbator__cpu_step` whole; each call is exactly one of: a jam read, an NMI sequence, an IRQ sequence, or one complete instruction. There is no resumable micro-step state. What does straddle the boundary is *pending* work: a queued OAM DMA (`bus.oam_dma_pending`), a queued DMC DMA (`apu.dmc.dma_pending` and its phase fields), the IRQ poll latch, the NMI edge, the open-bus latches and the PPU dot phase. All of them are plain fields of `struct nesturbator`, so a field-by-field visitor captures them. No hidden state lives outside the instance (no function-local statics, no mutable file-scope data; `global_symbols.cmake` enforces it).

**Corrections to the milestone research** (details in Section 2.3):
1. `map.a12` is **not** dead. `src/ppu.c:16-20` reads and writes it as the edge filter in front of the board's `ppu_a12` hook. `nesturbator__mapper_load` zeroes `map`, so a load that forgets to rebuild it from `ppu.bus_addr` makes MMC3 miss an A12 fall and diverge. Do not delete it; derive it.
2. ARCH's three statuses (`CORRUPT = 10`) are replaced by D-03's five (8 to 12); ARCH's `size_t *written` out-parameter is gone (D-01); ARCH's per-section tags conflict with D-13 and are dropped; the CRC covers every byte except the CRC field (D-16), not "the field read as zero".
3. `cpu` has 13 serializable bytes, not 14, and `nmi_prev` and `halted_in_read` are never read anywhere in `src/`.
4. A hostile `ppu.ppu_ticks` far below `ticks` makes the next `run_frame` loop for hours (`src/ppu.c:472`). VALIDATE needs one cross-section check (`ppu_ticks == ticks` at a boundary, probe-verified) to prevent a denial of service.
5. Several bounds are memory-safety or UB bounds, not hygiene: `ppu.fine_x <= 7`, `ppu.eval_*` and `sprite_count`, `apu.dmc.output <= 127`, `env_decay <= 15`, `synth.impulse_head <= 15`, and int64 bounds on the synthesis history (Section 3).

**Cost.** A save or load is dominated by one CRC-32 pass: 9 to 80 microseconds for 5 to 45 KiB with a plain 256-entry table (measured), against a measured emulation cost of about 0.8 to 1.1 ms per frame in the Release build. Fixed per-cartridge state is cheap enough for rewind at 60 Hz (under 1% of the 16.7 ms real-time budget per save).

**Primary recommendation:** Build `src/state.c` (visitor with SIZE/WRITE/VALIDATE/APPLY), `src/state.h` (static-inline primitives so mapper files and `bus.unit` link), `src/crc32.c`, a mandatory `state` op on all six boards, and rebuild `map.a12` from `ppu.bus_addr` after `nesturbator__mapper_load`. Prove it with an every-boundary fork test over the first frames of all six boards plus a synthetic DMA/IRQ stress image, a sweep that validates every scalar field, and six-platform hash pins.

## Architectural Responsibility Map

This is a C library plus a CLI, so the tiers are the project's own layers rather than web tiers.

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| Field list, size, bytes, validation, apply | Core library (`src/state.c`) | Mapper boards (`state` op) | One visitor per struct keeps size, write, validate and apply from drifting; boards own their register lists next to their code |
| ROM identity CRC-32 | Core library (`src/cartridge.c` at load) | `src/crc32.c` | Computed once from immutable bytes; save must not recompute |
| Byte-for-byte file I/O of states | Runner (`runner/main.c`, `runner/save.c`) | — | The library does no file I/O (CLAUDE.md rule); atomic writes already exist as `nesturbator_run_file_replace` |
| State hash (SHA-256) | Runner (`runner/sha256.c`) | Test scripts | SHA stays out of the core; the runner already hashes frames with it |
| Battery-generation rule | Core library (load path) | Runner `save_if_changed` | The counter lives in the instance; the host decides whether to write a file |
| Cross-platform byte identity proof | CI hash pins (`tests/runner/hashes.txt`) | `hash-equality` job | Six legs write `hashes.txt`; the job demands byte-identical files |
| Exit-status mapping (5 for rejected) | Runner | README table + test | Library statuses stay distinct; runner collapses `ERR_STATE_*` to 5 |
| libretro serialize, `.info` claims | libretro adapter | — | Phase 13, not here |

## 1. Research-flag spike: instruction boundaries and boundary-straddling state

### 1.1 The boundary is always an instruction boundary

`src/frame.c:52-58` (read this session):
```
        uint64_t target = inst->ticks + NESTURBATOR_TICKS_PER_FRAME;
        inst->ppu.video_output = io->video;
        inst->ppu.video_pitch = io->video_pitch;
        while (inst->ticks < target && inst->cpu.jammed == 0u) {
            nesturbator__cpu_step(inst);
        }
        nesturbator__ppu_run_until(inst, inst->ticks);
```
`nesturbator__cpu_step` (`src/cpu.c:486-525`) does exactly one of, in this order: a jammed read (`if (nes->cpu.jammed) {... nesturbator__bus_read(nes, 0xFFFFu); return;}`, lines 489-492); the NMI sequence when `nes->cpu.nmi_pending != 0u` (496-508); the IRQ sequence when `nes->cpu.poll_latch != 0u && (nes->cpu.p & FLAG_I) == 0u` (513-525); otherwise `uint8_t opcode = fetch(nes);` and the instruction switch (527 onward). Each branch is straight-line C calling `nesturbator__bus_read/write` and returns before the next iteration. There is no state machine that can be resumed mid-instruction. [VERIFIED: src/frame.c:55-57, src/cpu.c:486-527]

Consequences:
- **Residual ticks.** The loop exits when `ticks >= target`, so `ticks` overshoots by the tail of the last instruction. The next frame's target is relative (`inst->ticks + NESTURBATOR_TICKS_PER_FRAME`, frame.c:52), so the overshoot is not "owed"; it is only visible in `ticks`. The pins show it: `dabg/boot/frame 1 ticks 714792` (tests/runner/hashes.txt line 3) against the nominal 714732. [VERIFIED: tests/runner/hashes.txt:3]
- **No mid-DMA state.** An OAM DMA runs to completion inside a single `nesturbator__bus_read` call (`src/bus.c:133-138`, loop at 123-128); a DMC DMA runs inside `bus_read_cycle` (`bus.c:100-111`, `dmc_dma` 64-98). Only the *pending* flags can straddle the boundary.
- **JAM.** A JAM return (`frame.c:62-64`) leaves `ticks` advanced but skips `audio_rem`, `frame_number`, `io->ticks` updates (`frame.c:67-71`). The state is still a consistent point: restore every field and the next `run_frame` returns `STOP_JAM` at its first check (`frame.c:12-14`). D-06 holds without special casing. [VERIFIED: src/frame.c:12-14, 62-71]
- **Frame-scoped pointers.** `video_output`, `video_pitch` are NULL/0 after the frame (`frame.c:60-61`); `sample_output`, `sample_limit`, `sample_count` are reset by `apu_end_frame` (`apu.c:196-198`). `tests/core/test_boundary.c:46-50` already asserts them plus `synth.pcm_count == 0`, `synth.pcm_read == synth.pcm_write` and `synth.overflow == 0` after every frame and after a JAM (`boundary_clean`). Phase 11's check is in place; the PCM ring is therefore not state. Note `pcm_read == pcm_write` is **not** 0 in general (both advance by the frame's sample count), so APPLY must write 0 to `pcm_read`, `pcm_write`, `pcm_count` and `overflow` to leave a normalised ring. [VERIFIED: tests/core/test_boundary.c:46-50, src/synth.c:2764-2784]

### 1.2 Fields that straddle a frame boundary (all in the instance, all must be saved)

| Item | Field(s) | Where it is written/consumed | Why a boundary can cut it |
|------|----------|------------------------------|---------------------------|
| Pending OAM DMA | `bus.oam_dma_pending`, `bus.oam_dma_page` | set `src/bus.c:150-152` (`nes->bus.oam_dma_page = value; nes->bus.oam_dma_pending = 1u;`), consumed at the *next* read `bus.c:133-137` | The instruction after `STA $4014` may be the first of the next frame |
| Pending DMC DMA | `apu.dmc.dma_pending`, `dma_halt_phase`, `dma_load_waiting`, `enable_delay`, `buffer_empty`, `sample_buffer`, `remaining`, `address` | set `src/apu.c:283-291`, consumed `src/bus.c:103` | The DMA is performed on the next CPU read cycle |
| GET/PUT phase | `bus.apu_get_put_phase` | toggles every cycle `src/bus.c:26` | Aligns DMA and register races |
| Interrupt poll | `cpu.poll_latch`, `cpu.irq_line` | `bus.c:22` (`nes->cpu.poll_latch = nes->cpu.irq_line;`), `apu.c:110` (`irq_update`) | The poll sample of the last cycle decides the next instruction boundary |
| NMI edge | `cpu.nmi_pending` | set `ppu.c:488`, `639`; cleared `494`, `568`, `641` | Vblank start can land in the last instruction of a frame |
| Open bus | `bus.open_bus`; `ppu.io_bus`, `ppu.io_bus_age` | `bus.c:49-61,126,144`; `ppu.c:476-479,588-589,622-623` | Decays by age counter (160000 PPU dots) |
| PPU dot phase | `ppu.scanline`, `dot`, `odd_frame`, `ppu_ticks`, `vblank_suppress`, `reset_flag` | `ppu.c:470-541` | The frame ends at an instruction boundary, not at a PPU frame boundary |
| Cycle count | `cpu_cycle` | `bus.c:32` | Compared with MMC1 `last_write` and MMC3 `low_cycle` |
| Audio phase | `apu.sample_phase`, `audio_rem` | `apu.c:333-335`, `frame.c:67` | `sample_phase` is overwritten at `begin_frame` (`apu.c:182`) but a `nesturbator_reset` between frames runs bus cycles and `synth_transition` reads it (`synth.c:2728`) |
| Synthesis history | `synth.mixed_level`, `impulse[16]`, `impulse_head`, `integrator_q15`, `hp_*`, `lp_output_q30` | `synth.c:2721-2771` | Filter memory spans frames |

`cpu.halted_in_read` and `cpu.nmi_prev` are dead: `grep` finds no read of either in `src/` (`nmi_prev` is only written as 0 at `ppu.c:615`; `halted_in_read` has no reference outside `internal.h:47`). [VERIFIED: grep over src/*.c this session]

### 1.3 Probe results (invariants that hold at every frame boundary)

A throwaway program (scratchpad, not in the repo) linked `build/ci/libnesturbator.a`, ran 400 frames with varied input on 10 ROMs (dabg, nesteroids, rhde, runner, accuracycoin, M1 SXROM, M2, M3, M7, derived MMC3 battery, M4 256K) and checked after every `run_frame`: [VERIFIED: probe run, 4000 boundaries, 0 violations each]

| Invariant | Result |
|-----------|--------|
| `integrator_q15 + sum(impulse[i]) == mixed_level * 32768` | holds (also by construction: kernel coefficients are corrected to sum 32768, `synth.c:2737-2739`) |
| `ticks == 24 * cpu_cycle` | holds (only `bus.c:18,28` advance `ticks` with a cartridge) |
| `ppu_ticks == ticks` (gap in [0,8), observed max gap 0) | holds; `ppu_ticks % 8 == 0` |
| `map.a12 == (ppu.bus_addr >> 12) & 1` | holds |
| `cpu.p & 0x30 == 0x20` | holds (not recommended as a validation rule; see 3.5) |
| `apu.sample_phase < 315000`, `ppu.status & 0x1f == 0`, `dmc.output <= 127` | hold |
| `cpu.irq_line == (frame_irq && !inhibit) \|\| dmc.irq \|\| mapper.irq` | holds (stored value is still saved) |
| `mmc1.last_write <= cpu_cycle`, `mmc3.low_cycle <= cpu_cycle` | hold |
| Observed maxima: `\|impulse\|` 2.2e8 (2^27.7), `\|integrator\|` 5.1e8, `\|hp_*\|` 1.7e13 (2^43.9), `\|lp\|` 6.2e12; `ppu.eval_n` 64; `io_bus_age` 160000 | used for bound choice, Section 3 |

Coverage warning: the committed games barely touch the DMC. The probe saw `dmc.remaining` at most 1 (accuracycoin menu) and 0 on every other ROM. DMC DMA and OAM-DMA straddling therefore will not be exercised by "several frames" of committed games. See Pitfall P16.

## 2. State inventory (current, field by field)

Classes: **(a)** in the state; **(b)** derived or rebuilt on load; **(c)** cartridge-constant, not saved; **(d)** host-owned or frame-scoped, not saved. Line numbers are `src/internal.h` unless noted.

### 2.1 `struct nesturbator` (187-213)

| Field | Class | Note |
|-------|-------|------|
| `allocator` (188) | d | Never touched by load |
| `frame_number` (189) | a | u64. Comment must change from "frames run since create" (D-08) |
| `save_generation` (193) | d | Never serialized; load may add exactly 1 (Section 6) |
| `ticks` (194) | a | u64; comment "ticks run since create" changes (D-08) |
| `cpu`, `bus`, `apu`, `synth`, `ppu` | a | field by field, tables below |
| `transition_sink`, `transition_sink_context` (199-200) | d | Test hook owned by the runner's audio hash; load must not clear it |
| `cart` (202) | c + a | pointers/sizes constant; RAM bytes saved (2.2) |
| `mapper` (203) | a | `id`, `submapper` are identity (checked, not stored in a section); `irq` + board union stored |
| `map` (204) | b | rebuilt by `nesturbator__mapper_load` (`cartridge.c:276-284`), plus `map.a12` (2.3) |
| `cpu_cycle` (209) | a | u64 |
| `profile` (210) | a | 2 bytes (`ane_magic`, `lxa_magic`); set only by `nesturbator_create` (`instance.c:125-126`), not by cartridge load, so a state must carry it |
| `audio_rem` (211-212) | a | u32, validate `< 315000` |

### 2.2 Component structs

| Struct (lines) | Saved bytes | Fields |
|----------------|-------------|--------|
| `cpu` (43-49) | 13 | `pc`(2), `a x y s p`(5), `nmi_prev nmi_pending irq_line poll_latch halted_in_read jammed`(6) |
| `bus` (53-60) | 2059 | `ram[2048]`, `open_bus`, `apu_get_put_phase`, `oam_dma_pending`, `oam_dma_page`, `input_buttons[2]`, `controller_latch[2]`, `controller_shift[2]`, `controller_strobe`. **Not saved:** `input_pending[2]` (host input, set before every frame) |
| `apu` (87-102) | 93 | `pulse[2]` (15 each: `timer counter reg[4] phase length env_divider env_decay env_start sweep_divider sweep_reload`), `triangle` (13), `noise` (12), `dmc` (22), then `enabled pulse_clock_phase frame_mode frame_pending_mode frame_irq_inhibit frame_irq frame_irq_clear_pending frame_reset_delay`(8), `frame_cycle`(4), `sample_phase`(4). **Not saved (d):** `sample_output`, `sample_limit`, `sample_count` |
| `synth` (108-119) | 181 | `mixed_level`(4), `impulse[16]`(128), `impulse_head`(1), `integrator_q15`(8), `hp_input_q30[2]`(16), `hp_output_q30[2]`(16), `lp_output_q30`(8). **Not saved:** `pcm[1024]`, `pcm_read`, `pcm_write`, `pcm_count`, `overflow` (APPLY zeroes the last four) |
| `ppu` (121-156) | 2470 | `control mask status oam_addr`, `address_latch fine_x read_buffer io_bus`, `io_bus_age`(4), `vblank_suppress`, `v t`, `nametable[2048]`, `palette[32]`, `oam[256]`, `secondary_oam[32]`, `sprite_count eval_count eval_n eval_m eval_remaining eval_copy_remaining eval_latch eval_target`, `eval_sprite_zero[8]`, `sprite_zero[8] sprite_x[8] sprite_attr[8] sprite_lo[8] sprite_hi[8]`, `bus_addr`(2), `ale_latch`, `bg_nt bg_at bg_lo bg_hi`, `bg_shift_lo bg_shift_hi`(4), `at_shift_lo at_shift_hi at_latch_lo at_latch_hi`, `ppu_ticks`(8), `scanline`(2), `dot`(2), `odd_frame`, `reset_flag`. **Not saved (d):** `video_output`, `video_pitch` |
| `cart` (158-170) | `prg_ram_size` + (8192 if `chr_is_ram`) | RAM contents: the whole `prg_ram` allocation (`[V work][N NVRAM]`, plus trainer bytes at +0x1000) and the 8 KiB CHR-RAM. All pointers/sizes are class c. New field: `rom_crc32` |
| `mapper` (mapper.h:47-89) | 1 + board bytes | `irq`; NROM 0, UxROM/CNROM/AxROM 1 (`bank`), MMC1 14 (`shift count control_x chr0 chr1 prg` + `last_write` u64), MMC3 24 (`select r[8] mirror a001_x latch counter reload irq_enable a12` + `low_cycle` u64) |

Byte counts are computed from the field lists above (arithmetic on the structs read this session), not measured; the plan must pin the real sizes.

### 2.3 Deltas against SUMMARY.md / ARCHITECTURE.md

| # | Research said | Current code / decision | Action |
|---|---------------|-------------------------|--------|
| 1 | `map.a12` is dead, delete it in the tune-up phase | **Live.** `src/ppu.c:16-20`: `if (level != nes->map.a12) { nes->map.a12 = level; if ((nes->map.watch & NESTURBATOR_WATCH_PPU_A12) != 0u) nes->map.ops.ppu_a12(...)`. Phase 11 (correctly) did not delete it. `src/mapper_mmc3.c:180-182` says the board never reads `map.a12`, which is why it looked dead | Keep the field. After `mapper_load` in APPLY set `map.a12 = (uint8_t)((ppu.bus_addr >> 12) & 1u)`; probe-verified invariant |
| 2 | `cpu` is 14 bytes | 13 bytes; `nmi_prev`, `halted_in_read` never read | Still serialize them (every field is visited; validated 0/1 and 0), or omit with a `sizeof` tripwire; recommend serialize |
| 3 | Statuses FORMAT/CARTRIDGE/CORRUPT (8-10) | D-03: five statuses 8-12 | Use D-03 |
| 4 | `save_state(..., size_t *written)` | D-01: no out-parameter | Use D-01 |
| 5 | Nine sections each with `tag + length` | D-13: no tagged sections | Plain sequential sections, fixed order; no tags |
| 6 | `MAPR` section padded to a fixed 32 bytes | Size is per cartridge anyway (D-01) | No padding; each board writes its natural size |
| 7 | CRC computed with the field "read as zero" | D-16: covers every byte except the CRC field | Two-range CRC update that skips the 4 CRC bytes; no zeroing, no write to a `const` buffer |
| 8 | Header carries `prg_rom_size`, `chr_rom_size`, `prg_ram_size`, `chr_ram_size` | D-03/D-04 identity is mapper, submapper, region, ROM CRC | Drop the four geometry fields; ROM CRC covers header bytes 0-15, and total size equals `state_size(inst)` already |
| 9 | `overflow` listed as a serialized synth field | `overflow == 0` at every boundary (test_boundary) | Do not save; APPLY writes 0 |
| 10 | Tune-up phase will prove `pcm_count == 0` | Done: `tests/core/test_boundary.c` | Nothing to do; PCM ring is not state |
| 11 | "declaration-baseline guard refreshed" (SUMMARY) | It is a SHA-256 of the whole comment-stripped header: `tests/cmake/vector_api_policy.cmake:6` | Plan a recompute step (Pitfall P3) |
| 12 | D-07 says the README status table is checked "the same way the exit-code table is checked" | No such checker exists: `grep -ri "exit status\|exit table" tests scripts .github` finds nothing | The plan must create a small `tests/cmake` script that parses the enum in `include/nesturbator.h` against the README table |

## 3. Valid ranges (what VALIDATE must check)

Legend: **S** = needed for memory safety or defined behaviour (a wrong value reads or writes out of bounds, shifts by a negative amount, or overflows a signed integer); **L** = needed to prevent a hang; **I** = invariant the core always keeps (enum, boolean, counter), cheap, recommended because D-04 step 7 requires "range, enum and reserved-zero" validation. "any" = every value is legal. Each rule was checked against the use sites listed.

### 3.1 Header, core scalars, CPU, bus

| Field | Rule | Class | Why / use site |
|-------|------|-------|----------------|
| magic, version, header_size | exact | FORMAT | D-04 step 3 |
| mapper_id, submapper, region, rom_crc32 | equal to loaded cartridge; region constant 0 (the instance has no region field; NTSC only) | CARTRIDGE | D-04 step 4 |
| total_size | equals `size` and `state_size(inst)` | SIZE | step 5 |
| reserved bytes | all zero | INVALID | D-16 (reached only after the CRC passes) |
| `behaviour_revision` | any | recorded, not enforced | D-13 |
| `frame_number`, `cpu_cycle`, `profile` bytes | any | — | — |
| `ticks` | any; recommend `ticks == 24 * cpu_cycle` (same section) | I | `bus.c:18,28,32`; probe-verified |
| `audio_rem` | `< 315000` | I | `frame.c:30` divides by 315000 |
| cpu `nmi_prev nmi_pending irq_line poll_latch jammed` | `<= 1` | I | `halted_in_read == 0` (dead) |
| cpu `pc a x y s p` | any | — | nothing indexes by them |
| bus `ram`, `open_bus`, `oam_dma_page`, `input_buttons`, `controller_latch`, `controller_shift` | any | — | — |
| bus `apu_get_put_phase`, `oam_dma_pending`, `controller_strobe` | `<= 1` | I | compared with `!= 0` / used as phase |

### 3.2 APU and synthesis

| Field | Rule | Class | Use site |
|-------|------|-------|----------|
| `pulse.phase` | `<= 7` | **S** | `duty[...] >> (7u - pulse->phase)` (`apu.c:40`): unsigned underflow gives a shift count above the width, UB |
| `triangle.phase` | `<= 31` | **S** | `triangle_sequence[triangle->phase]` (`apu.c:48`), 32 entries |
| `pulse.env_decay`, `noise.env_decay` | `<= 15` | **S** | returned as a channel level, which indexes `nesturbator__pulse_mix[31]` (sum of two) and `nesturbator__tnd_mix[16][16][128]` (`apu.c:81`) |
| `dmc.output` | `<= 127` | **S** | indexes the 128-entry last dimension of `tnd_mix` (`apu.c:81`) |
| `pulse.timer`, `pulse.counter`, `triangle.timer`, `triangle.counter` | `<= 0x7ff` | I | 11-bit |
| `pulse.sweep_divider`, `pulse.env_divider`, `noise.env_divider` | `<= 15` (sweep `<= 7`) | I | `envelope_clock`, `length_clock` |
| `triangle.linear`, `linear_reload` | `<= 0x7f` | I | `apu.c:435` masks `& 0x7fu` |
| `noise.lfsr` | `<= 0x7fff` | I | 15-bit shift register |
| `dmc.bits` | `<= 8` | I | `apu.c:270` |
| `dmc.remaining` | `<= 4081` | I | `(255 << 4) \| 1` (`bus.c:91`, `apu.c:412`) |
| `dmc.timer`, `dmc.counter` | `<= 214` | I | `dmc_period[]/2` (428/2) |
| `enabled` | `<= 0x1f` | I | `apu.c:392` |
| `frame_mode`, `frame_pending_mode`, `frame_irq_inhibit`, `frame_irq`, `frame_irq_clear_pending`, `pulse_clock_phase`, `dmc.irq`, `dma_pending`, `dma_halt_phase`, `dma_load_waiting`, `buffer_empty`, `env_start`, `sweep_reload`, `linear_reload_flag` | `<= 1` | I | booleans |
| `frame_reset_delay`, `dmc.enable_delay` | `<= 4` | I | set to 1-4 (`apu.c:359,387,424`) |
| `frame_cycle` | `< 37282` | I | `apu.c:327-329` wraps at 29830 or 37282 |
| `sample_phase` | `< 315000` | **S**-adjacent (I) | `phase_numerator / 315000` selects the kernel phase (`synth.c:2728-2730`); a huge value yields `phase >= 32` and indexes `nesturbator__synth_kernel[phase]` out of range. Treat as **S** |
| `synth.mixed_level` | `0..32767` | **S** | `delta = level - mixed_level` is `int32_t`; INT32_MIN would overflow (`synth.c:2724`); the mixer clamps at 32767 (`apu.c:82`) |
| `synth.impulse_head` | `<= 15` | **S** | `synth->impulse[head]` direct index (`synth.c:2750-2752`) |
| `synth.impulse[i]`, `integrator_q15` | `abs <= 2^40` | **S** | avoids signed overflow in `integrator * 32768` (`synth.c:2755`) and the accumulations; derivation: at most ~600 transitions in the 16-sample window times `delta (2^15) * coef (2^15)` = 2^40; observed max 2^28.6 |
| `hp_input_q30[2]`, `hp_output_q30[2]`, `lp_output_q30` | `abs <= 2^52` | **S** | `multiply_q30` splits by 2^30 so values below 2^62 are safe; theoretical signal bound is `32767 * 32768 * 32768 ≈ 2^45`; observed max 2^43.9 |
| optional strong check | `integrator_q15 + sum(impulse) == mixed_level * 32768` (after the bounds, so no overflow) | I | exact by construction and probe-verified; makes the history incoherence-proof. Recommended |

### 3.3 PPU

| Field | Rule | Class | Use site |
|-------|------|-------|----------|
| `fine_x` | `<= 7` | **S** | `15u - fine_x`, `7u - fine_x` shift counts (`ppu.c:380-381`) |
| `sprite_count` | `<= 8` | **S** | loop bound over `sprite_x[8]` etc. (`ppu.c:403-405`) |
| `eval_count` | `<= 8` | **S** | `secondary_oam[(slot) * 4]`, `eval_sprite_zero[slot]` (`ppu.c:207,228-230`) |
| `eval_copy_remaining` | `<= 3`, and `!= 0` requires `eval_count >= 1` | **S** | `slot_base = (uint16_t)(ppu->eval_count - 1u) * 4u` (`ppu.c:207`): with `eval_count == 0` the index wraps to ~262140 |
| `eval_remaining` | `<= 3` | I | set to 3 only (`ppu.c:237`) |
| `eval_m` | `<= 3` | **S** | `oam[eval_n * 4 + eval_m]`, `secondary_oam[slot_base + eval_m]` (`ppu.c:203,208`) |
| `eval_n` | `<= 64` | **S** | guarded by `if (ppu->eval_n >= 64u) return;` (`ppu.c:195`) but 64 is the true maximum (probe: 64) |
| `scanline` | `<= 261` | I (safe either way: pixel store guarded by `< 240`) | `ppu.c:518-519` |
| `dot` | `<= 340` | I | `fetch_step` returns for `dot > 340` (`ppu.c:342`) |
| `ppu_ticks` | `== ticks` (or `ticks - ppu_ticks < 8`) | **L** | `while (nes->ppu.ppu_ticks + 8u <= ticks)` (`ppu.c:472`): with `ppu_ticks` far below `ticks` the next frame iterates up to `(ticks - ppu_ticks)/8` times, effectively forever. Needs the CORE section's `ticks`, so VALIDATE keeps it in a context field |
| `io_bus_age` | `<= 160000` | I | `ppu.c:476-479` |
| `v`, `t` | `<= 0x7fff` | I | 15-bit; all uses are masked, so safe |
| `bus_addr` | `<= 0x3fff` | I | `set_bus` masks (`ppu.c:13`) |
| `status` | `& 0x1f == 0` | I | only bits 7-5 are ever set (`ppu.c:486,414,236`, clear mask `0x1f` at 493) |
| `palette[i]` | `<= 0x3f` | I | writes mask `& 0x3f` (`ppu.c:466`); keeps pixel bits 9-15 zero |
| `bg_at` | `<= 3`, `at_latch_lo/hi`, `address_latch`, `vblank_suppress`, `odd_frame`, `reset_flag`, `eval_sprite_zero[]`, `sprite_zero[]` | `<= 1` (bg_at `<= 3`) | I | booleans and 2-bit |
| rest (`control mask oam_addr read_buffer io_bus ale_latch eval_latch eval_target bg_nt bg_lo bg_hi shifters sprite_x/attr/lo/hi nametable oam secondary_oam`) | any | — | — |

### 3.4 Mapper boards (`mapper.h:47-89`, board files)

Every bank value wraps by a true modulo against the validated cartridge size (`internal.h:218-227`: `bank_1k % (uint32_t)(nes->cart.prg_size / 1024u)`), so a bank register can never point outside the allocation; ranges here are therefore invariants (I), not safety rules, except where noted. Checking them against bank *counts* is unnecessary.

| Board | Fields and rules |
|-------|------------------|
| common | `irq <= 1` |
| NROM | no fields |
| UxROM, CNROM, AxROM | `bank` any byte (raw written value, modulo applied in rebuild; AxROM uses `& 7` and bit 4) |
| MMC1 | `shift <= 0x1f`, `count <= 4`, `control_x <= 0x1f`, `chr0 <= 0x1f`, `chr1 <= 0x1f`, `prg <= 0x1f` (all come from the 5-bit shift register, `mapper_mmc1.c` cpu_write); `last_write` any (relation to `cpu_cycle` holds but crosses sections; unsigned compare, no UB) |
| MMC3 | `select`, `r[8]`, `mirror`, `a001_x`, `latch`, `counter` any byte (**S** is satisfied: `r[select & 7]`, masks at rebuild); `reload`, `irq_enable`, `a12 <= 1`; `low_cycle` any |

### 3.5 Things deliberately not validated

`cpu.p` (probe: `p & 0x30 == 0x20` always, but nothing depends on it), `cpu.irq_line` consistency (derivable but stored), cross-section relations other than `ticks`/`ppu_ticks`/`cpu_cycle` (`low_cycle <= cpu_cycle` and the like only feed unsigned subtraction). Over-strict rules risk rejecting a state the core produced, which the fork test would catch, but every extra rule is a format commitment (D-13).

## 4. Cost and size

**Size (computed).** Fixed core: 64 header + 30 core scalars + 13 + 2059 + 93 + 181 + 2470 = 4910 bytes, plus the mapper section (1 + board bytes: NROM 1, UxROM/CNROM/AxROM 2, MMC1 15, MMC3 25), plus `prg_ram_size`, plus 8192 when `cart.chr_is_ram`. For the committed boards:

| Board (ROM) | Estimated bytes |
|-------------|-----------------|
| NROM with CHR ROM (nesteroids) | ~4,912 |
| NROM with CHR RAM (dabg) | ~13,104 |
| CNROM (M3) | ~4,913 |
| UxROM, AxROM (M2, M7; CHR RAM) | ~13,105 |
| MMC3 256K CHR-ROM (M4 256K) | ~4,936 |
| MMC3 8K battery + CHR RAM (derived m4tkrom) | ~21,320 |
| MMC1 with 32K PRG RAM + CHR RAM (M1 SXROM) | ~45,886 (the largest) |

These are arithmetic from the field lists, tagged ESTIMATE; the pins in `hashes.txt` will carry the measured sizes (D-15). The size is a pure function of `cart.prg_ram_size`, `cart.chr_is_ram` and the board id, all fixed by `nesturbator_load_cartridge` (`cartridge.c:325-343`) and zeroed by unload (`cartridge.c:242`), so "constant from load to unload, 0 with no cartridge" holds by construction. A failed load leaves the previous cartridge and therefore the previous size (header contract, `nesturbator.h:290-293`).

**Time (measured on this machine, Apple clang, Release `build/ci`).**
- Emulation: 600 frames in 0.67 s (dabg) and 0.49 s (M1 SXROM) including process start, so roughly 0.8 to 1.1 ms per frame. ASan build: 120 frames in 2.02 s, about 17 ms per frame (~16x). [VERIFIED: timed runs this session]
- CRC-32, 256-entry table, one pass: 8.9 us at 5 KB, 22.8 us at 13 KB, 37.0 us at 21 KB, 79.3 us at 45 KB. Nibble table 1.9x slower, bitwise 3.4x slower. `memcpy` of the RAM is under 1 us. [VERIFIED: micro-benchmark in scratchpad; table, nibble and bitwise agree, and `crc32("123456789") == 0xCBF43926`]
- Visitor work: about 700 scalar fields, each a branch plus a few byte stores, estimated 5 to 10 us per pass (SIZE, WRITE, VALIDATE and APPLY are each one pass). [ASSUMED: not measured; Phase 15 owns benchmarks]
- Save ≈ WRITE + RAM copy + CRC ≈ 15 to 90 us. Load ≈ CRC + VALIDATE + APPLY + `mapper_load` ≈ 30 to 110 us.

Conclusion: one save per displayed frame costs 0.1 to 0.5% of the 16.7 ms real-time budget; run-ahead of one frame (one save + one load per frame) costs about 1%. A plain table CRC is enough; slicing-by-8 or a "same-instance fast path" is unnecessary now (SUMMARY's fallback stays unused).

## 5. Recommended format (inside Claude's discretion)

Header, 64 bytes, every multi-byte field written byte by byte little-endian:

| Offset | Size | Field | Load check class |
|--------|------|-------|------------------|
| 0 | 8 | magic `89 4E 54 53 0D 0A 1A 0A` (`\x89NTS\r\n\x1a\n`, PNG-style: high-bit byte, CRLF, DOS EOF, LF) | FORMAT |
| 8 | 2 | `format_version` = `NESTURBATOR_STATE_FORMAT_VERSION` | FORMAT |
| 10 | 2 | `header_size` = 64 | FORMAT |
| 12 | 4 | `total_size` | SIZE |
| 16 | 4 | `behaviour_revision` (recorded) | CHECKSUM |
| 20 | 2 | `mapper_id` | CARTRIDGE |
| 22 | 1 | `submapper` | CARTRIDGE |
| 23 | 1 | `region` (0 = NTSC) | CARTRIDGE |
| 24 | 4 | `rom_crc32` | CARTRIDGE |
| 28 | 4 | `state_crc32` over every byte except [28,32) | CHECKSUM |
| 32 | 32 | reserved, zero | CHECKSUM (CRC covers them), then INVALID if nonzero after a recomputed CRC |

Flip-every-byte expectations that follow from D-04's order: offsets 0-11 give FORMAT, 12-15 SIZE, 20-27 CARTRIDGE, and everything else CHECKSUM (a flip in the behaviour revision, the CRC field, the reserved bytes or the body breaks the CRC first). A `header_size` mismatch is classed FORMAT here (D-04 does not name it; it is a format property).

Body, no tags, fixed order: CORE (frame_number 8, ticks 8, cpu_cycle 8, audio_rem 4, profile 2), CPU, BUS, APU, SYNTH, PPU, MAPPER (`irq` then the board's own bytes), PRG-RAM (whole `prg_ram_size` allocation), CHR-RAM (8192 when `chr_is_ram`). Each section follows the field order of the struct for reviewability.

ROM identity: `rom_crc32 = CRC-32(IEEE, reflected poly 0xEDB88320) over cart.bytes[0..16), cart.prg[0..prg_size), and cart.chr[0..chr_size) when !chr_is_ram`, computed at the end of `nesturbator_load_cartridge` into a new `cart.rom_crc32`. The trainer is not hashed: it only initialises RAM, and the header bit that announces it is in the first 16 bytes. No CRC-32 exists in the tree today (`grep -ri crc src runner host libretro include` is empty).

## 6. Battery save generation (STATE-05)

**Today.** The counter is incremented only on CPU bus writes that land inside the battery span. `src/bus.c:164-175`:
```
    } else if (addr >= 0x4020u) {
        uint8_t *page = nes->map.cpu_w[(addr - 0x4000u) >> 10];
        if (page != NULL) {
            page[addr & 0x3ffu] = value;
            ...
            if (nes->cart.save_size != 0u && page >= nes->cart.save &&
                page < nes->cart.save + nes->cart.save_size)
                nes->save_generation++;
```
It is a `uint64_t` in the instance (`internal.h:193`), zeroed only by create, left alone by load, reset and unload (`nesturbator.h:390-395`; `instance.c:205-208` returns it; `nesturbator_save_generation(NULL)` is 0). The span is `cart.save = prg_ram + ram_work`, `cart.save_size = ram_nv` (`cartridge.c:333-336`), a sub-range of the PRG-RAM allocation that the state stores whole. [VERIFIED: src/bus.c:172-174, src/cartridge.c:333-336, src/instance.c:205-208]

**Rule for load (D-04 step 8, STATE-05).** After VALIDATE succeeds and before copying PRG-RAM, compare the instance's span with the same bytes inside the buffer (`buf + prg_ram_offset + (cart.save - cart.prg_ram)`, length `save_size`). If they differ, `save_generation++` exactly once; otherwise leave it. Then copy PRG-RAM and CHR-RAM into the existing allocation (pointers do not move, so a host's `nesturbator_get_memory` pointer stays valid). Guard every `memcpy`/`memcmp` with `size != 0` (Pitfall P4). The counter is never serialized, never decreases and is not touched by save.

Test shapes (criterion 5): (a) load state S, then `load_state(S)` 600 times, generation unchanged each time; (b) mutate one span byte through `nesturbator_get_memory`, load S: generation `+1` exactly, a second load of S `+0`; (c) a state whose span equals the instance's span but whose work RAM differs: `+0` (the span, not the allocation, is compared); (d) no-battery cartridge: never moves; (e) a refused load never moves it. The runner side is already tolerant: `save_if_changed` writes only if the generation moved **and** the bytes differ from the last-written shadow (`runner/main.c:275-288`), so a bump with unchanged bytes writes nothing.

## 7. Where things go: files, scaffolding, edits

### 7.1 New and changed source files

| File | Change |
|------|--------|
| `src/state.h` (new, internal) | `struct nesturbator__sio`, modes, `static inline` primitives (`sio_u8/u16/u32/u64/i32/i64/bytes`), so mapper files and `bus.unit` need no new link unit |
| `src/state.c` (new) | per-struct `*_io` functions, header, the three public functions, load pipeline |
| `src/crc32.c` (new) | `uint32_t nesturbator__crc32(uint32_t crc, const uint8_t *p, size_t n)` with a `static const uint32_t[256]` table (rodata: passes `global_symbols.cmake`). Also needed by `cartridge.c` |
| `src/cartridge.c` | compute `cart.rom_crc32` at the end of load; add `state` op to the probe/ops fill path unchanged |
| `src/internal.h` | `rom_crc32` in `struct nesturbator__cartridge`; comment edits for `frame_number`/`ticks` (D-08); declare crc and state helpers; **keep** `map.a12` |
| `src/mapper.h` | `void (*state)(struct nesturbator__mapper *m, struct nesturbator__sio *io)` in `struct nesturbator__mapper_ops` (mapper.h:30-44) |
| `src/mapper_{nrom,uxrom,cnrom,axrom,mmc1,mmc3}.c` | one `state` function each; set `out->state` in each `*_ops` fill |
| `CMakeLists.txt` | add `src/state.c`, `src/crc32.c` to the library list (root `CMakeLists.txt` `add_library(nesturbator STATIC ...)`) |
| `include/nesturbator.h` | statuses 8-12, `NESTURBATOR_STATE_FORMAT_VERSION 1`, three functions, "Checks in order:" comments, `frame_number`/`ticks` comment edits (`nesturbator.h:209-212`), `nesturbator_get_memory`/`save_generation` comment gains the state-load rule |
| `runner/main.c`, `runner/CMakeLists.txt` | options and loop (Section 9); no new source file needed |
| `README.md` | "Save states" section, status table, runner flag text, frame-index semantics, format-version table, rewind-reverts-save-RAM sentence; update the "Battery saves" paragraph (README.md "Load (including trainer bytes), reset, unload and writes through the pointer do not count") to mention state loads |

### 7.2 Test scaffolding to reuse

- `tests/check.h`: `CHECK`, `CHECK_EQ_U64`, `CHECK_EQ_HEX`, `CHECK_DONE()`; failures are counted, the run continues.
- `tests/ines.h`: header-only synthetic iNES builder (`struct ines_spec`, `ines_build`, `prg_code`, `segments`, mapper/NES2 fields). Used by `test_boundary.c`, `test_save.c`. Gives no-ROM-bytes images for the cross-ROM matrix, a stress image and the battery cases.
- `tests/mapper_test.h`: a fake board for the seam; `mapper_test_fill` sets the five existing ops individually, so once `state` is added to `struct nesturbator__mapper_ops` it must also set `out->state` (or any local ops struct is uninitialised there).
- Committed boards: `tests/roms/dabg.nes` (mapper 0, NES 2.0, CHR RAM), `nesteroids.nes`, `rhde.nes`, `accuracycoin.nes` (mapper 0), `runner.nes` (mapper 1 with battery), `tests/roms/hm/M1_*`, `M2_P128K_CR8K_V.nes`, `M3_P32K_C32K_H.nes`, `M4_P128K_CR8K.nes`, `M4_P256K_C256K.nes`, `M7_P128K_CR8K.nes`. Derived ROMs are built at build time into `build/<preset>/tests/holymapperel/derived/` by `tests/cmake/hm_derive.cmake` (`m4w8k`, `m4tkrom` = battery MMC3). Headers read this session: `4e45531a 02 00 00 08 ...` for dabg, `.. 20 00 12 08 ... 90 07` for M1 SXROM 32K.
- Registration helpers in `tests/CMakeLists.txt`: `nesturbator_core_test(name source [args])` (lines 510-515; links the library, public header only; add `target_include_directories(... ${PROJECT_SOURCE_DIR}/src)` for internal access as `core.boundary` does) and `nesturbator_runner_test(NAME ARGS EXIT [IGNORE_STDOUT] [EXPECT] [EXPECT_ERR])` (lines 8-22) for single-run runner checks. Multi-step runner checks use a `-P` script like `tests/cmake/runner_save.cmake` (cases registered with a `runner_save_case` function, `FIXTURES_REQUIRED`).
- Whitebox access: tests may include `internal.h` and cast the opaque instance to `struct nesturbator` (`test_boundary.c` does), which gives a byte snapshot for "instance byte-identical" (`memcpy` of the struct copies padding too, so `memcmp` before and after a refused load is meaningful) and the 0xA5 poison fill.

### 7.3 Hash-pin machinery (what must change for `state_format`, state hashes, sizes)

`write_hashes.cmake` runs the runner and collects rows; `hash_keys.cmake` is the one list of expected keys; `hash_inventory.cmake` (used by CI's `hash-equality` job, `.github/workflows/ci.yml:161-174`) demands all six `hashes.txt` files be byte-identical and parses every row with exactly two shapes (`tests/cmake/hash_inventory.cmake:69-74`):
```
      if("${row}" MATCHES "^([^ ]+) sha256 ([0-9a-f]+)$")
...
      elseif("${row}" MATCHES "^([^ ]+) ([0-9]+) ticks ([0-9]+) sha256 ([0-9a-f]+)$")
```
`write_hashes.cmake:~290-296` repeats both regexes to derive `generated_keys`, and a selftest fixture (`write_hash_fixture`, hash_inventory.cmake) fabricates rows per key shape. `runner.write_hashes.content` compares the generated file with the committed one; under the `asan` preset both tests are left out (`NESTURBATOR_SHORT_RUNS`, tests/CMakeLists.txt:464-483), so state pins are checked on the `ci` legs only, while the rejection tests below run under ASan.

Design that needs the least machinery: have `--hash-state N` print `state <N> ticks <T> size <S> sha256 <hex>`; `write_hashes.cmake` emits per board the existing shape-2 row `<board>/state <N> ticks <T> sha256 <hex>` and one new shape-3 row `^([^ ]+) ([0-9]+)$` used for both `state_format 1` and `<board>/state_size <S>`. Extend the regex pair in `hash_inventory.cmake`, `write_hashes.cmake`, the fixture generator and `hash_keys.cmake` consistently; a test compares the `state_format` row with the macro in `include/nesturbator.h` (D-15). Recommended pins: the six boards (NROM dabg, MMC1 M1 SXROM, UxROM M2, CNROM M3, MMC3 derived m4tkrom, AxROM M7) at frames 0 and 60, i.e. 12 hash rows, 6 size rows, 1 format row; six extra runner invocations of at most 60 frames.

## 8. Standard Stack

No new dependency, vendored file, or tool (CONTEXT specifics: "a copy-paste beats another dependency").

### Core
| Component | Version | Purpose | Why standard here |
|-----------|---------|---------|-------------------|
| C17, no extensions | project-wide | `src/state.c`, `src/crc32.c`, mapper ops | Project rule; core links only memcpy/memset/memcmp |
| CRC-32 (IEEE 802.3, reflected 0xEDB88320) | n/a | State checksum and ROM identity | One 1 KiB const table; test vector `crc32("123456789") = 0xCBF43926`, `crc32("") = 0` [VERIFIED: zlib.crc32 and a local table implementation agree this session] |
| `runner/sha256.c` | in-repo | `--hash-state` | Already hashes frames; SHA stays out of the core |
| `nesturbator_run_file_replace` | in-repo (`runner/save.h`) | Atomic state file write | Phase 11 D-01; returns 0, or 1 after printing one line |
| CMake 3.25+ / Ninja / CTest | local: cmake 4.4.3, Ninja present | Build and tests | Existing presets `dev`, `ci`, `asan`, `nofp`, `hygiene` |

### Alternatives Considered
| Instead of | Could use | Tradeoff |
|------------|-----------|----------|
| 256-entry CRC table | nibble table or bitwise | 1.9x / 3.4x slower (measured); the table is 1 KiB of rodata, which `global_symbols.cmake` accepts |
| Static-inline primitives in `state.h` | non-inline in `state.c` | Non-inline breaks `bus.unit`, which compiles the mapper files without `state.c` (Pitfall P2) |
| Per-section tags | none | D-13 forbids tagged sections |

**Installation:** none.

### Package Legitimacy Audit

No external package is installed or recommended by this phase, so the legitimacy gate (`package-legitimacy check`) does not apply. Packages removed (SLOP): none. Packages flagged (SUS): none.

## 9. Runner integration points (`runner/main.c`)

All line numbers from `runner/main.c` as read this session.

| Item | Where | Notes |
|------|-------|-------|
| Usage banner | 53-70 | add three lines; message strings begin `nesturbator-run:` |
| `parse_count_n` | 74-95 | **returns 0 for the value 0** ("from 1 to 4294967295"). `--state-at-frame 0:FILE` and `--hash-state 0` need a variant that accepts 0 (Pitfall P10) |
| `parse_dump` | 103-114 | model for `N:FILE`: splits at the first colon so FILE may contain more |
| `options` struct / `free_options` | 145-167 | arrays are sized `argc` (a 2-arg option consumes two args, so `argc` entries always suffice); add `state_frames[]`, `state_paths[]`, `hash_state[]`, `load_state_path` and free them |
| Option loop | 182-241 | `--dump-frame` branch at 221-228 is the template; duplicate detection for `--hash-audio`/`--save-dir` shows the "exit 2 via `usage(...)`" idiom; `--load-state` once, same-N-twice exits 2 (D-11) |
| Post-parse validation | 242-259 | `--movie` + `--load-state` -> `usage("--movie cannot be combined with --load-state")` (D-12). State flags need `--rom`; a no-cartridge instance has `state_size == 0`, so decide: exit 2 `--state-at-frame needs --rom` (recommended, matching `--save-dir needs --rom`, 252-253) |
| Exit codes today | 794 | 4 for save-size mismatch; 1 failures; 2 via `usage`. Add 5 for `ERR_STATE_*` |
| Range checks | 715-730 | existing `--hash-frame`/`--dump-frame` vs `opt.frames`; the D-09 range checks against K run after `--load-state` (D-10 step 5), so they move after the load |
| Cartridge and battery load | 744-808 | `save.generation = nesturbator_save_generation(inst);` at 805 is recorded **before** `--load-state` (Open Question 2) |
| Frame loop | 829-882 | `for (uint64_t n = 1; n <= opt.frames && status == 0; n++) { uint32_t f = (uint32_t)n; ...` `f` also indexes the movie and the hash/dump lists, so start at `K + 1` for a loaded run; because movies are refused with a load, only `listed(...)` uses change. `(uint32_t)n` truncates beyond 2^32 (K is u64) so compare as u64 |
| State outputs | after 868-877 | for each frame, after the frame hash line and the dump block, emit state hash / file; before the loop emit those for N == K |
| Exit flush | 884-886 | `save_if_changed` runs after any loop end |
| Reading a state file | new | bounded read: allocate `state_size + 1`, `fread` up to that; a longer file then becomes a SIZE rejection (exit 5) without a huge allocation, and a directory or missing file (`ferror`/NULL) is exit 1 (D-07). Do not trust `ftell`: on Linux `fopen` of a directory succeeds |
| Writing a state | new | `nesturbator_run_file_replace(path, buf, size)`; failure already printed its line, exit 1 |

Stdout order recommendation (deterministic for `expect_output.cmake`): per frame N, the existing `frame N ...` line, then the `state N ...` line, then dump files; the `audio ...` lines last (unchanged). `--hash-audio` after a load hashes only the frames run after the load.

## 10. Architecture Patterns

### System data flow
```
Host / runner                 nesturbator (library)                      Instance memory
-------------                 -------------------                      ----------------
state_size(inst) ----------->  cart.prg_ram_size + chr_is_ram + board    (pure read)
save_state(inst,buf,cap) --->  check args, cartridge, capacity
                               header fields -> buf[0..64)
                               WRITE visitor: CORE,CPU,BUS,APU,SYNTH,PPU,MAPPER
                               memcpy PRG-RAM, CHR-RAM
                               crc = crc32(buf[0..28)) then crc32 over buf[32..end)
                               store crc at [28..32)                     (inst unchanged)
load_state(inst,buf,size) -->  1 args  2 size>=64  3 magic/version/header_size
                               4 identity (mapper, submapper, region, rom_crc32)
                               5 size==state_size && total_size==size
                               6 CRC            -> CHECKSUM
                               7 VALIDATE walk  -> INVALID   (nothing written yet)
                               8 APPLY walk -> inst fields; span compare; RAM copy
                                 pcm ring zeroed; mapper_load(inst) rebuilds map;
                                 map.a12 = bus_addr bit 12; generation +1 if span differs
```

### Recommended structure
```
src/state.h     sio type + static inline primitives (mapper files include it)
src/state.c     struct field lists, header, public API
src/crc32.c     table + update function (also used by cartridge.c)
tests/core/test_state.c         API shape, size, no-alloc, const, header, rejection codes, generation
tests/core/test_state_fork.c    fork matrix (committed ROMs + synthetic stress)
tests/core/test_state_sweep.c   flip-every-byte and CRC-recomputed field sweeps
tests/cmake/status_table.cmake  README table vs enum
tests/cmake/runner_state.cmake  multi-step runner cases
```

### Pattern 1: one field list, four modes
**What:** a `struct nesturbator__sio { int mode; uint8_t *w; const uint8_t *r; size_t n; int bad; uint64_t ticks_seen; }` and primitives that behave per mode: SIZE adds the width, WRITE stores LE bytes from the field, VALIDATE reads LE bytes and checks `<= max` without touching the field, APPLY reads and stores. VALIDATE and APPLY walk the same `*_io` functions over the same bytes, so APPLY cannot fail and no staging copy is needed.
**When:** every struct in 2.2, and the mapper `state` op.
**Illustrative sketch** (names from the real structs; helper names are suggestions):
```c
/* src/state.h */
static inline void sio_u8(struct nesturbator__sio *io, uint8_t *v, uint32_t max)
{
    switch (io->mode) {
    case SIO_SIZE: io->n += 1u; break;
    case SIO_WRITE: *io->w++ = *v; break;
    case SIO_VALIDATE: if (*io->r++ > max) io->bad = 1; break;
    default: *v = *io->r++; break;            /* SIO_APPLY */
    }
}
/* src/state.c: the CPU list (13 bytes) */
static void cpu_io(struct nesturbator__sio *io, struct nesturbator__cpu *c)
{
    sio_u16(io, &c->pc, 0xffffu);
    sio_u8(io, &c->a, 255u); sio_u8(io, &c->x, 255u); sio_u8(io, &c->y, 255u);
    sio_u8(io, &c->s, 255u); sio_u8(io, &c->p, 255u);
    sio_u8(io, &c->nmi_prev, 1u); sio_u8(io, &c->nmi_pending, 1u);
    sio_u8(io, &c->irq_line, 1u); sio_u8(io, &c->poll_latch, 1u);
    sio_u8(io, &c->halted_in_read, 0u); sio_u8(io, &c->jammed, 1u);
}
```
`nesturbator_state_size` and `save_state` take a `const` instance but the visitor takes non-const pointers; cast once at the top (`(struct nesturbator *)inst`) with a comment: SIZE and WRITE modes never store through a field pointer, and the object is not defined `const`, so this is defined behaviour. Prove it with a test that snapshots the struct before and after a save.

### Pattern 2: validate, then apply
`load_state` runs checks 1-6, then VALIDATE over every section, then APPLY. APPLY steps in order: top-level scalars and each struct; `synth.pcm_read = pcm_write = pcm_count = 0`, `overflow = 0`; compare span; copy PRG-RAM and CHR-RAM; `nesturbator__mapper_load(inst)` (rebuilds `map` and the pages from the restored registers; `init` does not touch registers, mapper.h:31-33); then `inst->map.a12 = (uint8_t)((inst->ppu.bus_addr >> 12) & 1u)`; then the generation bump. `nesturbator__mapper_load` returns 0 only when the id has no board, which identity checking prevents; treat 0 as unreachable.

### Pattern 3: derive, never save, what the cartridge or other fields can recompute
`map` (pages, `nt[]`, `watch`, `ops`) from registers; `map.a12` from `bus_addr`; `cart` pointers from the allocation; PCM ring is empty.

### Anti-patterns to avoid
- **Struct `memcpy`** into the buffer: padding, `uint64_t` alignment, pointers (the structs hold `video_output`, `sample_output`, `ops` function pointers). Always field by field.
- **Applying while validating**: a half-applied load breaks D-04.
- **A mapper `state` op chosen by `NULL` test**: boards must always provide it; add a table test over ids {0,1,2,3,4,7} asserting `ops.state != NULL`.
- **Reading the instance during VALIDATE** for cross-field rules: the instance still holds the old state. Use `io->ticks_seen` captured from the CORE section.
- **Touching `transition_sink`, `allocator`, `input_pending`, `save_generation` (other than the single bump) in load.**

## Don't Hand-Roll

| Problem | Don't build | Use instead | Why |
|---------|-------------|-------------|-----|
| Atomic file replace for states | new temp/rename code | `nesturbator_run_file_replace` | Phase 11 D-01; failure-injected already |
| Hashing a state in the runner | a second SHA | `runner/sha256.c` (`nesturbator_run_sha256_*`) | exists, tested by `runner.sha256` |
| Synthetic ROMs for tests | committed binaries | `tests/ines.h` + `tests/cmake/hm_derive.cmake` | no new ROM bytes (hygiene) |
| Per-board test ROM inventory | a new list | `tests/holymapperel/roms.cmake`, `hash_keys.cmake` | already hashed on six legs |
| Parsing README tables in tests | an external tool | a `tests/cmake` `-P` script with `file(STRINGS ... REGEX)` | project pattern (no new tool, D-15) |
| Byte-order handling | host-endian tests | byte-wise shifts in the primitives | identical on all six platforms by construction |

**Key insight:** the only genuinely new algorithm is the CRC; everything else is bookkeeping that must stay consistent, which is why one field list drives all four modes.

## Common Pitfalls

### P1: Forgetting to rebuild `map.a12` (silent MMC3 divergence)
**What goes wrong:** after `nesturbator__mapper_load` zeroes `map`, `map.a12` is 0 while `ppu.bus_addr` may have A12 high. The next A12 fall (level 0 equals `map.a12`) never reaches `mmc3_ppu_a12`, so `low_cycle` is not recorded and the next rise is not counted (`ppu.c:16-20`, `mapper_mmc3.c:187-193`). IRQ timing then differs from the uninterrupted run.
**Why:** SUMMARY/ARCH called the field dead; `mapper_mmc3.c:180-181` says the board "never" reads it.
**Avoid:** rebuild it after `mapper_load` (Pattern 2); fork tests on an MMC3 ROM with A12 activity (M4 ROMs, derived m4tkrom) at several frames.
**Warning signs:** MMC3 fork divergence only after the first scanline IRQ following the load.

### P2: Link failures in `bus.unit` (and any other explicit source list)
**What goes wrong:** `tests/CMakeLists.txt:1013-1026` compiles `bus.c instance.c cpu.c apu.c synth.c cartridge.c` + the six mapper files + `ppu.c` directly, not the library. A call from `cartridge.c` to `nesturbator__crc32` (new `crc32.c`) or from a mapper file to a non-inline `state.c` helper leaves unresolved symbols.
**Avoid:** keep the `sio` primitives `static inline` in `state.h`; either add `src/crc32.c` to the `bus.unit` list in the same change or declare the CRC `static inline` where `cartridge.c` can see it. `frame.c`, `palette*.c`, `testcard.c` are already absent from that list, so `state.c` must not be referenced from the files that are present.

### P3: The public-declaration guard fails the moment the header grows
**What goes wrong:** `tests/cmake/vector_api_policy.cmake:6` pins `PHASE8_DECLARATIONS_SHA256` over the header with comments, whitespace and the three `NESTURBATOR_VERSION_*` defines removed. New statuses, macro and functions change it.
**Avoid:** recompute in the same commit. The failure message prints the new value (`expected ..., found <sha>`), or run `cmake -DSOURCE_DIR=<repo> -P tests/cmake/vector_api_policy.cmake` and copy `found`. Update the baseline comment (it says "Phase 8"). The SELFTEST case still passes.

### P4: `memcpy`/`memcmp` with a NULL pointer and size 0
**What goes wrong:** `cart.prg_ram` is NULL when `prg_ram_size == 0` and `cart.save` is NULL without a span; `memcpy(NULL, p, 0)` is undefined and UBSan's nonnull-attribute check aborts the `asan` preset (`-fno-sanitize-recover=all`).
**Avoid:** `if (n != 0u)` around every copy and compare (`instance.c:200` shows the existing `save_size != 0u` idiom).

### P5: Hostile `ppu_ticks` hangs the next frame
**What goes wrong:** `while (nes->ppu.ppu_ticks + 8u <= ticks)` (`ppu.c:472`) runs `(ticks - ppu_ticks)/8` iterations; a CRC-valid state with `ppu_ticks = 0`, `ticks = 2^50` never finishes.
**Avoid:** VALIDATE `ppu_ticks == ticks` (probe-verified at boundaries) using `io->ticks_seen`; add a sweep case that must return INVALID within the test timeout.

### P6: Signed overflow in the synthesis history
**What goes wrong:** `integrator_q15 * INT64_C(32768)` and `multiply_q30` overflow for large inputs, and `level - mixed_level` overflows `int32_t` (`synth.c:2724,2755,2673-2678`); UBSan aborts the next frame after a CRC-valid hostile load.
**Avoid:** the bounds in 3.2 (all **S**), plus the sum invariant. Test: a CRC-recomputed state with each int field at its bound `+1` returns INVALID, and each at exactly the bound loads and survives a frame under UBSan.

### P7: Array indexing and shift counts driven by PPU fields
**What goes wrong:** `fine_x`, `eval_*`, `sprite_count` index or shift (`ppu.c:195-208,380-381,403`). `eval_copy_remaining != 0` with `eval_count == 0` computes `(uint16_t)(eval_count - 1u) * 4u`, an index near 262140 into a 32-byte array.
**Avoid:** every rule marked **S** in 3.3 incl. the pairing rule. Add a targeted sweep that sets each scalar to `max + 1` and 255 and recomputes the CRC.

### P8: Mixer table indices driven by APU fields
**What goes wrong:** `dmc.output`, `env_decay`, `triangle.phase`, `pulse.phase` index `tnd_mix`, `pulse_mix`, `triangle_sequence`, `duty` shifts (`apu.c:40,48,81`).
**Avoid:** rules **S** in 3.2.

### P9: `hashes.txt` row shapes are enforced in four places
See 7.3: `hash_inventory.cmake` (two regexes + fixture), `write_hashes.cmake` (key derivation), `hash_keys.cmake` (expected keys), and the CI `hash-equality` job. Adding `state_format 1` or a size row without touching all four fails `hash.inventory.self_test` and `runner.write_hashes`. `rom_coverage.cmake` also reads `hash_keys.cmake`.

### P10: `--state-at-frame 0:FILE` is rejected by the existing parser
`parse_count_n` (`main.c:74-95`) returns 0 for the value 0 and `parse_dump` calls it. Frame 0 is required by STATE-03/D-09. Write a separate "N >= 0" parse and keep `--dump-frame` as is.

### P11: Frame loop and K
The loop variable `f` indexes the movie and the hash/dump lists and is `(uint32_t)n`. After a load, start at `K + 1` (`K` is u64), compare indices as u64, and range-check `--hash-state`/`--state-at-frame` against `K <= N <= --frames`, `--hash-frame`/`--dump-frame` against `K < N <= --frames` (D-09). `N == K` outputs come from the state just loaded, before the loop.

### P12: `--save-dir` plus `--load-state` rewrites the `.sav` at exit
`save.generation` is recorded at `main.c:805`, before the state is applied (D-10 step 4). A state whose battery bytes differ bumps the generation; `save_if_changed` then writes the state's battery bytes over the user's `.sav` at exit. This follows the library contract but may surprise. See Open Question 2.

### P13: A "fresh instance pre-filled with another game" is only partly poisoned
`nesturbator_load_cartridge` memsets `bus`, `apu`, `ppu`, `cpu`, `mapper`, `synth`, `map` and the cart, and zeroes `ticks`, `frame_number`, `audio_rem`, `cpu_cycle` (`cartridge.c:344-358`). It does **not** reset `profile`, `save_generation`, `transition_sink` or `allocator`. Loading a different game first therefore leaves almost nothing stale. A meaningful pre-fill needs (a) run *the same* cartridge on different inputs so every field diverges, and (b) a whitebox poison: fill `cpu/bus/apu/synth/ppu` with 0xA5 after load (keep `mapper.id/submapper` and cart) so any field missing from the visitor shows up as a divergence. Remember host-owned `bus.input_pending` persists; set input explicitly in the harness.

### P14: Unsaved-but-observable fields hide until frames later
Mitigations are layered: `_Static_assert(sizeof(struct nesturbator__cpu) == N)` tripwires for each pointer-free struct (cpu, bus, pulse, triangle, noise, dmc, synth, plus ppu with its two pointers either excluded or asserted via `offsetof` of the last field), the poison test above, and an every-boundary fork over the first frames.

### P15: Casting away `const` in `save_state`/`state_size`
Defined behaviour as long as nothing stores through the pointer in SIZE/WRITE mode; document it, and test with a struct snapshot.

### P16: Committed games do not exercise DMC/OAM DMA straddling
See 1.3: `dmc.remaining` max 1 across 10 ROMs. Add a synthetic stress image built with `ines.h` that loops a DMC sample, `STA $4014` every frame, enables the frame IRQ with `CLI` and writes bank registers every frame, for NROM and (with mapper ids) the other boards; fork at *every* frame boundary over the first 60 to 120 frames so cut points cover all straddle states.

### P17: CRC and magic details
Skip the CRC field instead of zeroing it (D-16); never write into the caller's `const` load buffer. Magic bytes include 0x89, CRLF and 0x1A so text-mode mangling and CRLF translation are detected. Add the `"123456789"` test vector to the CRC unit test.

### P18: Compiler flags
`-Wconversion -Wsign-conversion` reject implicit narrowing: write `(uint8_t)(v >> 8)` etc. `-Wmissing-prototypes` needs prototypes in `internal.h` for non-static functions. MSVC `/W4` builds the same code; keep statics `static`.

### P19: Header and README comment drift (rule 6)
Update together: `nesturbator.h:209-212` (`frame_number`, `ticks` "since create"), `internal.h:189,194`, the `nesturbator_get_memory`/`save_generation` block (`nesturbator.h:363-395`), the README "Battery saves" and runner sections, and the "Using the library" function list (add the three functions).

### P20: ASan time budget
ASan runs ~17 ms per frame (measured), 16x the Release build. Keep asan fork matrices to roughly 1,000 emulated frames per test and use the existing `NESTURBATOR_SHORT_RUNS` switch (`tests/CMakeLists.txt:464`, `core.reset --short`) to thin sweeps. CRC cost under ASan is also higher; sweep the ~5 KB scalar region exhaustively and stride through raw RAM bytes (any value is legal there).

## Code Examples

### Load pipeline order (illustrative)
```c
/* src/state.c. Statuses are D-03; steps are D-04. */
nesturbator_status nesturbator_load_state(nesturbator *inst, const void *buf, size_t size)
{
    const uint8_t *b = (const uint8_t *)buf;
    if (inst == NULL || buf == NULL) return NESTURBATOR_ERR_ARGUMENT;
    if (size < STATE_HEADER_SIZE) return NESTURBATOR_ERR_STATE_SIZE;
    if (memcmp(b, state_magic, 8u) != 0 || le16(b + 8) != NESTURBATOR_STATE_FORMAT_VERSION ||
        le16(b + 10) != STATE_HEADER_SIZE)
        return NESTURBATOR_ERR_STATE_FORMAT;
    if (inst->cart.bytes == NULL || le16(b + 20) != inst->mapper.id ||
        b[22] != inst->mapper.submapper || b[23] != 0u || le32(b + 24) != inst->cart.rom_crc32)
        return NESTURBATOR_ERR_STATE_CARTRIDGE;
    if (size != nesturbator_state_size(inst) || le32(b + 12) != size)
        return NESTURBATOR_ERR_STATE_SIZE;
    if (le32(b + 28) != crc_skip_field(b, size)) return NESTURBATOR_ERR_STATE_CHECKSUM;
    /* VALIDATE walk (sio.mode = SIO_VALIDATE) -> NESTURBATOR_ERR_STATE_INVALID, then APPLY. */
    ...
}
/* crc_skip_field: crc = nesturbator__crc32(0, b, 28); crc = nesturbator__crc32(crc, b + 32, size - 32);
   with the usual pre/post inversion carried by an "update" signature such as crc32(crc, p, n). */
```

### Battery-span compare (STATE-05)
```c
/* before copying PRG-RAM from the validated buffer */
size_t span_off = (size_t)(inst->cart.save - inst->cart.prg_ram);   /* work bytes first */
if (inst->cart.save_size != 0u &&
    memcmp(inst->cart.save, prg_ram_in_buf + span_off, inst->cart.save_size) != 0)
    changed = 1;
...
if (inst->cart.prg_ram_size != 0u) memcpy(inst->cart.prg_ram, prg_ram_in_buf, inst->cart.prg_ram_size);
...
if (changed) inst->save_generation++;      /* exactly once, after APPLY */
```
`inst->cart.save - inst->cart.prg_ram` is valid because `save = prg_ram + ram_work` points into the same allocation (`cartridge.c:333-336`).

### Rebuild order after APPLY of scalars
```c
if (!nesturbator__mapper_load(inst)) { /* unreachable: identity matched the loaded board */ }
inst->map.a12 = (uint8_t)((inst->ppu.bus_addr >> 12) & 1u);
```

## State of the Art

| Old approach | Current approach | Impact |
|--------------|------------------|--------|
| Struct dump with `memcpy` | field list visitor with canonical LE bytes | byte-identical across six platforms, safe against padding and pointer fields |
| Three coarse load failures | five statuses (D-03) | hosts and the runner can tell wrong game from damaged bytes |
| "map.a12 is dead" (milestone research) | live edge filter, derived from `ppu.bus_addr` | MMC3 states stay exact |

**Deprecated/outdated assumptions in the milestone research:** items 1-12 in 2.3.

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | Visitor cost is 5 to 10 us per pass | 4 | None for correctness; Phase 15 measures |
| A2 | ASan is ~16x slower on the CI Linux x64 leg too (measured on this arm64 Mac) | P20 | Sweeps may need a smaller stride; use `NESTURBATOR_SHORT_RUNS` |
| A3 | CRC timings on hosted runners are within 2x of this machine | 4 | Still microseconds against a ~1 ms frame |
| A4 | PNG-style magic bytes are a good fit for D-16 | 5 | Cosmetic; any 8 bytes with a non-text byte satisfies D-16 |
| A5 | The theoretical bounds 2^40 (impulse/integrator) and 2^52 (hp/lp) never reject a real state | 3.2 | A real state would fail INVALID; the fork test over six boards and the probe (max 2^28.6, 2^43.9) argue against it, but a pathological audio program might approach them. If doubt remains, widen to 2^44 / 2^56, which is still overflow-safe |
| A6 | `cpu.p` needs no validation | 3.5 | None; any byte is safe |
| A7 | MSVC and clang give identical `sizeof` for the pointer-free structs used in tripwires | P14 | Tripwire constants would differ per platform; fall back to `offsetof` of the last field or a visited-byte count test |
| A8 | Dropping the four geometry fields from the header loses no protection | 2.3 #8 | A CRC-32 collision on the ROM could pass identity; sizes are still exact via `state_size`, so memory safety is unaffected |

## Open Questions

1. **`--frames N` smaller than K after a load.**
   - Known: D-09 says `--frames N` is the last frame and an out-of-range index exits 2 naming K; N == K must work (it gives the `--hash-state K` round trip).
   - Unclear: N < K is not stated.
   - Recommendation: exit 2 with `--frames N is before the loaded state's frame K`.

2. **`--save-dir` together with `--load-state`.**
   - Known: the battery shadow and generation are captured before the state load (D-10 order); a state with different battery bytes bumps the generation, so the exit flush writes the state's battery bytes to `<name>.sav`.
   - Unclear: whether a diagnostic `--load-state` run should be allowed to overwrite the user's `.sav`.
   - Recommendation: follow the library contract (let it write), document it in the README runner section, and add a runner test. Re-capturing the shadow after the load would hide the state's save RAM from the `.sav` and contradict "the state includes battery RAM".

3. **State flags without `--rom`.** A no-cartridge instance has size 0 and `save_state` returns `ERR_STATE_CARTRIDGE`. Recommendation: refuse at parse time with exit 2 (`--state-at-frame needs --rom`), mirroring `--save-dir`.

4. **Row shapes in `hashes.txt`** (7.3). Recommendation: `state <N> ticks <T> size <S> sha256 <hex>` from the runner, a shape-2 row plus a shape-3 `<key> <integer>` row in the file.

5. **`nesturbator_version` reporting the format version.** Appending is not free: `internal.h:29` defines `NESTURBATOR_VERSION_SIZE_V1 ((uint32_t)sizeof(nesturbator_version))`, which would silently grow, and `tests/header/header_c.c` asserts `sizeof(nesturbator_version) == 24`. The library is static-only, so the compile-time macro is enough. Recommendation: do not add it in Phase 12.

6. **Whether the sum invariant and `ticks == 24 * cpu_cycle` belong in VALIDATE.** Both are exact today and probe-verified; they tighten what an attacker can load but become format commitments. Recommendation: include `ppu_ticks == ticks` (required, P5) and the synthesis sum invariant; skip `ticks == 24 * cpu_cycle` (a PAL/Dendy region would break it).

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| cmake | all builds | yes | 4.4.3 (min 3.25) | — |
| ninja | presets | yes | present (`/opt/homebrew/bin/ninja`) | — |
| clang / gcc | builds | yes | Apple clang (`/usr/bin/cc`) | — |
| Existing build trees | quick local runs | yes | `build/ci`, `build/asan`, `build/dev` present | reconfigure with `cmake --preset <name>` |
| Python 3 | not required by the project | yes (used only for a CRC cross-check in research) | — | — |
| valgrind, RetroArch | not needed in this phase | — | — | — |

No missing dependency blocks the phase.

## Validation Architecture

`workflow.nyquist_validation` is `false` in `.planning/config.json`; the orchestrator asked for this section explicitly, so it is included.

### Test Framework
| Property | Value |
|----------|-------|
| Framework | CTest driving in-repo C programs with `tests/check.h`, plus CMake `-P` scripts |
| Config file | `tests/CMakeLists.txt`, `CMakePresets.json` |
| Quick run command | `cmake --build --preset ci && ctest --preset ci -R '^(core\.state|runner\.state|state\.)' -j4` |
| Full suite command | `cmake --workflow --preset ci` (configure, build, 488+ tests, package) |
| Sanitizer command | `cmake --workflow --preset asan` (ASan + UBSan, `-fno-sanitize-recover=all`) |

CTest presets set `noTestsAction: error`, so an `-R` pattern that matches nothing fails loudly.

### Phase Requirements -> Test Map
| ID / criterion | Behavior | Test type | Automated command | File exists? |
|----------------|----------|-----------|-------------------|--------------|
| STATE-01, crit 4 | size 0 with no cartridge; size constant from load to unload over 1000 frames, across `reset`, JAM; save never allocates (counting allocator), leaves the instance byte-identical (struct snapshot), writes nothing on error, no `buf` write beyond `size`; statuses appended, values 8-12 | unit | `ctest --preset ci -R core.state$` | Wave 0 (`tests/core/test_state.c`) |
| STATE-01, crit 4 | header declarations compile as C17 and C++ | build | `ctest --preset ci -R header` | edit `tests/header/header_c.c`, `header_cxx.cpp` (function-pointer typedef checks) |
| STATE-01 | README "Save states" and status table match the enum | script | `ctest --preset ci -R state.status_table` | Wave 0 (`tests/cmake/status_table.cmake`) |
| STATE-01 | public declaration baseline refreshed | script | `ctest --preset ci -R vector.*api` (existing policy tests at `tests/CMakeLists.txt:788,791`) | edit `vector_api_policy.cmake` hash |
| STATE-02, crit 2 | header layout (offsets, magic, LE), CRC covers all but its field, `crc32("123456789")`, save-load-save idempotence, bytes identical on rerun | unit | `ctest --preset ci -R core.state$` | Wave 0 |
| STATE-02, crit 2 | state hashes, sizes and `state_format` pinned and equal on all six platforms | script + CI | `ctest --preset ci -R 'runner.write_hashes'`, CI `hash-equality` job (`hash_inventory.cmake`) | edit `write_hashes.cmake`, `hash_keys.cmake`, `hash_inventory.cmake`, `tests/runner/hashes.txt` |
| STATE-02 | `state_format` row equals the macro | script | `ctest --preset ci -R state.format_pin` | Wave 0 |
| STATE-03, crit 1 | fork: run N, save; fresh/poisoned B loads; frames, audio hash, ticks, final state bytes equal for the next M frames; N in {0, 1, several, an NMI/DMA-straddling frame}; six boards; every boundary over the first frames; saving every frame changes no hash | integration | `ctest --preset ci -R core.state_fork` | Wave 0 (`tests/core/test_state_fork.c`, stress image) |
| STATE-03 | poison test: 0xA5 fill of cpu/bus/apu/synth/ppu then load then run equals uninterrupted | integration | same | Wave 0 |
| STATE-03 | JAM both directions (D-06); first frame after load equals the uninterrupted next frame (D-08) | unit | `ctest --preset ci -R core.state$` | Wave 0 |
| STATE-04, crit 3 | cross-ROM matrix (6 boards + no cartridge, + same-board different ROM) -> CARTRIDGE; version byte -> FORMAT; truncation at every length -> SIZE; flip-every-byte -> class per offset (5 classes); CRC-recomputed per-field `max+1` and 255 -> INVALID; reserved nonzero -> INVALID; ppu_ticks hang case returns INVALID; instance byte-identical after every refusal | unit | `ctest --preset ci -R core.state_reject` and `ctest --preset asan -R core.state_reject` | Wave 0 (`tests/core/test_state_sweep.c`) |
| STATE-04 | hostile-but-valid loads (each scalar at its bound) survive one frame under UBSan | unit | `ctest --preset asan -R core.state_reject` | Wave 0 |
| STATE-05, crit 5 | 600 identical loads leave `save_generation` unchanged; changed span bytes raise it by exactly 1, once; refused load and no-battery cartridge never move it; README sentence present | unit + script | `ctest --preset ci -R core.state$`, `-R state.status_table` (README text check can live there) | Wave 0 |
| RUN-01, crit 2 | `--state-at-frame 0:F`, `N:F`, `--load-state`, `--hash-state`; resume equals uninterrupted hashes; N == K round trip; ranges exit 2 naming K; duplicate N exits 2; `--movie` + `--load-state` exits 2; rejected state exits 5 with one stderr word per class; unreadable file exits 1; files only at the given path, no `.tmp` left | runner script | `ctest --preset ci -R runner.state` | Wave 0 (`tests/cmake/runner_state.cmake`, cases registered like `runner_save_case`) |
| RUN-01 | `--hash-state` lines equal `sha256` of a file written by `--state-at-frame` | runner script | same | Wave 0 |

### Sampling Rate
- **Per task commit:** the quick run command above (seconds in the ci preset).
- **Per wave merge:** `cmake --workflow --preset ci`.
- **Phase gate:** `cmake --workflow --preset ci` and `cmake --workflow --preset asan` green, then `/gsd-verify-work`. Also run the `nofp` preset (the new code is integer-only; `-mgeneral-regs-only` would fail the build otherwise) and `hygiene` (no `.state` bytes tracked).

### Wave 0 Gaps
- [ ] `tests/core/test_state.c`, `test_state_fork.c`, `test_state_sweep.c`, a shared `tests/state_test.h` (poison fill, struct snapshot, CRC recompute using the library's `nesturbator__crc32`, stress-image builder) registered with `nesturbator_core_test` plus `target_include_directories(... src)`.
- [ ] `tests/cmake/status_table.cmake` (and the `state_format` pin check), each starting with `cmake_minimum_required(VERSION 3.25)`.
- [ ] `tests/cmake/runner_state.cmake` and its case registrations.
- [ ] Hash machinery edits listed in 7.3 and the regenerated `tests/runner/hashes.txt`.
- [ ] `tests/header/header_c.c` / `header_cxx.cpp` references to the three functions and the macro; `vector_api_policy.cmake` hash.
- [ ] Framework install: none.

### Mapping of success criteria to commands
1. Fork matrix and save-every-frame: `ctest --preset ci -R core.state_fork`; pins unchanged: `runner.write_hashes.content`.
2. Runner options, exit 5, six-platform pins: `ctest --preset ci -R runner.state`, CI `hash-equality`.
3. Rejection matrix under sanitizers: `ctest --preset ci -R core.state_reject`, `ctest --preset asan -R core.state_reject`.
4. Size shape, no-allocation, const save, statuses and README: `core.state`, `state.status_table`, header tests.
5. Generation: `core.state` (generation cases) and README text check.

## Sources

### Primary (HIGH confidence): read this session
- `src/internal.h`, `src/mapper.h`, `src/frame.c`, `src/bus.c`, `src/instance.c`, `src/cartridge.c`, `include/nesturbator.h`, `src/cpu.c` (400-550, 1355-1381), `src/ppu.c` (1-60, 100-675), `src/apu.c` (1-498), `src/synth.c` (2640-2786), `src/mapper_*.c`, `runner/main.c`, `runner/save.h`, `runner/CMakeLists.txt`, `CMakeLists.txt`, `CMakePresets.json`
- `tests/CMakeLists.txt` (1-120, 140-185, 455-535, 995-1035), `tests/check.h`, `tests/mapper_test.h`, `tests/ines.h`, `tests/core/test_boundary.c`, `tests/core/test_save.c`, `tests/header/header_c.c`, `tests/cmake/{hash_keys,hash_inventory,write_hashes,vector_api_policy,undefined_symbols,global_symbols,skip_policy,script_policy,expect_output,runner_save,rom_coverage}.cmake`, `tests/roms/manifest.txt`, `tests/holymapperel/roms.cmake`, `tests/runner/hashes.txt`
- `.planning/phases/12-save-states-in-the-library/12-CONTEXT.md`, `.planning/phases/11-*/11-CONTEXT.md`, `.planning/REQUIREMENTS.md`, `.planning/research/SUMMARY.md`, `.planning/research/ARCHITECTURE.md`, `.planning/config.json`, `CLAUDE.md`, `.claude/CLAUDE.md`
- Tool runs: probe over 10 ROMs x 400 frames (invariants and maxima), `/usr/bin/time` on the runner (ci and asan builds), CRC micro-benchmark and zlib cross-check.

### Secondary / Tertiary
- None. No web or Context7 lookups were needed; no emulator source was opened.

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH (no new dependency; everything is in the tree)
- Architecture and inventory: HIGH (source read in full for the state-relevant parts; invariants probe-verified)
- Ranges: HIGH for the S/L items (each tied to a use site); MEDIUM for the optional I items
- Cost: HIGH for CRC and frame timings (measured), MEDIUM for visitor cost (estimated)
- Pitfalls: HIGH (each cites file and line)

**Research date:** 2026-10-11
**Valid until:** until `src/internal.h`, `src/ppu.c` or `src/apu.c` change a struct (re-run the inventory); otherwise 30 days.
