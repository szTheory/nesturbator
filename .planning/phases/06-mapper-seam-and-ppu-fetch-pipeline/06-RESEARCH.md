# Phase 6: Mapper seam and PPU fetch pipeline - Research

**Researched:** 2026-10-10
**Domain:** C17 NES emulator core: per-board cartridge interface (page tables, write hook, IRQ OR) and a dot-accurate PPU fetch pipeline with the PPU address bus (A12) as literal state
**Confidence:** HIGH on the current code, the gates and the fetch schedule; MEDIUM on predicted hash changes (measured with a rough scratch prototype, not the final code)

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

The owner chose all four areas and asked for a fan-out: research each one,
run an adversarial pass, then adopt one coherent set of recommendations
without further questions. The decisions below are that set.

#### Mapper interface (plan 1)
- **D-01:** New `src/mapper.h`. A `const struct nesturbator__mapper_ops *` is
  chosen at load from a small const array indexed by mapper number. Its
  members are `init` (power-on registers; sets the watch mask), `rebuild`
  (page tables and `nt[4]` from the bank registers), `cpu_write`, `ppu_a12`,
  plus a `ppu_read` slot reserved for MMC2/MMC5. A NULL member is never called,
  because the core guards each event with the instance's `watch` mask bit, not
  with a pointer test. NROM is `src/mapper_nrom.c`. No `switch` on the mapper
  ID appears in `bus.c` or `ppu.c`.
- **D-02:** Two structs:
  - `struct nesturbator__mapper` holds the serialisable state: ID, submapper,
    an `irq` bit, and a union of per-board register structs. It contains no
    pointers.
  - `struct nesturbator__map` holds the derived state, never serialised: the
    ops pointer, the 1 KiB CPU read and write pages for `$4000-$FFFF`, the
    CHR read and write pages (8 each), `nt[4]`, `watch`, and the last A12
    level.

  After a load, the core resolves the ops from the ID, `init` restores only
  `watch`, and `rebuild` recreates the pages (ARCHITECTURE 6). Only the
  members Phases 6–10 use exist. Watch bits are `CPU_WRITE`, `BUS_CONFLICT`
  and `PPU_A12`; `PPU_READ` and `CPU_CLOCK` stay reserved numbers with no
  code. — **Reversibility:** costly — every board module written in
  Phases 7–10 is built on these two structs and the ops signatures.
- **D-03:** A NULL CPU read page returns `bus.open_bus` unchanged, as today
  for `$4018-$5FFF` and for `$6000-$7FFF` without RAM. A NULL write page
  drops the write. A NULL CHR write page drops the write, as CHR-ROM does
  today. With `BUS_CONFLICT` watched, `cpu_write` receives
  `value & mapped PRG byte` (NESdev "Bus conflict"). Each bank number is
  reduced with one `map_prg`/`map_chr` helper, `% page_count`, against the
  size the loader validated, so a register value cannot point outside the
  allocation. This is a true modulo, because NES 2.0 sizes need not be
  powers of two.
- **D-04:** CPU cycle stamp: a new `uint64_t cpu_cycle` in the instance,
  incremented once in `bus.c`'s `cycle()`. `ticks/24` is not used, because it
  holds only on NTSC. `cpu_write` receives the index of the cycle whose data
  was on the bus, after `cycle()` ran and before `bus_write` returns.
- **D-05:** IRQ OR: `apu.c`'s static `update_irq_line` becomes the shared
  `nesturbator__irq_update(nes)`. It sets `cpu.irq_line` to the frame IRQ OR
  `dmc.irq` OR `mapper.irq`. It runs whenever any source changes, so it stays
  event-driven. The direct `irq_line = 1u` in `bus.c`'s DMC path becomes a
  call. Polling at the start of `cycle()` is unchanged.
- **D-06:** NROM byte-identity: `rebuild` maps a 16 KiB PRG ROM into both
  `$8000` and `$C000`. CHR pages point into `chr`, with write pages only for
  CHR-RAM. `nt[]` comes from header flags 6 bit 0, exactly as `ppu.c`'s
  `ppu_read` derives it today. A PPU read with no cartridge still returns 0.
  The trainer and PRG-RAM at `$6000` behave as now. Plan 1 changes no fetch
  order; only the lookup changes.
- **D-07:** Hook rules: hooks receive time as an argument and never read
  `nes->ticks`. They never advance time, touch the bus or allocate. They
  update only `nes->mapper`, and may call `nesturbator__irq_update`.
- **D-08:** Test board: a `mapper_test` ops table, compiled into tests only,
  records each `(addr, value, cpu_cycle)` write and exposes a settable IRQ bit
  and an A12-edge log. It drives success criterion 2 (cycle-stamped writes;
  the IRQ line equals the mapper source OR an APU source, each raised and
  lowered independently). In plan 2 it also drives the A12 cases.

#### PPU address bus and A12 (plan 2)
- **D-09:** The PPU drives `ppu.bus_addr`, the 14 bits on the pins, through
  one function. When bit 12 changes and `PPU_A12` is watched, that function
  calls `ops->ppu_a12(nes, level, tick)`, with the tick of that dot. Both
  edges are reported, so MC-ACC's falling-edge clock fits later. The core
  applies no filter: MMC3 (Phase 9) applies its own "three M2 falls with A12
  low" rule, using the stamps. Rendering off, or lines 240–260: the bus shows
  `v`, so `$2006` writes and `$2007` increments produce A12 edges the mapper
  sees. — **Reversibility:** costly — MMC3 and its variants in Phases 9–10
  depend on these edge semantics.

#### Fetch fidelity (plan 2)
- **D-10:** Two-dot accesses with a literal bus (DEC.10, NESdev "PPU
  rendering"):
  - **Address dot:** drives `bus_addr`, and ALE stores the low 8 bits in
    `ale_latch`. A12 changes on this dot.
  - **Read dot:** rebuilds the address from live state (`v`, the fetched
    tile, secondary OAM) and reads `(live high 6 bits << 8) | ale_latch`.
    Caching the full address would quietly collapse this into the one-dot
    model.

  The schedule in 1-based dots, with a comment saying that the wiki's
  260/324 figures for MMC3 are 0-based:
  - **Dots 1–256 and 321–336:** NT, AT, BG lo and BG hi, two dots each.
  - **Dots 257–320:** per sprite slot, two garbage-NT accesses, then sprite
    lo and hi. Empty slots fetch tile `$FF`. In 8×16 mode empty slots fetch
    from `$1xxx`.
  - **Dots 337–340:** two dummy NT accesses. On an odd pre-render frame, the
    skip drops the read on dot 340 but keeps the address dot at 339.

  Increments and copies:
  - Coarse X increments at 8, 16, …, 256, 328 and 336.
  - Y increments at 256.
  - The horizontal copy happens at 257.
  - The vertical copies happen at 280–304 on the pre-render line only.
  - Shifters reload at 9, 17, …, 257, 329 and 337. They shift on dots 2–257
    and 322–337, feeding in 1 (AC "BG Serial In"), and are frozen otherwise
    (AC "Stale BG Shift Registers").

  Sprite patterns load per slot, so the dot-257 batch in `sprite_fetch`
  disappears. So does `background_pixel()`, which reads `t`.
- **D-11:** Each fetch action runs at one fixed point in the dot: the end of
  the dot, the existing 8-tick boundary. ALE and /RD are not split across
  half-dots, the multiplexer's output latches are not modelled, and neither
  is the /WR delay line. Each may be added only when a failing CTest requires
  it. A `$2006` second write and a `$2007` access still apply immediately,
  as today. The documented 1 to 1.5 dot delay of the `t` to `v` copy is not
  modelled in this phase.
- **D-12:** New PPU state:
  - `bus_addr`, `ale_latch`
  - `bg_nt`, `bg_at`, `bg_lo`, `bg_hi`
  - 16-bit pattern shifters `bg_shift_lo` and `bg_shift_hi`
  - 8-bit attribute shifters, each fed by a 1-bit latch

  Existing sprite arrays stay; `sprite_count` now counts the slots loaded.
- **D-13:** Test hook: an internal per-dot trace of
  `(scanline, dot, bus_addr, A12, v)` for one scanline. It is reachable from
  tests through `internal.h` and is not public API. CTest cases assert:
  - coarse-X and Y increments
  - the 257 copy and the 280–304 copies
  - NT, AT and pattern addresses on their address dots
  - A12 levels, including the empty-slot `$FF` fetches and 8×16 mode
  - bus = `v` with rendering off

#### Split-scroll oracle (plan 2)
- **D-14:** New `tests/ppu/test_split_scroll.c`, CTest `ppu.split_scroll`. It
  is self-checking, with no golden hash, and is not added to
  `tests/runner/hashes.txt`. The fixture:
  - An NROM image with vertical mirroring, built with `tests/ines.h`.
  - CHR from a fixed-seed integer LCG.
  - Nametable tile `(cx*37 + cy*11 + nt*101) & 0xff`, attributes from the
    LCG, and 12 distinct background colours.
  - All of it filled through `$2006`/`$2007` with rendering off.

  The test writes registers directly with
  `nesturbator__ppu_register_write`, at dots reached by stepping
  `nesturbator__ppu_run_until`. The convention is: the write lands after dot N
  has been processed. No CPU is involved.
- **D-15:** Expected values are hand-derived `static const` tables of
  `{first_line, X, Y}` segments, each with a comment citing the NESdev "PPU
  scrolling" sentence it rests on. Each scanline of the frame is compared
  with a plain sampler of the 512×240 plane. Before rendering, the fixture
  checks that no other origin samples the same 256-pixel row, which guards
  against a test that passes for the wrong reason. Cases:
  - a second `$2005` write inside, then after, the pre-render 280–304 window
  - an X-only split, SMB-style, written before and then after dot 257
  - a fine-X change that takes effect at once
  - the classic `$2006`/`$2005`/`$2005`/`$2006` split

  Frame-test writes stay at least 2 dots from 256, 257, 280 and 304. The
  exact edge dots belong to the D-13 `v` cases. A failure message names the
  case, the scanline and the expected and actual coarse/fine X/Y, decoded by
  a search over all origins that runs only on failure.

#### Seam gate and re-pin evidence
- **D-16:** No new policy script; the gates that already exist do the work.
  Plan 1's automated check is:
  - `git diff --exit-code main -- tests/runner/hashes.txt tests/accuracy/scoreboard.txt`
  - no `sha256 <64 hex>` line added or removed in the `tests/CMakeLists.txt`
    diff against `main`
  - `#define NESTURBATOR_BEHAVIOUR_REVISION 4` still in place

  Six-platform proof for the seam commit: push the branch at that commit,
  open a draft PR, and wait for `CI required` to go green, using
  `gh run watch`. Only then push plan 2, so that the PR's
  `cancel-in-progress` does not cancel the seam run. Record that run's ID in
  VERIFICATION.
- **D-17:** Plan 2 bumps the revision to 5 in `include/nesturbator.h` and in
  the `tests/core/test_api.c` assertion, in one commit. Verification shows
  `git log main..HEAD -S'NESTURBATOR_BEHAVIOUR_REVISION 5'` naming exactly
  that commit. The re-pin table comes from
  `git diff -U0 main -- tests/runner/hashes.txt`, plus the sha256 lines of
  the `tests/CMakeLists.txt` diff, with one hand-written reason per row or
  game. A row whose `ticks` value changes is a red flag: the fetch pipeline
  should not move frame timing, so verification states "ticks unchanged" or
  explains why they changed.
- **D-18:** Six-platform agreement is shown by the existing `hash-equality`
  job (`tests/cmake/hash_inventory.cmake`, 36 keys). The scoreboard rule is
  enforced by the existing protected-main baseline
  (`tests/cmake/prepare_scoreboard_baseline.cmake`,
  `tests/accuracy/test_scoreboard.c`). Verification cites both from the PR
  run. Measure the three NROM games before re-pinning:
  `diff build/ci/tests/hashes.txt tests/runner/hashes.txt` gives the changed
  rows per game.

### Claude's Discretion
- Exact file split inside `src/ppu.c` (one `(dot & 7)` dispatch function is
  expected), helper names, and the trace hook's exact form.
- Whether the `$2007`-during-rendering coarse-X-and-Y increment ([HWP.05])
  lands in plan 2. It is the same `v` logic, so include it if it is cheap and
  costs no protected scoreboard row.
- Performance: record frames per second (or ns per dot) before and after
  plan 2 in the verification. This is not a gate. If the `watch` branch or
  the indirect call costs more than about 3% of frame time on the slowest CI
  runner, keep the ops semantics and switch dispatch to a board-kind
  `switch` in a `static inline` helper in `mapper.h`.

### Deferred Ideas (OUT OF SCOPE)
- A CTest policy coupling hash re-pins to a revision bump
  (`tests/cmake/repin_policy.cmake`). Add it only if a later phase re-pins
  existing NROM rows again, or a re-pin slips through without a bump.
- The `$2006` copy delay (1 to 1.5 dots), sub-dot ALE/RD split, multiplexer
  output latches. Add them when an AccuracyCoin page 20–22 failure is traced
  to one of them.
- The `PPU_READ` hook (MMC2/MMC4/MMC5) and the `CPU_CLOCK` hook (FME-7,
  Namco 163): reserved numbers only, built with their boards.
- Lazy PPU catch-up (ARCHITECTURE 2): still a later option, gated on
  identical hashes.
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| MAP-01 | NROM loads through the per-board mapper interface (page tables, a four-entry nametable map, a write hook stamped with the CPU cycle, mapper IRQ ORed with the APU sources), and every v1 frame and audio hash stays byte-identical. | Current-code map of every `cart.*` consumer (section "Current code"); the global-symbols constraint on the ops table (Pitfall 1); the existing tests that poke `cart` directly and must migrate (Pitfall 3); the seam gate commands (Validation Architecture). |
| MAP-02 | The PPU advances `v` with coarse-X and Y increments and the dot-257 and dot-280-to-304 copies, and fetches background and sprite patterns on their documented dots. A synthetic split-scroll test matches its expected frame. The behaviour revision is bumped once, and each re-pinned hash is listed with the reason it changed. | Dot-by-dot schedule with A12 levels (Code Examples); measured hash impact on the three NROM games (Summary); pixel/shift ordering; split-scroll and fetch test design. |
</phase_requirements>

## Summary

The PPU today is a per-dot stepper (`nesturbator__ppu_run_until`, 8 ticks per dot, called at both M2 edges of every CPU cycle) but it **draws each background pixel from scratch from `t`** (`background_pixel`, four `ppu_read` calls per pixel) and **fetches all sprite patterns in one batch at dot 257** (`sprite_fetch`). `v` is touched only by `$2006` and `$2007`; it never drives rendering. Cartridge access is hard-wired to mapper 0: `bus.c` calls `nesturbator__cart_read` for `$8000+` and writes `cart.prg_ram` directly; `ppu.c` indexes `cart.chr` and derives mirroring from `cart.bytes[6] & 1` in two places. The time base needs no change: a CPU cycle is 24 ticks, a dot is 8, the PPU catches up at +9 and +24 ticks of each cycle, and all fetch actions can run at the existing end-of-dot boundary (D-11).

I measured the milestone's largest regression risk with a **throwaway scratch prototype** (a copy of `src/` with the D-10 pipeline roughly implemented, not committed): against the committed `tests/runner/hashes.txt` baseline, DABG (boot and all three movies) stays byte-identical, Nesteroids frames 30/60/120/180 change, RHDE frames 30/60/120/180 change, frame 1 is unchanged everywhere, `ticks` are unchanged in every row, the six audio hashes are unchanged, and AccuracyCoin pages 2, 14 and 17 still pass. So the predicted re-pin is 8 of 36 rows, no ticks, no audio. The causes are real hardware behaviour: Nesteroids writes `$2005`/`$2001`/`$2003`/`$2004` about 300 times per frame while rendering (a per-scanline raster effect), and RHDE toggles PPUCTRL bit 4 mid-line 189 (dots 197 and 251). The final numbers must be re-measured on the real code; the prototype is evidence of shape, not of values.

Two findings contradict or refine locked decisions and must reach the planner as implementation constraints (details in Pitfalls and Open Questions): (1) **D-01's "const array of ops pointers" would fail the existing `abi.global_symbols` test on Linux** (`nm` types a `static const` struct holding function pointers as `d` under `-fPIC`; I reproduced this with GCC on Alpine), and that test runs only in the CI `nofp` job, not in `--preset ci` on a Mac; fill the ops table by code into the instance instead. (2) D-10's "feeding in 1" is wrong for the low bitplane: the wiki says the low plane shifts in 0 and the high plane shifts in 1.

**Primary recommendation:** Plan 1: add `src/mapper.h`, `src/mapper.c` (resolver and page helpers), `src/mapper_nrom.c`; build the ops struct **by value in `nes->map`** from a fill function (no file-scope table of pointers); route every `cart.*` access through the page tables; add `cpu_cycle` and `nesturbator__irq_update`; migrate the three ppu tests that poke `cart`; gate with the D-16 commands. Plan 2: implement the D-10 schedule as one `fetch_step()` called in place of `sprite_fetch`, keep pixel composition at dot `x+1` so frame timing and sprite-0 hit timing do not move, include the rendering-time `$2007` increment, bump to 5, re-pin once, and test by single-stepping dots and reading `ppu.bus_addr`/`ppu.v` through `internal.h` (no trace hook needed).

## Architectural Responsibility Map

The "tiers" here are the internal layers of the core library, not web tiers.

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| CPU address decode, open bus, write dispatch | Bus (`src/bus.c`) | Mapper pages | Bus owns `$0000-$401F` and open bus; everything `$4020+` is looked up in page tables (D-03). |
| PRG/CHR/nametable lookup, bank registers, bus conflicts | Mapper (`mapper_*.c`, `mapper.h`) | Cartridge loader | Per-board logic lives only in board modules; the loader validates sizes and selects ops. |
| Mapper IRQ ORed with APU IRQs | APU/interrupt glue (`nesturbator__irq_update`, in `apu.c`) | Mapper (sets `mapper.irq`) | Keeps one event-driven writer of `cpu.irq_line`; `bus.unit` already links `apu.c`. |
| CPU-cycle stamp | Bus `cycle()` | — | The only place every CPU, DMA and halt cycle passes through. |
| Fetch schedule, `v` increments/copies, shifters | PPU (`src/ppu.c`) | — | Literal PPU state (DEC.10). |
| PPU address bus and A12 edge reporting | PPU (one `set_bus` function) | Mapper `ppu_a12` hook | PPU is the only driver of the pins; the mapper only observes. |
| MMC3-style filtering of A12 | Mapper (Phase 9) | — | The core applies no filter (D-09). |
| Pixel composition | PPU | — | Stays at dot `x+1` so hit/NMI timing is unchanged. |
| Frame/audio hash evidence | Runner + CTest (`runner.write_hashes*`) + CI `hash-equality` | — | Existing gates; no new policy script (D-16). |

## Standard Stack

No new dependencies, no vendored files (`libretro.h` stays the only one). [VERIFIED: 06-CONTEXT.md specifics "No new dependencies in this phase"]

### Core
| Component | Version | Purpose | Why Standard |
|-----------|---------|---------|--------------|
| C | C17, extensions off | Core and tests | CMakeLists.txt sets `CMAKE_C_STANDARD 17`, `CMAKE_C_EXTENSIONS OFF` [VERIFIED: CMakeLists.txt:11-13] |
| CMake presets | 3.25+ (local 4.4.3) | `ci`, `asan`, `nofp`, `hygiene` workflows | `CMakePresets.json` [VERIFIED: read this session] |
| In-repo `tests/check.h` | n/a | `CHECK`, `CHECK_EQ_U64`, `CHECK_EQ_HEX`, `CHECK_DONE` | Existing pattern for every ppu/core test |
| In-repo `tests/ines.h` | n/a | Synthetic iNES builder (`struct ines_spec`, `ines_build`) | Used by `test_reset.c`; fixtures for the seam and split-scroll tests |

### Supporting
| Component | Purpose | When to Use |
|-----------|---------|-------------|
| clang-format 18 (`/opt/homebrew/opt/llvm@18/bin/clang-format`) | `hygiene` preset format check | Before every commit; `.clang-format`: LLVM base, 4 spaces, 100 columns |
| `gh` 2.101 | `gh run watch` for the six-platform seam proof (D-16) | After pushing the seam commit |
| Docker (local) | Optional ELF `nm` check of the ops table (Alpine + gcc) | To reproduce the `abi.global_symbols` risk without a Linux box |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| D-01 const pointer array | Ops struct filled by code into `nes->map.ops` (by value) | Same boundary (no `switch` in bus/ppu), passes `abi.global_symbols` on ELF; costs ~7 pointers per instance |
| D-01 const pointer array | Board-kind `switch` in a `static inline` in `mapper.h` | Already allowed by Discretion as the performance fallback; also avoids data symbols; adds a `switch` for every new board |
| D-13 trace hook | Single-step `ppu_run_until(nes, nes->ppu.ppu_ticks + 8)` and read `ppu.bus_addr`/`ppu.v` | No production code, no per-dot `if`; the test board's `ppu_a12` log gives the edges with ticks |

**Installation:** none. **Package Legitimacy Audit:** not applicable, this phase installs no external package.

## Architecture Patterns

### Current code (read this session)

- **Time base.** `NESTURBATOR_TICKS_PER_FRAME 714732u` and "NTSC has 24 ticks per CPU cycle and 8 per PPU dot" [VERIFIED: src/internal.h:8-12]. `cycle()`:
  ```c
  nes->ticks += 9u;
  ppu_catch_up(nes);
  nes->cpu.poll_latch = nes->cpu.irq_line;
  nes->bus.apu_get_put_phase ^= 1u;
  nesturbator__apu_clock(nes);
  nes->ticks += 15u;
  ppu_catch_up(nes);
  ```
  [VERIFIED: src/bus.c:16-30]. `nesturbator__ppu_run_until` runs `while (nes->ppu.ppu_ticks + 8u <= ticks)`; each iteration is one dot: `dot++`, vblank set/clear, `sprite_evaluate`, `sprite_fetch`, the odd-frame skip, the 341-wrap, then pixel composition at `x = dot - 1` for dots 1-256 [VERIFIED: src/ppu.c:249-306]. A CPU write at tick `T` therefore lands after `floor(T/8)` dots; PPU dot ends are at multiples of 8 ticks, CPU cycle ends at multiples of 24, so a write always lands between whole dots.
- **Dot numbering.** The code's `dot` equals the wiki's cycle number: dot 0 is the idle step (executed as the wrap step where `dot` reaches 341), dots 1-256 are pixels, `scanline == 261` is pre-render. [VERIFIED: src/ppu.c:289-304]
- **Odd-frame skip.** `if (scanline == 261 && dot == 340 && odd_frame != 0 && (mask & 0x18) != 0) { scanline = 0; dot = 0; odd_frame = 0; continue; }` [VERIFIED: src/ppu.c:282-288]. Dot 340 of line 261 is never processed; the step after dot 339 lands on (0,0).
- **Where the PPU renders.** `background_pixel` (src/ppu.c:5-45) recomputes every pixel from `t` and `fine_x` with four `ppu_read`s; `sprite_fetch` (src/ppu.c:128-166) loads all `eval_count` slots in one go at `dot == 257` regardless of `mask`; `compose_pixel` (src/ppu.c:168-196) sets the sprite-0 hit while composing pixel `x` at dot `x+1`. `sprite_evaluate` is already per dot and stays.
- **Cartridge access, all of it:**
  - `nesturbator__ppu_read`: `return nes->cart.chr[addr & 0x1fffu];` and the mirroring `if (nes->cart.bytes != NULL && (nes->cart.bytes[6] & 1u) != 0u) ciram = (uint16_t)((table & 1u) * 0x400u + within); else ciram = (uint16_t)((table >> 1) * 0x400u + within);` [VERIFIED: src/ppu.c:198-223]; `nesturbator__ppu_write` repeats the mirroring and writes CHR only `if (nes->cart.chr_is_ram)` [VERIFIED: src/ppu.c:225-247].
  - `bus_read_data`: `addr >= 0x6000u && addr < 0x8000u && nes->cart.prg_ram != NULL` then `addr >= 0x8000u && nes->cart.bytes != NULL` → `nesturbator__cart_read` [VERIFIED: src/bus.c:55-58]. `bus_write`: PRG-RAM write at `addr >= 0x6000u && addr < 0x8000u && nes->cart.prg_ram != NULL`; nothing else reaches the cartridge [VERIFIED: src/bus.c:149-150].
  - `dmc_dma`: guards on `nes->cart.prg == NULL`, reads with `nesturbator__cart_read(nes, dmc->address)`, and sets `nes->cpu.irq_line = 1u;` directly [VERIFIED: src/bus.c:66-69, 81, 93].
  - `nesturbator__cart_read`: `return nes->cart.prg[(addr - 0x8000u) & (nes->cart.prg_size == 32768u ? 0x7fffu : 0x3fffu)];` [VERIFIED: src/cartridge.c:205-208].
  - `instance.c`: destroy frees `cart.bytes`; reset returns early when `cart.bytes == NULL`; `frame.c` treats `cart.bytes == NULL` as "no cartridge, draw the test card".
  - Load: validates mapper 0 only (`if (mapper != 0u ...)` for NES 2.0 and `(image[6] >> 4) != 0u` for iNES 1), sizes 16 or 32 KiB PRG and 8 KiB CHR (or CHR RAM), allocates `cart.bytes` with extra 8 KiB for trainer PRG-RAM and CHR RAM, then `memset(&inst->ppu, 0, ...)`, so after a load the PPU is at scanline 0 dot 0 (create sets scanline 261, load overwrites it) [VERIFIED: src/cartridge.c:56-118, 139-203].
- **IRQ.** `static void update_irq_line` sets `cpu.irq_line = (frame_irq && !frame_irq_inhibit) || dmc.irq` and is called from `apu_clock`, `apu_reset`, `$4015` read/write and `$4017` write paths (src/apu.c:106-110; calls at 329, 358, 371, 386, 425, 467). The CPU samples it as `nes->cpu.poll_latch = nes->cpu.irq_line;` at the start of each cycle, after the first PPU catch-up. [VERIFIED: src/apu.c:106-110, src/bus.c:22]
- **Reset.** `nesturbator_reset` keeps `cart`, runs `ppu_reset`, `apu_reset`, `cpu_reset`; it does not touch `ticks` [VERIFIED: src/instance.c:158-176]. `cpu_cycle` must therefore be zeroed at load/unload only.
- **Link-trap facts.** `bus.unit` compiles `bus.c apu.c synth.c cartridge.c ppu.c` directly and links no library [VERIFIED: tests/CMakeLists.txt:500-505]; the library source list is `instance.c cartridge.c frame.c bus.c apu.c synth.c ppu.c palette.c palette_ntsc.c testcard.c` [VERIFIED: CMakeLists.txt:52-62]. `cpu.vectors`/`cpu.unit` link only the CPU object. So any new file that `cartridge.c`, `bus.c`, `ppu.c` or `apu.c` call into must be added to both lists, and `ppu.c` must not call into `cpu.c` (comment in `ppu_reset`: "The PPU file may not call cpu.c (bus.unit link trap)").

### Pinned hashes and the scoreboard gate

- **Hash pins.** `tests/runner/hashes.txt` holds 36 sorted rows: for each of `nesteroids`, `dabg`, `rhde` the `boot/frame {1,30,60,120,180}` rows (`frame N ticks T sha256 H`) and `boot/audio/pcm` and `boot/audio/transitions`; plus `dabg/{p0,p1,both}/frame {1,30,60,120,180}`. `runner.write_hashes` writes `${build}/tests/hashes.txt` (COST 50, fixture `hashes_txt`); `runner.write_hashes.content` does `cmake -E compare_files` against the committed file [VERIFIED: tests/CMakeLists.txt:102-116; tests/cmake/write_hashes.cmake:57-130; tests/runner/hashes.txt read in full]. The no-cartridge test-card hash `b49e9be4...` and its audio hashes are pinned inline in `tests/CMakeLists.txt` (the only `sha256 <64 hex>` lines there, lines 27, 35, 36, 44, 45, 80) and cannot change (no cartridge, PPU not run).
- **Cross-platform.** CI `build` job writes `hashes.txt` per leg and uploads `hashes-<os>-<arch>`; `hash-equality` requires exactly 6 artifacts, 36 rows, byte-identical [VERIFIED: tests/cmake/hash_inventory.cmake:14-65; .github/workflows/ci.yml:100-135].
- **AccuracyCoin protection.** Three CTest cases `accuracycoin.page{2,14,17}` run `nesturbator-run --accuracycoin-page N --rom tests/roms/accuracycoin.nes --scoreboard tests/accuracy/scoreboard.txt` [VERIFIED: tests/CMakeLists.txt:130-139]. For every page except 14, **every test on the page must return pass** and its exact result byte must equal the scoreboard row (page 14 is restricted to six named APU tests) [VERIFIED: runner/main.c:350-390, 495-560]. `scoreboard.txt` has 17 sorted rows (page 17 is `CHR ROM is not writable`, `PPU Read Buffer` = `0x41`, `PPU Register Mirroring`, `PPU Register Open Bus`, `Palette RAM Quirks`; page 2 the addressing-mode tests; page 14 the APU tests). `accuracy.scoreboard` fails when a prior protected-`main` PASS row is missing or no longer `pass`; in CI the baseline is `git fetch origin refs/heads/main` via `prepare_scoreboard_baseline.cmake` [VERIFIED: tests/accuracy/test_scoreboard.c:100-140; prepare_scoreboard_baseline.cmake:1-50]. **Consequence for "any newly passing test may be added":** a test from a page other than 2/14/17 can be added only by also registering that page in the `foreach(page IN ITEMS 2 14 17)` loop, and the whole page then has to pass.

### System Architecture Diagram

```
 CPU (cpu.c) ──bus_read/bus_write──▶ bus.c cycle():  +9 ticks ─▶ ppu_run_until ─▶ poll_latch=irq_line
                                          │             apu_clock ─▶ +15 ticks ─▶ ppu_run_until
                                          │             cpu_cycle++            (plan 1, D-04)
        addr decode                       ▼
   $0000-$1FFF RAM  $2000-$3FFF PPU regs  $4000-$401F APU/IO     $4020-$FFFF  ── map.cpu_r/cpu_w pages ──▶ cart bytes / PRG-RAM
                                                                         │ if watch&CPU_WRITE (value&PRG if BUS_CONFLICT)
                                                                         └──▶ ops->cpu_write(nes, addr, value, cpu_cycle)
                                                                                    │ updates nes->mapper, may call irq_update, rebuild
 apu.c  frame_irq, dmc.irq ──┐                                                      ▼
 mapper.irq ─────────────────┴──▶ nesturbator__irq_update ──▶ cpu.irq_line (level, polled at cycle start)

 ppu_run_until: per dot (8 ticks)
   dot++ ─▶ vblank flags ─▶ sprite_evaluate ─▶ fetch_step() ─▶ [odd skip] ─▶ [341 wrap] ─▶ compose pixel (dot 1..256)
                                  │
        rendering line & mask&0x18?──no──▶ set_bus(v)           (rendering off / lines 240-260)
                                  │yes
                  shift (2..257, 322..337) ▶ address/read action by (dot&7) ▶ inc X / inc Y / copies ▶ reload
                                  │ every bus address goes through set_bus(addr)
                                  ▼
                    ppu.bus_addr ──bit 12 changed & watch&PPU_A12──▶ ops->ppu_a12(nes, level, ppu_ticks)
                                  │
              ppu_read/ppu_write ─▶ map.chr_r/chr_w pages (0-$1FFF), map.nt[4] → CIRAM (nametable), palette
```

### Recommended Project Structure
```
src/
├── mapper.h        # structs, ops, watch bits, static inline page helpers (new)
├── mapper.c        # resolver (id -> ops fill), map_prg/map_chr, page installers (new)
├── mapper_nrom.c   # NROM ops (new)
├── internal.h      # struct nesturbator gains mapper, map, cpu_cycle; ppu gains D-12 fields
├── cartridge.c     # load/unload: resolve, init, rebuild; cart_read retires
├── bus.c           # pages + write hook + cpu_cycle++ + irq_update call
├── apu.c           # nesturbator__irq_update (was static update_irq_line)
└── ppu.c           # plan 1: CHR/nt via map; plan 2: fetch_step, set_bus, shifters
tests/
├── core/test_mapper.c      # core.mapper: test board, cycle-stamped writes, IRQ OR (plan 1)
├── mapper_test.h           # the test board's ops table (tests only)
├── ppu/test_fetch.c        # ppu.fetch: v/bus/A12 per dot (plan 2)
├── ppu/test_split_scroll.c # ppu.split_scroll (plan 2)
└── ppu/ppu_fixture.h       # builds and loads a synthetic NROM via ines.h; returns struct nesturbator * (migrates ppu.render/sprites/registers)
```

### Pattern 1: ops table filled by code, not a file-scope table (replaces D-01's const array)
**What:** `struct nesturbator__map` holds `struct nesturbator__mapper_ops ops;` **by value**. `nesturbator__mapper_resolve(nes)` switches on the mapper ID inside `mapper.c` (loader code, not `bus.c`/`ppu.c`) and calls `nesturbator__mapper_nrom_ops(&nes->map.ops)`, which assigns the function pointers.
**When to use:** always in the library; tests may point `nes->map.ops` at a copy of their own table.
**Why:** `abi.global_symbols` fails the library if `nm` shows any `B b D d C` symbol, and its own header comment says "There is no exception for ELF .data.rel.ro: under position-independent code a file-scope const table of pointers (to functions or strings) is placed there, and nm reports it as d. So the core keeps no such table." [VERIFIED: tests/cmake/global_symbols.cmake:1-15 quoted; reproduced with GCC `-O2 -fPIC` on Alpine: `static const ops_t nrom = { a };` listed as `d nrom`]. Apple's `nm` prints `s` for the same object, and the abi tests are registered only when `NESTURBATOR_NOFP` is on (tests/abi/CMakeLists.txt:19-26; CI job `nofp` runs on ubuntu-24.04 and ubuntu-24.04-arm with `CC: gcc-14`). So the violation would pass `--preset ci` on the owner's Mac and every `build` leg and fail only in the `nofp` job.
```c
/* src/mapper.h (sketch; names are the planner's to finalise) */
struct nesturbator;
struct nesturbator__mapper_ops {
    void (*init)(struct nesturbator *nes);    /* power-on registers, sets map.watch */
    void (*rebuild)(struct nesturbator *nes); /* pages and nt[] from bank registers */
    void (*cpu_write)(struct nesturbator *nes, uint16_t addr, uint8_t value, uint64_t cpu_cycle);
    void (*ppu_a12)(struct nesturbator *nes, uint8_t level, uint64_t tick);
    uint8_t (*ppu_read)(struct nesturbator *nes, uint16_t addr); /* reserved, never called in v2 */
};
/* mapper_nrom.c */
void nesturbator__mapper_nrom_ops(struct nesturbator__mapper_ops *out);
```
Do not write `static const struct nesturbator__mapper_ops nrom = {...}` in a library file.

### Pattern 2: page tables as 1 KiB pointers
`const uint8_t *cpu_r[48]` and `uint8_t *cpu_w[48]` for `$4000-$FFFF` (index `(addr - 0x4000u) >> 10`; `$4000-$401F` is handled before the lookup), `chr_r[8]`, `chr_w[8]`, `uint8_t nt[4]` (CIRAM 1 KiB bank number 0 or 1). Zero-initialised `struct nesturbator` (every existing unit test does `memset(&nes, 0, sizeof nes)`) yields NULL pages: reads return open bus or 0, writes drop, `nt[]` all 0 (single-screen A), `watch == 0`. That is the safe default for bus.unit and ppu.registers.
`struct nesturbator__mapper` must not be an empty union under `-Wpedantic` (C17 forbids empty aggregates): give the union an NROM member such as `struct { uint8_t unused; } nrom;`. [ASSUMED: -Wpedantic rejects an empty union in C17; the project builds with `-Wpedantic` per CMakeLists.txt:20 and `/W4` on MSVC]

### Pattern 3: one bus function, one writer of `bus_addr`
```c
static void set_bus(struct nesturbator *nes, uint16_t addr14)  /* addr14 already & 0x3fff */
{
    uint16_t old = nes->ppu.bus_addr;
    nes->ppu.bus_addr = addr14;
    if (((old ^ addr14) & 0x1000u) != 0u && (nes->map.watch & NESTURBATOR_WATCH_PPU_A12) != 0u)
        nes->map.ops.ppu_a12(nes, (uint8_t)((addr14 >> 12) & 1u), nes->ppu.ppu_ticks);
}
```
The stamp is `ppu.ppu_ticks`, the tick at the end of the dot, not `nes->ticks` (which can be ahead during catch-up). `last A12 level` in `map` (D-02) is derivable from `bus_addr`; keep one source of truth.

### Anti-Patterns to Avoid
- **A `switch (mapper_id)` in `bus.c` or `ppu.c`** (D-01). Resolve once at load.
- **Caching the full fetch address between the address dot and the read dot** (D-10): the hybrid-address behaviour at dot 257 needs the live high bits.
- **Moving pixel composition to match the hardware's "first pixel at cycle 4" pipeline delay.** It would move sprite-0 hit and so `ticks` for games that poll `$2002`. Keep `x = dot - 1`.
- **Changing frame timing or the dot count** in plan 2. The prototype leaves every `ticks` value unchanged; a changed `ticks` is a bug.
- **A mutable file-scope variable or table in `src/`** (`abi.global_symbols`, `ENGINEERING §1`). The test trace/log storage lives in the test.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Synthetic ROM images | A new byte-array builder | `tests/ines.h` (`ines_build`) | Already used by `core.reset`; supports mapper/submapper fields for later phases |
| Frame hashing and the inventory | A new hash script | `runner.write_hashes`, `runner.write_hashes.content`, `hash_inventory.cmake` | D-16/D-18: the existing gates are the evidence |
| Scoreboard protection | A new policy | `accuracycoin.page{2,14,17}`, `accuracy.scoreboard` | Already enforced against protected `main` |
| Loading a cartridge in a PPU unit test | Poking `nes.cart.bytes`/`cart.chr` by hand | `ppu_fixture.h` that builds a mini NROM with `ines.h`, calls `nesturbator_create` + `nesturbator_load_cartridge`, then edits `nes->cart.chr[...]` | The page tables exist only after `rebuild`; hand-poking leaves them NULL |
| The 8-dot dispatch | Per-dot function pointers or a state machine table | One `switch ((dot - 1) & 7)` inside `fetch_step()` | Matches D-10; a table of pointers would also trip `global_symbols` |
| A12 edge filter | A filter in the core | Nothing (Phase 9 mapper) | D-09 |
| LCG for the split-scroll fixture | A library RNG | A 5-line fixed-seed integer LCG in the test | Deterministic on all platforms |

**Key insight:** every piece of evidence this phase needs already has a gate. The work is to route data through the new seam without moving a single frame or audio bit (plan 1), and then to move exactly the bits the hardware moves (plan 2).

## Runtime State Inventory

Not a rename/refactor/migration phase in the data sense (no stored data, services, OS registrations, secrets or installed artifacts reference anything being renamed). One build-system item: the new source files must be added to `CMakeLists.txt:52-62` and to the `bus.unit` source list at `tests/CMakeLists.txt:500-505`. Stored data: none. Live service config: none. OS-registered state: none. Secrets/env vars: none. Build artifacts: stale `build/ci` directories are regenerated by the preset configure step.

## Dot-by-Dot Fetch Schedule (for plan 2)

Numbering: the code's `dot` (0-340) equals the wiki's cycle numbers; an action "at dot N" runs at the end of dot N (D-11). Lines: visible 0-239 and pre-render 261, with `rendering = (mask & 0x18) != 0`. `v` = 15 bits `yyy NN YYYYY XXXXX`.

| Dots | Action at that dot | `bus_addr` driven | A12 |
|------|--------------------|-------------------|-----|
| 0 | Idle. Wiki: the bus "appears to be the same CHR address that is later used to fetch the low background tile byte starting at dot 5" | BG-lo address of the first tile (see Open Question 1) | `PPUCTRL` bit 4 |
| `d%8==1` in 1-256, 321-336 | NT address dot (ALE latches low 8) | `0x2000 \| (v & 0x0FFF)` | 0 |
| `d%8==2` | NT read dot → `bg_nt` | (high 6 live) \| latch | 0 |
| `d%8==3` | AT address dot | `0x23C0 \| (v & 0x0C00) \| ((v >> 4) & 0x38) \| ((v >> 2) & 0x07)` | 0 |
| `d%8==4` | AT read dot → `bg_at = (byte >> (((v >> 4) & 4) \| (v & 2))) & 3` | | 0 |
| `d%8==5` | BG lo address dot | `((ctrl & 0x10) << 8) \| (bg_nt * 16) \| ((v >> 12) & 7)` | `ctrl` bit 4 |
| `d%8==6` | BG lo read → `bg_lo` | | `ctrl` bit 4 |
| `d%8==7` | BG hi address dot | lo address + 8 | `ctrl` bit 4 |
| `d%8==0` | BG hi read → `bg_hi`; **coarse X increment** (8, 16, ..., 256, 328, 336); at 256 also **Y increment** | | `ctrl` bit 4 |
| `d%8==1`, d in {9, 17, ..., 249}, 257, 329, and 337 | **Shifter reload**: low byte of each pattern shifter ← `bg_lo`/`bg_hi`; attribute latches ← `bg_at` bits | | |
| 2-257 and 322-337 | **Shift** pattern shifters left 1 (low plane shifts in 0, high plane shifts in 1); attribute shifters left 1 feeding the 1-bit latch | | |
| 257 | NT-garbage address dot **using `v` before the copy**; then **horizontal copy** `v = (v & ~0x041F) \| (t & 0x041F)` | `0x2000 \| (v & 0x0FFF)` (pre-copy) | 0 |
| 258 | NT-garbage read: high 6 bits live (post-copy), low 8 from the latch → the wiki's "mix" | | 0 |
| 257-320 per slot `s = (d-257)>>3`, `p = (d-257)&7` | p0 garbage-NT addr, p1 read, p2 garbage-NT addr, p3 read (these use `0x2000 \| (v & 0x0FFF)`, the post-copy `v`), **p4 sprite-lo address, p5 read, p6 sprite-hi address, p7 read** | p4: pattern address of the slot's sprite; p6: +8 | sprite table (below) |
| 280-304, pre-render only | **Vertical copy** `v = (v & ~0x7BE0) \| (t & 0x7BE0)` each dot | | |
| 337, 339 | NT address dots | `0x2000 \| (v & 0x0FFF)` | 0 |
| 338, 340 | NT read dots (discarded) | | 0 |
| Odd frame, line 261, rendering on | The wiki: "jumping directly from (339,261) to (0,0), replacing the idle tick at the beginning of the first visible scanline with the last tick of the last dummy nametable fetch". The existing code lands on (0,0) after dot 339. | | |
| Any dot with rendering off, or lines 240-260 | `bus_addr = v & 0x3FFF`; `$2006` second write and `$2007` increments call `set_bus` too | `v & 0x3FFF` | `v` bit 12 |

Sprite pattern address (p4/p6), from secondary OAM slot `(y, tile, attr, x)`:
- 8x8: `((ctrl & 0x08) << 9) + tile*16 + row` (+8 for hi), `row = target - (y+1)` with the existing wrap rule, flipped when `attr & 0x80`. A12 = `ctrl` bit 3.
- 8x16: `((tile & 1) << 12) + (tile & 0xFE)*16 + (row >= 8 ? 16 : 0) + (row & 7)` (+8 for hi). A12 = `tile` bit 0.
- **Empty slot** (`slot >= eval_count`): tile `$FF`, fetched and discarded, sprite loaded "with a transparent set of values". 8x8: `$0FF0/$1FF0 + row` (A12 = `ctrl` bit 3). 8x16: range `$1FE0-$1FFF`, A12 = 1. The row bits for an empty slot are not documented [ASSUMED]; only A12 is observable to a mapper.

Verification of the timing against a mapper's view: with BG at `$0000`, sprites at `$1000` and no sprites on the line, my prototype reports A12 rising at dots 261, 269, ..., 317 and falling at 265, ..., 321 (period 8, low for 4 dots), i.e. the first rise at wiki "260" 0-based. With BG at `$1000` and sprites at `$0000` it reports rises at 5, 13, ..., 253 (and the 8-dot cadence through the line), a fall at 257, a rise at 325 (wiki "324" 0-based) and falls at 329, 337. [VERIFIED: scratch prototype, `ppu_a12` log; MMC3 wiki: "the IRQ counter should decrement on PPU cycle 260" and "324 of the previous scanline"] The 0-based/1-based comment D-10 asks for is therefore correct.

### Pixel selection and ordering within a dot (derived)
Order inside one dot: (1) shift if the dot is in 2-257 or 322-337; (2) run the fetch action and the increments/copies/reload for the dot; (3) the existing wrap logic; (4) compose pixel `x = dot - 1` for dots 1-256. Pixel `x` takes bit `15 - fine_x` of the pattern shifters and bit `7 - fine_x` of the attribute shifters. This ordering reproduces: first tile in the high byte at dot 1 (after the 16 shifts of 322-337 and the reloads at 329 and 337), second tile reaching bit 15 at dot 9, reload at 9 refilling the low byte without disturbing bits 15-8. [ASSUMED: derived from the wiki text, validated by the prototype (DABG and the AccuracyCoin pages are byte-identical or still pass) but not by a hardware reference; the D-13/D-15 tests are the proof]

## Common Pitfalls

### Pitfall 1: the ops table becomes a writable-data symbol on ELF
**What goes wrong:** `abi.global_symbols` fails in the CI `nofp` job (Linux, gcc-14) even though `--preset ci` and every `build` leg pass.
**Why:** see Pattern 1. A `static const` struct or array of function pointers is `.data.rel.ro` under PIC.
**How to avoid:** fill ops by code into the instance (or use a `static inline` switch). Add a task: after the seam commit, wait for `nofp` (not only `CI required`'s aggregate view) and optionally reproduce with `docker run --rm -v "$PWD":/w alpine sh -c "apk add --no-cache gcc musl-dev binutils && gcc -O2 -fPIC -c ... && nm ..."` on the built object.
**Warning signs:** `abi.global_symbols: ... defines writable data` listing `d <name>`.

### Pitfall 2: seam changes a byte
**What goes wrong:** a v1 hash moves in plan 1.
**Where it can happen:** the DMC DMA path (reads `$8000+` through `cart_read`, bypasses `bus_read_data`); 16 KiB mirror at `$C000`; the trainer's PRG-RAM at `$6000-$7FFF` (8 pages); `$4018-$5FFF` and RAM-less `$6000-$7FFF` open bus; writes to ROM dropped; PPU read with `cart.bytes == NULL` returning 0; nametable mirroring from `bytes[6] & 1` for **both** read and write; the load path zeroing `ppu`/`bus` before the map is built.
**How to avoid:** one helper for "CPU read from `$4020+`" used by both `bus_read_data` and `dmc_dma`; rebuild after every `memset(&inst->ppu, ...)` in load; `cpu_cycle`, `map`, `mapper` zeroed in unload and at load start.

### Pitfall 3: existing tests poke `cart` and break at the seam (plan 1) or at the pipeline (plan 2)
Plan 1 (CHR and mirroring move into `map`):
- `tests/ppu/test_render.c` sets `nes.cart.bytes = header; nes.cart.chr = chr;` (lines 18-19).
- `tests/ppu/test_sprites.c` sets the same (lines 20-21).
- `tests/ppu/test_registers.c` `test_nametable_mirroring_follows_cartridge_header` flips `header[6]` between reads (lines 59-73) and expects horizontal then vertical mirroring.
- `tests/core/test_reset.c` and `test_cartridge.c` use the public loader and read `nes->cart.prg_ram`/`cart.chr` after load, which still exist.
Plan 2: `ppu.render` fails with the pipeline (measured: `pixels[0] == 0x21u failed: 15 != 33` and `pixels[0] == (0x21u & 0x30u) | (5u << 6) failed: 320 != 352`) because it starts at scanline 0 with unprimed shifters and relies on `t`; it must start at scanline 261 (as `ppu.sprites` does) or run a full pre-render line first. `ppu.sprites`, `ppu.registers`, `core.reset`, `core.frame` and `core.apu` all passed against the prototype. [VERIFIED: scratch prototype, tests compiled against it]

### Pitfall 4: moving pixel timing
Compose pixel `x` at dot `x+1` exactly as now. The wiki's sprite-0-hit "cycle 2" and "first pixel output at cycle 4" describe the hardware pipeline; changing the model's hit dot is a separate behaviour change that can move `ticks` in games that spin on `$2002` (Nesteroids and DABG poll). Out of scope; the prototype keeps timing and shows ticks unchanged.

### Pitfall 5: shifter polarity and order
Low pattern plane shifts in 0, high plane shifts in 1 (wiki: "The logical value of these constants is 1 for the high bitplane and 0 for the low bitplane"); D-10's "feeding in 1" is correct only for the high plane. Reload after the shift in the same dot. Attribute shifters are fed the latch value, not a constant.

### Pitfall 6: hybrid garbage-NT at dot 257
Address dot (257) must capture the low 8 bits from `v` **before** the horizontal copy; the read dot (258) rebuilds the high 6 bits from the **post-copy** `v`. The next garbage NT pair (259/260 and later) uses post-copy `v` for both. The wiki: "the lower eight bits reflect both increments of v that happen on dot 256 but the upper six bits have already been reloaded".

### Pitfall 7: `$2007` during rendering now matters, and flips a hash
Today `v` does not drive rendering, so `$2007` writes during rendering are harmless to the picture. With a `v`-driven pipeline, a plain `v += 1/32` mid-render corrupts scroll. The wiki: during rendering `$2007` access triggers a coarse X increment and a Y increment simultaneously and "is not affected by the status of the increment bit". Nesteroids does 81 `$2007` writes while rendering in 180 frames. Measured: with the plain increment its frame-30 hash is `acb7f630...`; with the rendering-time increment it is `11fed47a...` (frames 60/120/180 also differ). The choice changes the re-pinned values, so decide it before re-pinning and make it in the same commit as the pipeline; recommended: implement the documented behaviour (Discretion item, "cheap": two function calls in the `$2007` read and write paths, no protected scoreboard row affected, pages 2/14/17 still pass in the prototype). [VERIFIED: scratch prototype; CITED: https://www.nesdev.org/wiki/PPU_scrolling]

### Pitfall 8: state that is not reset
`ppu` is zeroed by `memset` at load and unload (new fields start at 0). `nesturbator_reset` keeps `v`, VRAM and now also the shifter and latch state; `core.reset` asserts specific fields only. Do not clear shifters on soft reset unless a test requires it. `cpu_cycle` survives a soft reset (like `ticks`).

### Pitfall 9: warnings and formatting are errors
The `ci` preset sets `CMAKE_COMPILE_WARNING_AS_ERROR ON`; flags are `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Wstrict-prototypes -Wmissing-prototypes -Wvla` (+ GCC `-Wduplicated-cond -Wlogical-op`), MSVC `/W4`. Every non-static function needs a prototype in `internal.h`/`mapper.h`. uint8/uint16 arithmetic needs explicit casts; `u` suffixes everywhere. `hygiene` runs clang-format 18 on all sources.

### Pitfall 10: release-please markers
`include/nesturbator.h` and `README.md` are release-please `extra-files`; do not put a second version string on a line carrying an `x-release-please` marker. The revision constant is on its own line (`include/nesturbator.h:53`). README line 535 says "the behaviour revision stays 4" about soft reset; reword it in plan 2 rather than leaving a stale number.

### Pitfall 11: stale shift state and the left-over sprite data
`sprite_count` is currently set at dot 257 even when rendering is off (it becomes `eval_count`, which is 0 because evaluation is skipped). Keep `sprite_count = eval_count` at 257 regardless of `rendering`, otherwise stale sprites reappear when rendering is re-enabled mid-frame. Sprite slot arrays are overwritten on dots 261-320 of the same line whose pixels end at 256, so there is no tearing.

## Code Examples

Increments and copies (from the wiki's pseudocode, integer-only):
```c
/* Source: https://www.nesdev.org/wiki/PPU_scrolling ("Wrapping around") */
static void inc_coarse_x(struct nesturbator *nes)
{
    uint16_t v = nes->ppu.v;
    if ((v & 0x001fu) == 31u) {
        v = (uint16_t)((v & ~0x001fu) ^ 0x0400u);
    } else {
        v = (uint16_t)(v + 1u);
    }
    nes->ppu.v = v;
}
static void inc_y(struct nesturbator *nes)
{
    uint16_t v = nes->ppu.v;
    if ((v & 0x7000u) != 0x7000u) {
        v = (uint16_t)(v + 0x1000u);
    } else {
        v = (uint16_t)(v & ~0x7000u);
        uint16_t y = (uint16_t)((v & 0x03e0u) >> 5);
        if (y == 29u) { y = 0u; v ^= 0x0800u; }
        else if (y == 31u) { y = 0u; }
        else { y = (uint16_t)(y + 1u); }
        v = (uint16_t)((v & ~0x03e0u) | (uint16_t)(y << 5));
    }
    nes->ppu.v = v;
}
```
Dispatch skeleton (shape that the prototype ran; helper names are the planner's):
```c
static void fetch_step(struct nesturbator *nes)
{
    struct nesturbator__ppu *p = &nes->ppu;
    unsigned d = p->dot;
    int render_line = p->scanline < 240u || p->scanline == 261u;
    if (!render_line || (p->mask & 0x18u) == 0u) {
        if (d == 257u) p->sprite_count = p->eval_count;
        set_bus(nes, (uint16_t)(p->v & 0x3fffu));
        return;
    }
    if ((d >= 2u && d <= 257u) || (d >= 322u && d <= 337u)) shift_background(nes); /* lo<<1|0, hi<<1|1 */
    if ((d >= 1u && d <= 256u) || (d >= 321u && d <= 336u)) {
        background_access(nes, (d - 1u) & 7u);              /* NT AT lo hi, address/read pairs */
        if (((d - 1u) & 7u) == 7u) { inc_coarse_x(nes); if (d == 256u) inc_y(nes); }
        if ((d & 7u) == 1u && d >= 9u) reload_shifters(nes);
    } else if (d >= 257u && d <= 320u) {
        if (d == 257u) { p->sprite_count = p->eval_count; reload_shifters(nes); }
        sprite_access(nes, (d - 257u) >> 3, (d - 257u) & 7u); /* garbage NT x2, lo, hi */
        if (d == 257u) p->v = (uint16_t)((p->v & ~0x041fu) | (p->t & 0x041fu)); /* after the address dot */
        if (p->scanline == 261u && d >= 280u && d <= 304u)
            p->v = (uint16_t)((p->v & ~0x7be0u) | (p->t & 0x7be0u));
    } else if (d >= 337u && d <= 340u) {
        if (d == 337u) reload_shifters(nes);
        dummy_nt_access(nes, d);                              /* addr on odd dots, read on even */
    }
}
```
Test-side stepping (no production hook): after `ppu_run_until(nes, nes->ppu.ppu_ticks + 8u)` the state is "dot N processed"; read `nes->ppu.bus_addr`, `nes->ppu.v`, `nes->ppu.dot`, `nes->ppu.scanline`. The test board's `ppu_a12` callback appends `(level, tick)` to a test-owned array.

Seam test shape (core.mapper, plan 1): build an NROM image with `ines.h`, `nesturbator_create`, `nesturbator_load_cartridge`, then set `nes->map.ops = mapper_test_ops; nes->map.watch = CPU_WRITE;` and call `nesturbator__bus_write(nes, 0x8000u, v)` several times, asserting the recorded `cpu_cycle` increases by 1 per write and that OAM DMA and DMC stall cycles also advance it. IRQ test: for each of the four combinations of `apu.frame_irq`(with `frame_irq_inhibit` 0), `apu.dmc.irq`, `mapper.irq`, call `nesturbator__irq_update` and compare `cpu.irq_line` with the OR; raise and lower each source independently.

## State of the Art (this codebase)

| Old Approach (v1) | New Approach (plan 2) | Impact |
|-------------------|-----------------------|--------|
| Pixels drawn from `t` per pixel; `v` irrelevant to rendering | `v`-driven fetch pipeline with shifters | Mid-frame `$2005/$2006/$2000` writes behave as on the console (delayed to the next copy) |
| Sprite patterns batch-loaded at 257 | Per-slot loads on 261-320, empty slots fetch `$FF` | A12 rises on the documented dots (MMC3, Phase 9) |
| Cartridge hard-wired in `bus.c`/`ppu.c` | Page tables, nt map, write hook | Boards in Phases 7-10 are single modules |
| `irq_line` written by the APU only | One `nesturbator__irq_update` OR of three sources | MMC3/FME-7 IRQ lines (Phases 8-9) |

**Deprecated/outdated:** `background_pixel()` (reads `t`), `sprite_fetch()` batch, `nesturbator__cart_read` (replaced by pages), `static update_irq_line`.

## Measured Impact on the Pinned Hashes (scratch prototype; re-measure on the real code)

Method: copied `src/` to a scratch directory, implemented the D-10 schedule roughly (ordering, shifter polarity and increments as in this document, plain `v` increment for `$2007` in the first variant), built a driver that prints `frame N ticks T sha256 H` exactly as the runner does, and compared with the unmodified sources (whose output equals the committed `tests/runner/hashes.txt` rows).

| Game | Result with the pipeline | Cause (measured) |
|------|--------------------------|------------------|
| dabg boot, p0, p1, both (20 rows) | byte-identical | no PPU register writes while rendering (`$2000` 2, `$2001` 3, `$2005` 4 in total) |
| nesteroids frames 30/60/120/180 | changed; frame 1 unchanged; `ticks` unchanged | about 294 `$2005`, 37 `$2001`, 38 `$2003` writes per frame while rendering, starting on lines 4-11; 2125 pixels on 35 rows (7-94) differ at frames 30 and 60 |
| rhde frames 30/60/120/180 | changed; frame 1 unchanged; `ticks` unchanged | `$2000` writes `0x89` at line 189 dot ~197 and `0x99` at dot ~251 (BG pattern table bit toggled mid-line); only row 189, columns 197-255 differ (50 and 57 pixels) |
| all six audio hashes | unchanged | `ticks` and CPU/APU timeline unchanged |
| accuracycoin pages 2, 14, 17 | pass, same frame counts (34, 203, 249) | |

Prototype speed: 180 Nesteroids frames took 0.15-0.16 s before and 0.20-0.21 s after (Apple clang -O2, one machine); about 1100 and 850 frames per second. Record the real numbers in VERIFICATION (not a gate).

Expected re-pin for the final code: those 8 rows (nesteroids and rhde frames 30/60/120/180), subject to the `$2007` decision above and to dot-0/odd-frame details; if DABG or an audio row changes, find out why before pinning. The per-row reason is a hand-written sentence like the causes above, obtained by dumping the frames before and after (`nesturbator-run --dump-frame N:FILE`) and diffing the rows.

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | `-Wpedantic` rejects an empty union, so the serialisable register union needs a placeholder member | Pattern 2 | Compile error (found immediately) |
| A2 | Empty-slot row bits (the low 4 address bits) are not documented; only A12 is observable | Schedule | None for A12; a future mapper that decodes pattern address low bits could differ |
| A3 | The pixel/shift ordering (shift, fetch/increments, wrap, compose, reload at the listed dots) is the correct reading of the wiki; the prototype agrees with DABG and AccuracyCoin but no hardware reference was run | Pixel selection | A one-pixel horizontal offset or wrong tile at line starts; caught by the D-15 split-scroll cases, which must be derived from the wiki independently, not from the implementation |
| A4 | Dot-0 bus value (BG-lo address) matters only to mappers (MMC3 behaviour on BG=$1000 and the odd-frame line) | Open Question 1 | Phase 9 counter differs from hardware by one clock on some alignments |
| A5 | Final re-pin set equals the prototype's 8 rows | Measured Impact | Plan 2 verification lists a different set; the process in D-17/D-18 still applies |
| A6 | `ppu_a12` tick should be `ppu.ppu_ticks` (dot-end) | Pattern 3 | Phase 9's M2-fall counting must use the same convention |

## Open Questions

1. **Dot-0 bus value.**
   - What we know: the wiki says dot 0's bus "appears to be" the BG-lo address; with BG at `$1000` that makes A12 high at dot 0, which interacts with the odd-frame skip and the MMC3 "counts twice every other frame" note.
   - What's unclear: whether D-10 intends to model it (it lists no dot-0 action).
   - Recommendation: drive `bus_addr` at dot 0 to the BG-lo address of the pending tile (`bg_nt` from the last dummy NT read, fine Y from `v`), hash-neutral, A12 = `ctrl` bit 4; add one ppu.fetch case. If the planner wants to defer it, record it in the plan as a Phase 9 input.
2. **`$2007` during rendering.** Recommend including it (Pitfall 7); needs an explicit decision in the plan because it changes the re-pinned Nesteroids values.
3. **Odd-frame last dummy read.** D-10: skip "drops the read on dot 340 but keeps the address dot at 339". Wiki: the last dummy NT fetch's tick replaces the idle tick at (0,0). A mapper sees one extra NT read at (0,0) on the wiki reading. Hash-neutral; A12-neutral (NT, A12 = 0). Pick one and test it; the existing `continue` structure makes "dropped" the zero-cost choice.
4. **D-01 deviation.** The "const array of ops pointers" must become a filled-by-code struct (or a switch helper) to pass `abi.global_symbols`. The planner should record this as an implementation constraint on D-01 (interface members, watch mask, NULL-member rule and "no switch in bus.c or ppu.c" all stay).
5. **OAMADDR forced to 0 during dots 257-320** (hardware behaviour in the wiki's OAM pages) is not modelled today and not required by this phase. Leave out; note for AccuracyCoin pages 19-22 work.

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| cmake | all presets | yes | 4.4.3 | none needed |
| ninja | presets | yes | 1.13.2 | none needed |
| Apple clang | local `ci`, `asan`, `nofp` builds | yes | 21.0.0 | none needed |
| clang-format 18 | `hygiene` preset | yes | `/opt/homebrew/opt/llvm@18/bin/clang-format` | none needed |
| gcc-14 / clang-18 (Linux) | CI `nofp`, `asan` jobs | no (local) | CI images | Docker (present) with Alpine gcc for a quick ELF `nm` check; the CI jobs are the gate |
| gh | `gh run watch` (D-16) | yes | 2.101.0 | none needed |
| Docker | optional ELF check | yes | present | skip, rely on CI `nofp` |
| python3 | scratch analysis only | yes | 3.14.4 | none needed (not a project dependency) |
| Network | NESdev wiki, CI | yes | n/a | |

Note: `cmake --workflow --preset nofp` can run locally with Apple clang, but Mach-O `nm` types const data as `s`, so a local pass does not prove the ELF result.

## Validation Architecture

`workflow.nyquist_validation` is `false` in `.planning/config.json`; this section is included because the orchestrator asked for it.

### Test Framework
| Property | Value |
|----------|-------|
| Framework | CTest with in-repo `tests/check.h` (no external framework) |
| Config file | `CMakePresets.json`, `tests/CMakeLists.txt` |
| Quick run command | `cmake --build --preset ci && ctest --preset ci -R "^(core\.mapper\|ppu\.\|bus\.unit\|core\.cartridge\|core\.reset\|core\.api\|accuracycoin\.\|accuracy\.scoreboard)"` |
| Full suite command | `cmake --workflow --preset ci` (parallel, `jobs 4`) |
| Other gates | `cmake --workflow --preset asan`, `--preset nofp`, `--preset hygiene`; CI `hash-equality`, `CI required` |

### Phase Requirements → Test Map
| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|--------|----------|-----------|-------------------|-------------|
| MAP-01 | NROM through ops table; v1 hashes byte-identical | existing + diff gate | `ctest --preset ci -R runner.write_hashes` and `git diff --exit-code main -- tests/runner/hashes.txt tests/accuracy/scoreboard.txt` | exists |
| MAP-01 | CPU write arrives with its CPU cycle (incl. DMA/halt cycles) | unit | `ctest --preset ci -R core.mapper` | Wave 0 (`tests/core/test_mapper.c`, `tests/mapper_test.h`) |
| MAP-01 | Mapper IRQ ORed with frame and DMC IRQ, each independent | unit | `ctest --preset ci -R core.mapper` | Wave 0 |
| MAP-01 | Page lookup edge cases: open bus, NULL write page, trainer RAM, 16 KiB mirror, CHR-ROM not writable | unit/integration | `ctest --preset ci -R "core.cartridge\|bus.unit\|accuracycoin.page17"` | exists (extend) |
| MAP-01 | Core still links only memory functions; no writable data | abi | `cmake --workflow --preset nofp` (CI Linux) | exists |
| MAP-02 | `v` coarse-X/Y increments, 257 copy, 280-304 copies, NT/AT/pattern addresses, A12 levels (incl. empty slots, 8x16), bus = `v` with rendering off | unit | `ctest --preset ci -R ppu.fetch` | Wave 0 (`tests/ppu/test_fetch.c`) |
| MAP-02 | Split-scroll frames match hand-derived expectations (4 cases) | unit | `ctest --preset ci -R ppu.split_scroll` | Wave 0 (`tests/ppu/test_split_scroll.c`) |
| MAP-02 | Revision is 5 exactly once; re-pinned hashes agree on six platforms | existing + git | `ctest --preset ci -R "core.api\|runner.write_hashes"`; `git log main..HEAD -S'NESTURBATOR_BEHAVIOUR_REVISION 5'`; CI `hash-equality` | exists (edit `test_api.c:96`) |
| MAP-02 | Protected AccuracyCoin rows keep passing | integration | `ctest --preset ci -R "accuracycoin\|accuracy.scoreboard"` | exists |
| MAP-02 | Existing PPU tests keep their meaning after the pipeline | unit | `ctest --preset ci -R "ppu\."` | `ppu.render` must be rewritten (Pitfall 3) |

### Sampling Rate
- **Per task commit:** the quick run command above (seconds).
- **Per wave merge:** `cmake --workflow --preset ci`.
- **Seam gate (plan 1 commit):** the three D-16 commands, then `gh run watch` on the draft PR until `CI required` is green (record the run ID).
- **Phase gate:** full `ci`, then `asan`, `nofp`, `hygiene`, then the PR run's `hash-equality` and scoreboard job before `/gsd-verify-work`.

### Wave 0 Gaps
- [ ] `tests/core/test_mapper.c` and `tests/mapper_test.h` (test board: records `(addr, value, cpu_cycle)`, settable IRQ bit, A12 edge log), registered as `core.mapper`.
- [ ] `tests/ppu/ppu_fixture.h` (build + load a synthetic NROM with `ines.h`, hand back `struct nesturbator *`), used to migrate `ppu.render`, `ppu.sprites`, `ppu.registers`.
- [ ] `tests/ppu/test_fetch.c` (`ppu.fetch`) and `tests/ppu/test_split_scroll.c` (`ppu.split_scroll`) with `add_executable` blocks following the pattern at `tests/CMakeLists.txt:170-184`.
- [ ] `src/mapper.c` and `src/mapper_nrom.c` added to `CMakeLists.txt:52-62` **and** to `bus.unit` at `tests/CMakeLists.txt:500-505`.
- [ ] `core.api` assertion `CHECK_EQ_U64(v.behaviour_revision, 4);` (tests/core/test_api.c:96) changes to 5 in plan 2 only.

Test-design notes for plan 2: the split-scroll fixture must start at scanline 261 with rendering enabled before dot 257 (and before 280 for the vertical copy), prime the first two tiles by running the pre-render line, and keep the fixture's own sampler independent of the implementation. Use `ppu_run_until(nes, nes->ppu.ppu_ticks + 8u)` to land "after dot N". Expected `v` values for the D-13 cases can be written down from the wiki formulas without running the code.

## Security Domain

`security_enforcement` is `false` in `.planning/config.json`, so the ASVS section is omitted. Relevant robustness note carried from D-03: bank numbers are reduced with `% page_count` against loader-validated sizes so a register value cannot index outside the allocation; the ROM loader fuzz corpus (`tests/fuzz`) and the `asan` preset exercise this path.

## Sources

### Primary (HIGH confidence)
- This repository, read this session: `src/ppu.c`, `src/bus.c`, `src/cartridge.c`, `src/instance.c`, `src/frame.c`, `src/internal.h`, `src/apu.c` (IRQ lines), `src/cpu.c` (poll), `CMakeLists.txt`, `CMakePresets.json`, `tests/CMakeLists.txt`, `tests/cmake/{global_symbols,undefined_symbols,hash_inventory,write_hashes,prepare_scoreboard_baseline,script_policy}.cmake`, `tests/abi/CMakeLists.txt`, `tests/accuracy/test_scoreboard.c`, `tests/accuracy/scoreboard.txt`, `tests/runner/hashes.txt`, `tests/ines.h`, `tests/check.h`, `tests/ppu/test_{render,sprites,registers}.c`, `tests/core/test_{api,cartridge,reset}.c`, `runner/main.c` (AccuracyCoin mode), `.github/workflows/ci.yml`, `release-please-config.json`, `include/nesturbator.h`, `README.md`, and the `.planning/preparation` files NES-HARDWARE-PPU-CARTRIDGE §2-5, ARCHITECTURE §2/4/6, DECISIONS (DEC.10/15/19).
- Scratch prototype and probes (not committed; in the session scratchpad): hash/ticks comparison for the three games and the DABG movies, AccuracyCoin pages 2/14/17, A12 edge logs, register-write histograms, GCC `-fPIC` `nm` on Alpine.

### Secondary (MEDIUM confidence)
- NESdev Wiki "PPU rendering" (raw wikitext fetched this session): https://www.nesdev.org/wiki/PPU_rendering [CITED]
- NESdev Wiki "PPU scrolling" (raw wikitext): https://www.nesdev.org/wiki/PPU_scrolling [CITED]
- NESdev Wiki "MMC3" (raw wikitext; A12 filter, cycles 260 and 324 0-based, empty-slot `$FF`, 8x16 note): https://www.nesdev.org/wiki/MMC3 [CITED]
- NESdev Wiki "Bus conflict": https://www.nesdev.org/wiki/Bus_conflict [CITED]

### Tertiary (LOW confidence)
- None used. The research-provider seam (`research-plan`, Context7 etc.) was not used: all provider flags in `.planning/config.json` are false and WebFetch failed, so the wiki text was fetched with `curl` from `?action=raw`. No GPL/LGPL emulator source was opened; the prototype was written from the wiki text only.

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH, nothing new; all read from the repo.
- Architecture (seam): HIGH on the current code; the global-symbols constraint reproduced on ELF.
- Fetch schedule: HIGH on documented dots and A12 levels (wiki plus prototype A12 log matching the MMC3 page's 260/324); MEDIUM on dot-0 and the odd-frame read.
- Hash impact: MEDIUM, a rough prototype; direction and ticks/audio stability are strong evidence, exact values need the real implementation.
- Pitfalls: HIGH, each was observed (test failures, nm output, hash flips).

**Research date:** 2026-10-10
**Valid until:** 2026-11-09 (the code is the moving part; re-check `src/` against `main` before planning if other phases land first)
