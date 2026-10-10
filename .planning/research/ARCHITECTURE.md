# Architecture Patterns

**Domain:** NES emulator core in C17 (library, headless runner, libretro adapter), milestone v2: MMC1, MMC3, UxROM, CNROM, AxROM, battery saves, `reset`, Holy Mapperel and per-board hashes
**Researched:** 2026-10-09
**Scope:** only what the new features change. Read from the repo at branch `docs/milestone-v2` (src/, include/, runner/, libretro/, tests/) and `.planning/preparation/` (ARCHITECTURE sections 6-7, DECISIONS DEC.15-17, LIBRETRO-AND-RUNNER sections 1 and 5, NES-HARDWARE-PPU-CARTRIDGE). No GPL or LGPL emulator source was opened.
**Overall confidence:** HIGH for the integration points (read directly from the code); MEDIUM for hardware details taken from the preparation files (each cites its NESdev revision); the per-board bus-conflict defaults and the exact MMC3 sprite-fetch timing are flagged below for verification against test ROMs.

## Headline Findings

1. **The NROM path is hard-wired in six places** (table below). All six move behind one mapper seam before any new board is added. The seam is also the cheapest place to prove "NROM hashes did not change".
2. **The PPU does not yet have the state MMC3 needs, and it is not only A12.** `src/ppu.c` renders every background pixel from `t` plus the absolute scanline (`background_pixel`), and calls `nesturbator__ppu_read` twice per pixel for pattern bytes. Nothing in `nesturbator__ppu_run_until` advances `v` (no coarse-X increment, no Y increment at dot 256, no horizontal copy at 257, no vertical copy at 280-304). Consequences: (a) A12 would toggle at pixel rate, not at the hardware fetch schedule, so the MMC3 counter cannot be driven from `ppu_read` as it stands; (b) any mid-frame `$2005`/`$2006` write (the split-scroll status bar used by most MMC3 games and many MMC1 games) renders wrongly. The PPU fetch pipeline is a prerequisite for MMC3 and for the boot-to-play proof, not a mapper detail. Plan it as its own phase or plan, ahead of MMC3.
3. **`cpu.irq_line` is a single byte that `apu.c` overwrites** (`update_irq_line`). A mapper IRQ needs a second source, so the line must become the OR of sources.
4. **Battery RAM should cross the API as a stable pointer plus a generation counter**, not as copy-in/copy-out functions: libretro's `retro_get_memory_data` requires a pointer that stays valid from load to unload, and the runner can use the same pointer. One new function replaces four.
5. **Reset needs an API call (`nesturbator_reset`) and a mapper hook.** `retro_reset` is an empty function today and `nesturbator_load_cartridge` is the only place the reset state is built.

## Where NROM Is Hard-Wired Today

| # | Location | What is hard-wired | Needed instead |
|---|----------|--------------------|----------------|
| 1 | `src/cartridge.c` `validate_image` | Mapper must be 0 in both iNES and NES 2.0 branches; PRG must be 16 or 32 KiB; CHR must be 8 KiB; iNES flag bits 1 and 3 (battery, four-screen) and trainer-less PRG-RAM rejected; NES 2.0 PRG-RAM/NVRAM sizes must be zero | Header parse produces a layout record (mapper, submapper, sizes, mirroring, battery, trainer); the board then accepts or rejects the layout |
| 2 | `src/cartridge.c` `nesturbator__cart_read` | `prg[(addr - 0x8000) & (prg_size == 32768 ? 0x7fff : 0x3fff)]`; also reached by the DMC fetch in `src/bus.c` `dmc_dma` | One page-table lookup shared by CPU reads and DMC |
| 3 | `src/cartridge.c` `nesturbator_load_cartridge` | Reset vector read as `prg[prg_size - 4]`; PRG-RAM allocated only when a trainer is present (8 KiB at `copy + size`); CHR-RAM fixed at 8 KiB; one allocation laid out by hand-computed offsets | Vector read through the page table after the board builds power-on mapping; named regions (image, PRG-RAM, CHR-RAM) laid out by one helper |
| 4 | `src/ppu.c` `nesturbator__ppu_read` and `nesturbator__ppu_write` (nametable branch, twice) | Mirroring read from `nes->cart.bytes[6] & 1` on every nametable access; CHR read as `chr[addr & 0x1fff]`; CHR writes allowed only if `chr_is_ram` | Per-instance `nt_map[4]` and `chr_off[8]` tables set by the board |
| 5 | `src/bus.c` `bus_read_data` and `nesturbator__bus_write` | `$6000-$7FFF` is flat `prg_ram[addr - 0x6000]` whenever `prg_ram != NULL`; `$8000+` writes fall through unhandled; no `$4020-$5FFF` route; no write-protect or enable | `$8000-$FFFF` writes (and optionally `$4020-$7FFF`) call the board; PRG-RAM goes through enable and protect flags and bumps the save generation |
| 6 | `src/apu.c` `update_irq_line` and `src/bus.c` `dmc_dma` | `cpu.irq_line` is assigned from APU flags only; `dmc_dma` also sets it directly | `irq_line` = APU frame IRQ OR DMC IRQ OR mapper IRQ, recomputed in one place |

Other fixed points that the work touches but that need no redesign: `src/frame.c` only checks `cart.bytes == NULL` for the test card; `nesturbator_unload_cartridge` and `nesturbator_destroy` free `cart.bytes`; the libretro adapter and runner call only `nesturbator_load_cartridge`, so they gain boards for free. Runner message text "malformed or unsupported mapper-0 cartridge" and the header comments for `NESTURBATOR_ERR_CARTRIDGE`, `nesturbator_load_cartridge` and `nesturbator_frame` ("mapper-0") must be reworded (CLAUDE.md rule 6: docs change with behaviour).

## Recommended Architecture

```
                       nesturbator.h  (append-only: reset, memory regions, save generation)
                                   |
  runner/main.c  ---------------- + ---------------  libretro/libretro.c
  (--save-dir, flush)                               (get_memory_data, retro_reset)
                                   |
                           src/instance.c, src/frame.c
                                   |
        +--------------------------+---------------------------+
        |                          |                           |
   src/cpu.c  <--- src/bus.c ---> src/apu.c              src/ppu.c
     |   irq_line = OR(sources)      |                      |  fetch schedule: every PPU
     |                               | $4020-$FFFF          |  bus access calls
     |                               | writes, PRG reads    |  nesturbator__ppu_bus_access
     |                               v                      v
     +------------------> src/mapper.c  (registry, page tables, dispatch)
                                   |
        +---------+---------+------+------+----------+----------+
   mapper_nrom  mapper_    mapper_  mapper_    mapper_    mapper_
        .c     uxrom.c    cnrom.c  axrom.c    mmc1.c     mmc3.c
                                   |
                          src/ines.c (header parse, layout)  <- src/cartridge.c (ownership, load/unload)
```

### Component Boundaries

| Component | Status | Responsibility | Communicates With |
|-----------|--------|---------------|-------------------|
| `src/internal.h` | MODIFIED | New `struct nesturbator__cartridge` fields, `struct nesturbator__mapper_ops`, per-board state union, `NESTURBATOR_MEMORY_*` internal helpers | everything |
| `src/ines.c` | NEW | Parse iNES 1 and NES 2.0 into `struct nesturbator__layout`; size arithmetic stays checked (`checked_add`, `checked_mul` move here); no allocation | `cartridge.c`, fuzz target via `nesturbator_load_cartridge` |
| `src/cartridge.c` | MODIFIED | Allocation of the single owned buffer (image copy, PRG-RAM, CHR-RAM), trainer copy, calls the board's `init`, unload; exposes `nesturbator__cart_read` unchanged in name | `ines.c`, `mapper.c`, `instance.c` |
| `src/mapper.c` | NEW | Registry keyed by mapper number (static const table of ops, no mutable statics), page-table helpers `map_prg(slot_kb, bank, size_kb)`, `map_chr(...)`, `set_mirroring(...)`, `nesturbator__cart_write`, `nesturbator__ppu_bus_access`, `nesturbator__irq_recompute` | `bus.c`, `ppu.c`, boards |
| `src/mapper_nrom.c` | NEW | The current behaviour, expressed through the seam; no register writes | `mapper.c` |
| `src/mapper_uxrom.c`, `mapper_cnrom.c`, `mapper_axrom.c` | NEW | Mapper 2, 3, 7: one latch each, optional bus conflict, AxROM single-screen select | `mapper.c` |
| `src/mapper_mmc1.c` | NEW | Serial shift register, consecutive-write filter, four registers, PRG-RAM enable, SxROM variants | `mapper.c`, `bus.c` (cycle count in write call) |
| `src/mapper_mmc3.c` | NEW | Bank registers, mirroring, PRG-RAM enable/protect, scanline counter driven by A12 with the M2 filter, IRQ output | `mapper.c`, `ppu.c` (A12), `bus.c` (IRQ line) |
| `src/bus.c` | MODIFIED | `$8000+` writes dispatch to the board; `$6000-$7FFF` through `cart` flags; DMC read via page table; call `nesturbator__irq_recompute` | `mapper.c`, `apu.c` |
| `src/apu.c` | MODIFIED (one line) | `update_irq_line` calls `nesturbator__irq_recompute` | `mapper.c` |
| `src/ppu.c` | MODIFIED (large for MMC3) | Mirroring and CHR through tables; one `ppu_bus_access` chokepoint; fetch schedule and `v` pipeline (see below) | `mapper.c` |
| `src/instance.c` | MODIFIED | `nesturbator_reset`, `nesturbator_get_memory`, `nesturbator_save_generation`; destroy unchanged | `cartridge.c` |
| `include/nesturbator.h` | MODIFIED (append only) | See "Public Header Changes" | all hosts |
| `runner/main.c` | MODIFIED | `--save-dir`, load `.sav` after cartridge load, flush on exit/interval; rewords the mapper-0 message | library API only |
| `libretro/libretro.c` | MODIFIED | `retro_get_memory_data/size`, `retro_reset` | library API only |
| `tests/...` | NEW and MODIFIED | See "Tests and CI" | |

### The Mapper Interface

Per-instance state only. The ops table is `static const` (read-only data is fine under "no mutable static"); per-board registers live in a union inside the instance, so a later save state is a memcpy of the union and a rebuild of the tables.

```c
/* Resolved cartridge header, produced by src/ines.c. */
struct nesturbator__layout {
    uint16_t mapper; uint8_t submapper;
    size_t prg_size, chr_size, chr_ram_size;
    size_t prg_ram_size, prg_nvram_size;   /* nvram is the battery part */
    uint8_t trainer, battery, mirror_vertical, four_screen;
};

struct nesturbator__mapper_ops {
    uint16_t id;
    /* Accept or refuse the layout (sizes, submapper); decide PRG-RAM size. */
    int  (*accept)(struct nesturbator__layout *layout);
    /* Power-on: set registers, then call the rebuild helpers. */
    void (*power_on)(struct nesturbator *nes);
    /* Warm reset: registers a real reset clears. NULL: nothing changes. */
    void (*reset)(struct nesturbator *nes);
    /* CPU write to $4020-$FFFF; cpu_cycle = nes->ticks / 24 at this access. */
    void (*cpu_write)(struct nesturbator *nes, uint16_t addr, uint8_t value, uint64_t cpu_cycle);
    /* Every PPU bus address, tagged with PPU time. NULL when the board
       watches nothing: the PPU pays one pointer test per access. */
    void (*ppu_bus)(struct nesturbator *nes, uint16_t addr, uint64_t ppu_ticks);
};
```

Cartridge-side view used by the buses (replaces `prg`/`chr` arithmetic):

```c
struct nesturbator__cartridge {
    uint8_t *bytes; size_t size;          /* one owned allocation, as today */
    uint8_t *prg, *chr, *prg_ram;         /* region bases inside it */
    size_t prg_size, chr_size, prg_ram_size, prg_nvram_size;
    uint32_t prg_off[32];                 /* 1 KiB pages, $8000-$FFFF, offset into prg */
    uint32_t chr_off[8];                  /* 1 KiB pages, $0000-$1FFF, offset into chr */
    uint8_t  nt_page[4];                  /* CIRAM 1 KiB page 0/1 for each nametable */
    uint8_t  chr_is_ram, prg_ram_enabled, prg_ram_writable;
    uint8_t  irq;                         /* board IRQ output level */
    uint64_t save_generation;
    const struct nesturbator__mapper_ops *ops;
    union { struct nesturbator__mmc1 mmc1; struct nesturbator__mmc3 mmc3; uint8_t latch; } m;
};
```

Decisions inside this shape:

- **Offsets, not pointers, in the page tables.** `uint32_t` offsets into `prg` and `chr` are rebuilt from the board registers, never serialised, and avoid pointer-width differences in anything later hashed or saved. DEC.15 (1 KiB pages) is kept: MMC3 banks CHR at 1 KiB and PRG at 8 KiB. 32 + 8 entries is 160 bytes.
- **Mirroring is a four-entry table**, set by helpers `mirror_horizontal/vertical/single(0|1)` plus a header default. This removes `cart.bytes[6]` from `ppu.c`, and makes AxROM single-screen and MMC1/MMC3 runtime mirroring the same code path. Four-screen stays rejected at load (it needs 4 KiB of cartridge VRAM); say so in the header comment.
- **PRG-RAM is allocated by the layout, not by the trainer.** Allocate when: battery, trainer, NES 2.0 declares PRG-RAM/NVRAM, or the board is MMC1/MMC3 and the header is iNES 1 (default 8 KiB; 32 KiB only when NES 2.0 says so, as SUROM/SXROM need). Battery-backed bytes are the first `prg_nvram_size` bytes of that region so the pointer handed to hosts is stable and file-compatible.
- **Reads.** `nesturbator__cart_read(nes, addr)` becomes `prg[prg_off[(addr >> 10) & 31] + (addr & 0x3ff)]`. `dmc_dma` keeps calling it, so DMC reads follow the banks with no extra code. The `cart.prg == NULL` guard in `dmc_dma` stays valid.
- **Writes.** `nesturbator__bus_write` adds `else if (addr >= 0x8000u) nesturbator__cart_write(nes, addr, value)` after the PRG-RAM branch. `nesturbator__cart_write` computes `cpu_cycle = nes->ticks / 24u` (the same expression `oam_dma` already uses) and calls the board. Because `cpu.c` performs the 6502's read-modify-write dummy write, MMC1's "ignore a write on the cycle right after another write" rule works from this one number.
- **Bus conflicts** are a shared helper `nesturbator__bus_conflict(nes, addr, value)` returning `value & nesturbator__cart_read(nes, addr)`, called by the board only when its conflict flag is set. The flag comes from the NES 2.0 submapper (mapper 2 and 3: 1 none, 2 AND; mapper 7: per the NESdev table). For submapper 0 (all iNES 1 files): AND for mappers 2 and 3 (preparation notes all licensed dumps are submapper 2), none for mapper 7 because the dump split is 27 without to 31 with. MEDIUM confidence; Holy Mapperel and a synthetic conflict test settle it.
- **Open bus.** Reads of disabled PRG-RAM (MMC1 RAM-disable, MMC3 `$A001` bit 7) must return the open-bus latch. Today's `bus_read_data` already falls through to `open_bus` when `prg_ram == NULL`; make the condition `prg_ram != NULL && prg_ram_enabled`.

### IRQ Line

Replace the single assignment with a recompute:

```c
/* src/mapper.c */
void nesturbator__irq_recompute(struct nesturbator *nes)
{
    nes->cpu.irq_line = (uint8_t)(((nes->apu.frame_irq != 0u && nes->apu.frame_irq_inhibit == 0u) ||
                                   nes->apu.dmc.irq != 0u) || nes->cart.irq != 0u);
}
```

`apu.c`'s `update_irq_line` and `dmc_dma` call it. The mapper sets `cart.irq` and calls it when the counter fires or is acknowledged (`$E000` write on MMC3). Timing needs no new mechanism: `cycle()` in `bus.c` runs the PPU to the M2 rising edge and only then latches `poll_latch = irq_line`, so an A12 edge detected during PPU catch-up is seen by the CPU's poll in that same cycle, which is the hardware ordering the existing APU IRQ already relies on. `cpu_step` samples `poll_latch`; no CPU change.

### PPU Bus Observation and the MMC3 Counter

Add one chokepoint, `nesturbator__ppu_bus_access(nes, addr)`, which does `if (nes->cart.ops->ppu_bus) ops->ppu_bus(nes, addr, nes->ppu.ppu_ticks)`. Call it from every real PPU bus access:

- fetches from the (new) hardware fetch schedule in `ppu_run_until` (background: nametable, attribute, pattern low, pattern high; sprites: eight slots per line, including the unused-slot dummy fetches);
- `$2007` reads and writes (address `v`);
- the `$2006` second write when rendering is off (this is how games clock the MMC3 counter by hand; prep file HWP.12).

Pixel composition must stop calling the notifying read. Split `nesturbator__ppu_read` into a side-effect-free `ppu_peek` (used for pixel generation and by the `$2007` path's data lookup) and `ppu_fetch` (peek + `ppu_bus_access`). If the fetch pipeline is rebuilt with shift registers, pixel composition reads only the registers and the question disappears.

MMC3 counter, using only integer PPU time:

```c
static void mmc3_ppu_bus(struct nesturbator *nes, uint16_t addr, uint64_t ppu_ticks)
{
    struct nesturbator__mmc3 *m = &nes->cart.m.mmc3;
    uint8_t a12 = (uint8_t)((addr >> 12) & 1u);
    if (a12 == 0u) {
        if (m->a12_high != 0u) { m->a12_high = 0u; m->a12_low_since = ppu_ticks; }
        return;
    }
    if (m->a12_high == 0u) {            /* rising edge */
        m->a12_high = 1u;
        /* The counter ignores a rise unless A12 stayed low for three M2 falling
           edges, i.e. about three CPU cycles. [HWP.12] */
        if (ppu_ticks - m->a12_low_since >= 3u * 24u)
            mmc3_clock_counter(nes);
    }
}
```

`mmc3_clock_counter` reloads from the latch when the counter is zero or a reload is pending, otherwise decrements, and raises `cart.irq` when the result is zero and IRQs are enabled. The reload-on-zero versus new-behaviour distinction (Sharp versus NEC, submapper 4) is a one-flag difference chosen at `power_on`; implement the Sharp/"every line when latch is 0" behaviour first and pin it with the `mmc3_test_2` ROMs (HWP.20), then decide on submapper 4.

**Required PPU change for correct edges.** With rendering on, real A12 follows: background pattern fetches at dots 5-6 and 7-8 of each 8-dot group (1-256 and 321-336), sprite pattern fetches for slots 0-7 at dots 261-320 (8 dots per slot, low byte then high byte), nametable "garbage" fetches in between. With the usual configuration (background `$0000`, sprites `$1000`) A12 rises once per line at the first sprite fetch, around dot 260, which is where the counter clocks. The cheapest faithful implementation:

1. Keep sprite evaluation as is.
2. Replace the `dot == 257` bulk `sprite_fetch` with eight per-slot fetches at their hardware dots, always eight (an unused slot fetches tile `$FF` with the current sprite-size and pattern-table bits), each via `ppu_fetch`.
3. Add the background fetch schedule and the `v` machinery (coarse-X increment per 8 dots, Y increment at dot 256, horizontal copy at 257, vertical copy at 280-304 of pre-render when rendering is on), and drive pixels from `v` and shift registers rather than from `t` plus `y`.

Step 3 is also what makes split-scroll and mid-frame `$2006` tricks work. It will change frame output for any pinned ROM that relied on the old behaviour, so bump `NESTURBATOR_BEHAVIOUR_REVISION` and re-pin `tests/runner/hashes.txt` in the same change, with AccuracyCoin (`tests/accuracy/scoreboard.txt`) as the regression guard (a passing test must stay passing). If the roadmap wants a smaller first step, an A12-schedule-only shim (emit the right addresses at the right dots, leave pixel generation alone) is possible, but it leaves the scroll bug and costs rework; recommended against. LOW confidence on how many v1 hashes move; run the three NROM games before and after.

### Battery RAM Across the API

```
cart load ──> nesturbator__cartridge: prg_ram [ nvram | work ]  (stable for load..unload)
                    │ CPU write accepted (enabled, not protected, inside nvram)
                    ▼
            cart.save_generation += 1
                    │
   nesturbator_get_memory(SAVE_RAM) -> (ptr, size)      nesturbator_save_generation()
        │                                    │
 runner: after load, memcpy .sav -> ptr      runner: after each frame, if generation changed
         (size must equal); on exit/interval  and interval elapsed -> write temp file, rename
 libretro: retro_get_memory_data returns ptr; RetroArch itself reads/writes .srm via ptr
```

- The library never touches files (core rule: links only the C memory functions).
- **Load order matters:** trainer bytes are copied into PRG-RAM at load; a `.sav` copied in afterwards overwrites them, as on hardware. libretro does the same: RetroArch fills the save-RAM pointer after `retro_load_game` returns.
- Size is the iNES 1 default of 8 KiB when the battery bit is set and no NES 2.0 size is given, or the NES 2.0 `prg_nvram` size. File compatibility with existing `.sav` files holds when the byte order and size match (raw bytes, no header).
- A write counts toward `save_generation` only when it lands in the nvram span while the RAM is enabled and writable; MMC3's `$A001` protect bit and MMC1's enable bit therefore need no separate dirty logic.
- Generation is a monotonic counter the host compares with its last seen value. The preparation text mentions a host acknowledgement; skip the acknowledge call in v2 (nothing needs it, and the runner can compare locally). Revisit with SEED-002.
- Determinism: loading a `.sav` changes behaviour only when the host chooses to. The runner reads a file only when `--save-dir` is given, so existing hash commands are unaffected.
- Flush in the runner: temp file in the same directory, `fflush`, rename over the target (on Windows `rename` fails if the target exists; use the platform replace call in the runner, which may use the platform, not the core).

### Reset

`nesturbator_reset(inst)`: valid with or without a cartridge (no cartridge: no-op returning `NESTURBATOR_OK`).

| State | Warm reset |
|-------|-----------|
| CPU RAM, PRG-RAM, CHR-RAM, nametable RAM, palette, OAM | kept |
| CPU | `s -= 3`, `I` set, `pc` from `$FFFC/$FFFD` read through the page table after the board's `reset` hook; jammed cleared |
| APU | `$4015` equivalent (channels silenced), frame counter as after the last `$4017` write, synth history reset or kept (choose and document; keep `frame_number` and `ticks` monotonic "since create", as the header already says) |
| PPU | `control`, `mask`, scroll latch and `$2005/$2006` toggle cleared; dot position policy documented |
| Mapper | board `reset` hook: MMC1 shift register cleared and control OR `$0C`; MMC3 IRQ disabled and counters cleared; discrete boards keep or clear their latch per documented hardware (UxROM/CNROM boards keep it in real hardware only by luck; clear it to bank 0 to be deterministic and document) |
| Input, controller shift | cleared |

Implement it in two steps so v1 debt closes early: an NROM-only version in the tune-up phase that fixes the API shape (the `reset` ops pointer is NULL for NROM), then the board hooks arrive with each board. `retro_reset()` becomes `nesturbator_reset(inst)`. The reset vector fetch through the page table (instead of `prg[prg_size - 4]`) is part of the seam refactor, so power-on and reset share one code path.

### Data Flow Changes

CPU write to `$8000+`: `cpu.c` -> `bus_write` (cycle advance, PPU caught up to both M2 edges) -> `cart_write` -> board `cpu_write` (optional bus conflict) -> board updates registers -> board calls `map_prg/map_chr/set_mirroring` -> tables change. The very next PPU fetch or CPU read sees the new tables, because PPU catch-up completes before each access.

PPU fetch: `ppu_run_until` dot loop -> `ppu_fetch(addr)` -> `chr_off[addr >> 10]` or `nt_page[...]` -> byte; `ppu_bus_access(addr)` -> board `ppu_bus` (MMC3 only) -> maybe `cart.irq = 1` -> `irq_recompute` -> `cpu.irq_line` -> latched into `poll_latch` at the next M2 rise.

Save RAM: described above.

## Patterns to Follow

### Pattern 1: Seam first, with a no-change proof
**What:** Route NROM through the new seam as mapper 0 before adding a board. **When:** first mapper phase. **Proof:** `tests/runner/hashes.txt` frame and audio hashes, AccuracyCoin scoreboard and the libretro host test do not change, and `NESTURBATOR_BEHAVIOUR_REVISION` is not bumped.

### Pattern 2: Boards declare what they watch
**What:** `ppu_bus == NULL` for every board except MMC3 means the PPU loop pays a pointer test per access and nothing else (DEC.15). **When:** always; do not add a generic per-dot callback.

### Pattern 3: Rebuild tables from registers
**What:** Each board has one `rebuild(nes)` that reads its registers and fills `prg_off`, `chr_off`, `nt_page`. Writes only update registers, then call `rebuild`. **Why:** the same function serves power-on, reset and a later state load.

### Pattern 4: Synthetic cartridges in tests
**What:** Tests build iNES images in code (a few bytes of 6502 program, banks filled with marker bytes) instead of committing ROMs. **Why:** CLAUDE.md rule 3 allows no ROM bytes beyond `tests/roms/manifest.txt`; this needs no manifest line and tests register writes, conflicts, mirroring, the MMC1 serial protocol and the MMC3 counter precisely. Real ROMs (Holy Mapperel, an open-licence game per board) add frame-hash coverage on top.

## Anti-Patterns to Avoid

### Anti-Pattern 1: Notifying the mapper from pixel composition
**What:** Leaving `nesturbator__ppu_read` as the single read path and adding the mapper call inside it. **Why bad:** A12 would be sampled twice per pixel at the wrong times; the MMC3 counter would clock wildly or never. **Instead:** peek for composition, fetch-with-notify only from the dot schedule and the CPU-driven `$2006/$2007` paths.

### Anti-Pattern 2: Per-mapper `if (mapper == N)` in `bus.c` and `ppu.c`
**What:** Switch statements spread across the buses. **Why bad:** every board touches the hottest files; merge conflicts and regressions. **Instead:** the ops table; `bus.c` and `ppu.c` know only the page tables and two hooks.

### Anti-Pattern 3: Timestamps from wall clock or host calls
**What:** The MMC3 A12 filter or MMC1 consecutive-write rule using anything but emulated ticks. **Why bad:** breaks determinism. **Instead:** `nes->ticks / 24` and `ppu.ppu_ticks`, both already in the instance.

### Anti-Pattern 4: Separate dirty tracking per board
**What:** Each board sets a dirty flag when it thinks RAM changed. **Why bad:** missed or spurious saves. **Instead:** one write path for PRG-RAM in `bus.c` that bumps the generation.

### Anti-Pattern 5: Growing the allocation at runtime
**What:** Allocating on reset or bank change. **Why bad:** header says allocation happens only at create and load. **Instead:** everything sized at load from the layout; reset reuses it.

## Public Header Changes (`include/nesturbator.h`, append only, ABI stays 1)

| Change | Detail |
|--------|--------|
| `nesturbator_status nesturbator_reset(nesturbator *inst)` | Warm reset as the table above; NULL gives `NESTURBATOR_ERR_ARGUMENT`. Comment states what is kept and cleared. |
| `enum nesturbator_memory { NESTURBATOR_MEMORY_SAVE_RAM = 0, NESTURBATOR_MEMORY_SYSTEM_RAM = 1 }` | Values fixed once published. SAVE_RAM is the battery span only. SYSTEM_RAM (the 2 KiB `bus.ram`) is free to add and is what `RETRO_MEMORY_SYSTEM_RAM` and cheat/achievement tools read; include it only if the plan wants it (one `case`). |
| `nesturbator_status nesturbator_get_memory(nesturbator *inst, enum nesturbator_memory kind, void **data, size_t *size)` | Pointer valid from successful load until unload or destroy; `size` 0 and `*data` NULL when absent (no cartridge, no battery). Supersedes read/write-save functions: hosts copy to and from the pointer. |
| `uint64_t nesturbator_save_generation(const nesturbator *inst)` | Increments when battery RAM changes; 0 without a cartridge. |
| Reworded comments | `NESTURBATOR_ERR_CARTRIDGE` ("outside the supported profile": mappers 0-4 and 7), `nesturbator_load_cartridge` (mappers, battery, PRG-RAM sizes, trainer, four-screen still refused), `nesturbator_frame` ("Mapper-0 ..." sentences), `nesturbator_create` ("The CPU does not yet run during frames" is already stale). |
| `NESTURBATOR_BEHAVIOUR_REVISION` | Bump with the PPU pipeline change (and any reset behaviour that alters output), not with adding boards that change no existing output. |
| Not needed in v2 | A new status code (use `NESTURBATOR_ERR_ARGUMENT` for a wrong-sized save by the host's own check), a battery-layout version constant (raw bytes have no layout), an `nesturbator_info` extension. |

Guards to update in the same change: the public API declaration baseline (refreshed in v1 phase 3; look under `tests/` for the declaration-guard test in `tests/core/test_api.c` and `tests/cmake/vector_api_policy.cmake`), `tests/header/header_c.c` and `header_cxx.cpp` (new declarations must compile in both languages), `tests/cmake/global_symbols.cmake` and `undefined_symbols.cmake` (every non-static new symbol is `nesturbator__`-prefixed), and the `nofp` source scan (no floating point in the mapper code).

## Runner and libretro Changes

**Runner (`runner/main.c`, `usage`, README):**
- New option `--save-dir DIR`. After `nesturbator_load_cartridge`, derive `<rom basename>.sav`; if the file exists and `nesturbator_get_memory(SAVE_RAM)` has a matching size, copy it in; a size mismatch is exit 1 with a message (never silently accept).
- Flush when `nesturbator_save_generation` has changed: at exit, and every `--save-interval N` frames if given (default: exit only, keeping hash runs fast). Atomic temp-and-rename. Exit status unchanged (0, 1, 2).
- Without `--save-dir` nothing is read or written, so every existing hash command behaves as before.
- Reword "unsupported mapper-0 cartridge". A `--reset-at N` option is not needed: `nesturbator_reset` is tested from a C test calling the API, and the movie format is out of scope until SEED-002.

**libretro (`libretro/libretro.c`):**
- `retro_get_memory_data(RETRO_MEMORY_SAVE_RAM)` returns the pointer from `nesturbator_get_memory`, `retro_get_memory_size` its size; NULL and 0 with no battery or no game (L498 permits). Optionally `RETRO_MEMORY_SYSTEM_RAM` (id 2).
- `retro_reset` calls `nesturbator_reset(inst)` when `inst != NULL`.
- No file code in the adapter; RetroArch writes the `.srm` (LIBRETRO-AND-RUNNER section 1).
- `retro_unload_game`/`retro_deinit` already destroy the instance; the pointer must not be used after, which RetroArch guarantees.
- The existing `tests/libretro/libretro_host.c` gains: save RAM pointer non-NULL and size 8192 for a battery image, write through the pointer then read via CPU; `retro_reset` restarts execution (frame hash after reset equals a fresh instance's first-frame hash when RAM contents are cleared in the test image).

## Tests and CI Hooks

| Test | Location | Notes |
|------|----------|-------|
| Seam no-change | existing `tests/runner`, `tests/accuracy`, `tests/core/test_cartridge.c` | Must pass untouched before any board lands |
| Loader | `tests/core/test_cartridge.c`, `tests/fuzz/corpus/*` | The `mapper-bits` corpus entry changes meaning; regenerate corpus for new valid and invalid headers; keep the fuzz target unchanged |
| Board register tests | `tests/mappers/test_<board>.c` (new dir, registered in `tests/CMakeLists.txt`) | Synthetic images; MMC1 serial and consecutive-write case; MMC3 counter with scripted PPU addresses plus an end-to-end run; bus-conflict AND |
| Holy Mapperel | `tests/roms/holy-mapperel-*.nes` plus `tests/roms/manifest.txt` line (zlib, pinned commit, SHA-256) | It reports by on-screen code and beeps, not memory: pin frame hashes, and decode the 4-digit code from a dumped frame only if a hash proves too opaque |
| Per-board frame hashes | `tests/runner/hashes.txt` | One open-licence game or test ROM per board, six-platform matrix |
| Battery | `tests/runner` and `tests/libretro` | Write through game or test image, flush file, restart runner with same `--save-dir`, assert RAM equal; atomic rename covered on Windows runner |
| Reset | `tests/core` | After `nesturbator_reset`, RAM kept, CPU at reset vector, MMC1 control register `|= $0C` |
| Trainer through host | `tests/runner` and `tests/libretro` | Trainer-bearing image via `--rom` and via `retro_load_game`; the load path is already shared |

## Build Order

Dependencies drive it: everything needs the seam; MMC3 needs the PPU pipeline; battery needs a board that has PRG-RAM (MMC1); reset needs per-board hooks only for MMC1/MMC3.

1. **Tune-up inputs that touch architecture**: `nesturbator_reset` (NROM version) and `retro_reset`; the trainer-bearing host-path test. No dependency on later work and it fixes the API shape early.
2. **Mapper seam, NROM through it** (`ines.c`, `mapper.c`, `mapper_nrom.c`, page tables, mirroring and CHR via tables, IRQ OR, `$8000+` write route). No hash change. Highest risk of silent regression, so it goes first and alone.
3. **UxROM, CNROM, AxROM** (`mapper_uxrom.c`, `mapper_cnrom.c`, `mapper_axrom.c`): about 30-50 lines each; exercises PRG and CHR bank switching, single-screen mirroring and bus conflicts; first Holy Mapperel and per-board hashes. Cheap and validates the seam before the hard boards. (The seeds list MMC1 first; the order of these two phases is the roadmap's choice and does not change dependencies.)
4. **MMC1 plus battery saves** (`mapper_mmc1.c`, PRG-RAM allocation rules, `get_memory`, `save_generation`, runner `--save-dir`, libretro memory hooks, relax the header rejections for battery and NVRAM). Delivers the first end-to-end user-visible feature (saves in RetroArch).
5. **PPU fetch pipeline** (`v` machinery, hardware fetch schedule, `ppu_peek` versus `ppu_fetch`, per-slot sprite fetch). Behaviour revision bump, re-pin hashes. Do not combine with a board in the same plan.
6. **MMC3** (`mapper_mmc3.c`, A12 filter, IRQ, PRG-RAM protect, `mmc3_test_2` suite or the Holy Mapperel IRQ screens). Needs 5.
7. **Boot-to-play proof**: choose the game after steps 4-6 using what actually renders correctly; prefer a game that exists as an open-licence or user-supplied test under the "no ROM bytes" rule (CLAUDE.md rule 3). A scripted input movie (existing `runner/movie.c`) plus RAM peek (`nesturbator_peek_cpu_ram`) can assert the "defined game-state change" without a human.

**Research flags for phases:**
- Step 5 (PPU fetch pipeline): needs phase-level research against the PPU timing sections of NES-HARDWARE-PPU-CARTRIDGE.md and AccuracyCoin results; highest uncertainty and highest regression risk.
- Step 6 (MMC3): needs the exact IRQ reload semantics, M2 filter threshold and submapper variants verified against `mmc3_test_2`.
- Steps 2-4: standard patterns once the preparation docs are open.
- Step 4: check the SxROM family (CHR-bank bits reused for PRG-RAM enable and 256 KiB PRG half) and the NES 2.0 size fields for the chosen test game.

## Scalability Considerations

| Concern | One board family | All six + later tiers | Notes |
|---------|------------------|-----------------------|-------|
| CPU read cost | one table lookup | same | `prg_off` is two loads; measure if the vector suite slows |
| PPU cost | pointer test per access (NULL) | MMC3 adds a compare per access | Only rendering-enabled lines have accesses |
| State size | union sized to the largest board | MMC5/VRC add fields | Union is `memset`-able; save-state layout is SEED-002 |
| Next mappers (MMC2/4, VRC, FME-7) | add an ops table row | page-table tweaks, `cpu_cycle` hook | Ops needs one more optional hook (CPU-cycle clock) when the first IRQ-by-CPU-cycle board arrives; do not add it now |

## Sources

- `.planning/preparation/ARCHITECTURE.md` sections 6 and 7 (banking, events a mapper watches, memory regions, battery RAM) - HIGH, project decision record
- `.planning/preparation/DECISIONS.md` DEC.15-17, DEC.19 - HIGH
- `.planning/preparation/LIBRETRO-AND-RUNNER.md` sections 1 and 5 (save RAM ids, runner `--save-dir`, temp-and-rename flush) - HIGH
- `.planning/preparation/NES-HARDWARE-PPU-CARTRIDGE.md` section 2 (MMC3 A12 rule after three M2 falling edges, HWP.12), bus conflicts (HWP.13), submappers and battery counts (HWP.22) - MEDIUM (documentation summary citing NESdev wiki revisions)
- `.planning/preparation/CONFORMANCE.md` (Holy Mapperel zlib, boards covered, reporting method; `mmc3_test_2` HWP.20) - MEDIUM
- Code read directly: `src/internal.h`, `src/bus.c`, `src/cartridge.c`, `src/instance.c`, `src/ppu.c`, `src/frame.c`, `src/apu.c` (`update_irq_line`), `src/cpu.c` (IRQ and NMI sampling), `include/nesturbator.h`, `runner/main.c`, `libretro/libretro.c` - HIGH
- libretro.h memory and `retro_reset` contracts, as cited in LIBRETRO-AND-RUNNER (L498, L509, L3591, L7812-7826) - HIGH
