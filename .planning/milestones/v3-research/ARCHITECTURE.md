# Architecture Patterns

**Domain:** NES emulator core in C17 (library, headless runner, libretro adapter), milestone v3 "Save states and rewind"
**Researched:** 2026-10-11
**Scope:** only what the new features need: fixed-size save states, libretro serialize / rewind / run-ahead / netplay, the runner process contract (state flags, signal flush, exit statuses) and speed measurement. The v2 architecture (time, mapper seam, PPU fetch pipeline) is unchanged and is not repeated here.
**Overall confidence:** HIGH for the inventory and the serialization design (read from the source); MEDIUM for the Windows signal test and the exact size figures (to be confirmed by measurement in the phase).

## 1. Recommended architecture in one paragraph

Add one new core module, `src/state.c`, that owns the state format. It contains **one field list per state struct, written once as a visitor** (`sio`), driven in three modes: SIZE (count bytes), WRITE (emit canonical little-endian bytes) and READ with a VALIDATE pass before an APPLY pass. Because the same list serves all modes, the size query, the writer and the loader cannot drift apart, and every field carries its legal range in the list, so "validated in full before any of it is applied" costs no staging copy. The mapper seam gets one new op (`state`) so each board declares its own register fields next to its own code. The cartridge gets one new derived field (a CRC-32 of the ROM identity, computed at load). The public header gets three functions and three appended status codes. The libretro adapter and the runner are thin clients of those functions. Nothing in the deterministic core gains a clock, a global, a float or an allocation.

## 2. State inventory (what is state, derived, or host-owned)

Read from `src/internal.h`, `src/mapper.h`, `src/cartridge.c`, `src/instance.c`, `src/frame.c`. "Serialize" means the field is in the state; "derive" means it is rebuilt after load; "host" means the host owns it and a load must leave it alone.

### 2.1 `struct nesturbator` (top level)

| Field | Class | Note |
|-------|-------|------|
| `allocator` | host | Never serialized; load must not touch it. |
| `frame_number` | serialize | Reported to the host in `nesturbator_frame`; a rewound state must report the frame it was taken at. |
| `ticks`, `cpu_cycle`, `audio_rem` | serialize | `ticks` can exceed a frame boundary by residual ticks of the crossing instruction; that residue is part of the state. `cpu_cycle` is compared against MMC1/MMC3 `last_write`/`low_cycle`, so it must be saved with them. |
| `save_generation` | host-monotonic | NOT serialized (rewinding it would break the "never decreases, compare with `!=`" contract). A load bumps it by one when the battery span's bytes changed (see 4.4). |
| `transition_sink`, `transition_sink_context` | host | Test hook owned by the runner's audio hash; untouched by load. |
| `profile` (`ane_magic`, `lxa_magic`) | serialize | Constants today. Serialize them so a future per-console profile does not silently change a loaded state. |
| `cpu`, `bus`, `apu`, `synth`, `ppu`, `mapper` | serialize (field by field) | See below. Never `memcpy` a struct: padding bytes are indeterminate and would leak into state hashes, and `uint64_t`/`int64_t` alignment differs between targets. |
| `cart` | mixed | Pointers and sizes are derived from the loaded image; the contents of the RAM regions are state (2.3). |
| `map` | derive | Entirely rebuilt by `nesturbator__mapper_load` (it memsets then runs `init` and `rebuild`). `map.a12` is a dead field today (no reader found in `src/`; MMC3 keeps its own `reg.mmc3.a12`): delete it in the tune-up phase so nobody serializes it by mistake. |

### 2.2 Component structs

| Struct | Serialize | Not serialized | Validation needed to keep a corrupt file memory-safe |
|--------|-----------|----------------|------------------------------------------------------|
| `cpu` | all 14 bytes: `pc a x y s p nmi_prev nmi_pending irq_line poll_latch halted_in_read jammed` | none | Booleans 0/1. `halted_in_read` must be 0 at a frame boundary (no mid-instruction state exists: run calls stop at instruction boundaries). |
| `bus` | `ram[2048]`, `open_bus`, `apu_get_put_phase`, `oam_dma_pending`, `oam_dma_page`, `input_buttons[2]`, `controller_latch[2]`, `controller_shift[2]`, `controller_strobe` | `input_pending[2]` (host-owned: the host sets it with `nesturbator_set_input` before every frame; `begin_frame` copies it to `input_buttons` before any CPU step) | Booleans. |
| `apu` and its channel structs | every field | `sample_output`, `sample_limit`, `sample_count` (frame-scoped caller-buffer bookkeeping; NULL/0 between frames, which is the only time the API can be called) | `pulse.phase` < 8, `triangle.phase` < 32, `dmc.bits` <= 8, `frame_mode` <= 1, `frame_cycle` below the sequence length, `sample_phase` below the sample period; all booleans 0/1. |
| `synth` | `mixed_level`, `impulse[16]`, `impulse_head`, `integrator_q15`, `hp_input_q30[2]`, `hp_output_q30[2]`, `lp_output_q30`, `overflow` | `pcm[1024]` ring content (see below) | `impulse_head` < 16, `pcm_count` == 0 (see below). 64-bit fields are written as two's-complement little-endian. |
| `ppu` | every field including `nametable[2048]`, `palette[32]`, `oam[256]`, `secondary_oam[32]`, the fetch pipeline latches and shifters, `ppu_ticks`, `scanline`, `dot`, `odd_frame`, `reset_flag` | `video_output`, `video_pitch` (frame-scoped caller buffer; NULL/0 between frames) | `scanline` < 262, `dot` < 341, `sprite_count` <= 8, `eval_*` indices within their 8/32-byte arrays, booleans. |
| `mapper` | `id`, `submapper`, `irq`, plus the board's union members, via the board's own `state` op | none | `id` and `submapper` must equal the loaded cartridge's; per-board ranges (MMC1 `count` < 5; MMC3 `select` and `r[]` are masked at rebuild but validate anyway). |
| `cart` | PRG RAM bytes (`prg_ram`, whole `[work][NVRAM]` allocation, including trainer-initialised bytes) and CHR RAM bytes when `chr_is_ram` | `bytes`, `prg`, `chr`, `prg_ram`, `save`, all sizes: pointers into the single allocation, derived from the image | Raw bytes: nothing to validate. ROM bytes are never in the state; the header's identity check (3.2) ties the state to the ROM. |

**Audio ring invariant to confirm first.** `apu_end_frame` drains exactly the frame's sample count, and `run_frame` refuses a frame whose audio capacity is below that count, so `synth.pcm_count` should be 0 at every call boundary. If a test proves that (assert it in the tune-up phase), the 2 KiB `pcm` ring and its read/write indices drop out of the state entirely (`pcm_read` = `pcm_write` = 0 on load, `pcm_count` validated as 0). If the invariant does not hold, serialize the ring in logical order (oldest first, zero-padded to capacity) and reset the indices on load, so two instances with the same behaviour produce identical bytes regardless of ring rotation. Either way, the write side must be **normalised**: state bytes must be a function of observable state, not of how the ring happens to be rotated.

### 2.3 Size

Fixed per loaded cartridge, independent of options and of history: about 5 KiB of core state (2 KiB CPU RAM + 2 KiB nametable RAM + 256 OAM + registers, shifters and synthesis history) + `prg_ram_size` (0, 8, 16 or 32 KiB; 8 KiB when a trainer forces RAM) + 8 KiB when the board has CHR RAM. Largest case is around 45 KiB; typical games 13 to 21 KiB. Libretro requires the size never to grow between load and unload (L7699); this design satisfies it because the size is a pure function of the cartridge layout.

## 3. The serialization format

### 3.1 Principles

1. Explicit little-endian, written byte by byte (`out[0] = v & 0xff; ...`), so the bytes are identical on every platform and the core needs no byte-order test.
2. No pointers, no padding, no struct images, no floating point, no host sizes (`size_t`) in the stream.
3. Fixed size per cartridge; no optional sections; no variable-length fields.
4. Layout stable across frames: the same field is at the same offset in every state of a given cartridge. That matters for RetroArch's rewind, which stores deltas between consecutive states; small, aligned, stable layout keeps those deltas small.
5. Format version is separate from the library version, ABI version and behaviour revision. Any change to the field set bumps `format_version`. Older formats are refused with a distinct status rather than half-loaded; add migration only if a real saved-state population exists to protect.

### 3.2 Header (64 bytes, all little-endian)

| Offset | Size | Field | Rule on load |
|--------|------|-------|--------------|
| 0 | 8 | `magic` = `"NTSTATE"` + `0x1A` (any fixed 8 bytes with a non-text byte) | exact match, else `ERR_STATE_FORMAT` |
| 8 | 2 | `format_version` (starts at 1) | exact match |
| 10 | 2 | `header_size` (64) | exact match |
| 12 | 4 | `total_size` | equals `nesturbator_state_size(inst)` and the buffer length |
| 16 | 4 | `behaviour_revision` at save time | recorded, **not enforced** (see 3.4) |
| 20 | 2 | `mapper_id` | equals loaded cartridge |
| 22 | 1 | `submapper` | equals loaded cartridge |
| 23 | 1 | `region` (0 = NTSC; PAL/Dendy reserved) | exact match |
| 24 | 4 | `rom_crc32` over header bytes 0..15 of the image, PRG ROM and CHR ROM | equals the value computed at load, else `ERR_STATE_CARTRIDGE` |
| 28 | 4 | `prg_rom_size` | equals loaded |
| 32 | 4 | `chr_rom_size` (0 for CHR RAM) | equals loaded |
| 36 | 4 | `prg_ram_size` | equals loaded |
| 40 | 4 | `chr_ram_size` (0 or 8192) | equals loaded |
| 44 | 4 | `checksum`: CRC-32 of the whole state with this field read as zero | mismatch gives `ERR_STATE_CORRUPT` |
| 48 | 16 | reserved, must be zero | non-zero gives `ERR_STATE_FORMAT` (leaves room to append header fields under a new format version) |

Why a CRC-32 and not a SHA: the core links only the C memory functions and must not grow a hash library; a 1 KiB const table is the cheapest integrity primitive that catches truncation and bit rot. It is **not** a security boundary: memory safety comes from the per-field range checks in the validate pass, not from the checksum (a fuzzer, or an attacker, can recompute a CRC).

Cartridge identity is computed **once at load** and stored as a new derived field (`cart.rom_crc32`, zeroed with the rest of `cart` on unload). Do not recompute at save: `cart.bytes` holds the image followed by mutable RAM, and only `[0, 16)` + PRG + CHR-ROM are immutable, which is exactly the hashed range. The runner's SHA-256 stays in the runner for the Playstead fingerprint; the core does not need it.

### 3.3 Body layout

After the header, nine sections in fixed order, each prefixed by a `u32 tag` and `u32 length` (8 bytes; they cost nothing and turn a field-list drift into an immediate, named load failure instead of silent mis-parsing):

`CORE` (frame_number, ticks, cpu_cycle, audio_rem, profile) | `CPU ` | `BUS ` | `APU ` | `SYNT` | `PPU ` | `MAPR` (id, submapper, irq, then board registers padded to a fixed 32 bytes so the section length does not depend on the board) | `PRGR` (prg_ram bytes) | `CHRR` (chr ram bytes).

Lengths are checked against the SIZE-mode result before any field is parsed. Boards pad their register block to the fixed `MAPR` length with zeros, which the loader requires to be zero.

### 3.4 Compatibility policy (opinionated)

- **Reject** on a different `format_version`, `region`, cartridge identity or sizes.
- **Accept** a different `behaviour_revision`. The revision says that emulation of the same inputs changed, not that the state layout changed. Rejecting on it would invalidate every user's saved states at each accuracy fix, which is the complaint users feel most in other cores. Expose the stored revision to hosts that want to warn or refuse (a small `nesturbator_state_info` query is cheap to add later; not needed for v3). Netplay peers must run the same library version anyway, and libretro netplay already compares core version.
- No migration code in v3. State format 1 is the first.

## 4. The visitor and the load algorithm

### 4.1 One field list, three modes

```c
/* src/state.c (internal). One list per struct; the same list sizes, writes and reads. */
enum { SIO_SIZE, SIO_WRITE, SIO_VALIDATE, SIO_APPLY };

struct sio {
    int mode;
    uint8_t *w;          /* WRITE: output cursor */
    const uint8_t *r;    /* VALIDATE and APPLY: input cursor */
    size_t n;            /* SIZE: running byte count */
    int bad;             /* VALIDATE: first range failure */
};

/* v points at the field; max is the largest legal value (UINT32_MAX when any value is legal). */
static void sio_u8 (struct sio *io, uint8_t  *v, uint32_t max);
static void sio_u16(struct sio *io, uint16_t *v, uint32_t max);
static void sio_u32(struct sio *io, uint32_t *v, uint32_t max);
static void sio_u64(struct sio *io, uint64_t *v);
static void sio_i32(struct sio *io, int32_t  *v);   /* two's complement, LE */
static void sio_i64(struct sio *io, int64_t  *v);
static void sio_bytes(struct sio *io, uint8_t *p, size_t len);

static void cpu_io(struct sio *io, struct nesturbator__cpu *c)
{
    sio_u16(io, &c->pc, 0xffff);
    sio_u8(io, &c->a, 255); sio_u8(io, &c->x, 255); sio_u8(io, &c->y, 255);
    sio_u8(io, &c->s, 255); sio_u8(io, &c->p, 255);
    sio_u8(io, &c->nmi_prev, 1); sio_u8(io, &c->nmi_pending, 1);
    sio_u8(io, &c->irq_line, 1); sio_u8(io, &c->poll_latch, 1);
    sio_u8(io, &c->halted_in_read, 0); sio_u8(io, &c->jammed, 1);
}
```

In SIZE and WRITE modes the destination field is only read; in VALIDATE the field is not touched (the value comes from the buffer and is range-checked); in APPLY it is stored. VALIDATE and APPLY run the **same** `*_io` functions over the instance, so validation needs no staging struct and no extra memory, and APPLY cannot fail because VALIDATE already walked identical bytes with identical ranges.

Each board adds one op to `struct nesturbator__mapper_ops`:

```c
/* Visits this board's registers, in a fixed order, at most 32 bytes in total. */
void (*state)(struct nesturbator__mapper *m, struct sio *io);
```

`struct sio` must be visible to the mapper files, so declare it in `mapper.h` (or a small `state.h`) and keep the helpers `static inline` or in `state.c` behind `internal.h`. Six boards today (NROM, UxROM, CNROM, AxROM, MMC1, MMC3), a few lines each. A future board that forgets the op fails the "every board round-trips" test (7.2) because its registers would not be restored.

### 4.2 Save

`nesturbator_save_state(inst, buf, capacity, &written)`: check args, `capacity >= size` else `ERR_BUFFER_TOO_SMALL`, write the header and sections in WRITE mode, `memcpy` the RAM regions, compute the CRC over the finished buffer (with the field zeroed), store it. `inst` is `const`: saving changes nothing. No allocation.

### 4.3 Load (all-or-nothing)

```
1. args, length, header checks         -> ERR_STATE_FORMAT / ERR_STATE_CARTRIDGE
2. checksum                            -> ERR_STATE_CORRUPT
3. VALIDATE pass over every section    -> ERR_STATE_CORRUPT   (instance untouched so far)
4. APPLY pass: core scalars, cpu, bus (except input_pending), apu (except frame-scoped
   pointers), synth, ppu (except frame-scoped pointers), mapper registers
5. memcpy prg_ram and chr ram bytes into the existing allocation (pointers do not move)
6. nesturbator__mapper_load(inst)  -> map zeroed, ops/watch/pages/nt[] derived from the registers
7. save_generation bump if the battery span changed
```

Step 6 is already designed for this: the `init` comment in `mapper.h` says registers are "kept by a state load, so init does not touch them", and `mapper_load` is documented as callable after a load. After step 6 every page-table pointer is recomputed from bank registers and the cartridge pointers, which never change.

An error status changes no state, matching the header's existing convention: all failure paths are before step 4.

### 4.4 Battery RAM interaction (a pitfall decision)

The state contains PRG RAM including the NVRAM span, as a console save state does. A load therefore overwrites battery bytes and the host's `.sav` cache must notice. Rule: after step 5, compare the span with its previous contents (one `memcmp` before the copy) and add 1 to `save_generation` only if they differ. Bumping unconditionally would make a run-ahead host (one load per displayed frame) rewrite the `.sav` every frame; not bumping would let the runner's `--save-interval` miss a change. `nesturbator_get_memory` pointers stay valid because step 5 copies into the existing allocation.

### 4.5 Frame-scoped pointers

`ppu.video_output`, `ppu.video_pitch`, `apu.sample_output`, `apu.sample_limit`, `apu.sample_count` are set inside `run_frame` and reset to NULL/0 before it returns, and the state API can only be called between frames (single-threaded instance). Save never writes them; load never touches them. A test asserts they are NULL/0 after every frame.

## 5. Public API additions (`include/nesturbator.h`)

All appended; no ABI version bump (the header's own rule: appending functions and status codes does not raise `NESTURBATOR_ABI_VERSION`).

```c
#define NESTURBATOR_STATE_FORMAT_VERSION 1

enum {  /* appended to nesturbator_status */
    NESTURBATOR_ERR_STATE_FORMAT    = 8,   /* magic, format version, header, size or reserved bytes wrong */
    NESTURBATOR_ERR_STATE_CARTRIDGE = 9,   /* state belongs to a different cartridge, or none is loaded */
    NESTURBATOR_ERR_STATE_CORRUPT   = 10   /* checksum mismatch or a field out of range */
};

/* Bytes of one state for the loaded cartridge; 0 with no cartridge (the test card has no state).
   Constant from a successful load until unload, the next successful load or destroy. */
size_t nesturbator_state_size(const nesturbator *inst);

/* Writes exactly nesturbator_state_size() bytes to buf. ERR_ARGUMENT (NULL, no cartridge),
   ERR_BUFFER_TOO_SMALL. Does not change the instance. Call between frames. */
nesturbator_status nesturbator_save_state(const nesturbator *inst, void *buf, size_t capacity,
                                          size_t *written);

/* Replaces the emulated machine's state. Checks everything before changing anything; any error
   leaves the instance unchanged. Keeps host-owned data: allocator, pending input, the audio
   transition hook, nesturbator_save_generation's monotonic count. The next frame needs a fresh
   nesturbator_set_input from the host. */
nesturbator_status nesturbator_load_state(nesturbator *inst, const void *buf, size_t size);
```

Header comments must document: byte order, that the format version is separate, that a state loads across behaviour revisions, what the battery span does on load, "set input again", the layout guarantee (size constant per cartridge, identical bytes on all platforms). README gets a "Save states" section in the same change (project rule 6).

Naming note: the file/format name for hosts is "state"; leave `.state` extension to adapters/runner flags.

## 6. Integration points: new versus modified

### 6.1 Library (`src/`, `include/`)

| Item | New / Modified | Detail |
|------|----------------|--------|
| `src/state.c` | **New** | `sio`, `*_io` per struct, header, CRC-32 table, three public functions. Depends on `internal.h` and `mapper.h` only. |
| `include/nesturbator.h` | Modified (append) | 3 functions, 3 status codes, 1 version macro, doc comments. |
| `src/mapper.h` | Modified | New op `state`; declare `struct sio` access. |
| `src/mapper_*.c` (6) | Modified | One small `state` function each. |
| `src/cartridge.c` | Modified | Compute `rom_crc32` at the end of load; `cart` struct gets the field (`internal.h`). |
| `src/internal.h` | Modified | `rom_crc32` in `nesturbator__cartridge`; state helpers declared. Remove dead `map.a12`. |
| `src/instance.c`, `frame.c`, `cpu.c`, `bus.c`, `ppu.c`, `apu.c`, `synth.c` | **Unchanged** if the audio ring invariant holds and every field is already in the instance | State must be a pure read of the existing structs. If an inventory step finds hidden state outside the instance (a function-local static, a derived cache), that is a v3 tune-up bug to fix first, because it breaks determinism, not only states. |
| `CMakeLists.txt` | Modified | Add `state.c` to the library. |

### 6.2 libretro adapter (`libretro/libretro.c`, `.info`)

Currently `retro_serialize_size` returns 0 and the other two return false; `nesturbator_libretro.info` has `savestate = "false"`. Changes, all in the adapter, none in the core:

| Entry point | Change |
|-------------|--------|
| `retro_serialize_size` | `nesturbator_state_size(inst)`; 0 when `inst` is NULL or the test card is running (states unsupported there, which is accurate). |
| `retro_serialize(data, len)` | return false if `inst` NULL or `len < size`; else `nesturbator_save_state(inst, data, len, &w)`; return `st == OK`. Allocation-free, so RetroArch can call it every frame for run-ahead. |
| `retro_unserialize(data, len)` | `nesturbator_load_state(inst, data, len)` with `len` clamped to the core's size when the frontend passes a larger buffer (frontends may pad); a shorter buffer is refused. Return `st == OK`. |
| `.info` | `savestate = "true"` and `savestate_features = "deterministic"` (this level adds rewind-friendly serialized states, run-ahead and netplay; claim it only after the determinism tests in section 7 pass in CI). |
| Quirks | Call nothing: a fixed-size, pointer-free, fixed-byte-order state sets no quirk bit. Do not call `SET_SERIALIZATION_QUIRKS`. |
| Adapter-owned buffers | `video[]`, `xrgb[]`, `mono[]`, `stereo[]` are not in the core state and need no handling: every `retro_run` regenerates them. Not carrying the last frame in the state is correct because a load is always followed by a run. The adapter's `retro_reset`/`input` path is unchanged: input is polled and set on every `retro_run`, so the pending-input rule (4.3) is satisfied. |

`tests/libretro/libretro_host.c` already resolves `retro_serialize_size/serialize/unserialize` via `dlsym`; extend it to exercise them (7.4). The RetroArch smoke job (`tests/retroarch/`) can add a config with `rewind_enable` / `run_ahead_enabled` as an optional later step; the library-level tests carry the proof.

### 6.3 Runner (`runner/main.c` and new small files)

The runner already has a frame loop, an options struct, a `save_state` struct (battery flush bookkeeping, not to be confused with emulator states; rename it `battery_save` when touched to avoid the collision), and exit codes 0, 1, 2 and 4.

| Item | New / Modified | Detail |
|------|----------------|--------|
| `--state-save N:FILE` | New (phase with the core API) | After frame N completes, `nesturbator_save_state` into a malloc'd buffer, write through the existing temp-file-and-rename helper `nesturbator_run_save_flush` (generalise it: it already does write, flush, sync, checked close, rename). |
| `--state-load FILE` | New | After cartridge load and battery load, before frame 1: `nesturbator_load_state`; any state error gives a distinct exit status. The load should happen after battery `.sav` load so the state, which carries the span, wins as the user expects. |
| `--hash-state N` | New | Prints `state N sha256 <hex>` using the existing `sha256.c`. This is the cross-platform proof that state bytes are identical on the six runners (7.3). |
| Signal handling | New (`runner/signals.c`) | See 6.4. |
| Exit status table | Modified | See 6.5. |
| `--capabilities` | New, optional | JSON: version, state format version, mappers, save media; writes nothing. Only if the Playstead requirement is accepted into v3 scope. |
| `--fps` mode | New | See 6.6. |

### 6.4 Signal flush design

- Handlers do exactly one thing: set a `volatile sig_atomic_t` flag. No file I/O, no `malloc`, no `printf` in a handler (async-signal-safety), and no flush from the handler.
- Install with `sigaction` on POSIX (`SA_RESTART` off is fine; no blocking reads exist) for SIGTERM and SIGINT, and SIGUSR1 as "flush now" where `SIGUSR1` is defined. On Windows, `signal(SIGINT/SIGTERM/SIGBREAK, ...)`; ordinary `signal()` is C17 and sufficient for the flag pattern.
- The frame loop checks the flag **between frames** (the same safe point where `--save-interval` flushes, "after frame f has completed and before frame f + 1 starts"). A frame takes milliseconds, so the 2 s budget Playstead gives between SIGTERM and SIGKILL is safe, with one caveat below.
- On the flag: break the loop, run the existing exit flush (`save_if_changed`, which already runs "whatever ended the loop, including a JAM"), print any pending hash lines, free, exit with the documented status. SIGUSR1 clears its flag, runs `save_if_changed`, and continues.
- Caveat: `fsync` on a slow or contended disk can exceed 2 s. The flush is one small write (a battery span is at most 32 KiB). Document the bound; do not add a second thread.
- Also handle "signal arrived before the loop started" (flag already set at loop entry), and "signal during the exit flush" (second signal ignored, flag already set; do not restart the flush).
- Crash signals (SIGSEGV, SIGBUS, SIGABRT) stay unhandled, as the preparation notes propose.

Test seam: a deterministic way to deliver a signal at a known frame without sleeping. Preferred: the test harness spawns the runner with a ROM that plays forever (`--frames 4294967295`, a battery game or the save ROM fixtures in `tests/runner/save_rom.c`), waits for its first interval-flush file to appear, sends the signal, and asserts exit status, that the `.sav` equals the in-memory span, and that no `.tmp` file remains. On Windows, graceful delivery needs `CreateProcess` with a new process group plus `GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT)`; if that proves flaky on hosted runners, cover Windows with a runner-internal `--raise-signal-after N` test option (compiled into the runner only, never the library) that calls `raise(SIGTERM)` and keep the real-process test on POSIX. This is the one item in this milestone with real uncertainty; flag it for phase research.

### 6.5 Exit status table (documented in README and `--help`)

Existing: 0 done, 1 failure, 2 usage, 4 save-file size mismatch. Preparation proposes 3 (hash mismatch, with `--expect`) and 77 (unsupported capability). Recommended complete table:

| Status | Meaning |
|--------|---------|
| 0 | Completed, or stopped by SIGTERM/SIGINT after a successful flush (Playstead sent the signal; no distinct status needed) |
| 1 | Failure (I/O error, JAM, frame error, failed flush) |
| 2 | Usage error |
| 3 | Hash mismatch (only if `--expect` is in scope) |
| 4 | Battery save file has the wrong size (already shipped) |
| 5 | State file rejected (maps from the three state statuses; the stderr message names which) |
| 77 | Capability refused (only if `--capabilities` is in scope) |

Exit 0 on SIGTERM is an explicit choice (alternative: a dedicated status such as 128+15). Choose 0 because the host that sent the signal knows it did, and a clean flush is a clean exit; a requirement that the host must distinguish them would change this. Keep the table in one `enum` in the runner and one test per row.

### 6.6 Speed measurement

- **Runner FPS mode** (`--fps`, frames per second and frame-time percentiles): the runner, not the core, reads a monotonic clock (`clock_gettime(CLOCK_MONOTONIC)` / `QueryPerformanceCounter`; wrapped in one small `runner/clock.c`). The core stays free of time sources. Skip video conversion and PPM dumping; time only `nesturbator_run_frame` per frame into a preallocated `uint64_t` array (no allocation inside the loop), then sort once at the end for p50/p95/p99. Print fps as an integer ratio or one decimal; the output is explicitly non-deterministic and must never be asserted in CI except for presence/shape. Run with a committed test ROM from `tests/roms/manifest.txt` and a movie so the workload is fixed.
- **Per-PR instruction-count comparison**: a `perf` job (Linux x64 only) runs `valgrind --tool=cachegrind` on `nesturbator-run --frames N --rom <fixed ROM> --movie <fixed movie>` for the PR head and for the merge base (both built with the same flags), compares total instructions (`I refs`) with a script under `scripts/`, and fails above a threshold (start at +3%, not 0; instruction counts are stable but compilers shift). Instruction counts, not wall time, because hosted runners are noisy. The job also checks the frame hash printed by the same run equals the merge base's, which makes "faster without changing a single hash" a CI property.
- **State cost belongs in the benchmark**: add one fixed workload that does save, run 1 frame, load per iteration (the run-ahead pattern). Targets to measure, not to assume: a save or load of about 20 KiB should cost far less than 1% of a frame (a frame is about 0.7 million ticks of work, several milliseconds). If the CRC-32 or the two-pass load shows up in the profile, the cheap fixes are a slicing-by-4 CRC table or skipping the checksum for loads of a buffer the same instance just produced (an internal flag, not public API).

## 7. How determinism is tested

All of these run under `cmake --workflow --preset ci` (and the same binaries under `asan`), with no human step.

### 7.1 Core test file `tests/core/test_state.c`

1. **Size and shape**: `nesturbator_state_size` is 0 for a fresh instance and for the test card; after loading each fixture ROM it is constant across 1000 frames and equals bytes written; `save` into a buffer 1 byte short returns `ERR_BUFFER_TOO_SMALL` and writes nothing.
2. **Idempotence**: save, load into the same instance, save again: byte-identical. Save from instance A, load into a *fresh* instance B with the same ROM, save from B: byte-identical.
3. **Fork test (the central one)**: run A for N frames driven by a movie, save S. Continue A for M frames, recording per-frame video hash, per-frame audio hash, ticks, and the final state bytes. Create B, load the ROM, `load_state(S)`, run the same M frames with the same inputs: every frame hash, audio hash, tick count and the final state bytes equal A's. Pre-poison B's output buffers (fill 0xFF) and run B a few unrelated frames before the load, so any field missing from the state shows as a divergence.
4. **Rewind**: with one instance, save at N, run M, load, run M again: identical to the first M. Repeat for several N including N = 0 (right after load) and a frame inside heavy PPU activity.
5. **Run-ahead pattern**: loop for K frames: save; set input; run one frame; (optionally run a second frame with a different speculative input); load; set the real input; run the real frame. The sequence of real-frame video/audio hashes and the final state bytes equal an uninterrupted run with the same inputs. Include a variant that does 2 or 3 save/load cycles per real frame.
6. **Every board**: the above for NROM, UxROM, CNROM, AxROM, MMC1 (battery) and MMC3 (battery, IRQ-heavy) using the ROMs already in `tests/roms`, plus a battery span check (4.4): a load that changes the span bumps the generation by exactly one; a load that does not, leaves it unchanged.
7. **State-save round trip across behaviours that exercise hidden state**: save mid-OAM-DMA, mid-DMC-DMA, with a pending NMI/IRQ, right after `nesturbator_reset` (PPU write-ignore window active), after a JAM (state loads and the instance is still latched). The frame-boundary-only rule makes these reachable through frame-step positions chosen by scanning for them with `nesturbator_peek_cpu_ram`-free instrumentation; where a case cannot be reached from the API, test it at the `sio` level on a hand-built struct.
8. **Frame-scoped pointers**: after each frame `ppu.video_output`, `apu.sample_output` are NULL (white-box test with `internal.h`, like the existing core tests).

### 7.2 Negative tests

- Truncation at every length from 0 to size, wrong magic, wrong format version, non-zero reserved bytes, wrong total size, other cartridge, other board, each returns the intended status and **leaves the instance byte-identical** (compare `save_state` output before and after).
- **Flip-every-byte sweep**: for each byte offset of a good state, flip one bit, call `load_state`. Result is either `OK` (the flip landed in RAM bytes that are legal... which the checksum would catch, so expect `ERR_STATE_CORRUPT` for every flip outside the checksum field and for the checksum field itself) and **never** a crash or sanitizer report. A second variant recomputes the CRC after the flip (a test-only helper in `tests/`), so range validation is what is exercised: then `OK` or `ERR_STATE_CORRUPT`, never a crash, and on OK the instance must survive a few frames under ASan/UBSan without out-of-bounds access.
- A board with a missing `state` op fails a table-driven test that iterates `nesturbator__mapper_ops_for` ids 0..N.

### 7.3 Cross-platform byte identity

Runner prints `state N sha256 <hex>` for a fixed ROM and movie; the expected hashes are committed in `tests/runner/hashes.txt` alongside the existing frame hashes, and all six hosted runners must match. This is what proves explicit little-endian and the absence of padding or alignment leakage. It also pins the format: an accidental layout change shows up as a hash diff and forces a deliberate `format_version` decision.

### 7.4 Field-drift guards

- `tests/core/test_state_layout.c`: compile-time `sizeof` assertions for every state struct (`cpu`, `bus`, `apu` and channels, `synth`, `ppu`, `mapper`), guarded to 64-bit targets, with a comment pointing at `state.c`. Adding a field to a struct changes its `sizeof` (or the author consciously updates the number), which sends the author to `state.c`. This is a tripwire, not a proof; the fork test is the proof.
- Section `tag`/`length` pairs fail loudly when a `*_io` function changes without the layout constant.

### 7.5 libretro host test (`tests/libretro/libretro_host.c`)

Load game; `retro_serialize_size` > 0 and unchanged after frames; `retro_serialize` into an exact buffer succeeds, into a short buffer fails; run frames; `retro_unserialize` restores and the next frame's video hash equals the uninterrupted run's (reuse the host's frame hashing); a corrupted buffer returns false and the next frame still matches; a larger buffer is accepted; no-game (test card) reports size 0. Then the `.info` file's `savestate` and `savestate_features` lines are checked by an existing/added file-content test.

### 7.6 Fuzzing

Add `fuzz.state_loader` next to `fuzz.rom_loader` in `tests/fuzz`, built into the existing nightly libFuzzer job. The harness loads a fixed ROM per board (embedded or from the corpus selector byte), builds a valid state, XORs the fuzz input over it, **recomputes the CRC**, and calls `load_state`, then runs one frame if it returned OK (so a range-check gap becomes a sanitizer crash instead of a latent bug). Seed the corpus with real states. Add the replay build to the regular `ci` run as `fuzz.regress` already does for the ROM loader.

## 8. Patterns to follow

### Pattern 1: one field list, many modes
**What:** the `sio` visitor in 4.1. **When:** every serialized struct. **Why:** removes the classic three-copy bug (size function, save function and load function that disagree), documents each field's legal range where the field is listed, and keeps boards' register layouts beside their code.

### Pattern 2: validate, then apply
**What:** two passes over the same bytes, no mutation in the first. **When:** `load_state`. **Why:** the header contract "an error status changes no state" holds by construction, with zero staging memory.

### Pattern 3: derive, never save, anything the cartridge can recompute
**What:** page tables, nametable map, watch bits, ops table, cart pointers. **Why:** pointers would break across platforms and processes; the mapper seam was built for exactly this (`mapper_load` after a state load).

### Pattern 4: host-owned stays host-owned
**What:** allocator, pending input, transition hook, save generation are untouched by load. **Why:** run-ahead calls load every frame; any host data a load overwrote would corrupt the host's own bookkeeping.

## 9. Anti-patterns to avoid

| Anti-pattern | Why bad | Instead |
|--------------|---------|---------|
| `memcpy(&inst->ppu, ...)` of whole structs | Padding is indeterminate; `uint64_t` alignment differs by target; pointers inside; byte order | Field-by-field explicit LE |
| Serializing `map` or `cart` pointers | Dangling after load, differ per process | Derive via `mapper_load`; pointers never leave the instance |
| Serializing ROM bytes | Legal exposure (no ROM bytes in saved artefacts), size, pointless | Identity CRC and sizes in header; require the ROM loaded |
| Variable-size state (omit empty RAM, grow with options) | RetroArch needs a constant size for the session; rewind deltas | Size a pure function of the cartridge layout |
| Bumping `save_generation` on every load | Run-ahead would rewrite `.sav` each frame | Bump only when the span's bytes differ |
| Saving `save_generation` | Rewinding would make a monotonic counter go backwards | Host-monotonic, not state |
| Rejecting on `behaviour_revision` | Every accuracy fix would invalidate user states | Record, don't enforce |
| Doing I/O in the signal handler | Not async-signal-safe | Flag in handler, flush at the frame boundary |
| Putting a clock in the core for FPS | Breaks determinism rule and the "links only memory functions" rule | Measure in the runner |
| Wall-clock thresholds in CI | Hosted runners are noisy | Instruction counts under Cachegrind, hash equality as the correctness gate |
| A checksum as the only safety | Fuzzers and attackers can recompute it | Range checks on every field that indexes memory |

## 10. Data flow changes

```
host/runner/libretro                     core
-------------------                      ----
run_frame(io) ---------------------->    frame boundary reached, all frame-scoped pointers NULL
save_state(buf) -------------------->    sio WRITE over cpu,bus,apu,synth,ppu,mapper + RAM copies + CRC
        (bytes live with the host)
load_state(buf) -------------------->    header -> CRC -> VALIDATE -> APPLY -> RAM copy -> mapper_load
                                         (pages, ops, nt[] derived) -> save_generation maybe +1
set_input(...) --------------------->    pending input (host-owned, not touched by load)
run_frame(io) ---------------------->    begin_frame copies pending -> buttons; emulation continues
```

No new data flows from core to host except the three function results. No callbacks, no threads.

## 11. Scalability considerations

| Concern | Typical play | Rewind (10 s at 60 fps) | Run-ahead (1 to 2 frames) |
|---------|--------------|-------------------------|----------------------------|
| State size | 13 to 21 KiB | 600 states, about 8 to 13 MB uncompressed; fixed offsets let the frontend's delta compression work | same state, 1 save + 1 load per frame |
| Save/load cost | n/a | about 20 KiB copy + CRC, microseconds | must stay far below one frame; measured in the benchmark (6.6) |
| Allocation | none | none (caller-owned buffers) | none |
| Netplay | n/a | n/a | peers need identical bytes: guaranteed by LE + normalised ring + stable layout; checked by cross-platform state hash |

## 12. Suggested build order across phases

Phase numbers continue from v2 (which ended at Phase 10). Dependencies drive the order; the library API is the only hard prerequisite for the two adapters.

1. **Phase 11: Tune-up and state readiness.** The v2 debt items from the milestone scope, plus the items that make the state work small: delete dead `map.a12`; add the white-box assertions (frame-scoped pointers NULL after a frame; `synth.pcm_count` == 0 at a frame boundary); audit that no state lives outside the instance (grep for function-local `static` non-const objects; the core already claims none); rename the runner's `save_state` struct to `battery_save`. Standard patterns; no research needed.
2. **Phase 12: State format in the library.** `src/state.c`, `sio`, mapper `state` ops, `rom_crc32` at load, three API functions, status codes, header docs, README section, tests 7.1, 7.2, 7.4, plus the minimal runner flags `--state-save`, `--state-load`, `--hash-state` as test tooling so 7.3 (cross-platform hashes) runs in the same phase. **Ends with something to download and run**: the runner can write and resume a state. This is the largest phase and the one with real design weight; everything else depends on it.
3. **Phase 13: libretro serialize, rewind, run-ahead, netplay readiness.** Adapter entry points, `.info` flags, libretro host tests (7.5), the nightly state-loader fuzz target (7.6), optional RetroArch smoke with rewind/run-ahead enabled. Depends only on Phase 12. Claim `savestate_features = "deterministic"` only in the commit where the fork/run-ahead tests are green on all six runners.
4. **Phase 14: Runner process contract.** Signal flag and flush at frame boundary, SIGUSR1, exit status table (add 5, and 3/77 if scoped), README process-contract section, `--capabilities` if scoped. Independent of Phase 13; can be swapped with it if Playstead timing demands, since its only dependency on Phase 12 is the exit-5 state rejection. Needs research for the Windows signal test (6.4).
5. **Phase 15: Speed.** Runner FPS mode with percentiles, the Cachegrind `perf` job against merge base, state save/load microbenchmark workload, and only then any optimisation (CRC slicing, PPU hot paths). Last because it measures everything else, and because any optimisation is accepted only if every frame, audio and state hash is unchanged, which Phases 12 and 13 now make cheap to check.

## 13. Research flags for phases

| Phase | Flag |
|-------|------|
| 11 | Standard. Confirm the audio ring invariant first; it decides state layout (2.2). |
| 12 | Design is settled here; still needs a short spike to measure save/load cost and to confirm the field inventory is complete by running the fork test on all six boards. Highest value test: the poisoned-fresh-instance fork test. |
| 13 | Standard once Phase 12 lands. Verify exact `.info` key names and the levels they unlock against the pinned RetroArch docs at planning time (the preparation file lists them; I did not re-fetch them). |
| 14 | Needs deeper research: Windows graceful-signal delivery in CI, fsync latency bound against the 2 s budget, and whether SIGTERM exit 0 is acceptable to the host. |
| 15 | Needs a short spike: Cachegrind availability on the hosted Linux runner image and a stable threshold; whether the instruction count of a 5-second workload is stable enough across merge-base builds. |

## 14. Open questions

- Is `synth.pcm_count` always 0 at a frame boundary? (Decides whether the PCM ring is in the state; 2.2.)
- Is `behaviour_revision` mismatch really acceptable to the owner? (3.4: recommended accept-and-record.)
- Are `--capabilities` and `--expect` (exit 3, 77) in scope for v3, or only the signal flush and documented existing statuses? The table in 6.5 is complete either way; the rows are cheap to defer.
- Should the state of a no-cartridge instance exist? Recommended no (size 0): the test card has nothing worth saving, and libretro treats 0 as "no states", which is what a test-card session should report.

## Sources

- Source files read for this document (HIGH confidence, local): `include/nesturbator.h`, `src/internal.h`, `src/mapper.h`, `src/instance.c`, `src/frame.c`, `src/cartridge.c`, `libretro/libretro.c`, `libretro/nesturbator_libretro.info`, `runner/main.c`, `runner/save.h`, `tests/CMakeLists.txt`, `tests/libretro/libretro_host.c`, `tests/fuzz/rom_loader.c`.
- Project preparation notes (HIGH, local): `.planning/preparation/ARCHITECTURE.md` sections 2, 6, 7 and 8 (state requirements: fixed size, canonical byte order, no pointers, validated in full before apply; page tables rebuilt after load); `.planning/preparation/LIBRETRO-AND-RUNNER.md` sections 2 and 5 (serialization rules, `.info` levels, quirk bits, Playstead process contract and proposed exit statuses); `.planning/seeds/SEED-002-save-states-and-runner-contract.md`; `.planning/seeds/SEED-004-robustness-and-speed.md`.
- libretro serialization rules cited by line (L7699, L7707) come from the preparation notes' reading of the pinned `libretro.h` (MEDIUM; not re-read in this pass).
- Design choices not cited above (visitor pattern, CRC-32 integrity, two-pass load, Cachegrind instruction counts) are the author's recommendations from the code read, not external findings.
