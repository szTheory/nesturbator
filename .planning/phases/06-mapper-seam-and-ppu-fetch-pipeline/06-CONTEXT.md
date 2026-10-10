# Phase 6: Mapper seam and PPU fetch pipeline - Context

**Gathered:** 2026-10-10
**Status:** Ready for planning

<domain>
## Phase Boundary

Two plans, in this order, with no board in either:

1. **Mapper seam (MAP-01).** Every cartridge loads through one per-board
   interface: page tables, a four-entry nametable map, a CPU write hook stamped
   with the CPU cycle, and a mapper IRQ ORed with the APU sources. NROM moves
   onto it with every v1 frame and audio hash byte-identical.
2. **Fetch pipeline (MAP-02).** The PPU renders from `v` through background
   and sprite fetches on their documented dots, with shifters, the dot-257 and
   dot-280-to-304 copies, and the PPU address bus (and so A12) as literal
   state. This plan carries the milestone's only behaviour-revision bump
   (4 to 5).

New boards (Phase 7 onward), loader mapper rules (MAP-03, Phase 9) and the MMC3
counter itself are outside this phase.

</domain>

<decisions>
## Implementation Decisions

The owner chose all four areas and asked for a fan-out: research each one,
run an adversarial pass, then adopt one coherent set of recommendations
without further questions. The decisions below are that set.

### Mapper interface (plan 1)
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

### PPU address bus and A12 (plan 2)
- **D-09:** The PPU drives `ppu.bus_addr`, the 14 bits on the pins, through
  one function. When bit 12 changes and `PPU_A12` is watched, that function
  calls `ops->ppu_a12(nes, level, tick)`, with the tick of that dot. Both
  edges are reported, so MC-ACC's falling-edge clock fits later. The core
  applies no filter: MMC3 (Phase 9) applies its own "three M2 falls with A12
  low" rule, using the stamps. Rendering off, or lines 240–260: the bus shows
  `v`, so `$2006` writes and `$2007` increments produce A12 edges the mapper
  sees. — **Reversibility:** costly — MMC3 and its variants in Phases 9–10
  depend on these edge semantics.

### Fetch fidelity (plan 2)
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

### Split-scroll oracle (plan 2)
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

### Seam gate and re-pin evidence
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

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Phase scope
- `.planning/ROADMAP.md` — Phase 6 goal, success criteria 1–5, plan order, research flag
- `.planning/REQUIREMENTS.md` — MAP-01, MAP-02

### Design already decided
- `.planning/preparation/ARCHITECTURE.md` §2 (time, eager catch-up), §4 (PPU), §6 (cartridge and mappers; page tables never serialised)
- `.planning/preparation/DECISIONS.md` — DEC.10 (per-dot PPU, real bus fetches), DEC.15 (1 KiB page tables, declared events), DEC.19 (mapper order)
- `.planning/preparation/NES-HARDWARE-PPU-CARTRIDGE.md` §2 (bus, state, what mappers observe), §3 (hard cases), §5 (mappers)
- `.planning/preparation/CONFORMANCE.md` — AccuracyCoin pages 17–22, scoreboard design

### Hardware documentation (clean room: documentation only, no GPL/LGPL emulator source)
- NESdev Wiki "PPU rendering" — two-dot access, ALE/octal latch, garbage NT and `$FF` sprite fetches, 337–340 dummies, bus = v when not rendering: https://www.nesdev.org/wiki/PPU_rendering
- NESdev Wiki "PPU scrolling" — increments, copies, split X/Y order: https://www.nesdev.org/wiki/PPU_scrolling
- NESdev Wiki "MMC3" — A12 filter, 0-based 260/324, 8×16 quirk: https://www.nesdev.org/wiki/MMC3
- NESdev Wiki "Bus conflict", "Open bus behavior", "Cartridge connector", "IRQ"
- AccuracyCoin README (test descriptions): https://github.com/100thCoin/AccuracyCoin/blob/main/README.md
- Breaks PPU PAMUX / VRAM controller pages (circuit analysis): https://github.com/emu-russia/breaks

### Existing gates
- `tests/runner/hashes.txt`, `tests/cmake/write_hashes.cmake`, `tests/cmake/hash_inventory.cmake`
- `tests/cmake/prepare_scoreboard_baseline.cmake`, `tests/accuracy/test_scoreboard.c`, `tests/accuracy/scoreboard.txt`
- `tests/core/test_api.c` (behaviour-revision assertion), `include/nesturbator.h` (`NESTURBATOR_BEHAVIOUR_REVISION`)
- `.github/workflows/ci.yml` (`hash-equality`, `CI required`)

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `tests/ines.h`: builds iNES images in C; the split-scroll fixture and the test board use it.
- `tests/ppu/test_render.c`, `test_registers.c` and `test_sprites.c`: the patterns for driving `nesturbator__ppu_register_write` and `nesturbator__ppu_run_until` directly.
- `src/ppu.c` `sprite_evaluate`: already per-dot, and it stays.

### Established Patterns
- `bus.c` `cycle()`: 9 ticks to M2 rise, PPU catch-up, IRQ poll sample, APU clock, 15 ticks to M2 fall, catch-up. The cycle stamp goes here.
- `ppu.c` currently draws the background per pixel from `t` (`background_pixel`) and fetches sprite patterns in one batch at dot 257. Both are replaced in plan 2.
- Mirroring comes from `cart.bytes[6] & 1` in `ppu_read` and `ppu_write`. It becomes `map.nt[]`.
- `cartridge.c` `nesturbator__cart_read` masks PRG for 16 or 32 KiB. It becomes page tables.
- `apu.c` `update_irq_line` (static) becomes `nesturbator__irq_update`.
- House CMake policy scripts with mutation self-tests exist in `tests/cmake/`. None is added this phase.

### Integration Points
- `src/internal.h`: `struct nesturbator` gains `mapper`, `map` and `cpu_cycle`; `struct nesturbator__ppu` gains the D-12 fields.
- `src/cartridge.c` load/unload: choose ops, `init`, `rebuild`.
- `src/bus.c`: read decode, write path, DMC DMA read and DMC IRQ go through the map and the IRQ helper.
- `src/ppu.c`: `ppu_read`/`ppu_write` go through `chr_read`/`chr_write`/`nt`; the new fetch pipeline.

</code_context>

<specifics>
## Specific Ideas

- The owner's generic fan-out brief: research each decision through every
  relevant role lens, prefer primary sources, run an adversarial pass, then
  synthesise coherent one-shot recommendations and adopt them. Also: "another
  copy and paste is better than another dep". No new dependencies in this
  phase.
- None of the AccuracyCoin bus tests ("ALE + Read", "Hybrid Addresses",
  "$2007 Stress Test", "BG Serial In", "Stale BG Shift Registers") is on the
  protected scoreboard today. The regression risk is in the four protected
  PPU rows: "PPU Read Buffer", "Register Mirroring", "Register Open Bus" and
  "Palette RAM Quirks". Any newly passing test may be added to the scoreboard.

</specifics>

<deferred>
## Deferred Ideas

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

</deferred>

---

*Phase: 06-mapper-seam-and-ppu-fetch-pipeline*
*Context gathered: 2026-10-10*
