# Phase 9: MMC3 - Context

**Gathered:** 2026-10-10
**Status:** Ready for planning

<domain>
## Phase Boundary

Mapper 4 (MMC3) cartridges load and run on the Phase 6 mapper seam (BOARD-03):
- The scanline counter is clocked by real PPU A12 rises through the M2 filter.
- The Sharp revision is the default, and submapper 4 selects NEC.
- `$A001` gives PRG-RAM enable and write protect.
- Holy Mapperel's mapper 4 ROMs report `0000`.
- A nightly job runs blargg's `mmc3_test_2` and `mmc3_irq_tests` at a pinned
  commit.

The loader accepts exactly mappers 0, 1, 2, 3, 4 and 7, and `nesturbator-run`
names the reason when it rejects another mapper, MMC6 or a four-screen board
(MAP-03).

Out of scope: MMC6, MC-ACC, T9552 and hard-wired-mirroring submappers,
four-screen VRAM, mappers 118, 119 and 206, save states, the per-board
boot-to-play proof and the six-platform close-out (Phase 10).

</domain>

<decisions>
## Implementation Decisions

The owner chose all four areas: the Holy Mapperel mapper 4 ROMs, the A12
clock and IRQ model, the blargg nightly oracle, and the loader rules with
MAP-03. As in Phases 7 and 8, they asked for a fan-out with an adversarial
pass, then one coherent set of recommendations followed without further
questions.

Four researchers reported. The orchestrator checked their claims against the
code and reconciled two conflicts:
- The blargg `mmc3_irq_tests` ROMs have 16 KiB of PRG, which the first loader
  draft rejected (D-12).
- The first A12 draft counted M2 falls as `tick / 24`. `src/internal.h`
  documents that this is wrong off NTSC and that `cpu_cycle` is
  region-correct (D-05).

### Holy Mapperel mapper 4 ROMs and the `$A001` proof
- **D-01:** Commit two mapper 4 ROMs from the same v0.02 asset, pin and Zlib
  licence as Phase 7 D-01 and Phase 8 D-01. They go raw and byte-identical
  under `tests/roms/hm/`, and each gets a manifest line and a
  THIRD-PARTY-NOTICES entry.
  - Asset sha256: `70f85671e21f293599baebb662faeb06a4c04e9c9ceb283d96d4197f09e4ce7a`.
  - Extract with bsdtar and re-verify every hash, as before.
  - Both ROMs are NES 2.0, mapper 4, submapper 0, with no battery, no
    PRG-RAM and no four-screen.

  | File | Size | sha256 | Shape |
  |---|---|---|---|
  | `M4_P128K_CR8K.nes` | 131,088 | `edb301630dae50a8470c1fb914f436434a150c886d9537d072275b751a9b4125` | TNROM/TGROM-like: 8 KiB CHR-RAM. Also the base of the derived copies (D-02) |
  | `M4_P256K_C256K.nes` | 524,304 | `278041824af3a0530d6a1367fa1a1d3bb544d2dd1e97ad8372673f7fe0379b17` | TxROM with the largest CHR (256 KiB) |

  Not committed:
  - `M4_P128K`: the same header as CR8K, with a differing last bank.
  - `M4_P128K_CR32K`: CHR-RAM of 32 KiB, which is no real board and is
    rejected by D-12.

  `roms.cmake` entries: `m4tnrom` and `m4txrom`. The row-6 text is
  `PRG RAM MISSING`, which proves open bus for V = N = 0. N is pinned by the
  Phase 7 D-08 procedure, starting at 600.
- **D-02:** The shipped mapper 4 ROMs never exercise `$A001`. Holy Mapperel
  runs `mmc3_test_wram_protection` only when its `$6000` probe finds RAM, and
  none of the four mapper 4 ROMs declares any. A plain `0000` would therefore
  pass vacuously.

  The proof uses derived copies made at test time in the build directory and
  never committed:
  - A `cmake -P` step first checks the committed base file's sha256.
  - It then copies `M4_P128K_CR8K.nes` and patches the copy's header:
    - **`m4w8k`:** byte 10 = `0x07` (8 KiB work RAM). Expect `0000` and the
      row text `8K PRG RAM OK`, with no ` + BATTERY`. The row text is the
      anti-vacuity guard, because `has_wram` ran.
    - **`m4tkrom`** (save chain): byte 6 |= `0x02` and byte 10 = `0x70` (8 KiB
      NVRAM). This runs the Phase 8 D-04 two-run `:save` chain:
      - run 1 shows `8K PRG RAM OK`;
      - run 2 shows `8K PRG RAM OK + BATTERY`;
      - the run-1 `.sav` is 8,192 bytes with `SAVEDATA` at 0x100;
      - `.sav` byte 0 is `0xB6`. The write-protect check writes `$6B` with
        `$A001 = $C0`, so an emulator that ignores protect leaves `0x6B`.
        This is derived from `src/mmc3drivers.s` and `src/main.s` (wram_test,
        then the mapper test). The executor confirms it on the first run.
  - `roms.cmake` gains an entry form that names a base file and a patch. The
    derived files get no manifest line, because they never enter the tree,
    and they are not redistributed, which fits Zlib clause 2.
- **D-03:** Holy Mapperel checks only four things:
  - enable with write;
  - write protect;
  - disable reads not returning the old byte;
  - that an `$E000` write does not disturb RAM.

  A synthetic `tests/ines.h` CTest covers the rest:
  - writes are dropped while disabled;
  - bits 5-0 are ignored;
  - the even/odd decode across `$A000-$BFFF`;
  - the power-on state (D-08);
  - `nesturbator_save_generation` counts only enabled, writable writes
    (Phase 8 D-12).

### A12 clock and IRQ model
- **D-04:** The new board is `src/mapper_mmc3.c` (one file per board, Phase 7
  D-14). Watch bits: `NESTURBATOR_WATCH_CPU_WRITE | NESTURBATOR_WATCH_PPU_A12`.
  There is no bus-conflict watch, because the MMC3 has none. Non-MMC3 boards
  never get a `ppu_a12` call, so their hashes cannot move. The core applies
  no A12 filter; the board owns it, as the `src/mapper.h` contract says.
- **D-05:** The M2 filter counts M2 falls, never PPU reads or dots:
  - **On a fall:** the board records the current M2 fall index, taken from
    the region-correct CPU cycle count (`nes->cpu_cycle`, see
    `src/internal.h` near line 205). It does not use `tick / 24`, which is
    NTSC only.
  - **On a rise:** the board clocks the counter only if at least three M2
    falls have passed since the recorded fall (NESdev MMC3: "after the line
    has remained low for three falling edges of M2").
  - **Boundary:** the exact off-by-one, meaning how a PPU dot caught up at
    the M2 rise or fall maps to a fall index, is pinned by hook-level tests
    at both sides of the boundary and documented in a comment citing the
    page.
  - **Own level:** the board keeps its own last A12 level and fall index as
    plain integers in `reg.mmc3`. It never relies on the derived `map.a12`.
  - **Power-on:** a zeroed fall index means low since load. A first rise
    within three falls of load is filtered, which is harmless with rendering
    off.

  Rendering only ever produces low windows of 4 dots or of 12 dots and more,
  so the boundary matters only for CPU-driven `$2006`/`$2007` edges.
- **D-06:** Counter and revision come from NESdev MMC3 and blargg
  `5-MMC3`/`6-MMC3_alt`. On a filtered rise:
  1. `old = counter`, and note the reload flag.
  2. If `old == 0` or the reload flag is set, `counter = latch`; else
     `counter--`.
  3. Clear the reload flag.
  4. Fire if `counter == 0` and IRQ is enabled. The revision decides the
     rest:
     - **Sharp:** submapper 0, and every iNES 1 image, fires on that rule
       alone.
     - **NEC:** submapper 4 also requires `old != 0` or the reload flag to
       have been set.

  One routine; `nec = (nes->mapper.submapper == 4u)`. Firing sets
  `mapper.irq = 1` and calls `nesturbator__irq_update`. The "$C001 written
  twice" pathology is not modelled.
  - **Amended 2026-10-10 (research-driven, recorded at plan revision):**
    firing has no extra IRQ assertion delay by default. A one-CPU-cycle
    assertion delay is added only if both `mmc3_test_2/4-scanline_timing`
    and `mmc3_irq_tests/4.Scanline_timing` fail and their codes show the IRQ
    one cycle early or late; it is then pinned by a `core.mapper_mmc3` case.
    Any other failure pattern is diagnosed as a different defect, never
    patched with a delay (09-RESEARCH.md open question 1, RESOLVED as
    measured at execution). The counter and revision rules above are
    unchanged. Implemented by 09-11 Task 2.
- **D-07:** Registers, decoded by `addr & 0xE001`:
  - **`$8000`:** stores the select byte and rebuilds. Bits 2-0 pick R0-R7,
    bit 6 is the PRG mode and bit 7 is CHR inversion.
  - **`$8001`:** stores `R[sel & 7]` and rebuilds. Masks are applied at
    rebuild: R0/R1 `& 0xFE`, R6/R7 `& 0x3F`. Every bank wraps by true
    modulo against the validated size, so a 16 KiB PRG (D-12) aliases its
    two 8 KiB banks.
  - **`$A000`:** bit 0 = 0 gives vertical mirroring (nametables 0,1,0,1);
    1 gives horizontal (0,0,1,1). NESdev now words this as "arrangement",
    so the comment states both terms.
  - **`$A001`:** bit 7 enables RAM and bit 6 protects writes. Disabled RAM
    reads as open bus and drops writes. Protected RAM reads and drops
    writes. `rebuild` leaves the `cpu_r`/`cpu_w` pages NULL, as MMC1 does
    (Phase 8 D-10), so the bus needs no new branch.
  - **`$C000`:** sets the latch only.
  - **`$C001`:** clears the counter and sets the reload flag, with no IRQ.
  - **`$E000`:** disables IRQ and acknowledges it (`irq = 0`, then
    `irq_update`). The counter keeps running.
  - **`$E001`:** enables IRQ and touches nothing else.

  Clearing the board's line leaves the APU frame and DMC sources asserted.
- **D-08:** Power-on is a zeroed `reg.mmc3` (the Phase 8 D-16 rule: `init`
  never writes registers and no op is added). Zero already means:
  - PRG mode 0, no CHR inversion and vertical mirroring;
  - R0-R7 = 0, latch 0, counter 0, reload clear and IRQ disabled.

  The one exception is `$A001`. Hardware leaves it unspecified, but RAM must
  work for games and for blargg's `$6000` protocol without an `$A001` write.
  So it is stored encoded so that zero means enabled and writable, for
  example `a001 ^ 0x80`, with a comment citing NESdev MMC3.
  `nesturbator_reset` keeps every field, because the cartridge sees no reset
  line.
- **D-09:** Synthetic tests cover success criterion 1 at both levels.
  - **Hook level** (`tests/core/`, through `tests/mapper_test.h`):
    - The filter boundary on both sides, including a 72-tick gap that spans
      only two falls and does not clock.
      - **Amended 2026-10-10 (research-driven correction, recorded at plan
        revision):** the 72-tick case does not hold under the locked D-05
        derivation, because inside a hook `cpu_cycle` counts the M2 falls
        strictly before the dot (09-RESEARCH.md Summary item 2 and open
        question 2, RESOLVED). The boundary is pinned in CPU-cycle units
        instead: a rise 2 CPU cycles after the fall is filtered, and 3 and 4
        clock. D-05 is unchanged. Implemented by 09-01 Task 3.
    - A rise after a short high pulse restarts the low count.
    - A duplicate rise is ignored, and the first rise after load is
      filtered.
    - Sharp with latch 3 counts 3, 2, 1, 0 with an IRQ, then reloads with no
      IRQ.
    - Sharp with latch 0 fires on every clock and after a `$C001` reload at
      0.
    - NEC (submapper 4) with latch 0 fires once, does not fire on a reload of
      0, and fires on a `$C001` reload.
    - `$E000`, `$E001`, `$C000` and `$C001` behave as in D-07.
    - Clearing the board's line leaves the APU IRQ asserted.
    - The register layouts and masks, `$A000` polarity and the even/odd
      decode with mirrors.
    - A zeroed block is power-on.
  - **Through the real PPU** (`tests/ppu/` fixture plus the MMC3 board):
    - With BG at $0000 and sprites at $1000 (8x8), there is one clock per
      rendered line, including pre-render.
    - With no sprites, the empty-slot fetches still clock every line in 8x8
      (`$1FF0`) and in 8x16 (tile `$FF`).
    - With BG and sprites both at $0000 there are no clocks.
    - With BG at $1000 and sprites at $0000 the clock falls at the documented
      later dot.
    - With rendering off, `$2006` pairs and `$2007` stepping across
      `$0FFF`→`$1000` clock once.
    - A negative control: one clock per line despite the 4-dot low windows
      between sprite fetches, which guards PITFALLS pitfall 7.
    - A non-MMC3 board gets no `ppu_a12` call.

### Blargg nightly oracle
- **D-10:** A separate nightly lane fetches 12 ROMs from
  `https://github.com/christopherpow/nes-test-roms` at
  `95d8f621ae55cee0d09b91519a8989ae0e64753b`.
  - **Licence:** the repository states none, so the ROMs are fetched only and
    never committed, and no readme text enters the tree.
  - **Pins:** `tests/mmc3/pins.txt` lists sha256 and size per file, with the
    licence token `none-stated`. The executor records the sizes with
    `git cat-file -s`.
  - **Fetch:** `tests/cmake/fetch_blargg_mmc3.cmake` mirrors
    `fetch_vectors.cmake`: a sparse, blobless, depth-1 fetch, a marker-only
    delete, an in-tree refusal, stall limits, and a failure on any missing
    or extra `.nes`. `fetch_guard.cmake` gains the same offline refusal
    cases.
  - **CMake:** a `NESTURBATOR_MMC3_ORACLE` option and `mmc3-oracle`
    configure, test and workflow presets, with label `mmc3-oracle`,
    `noTestsAction: error` and JUnit under `build/mmc3-oracle/`.
  - **Job:** an `mmc3-oracle` job in `nightly.yml` that follows the
    `vectors-full` shape (contents: read, pinned action SHAs, test
    inventory, Failed keys, Verify run evidence, artifact upload). It feeds
    the rolling `report` issue. `nightly_workflow_policy.cmake` and the
    `pull_request` path filters are extended for it.
  - **Separation:** it is never in `ci`, because `ci` never fetches.

  | Path | sha256 | Expected |
  |---|---|---|
  | `mmc3_test_2/rom_singles/1-clocking.nes` | `b06d8a97f0ca672be92c841d6af7d1e650696e86e9cc0cf6eeb90d67a6ab499b` | pass |
  | `mmc3_test_2/rom_singles/2-details.nes` | `e7af16c764b119e60effb7b1cfeec3dd8e2e657041283693cdbbeedb4081f1e3` | pass |
  | `mmc3_test_2/rom_singles/3-A12_clocking.nes` | `b375f15b9f9d372c8084b9c50928be9e41a3ac48be831ce82d203c18891433ad` | pass |
  | `mmc3_test_2/rom_singles/4-scanline_timing.nes` | `14a220b9d1272acc7a820ab38e9762a7cdf2d54c65e753be87f23dfcaf1bb845` | pass |
  | `mmc3_test_2/rom_singles/5-MMC3.nes` | `e0824123d60b83868dac1189b28250f8e10376a01be468a5a74aa59937cb32ca` | pass (Sharp) |
  | `mmc3_test_2/rom_singles/6-MMC3_alt.nes` | `56698b6918453d161a8d4e51f66e363d6966b054939c8176c53c401a6b55269b` | unsupported (NEC) |
  | `mmc3_irq_tests/1.Clocking.nes` | `699d0644bd2b6ff4c9ba598c9609f4a3da536594b6363585b2caf82cf337ac88` | pass |
  | `mmc3_irq_tests/2.Details.nes` | `0af95238b69806c072c28aed0fa8ad812157dfee928a6c9cea8d5420268baade` | pass |
  | `mmc3_irq_tests/3.A12_clocking.nes` | `3b936e1079f12bdc5e55aa82def017ddc76fd79e3879ff0478d41ca718302e7c` | pass |
  | `mmc3_irq_tests/4.Scanline_timing.nes` | `3369e8f73a96ec97918c6c9440804a369a19256c57df91e62881555f21528894` | pass |
  | `mmc3_irq_tests/5.MMC3_rev_A.nes` | `6b662c2d08ee4094d89b6d1ddde330e47f81929fb642dc217b6e4c33f8926944` | unsupported (NEC) |
  | `mmc3_irq_tests/6.MMC3_rev_B.nes` | `d8a2af42cdafe8046b36109e6f6ff71ca0d7f5d62c7d5953e0aa1d1828a86088` | pass (Sharp) |

  The headers are iNES 1. `mmc3_test_2` is 32 KiB PRG with 8 KiB CHR-ROM;
  `mmc3_irq_tests` is 16 KiB PRG with CHR-RAM. Both get 8 KiB work RAM by
  D-13, which the `$6000` protocol needs.
- **D-11:** The oracle is a test-only helper, `tests/mmc3/oracle.c`, like
  `tests/holymapperel/decode.c` ("the shipped runner knows nothing of
  this"). It adds no runner flag and no public API.
  - **`mmc3_test_2`:** it reads the `$6000` protocol through
    `nesturbator__map_cpu_read` from `src/internal.h`.
  - **`mmc3_irq_tests`:** it reads zero page `$F8` through the public
    `nesturbator_peek_cpu_ram`. That suite has no `$6000` output; `$F8 == 1`
    only after the final pass.
  - **`$6000` pass:** all of the following:
    - signature `DE B0 61` at `$6001-$6003`;
    - `$80` observed before the final value, so stale memory cannot pass;
    - `$6000 == 0`;
    - text at `$6004` containing `Passed`.
  - **`$F8` pass:** `$F8 == 1` at the budget and still 1 after 60 more
    frames. The executor confirms once, against a `--dump-frame`, that this
    matches the on-screen PASSED.
  - **Failure reasons:** `timeout` (`$6000` still `$80` at the frame
    budget) and `no-signature`. Each test also has a ctest `TIMEOUT`.
  - **Results file:** `tests/mmc3/oracle.txt`, in the CONFORMANCE.md line-100
    format (`key status code frames hash`). It is kept separate from
    `tests/accuracy/scoreboard.txt`, whose CI baseline logic must not see
    network-only rows. The file's keys must equal the pinned list.
    - A `pass` row that fails fails the job.
    - An `unsupported` row must fail with its recorded code. If it passes or
      changes code, the job fails and demands an explicit edit.
    - Skip code 77 is not used, as `nightly_workflow_policy.cmake` already
      requires.
  - **Measured later:** the per-ROM frame budgets and the two unsupported
    codes (likely 2) are measured on the finished board and recorded.

### Loader rules and MAP-03
- **D-12:** The mapper 4 row in `board_profile_ok`:
  - **Submapper:** 0 or 4. Reject:
    - 1 (MMC6);
    - 2 (hard-wired mirroring);
    - 3 (MC-ACC, a different clock);
    - 5 (T9552);
    - 6-15.

    iNES 1 images are submapper 0, which is Sharp.
  - **PRG:** a power of two from 16 to 512 KiB. 16 KiB is accepted, against
    the first draft, because blargg's `mmc3_irq_tests` use it. True-modulo
    banking makes the two fixed banks alias correctly.
  - **CHR:** ROM of 8-256 KiB in powers of two, or exactly 8 KiB of RAM.
    CHR-NVRAM is rejected.
  - **NES 2.0 RAM:** (V, N) is one of (0, 0), (8 KiB, 0) or (0, 8 KiB). The
    battery bit must equal N ≠ 0 (as Phase 8 D-08). Every other size is
    rejected: 1 KiB (MMC6), 2 KiB, 16 KiB, and work plus NV together.
  - **iNES 1:** always 8 KiB of PRG-RAM. It is battery-backed when the
    battery bit is set and is work RAM otherwise (the Phase 8 D-07 shape).
    Known gap, documented in the README: Low G Man expects open bus and
    needs an NES 2.0 header.
  - **Early guard:** `mapper != 1u && !no_ram` becomes
    `mapper != 1u && mapper != 4u && !no_ram`.
  - **Rejected mappers:** 118, 119, 206 and 249 get no case and are
    rejected.
  - **Elsewhere:** `nesturbator__mapper_ops_for`, the board-profile set test
    (`test_board_switch_matches_profiles`: {0,1,2,3,4,7}, six boarded) and
    the mapper 4 RAM row of the README table are updated in the same change.
- **D-13:** The RAM layout and the save span reuse Phase 8 D-06 and D-11
  unchanged: one allocation, with work RAM first, and the span is the
  battery part. `$A001` gating follows D-07. A trainer still gets 8 KiB of
  work RAM. — **Reversibility:** costly. The span offset decides where users'
  `.sav` bytes land; it is the same layout as MMC1.
- **D-14:** Four-screen is already rejected for every mapper in
  `validate_image`, before the mapper is decoded, so no core change is
  needed. Library contract: every rejection stays `NESTURBATOR_ERR_CARTRIDGE`,
  the instance is unchanged, and no new public API or status is added.
- **D-15:** The runner names the reason with a runner-local
  `describe_rejection` that reads the 16 header bytes it already holds. It
  runs only after a failed load, guards `len >= 16` and the `NES\x1a` magic,
  and reads the mapper and submapper with the loader's NES 2.0 bit rule.
  `free(bytes)` moves after the message. Checks, in order:
  1. `nesturbator-run: four-screen cartridges are not supported`
  2. `nesturbator-run: MMC6 (mapper 4, submapper 1) is not supported`
  3. `nesturbator-run: mapper N is not supported; supported mappers are 0, 1, 2, 3, 4 and 7`
  4. Otherwise the existing generic line, with the mapper list updated.

  Exit status is 1 for all of these (0, 2 and 4 keep their meanings; 3 and
  77 stay reserved). One CTest per message feeds the seed image and checks
  status 1 and the text. The runner's top comment, `usage()`, the README
  (supported mappers, the RAM table, the Low G Man gap) and the public
  header comment near the load function change together (rule 6). The
  adapter's load failure stays a plain `false`.
- **D-16:** Fuzz seeds, each with a manifest line and a cartridge-test row:
  - **Valid:**
    - `valid-mmc3` (NES 2.0, 128K/128K, N = 8K, battery);
    - `valid-mmc3-ines1` (64K/64K, battery bit);
    - `valid-mmc3-tgrom` (512K, CHR-RAM, no RAM);
    - `valid-mmc3-sub4`;
    - `valid-mmc3-prg16k`.
  - **Rejects:**
    - `mmc3-submapper-1`, `mmc3-submapper-2`, `mmc3-submapper-3`,
      `mmc3-submapper-5` and `mmc3-submapper-15`;
    - `mmc3-four-screen` (iNES 1) and `mmc3-four-screen-nes2`;
    - `mapper-118`, `mapper-119`, `mapper-206` and `mapper-249`;
    - `mmc3-prg-8k`, `mmc3-prg-24k` (exponent form), `mmc3-prg-1m` and
      `mmc3-prg-exp-huge`;
    - `mmc3-chr-512k`, `mmc3-chr-24k`, `mmc3-chr-ram-16k`, `mmc3-chr-none`
      and `mmc3-chr-nvram`;
    - `mmc3-ram-1k`, `mmc3-ram-2k` and `mmc3-ram-16k`;
    - `mmc3-battery-no-nvram`, `mmc3-nvram-no-battery` and
      `mmc3-work-and-nv`.

  The planner checks whether existing seeds such as `mapper-bits` decode to
  mapper 4 and now load. Any such seed is re-tagged, not deleted.

### Claude's Discretion
- File names inside `tests/` and the CTest names, following the
  `holymapperel.*`, `core.*` and `runner.*` patterns.
- The exact `$A001` encoding that makes zero mean enabled and writable
  (D-08).
- The exact M2 fall-index derivation and boundary, pinned by tests (D-05).
- How the derived-ROM patch step is expressed in `roms.cmake` (D-02).
- The exact runner message wording beyond the three D-15 strings, which
  are fixed.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Requirements and prior decisions
- `.planning/REQUIREMENTS.md`: BOARD-03 and MAP-03.
- `.planning/ROADMAP.md`, Phase 9: the success criteria and the research
  flag.
- `.planning/phases/07-uxrom-cnrom-and-axrom/07-CONTEXT.md`: D-01 to D-10
  (Holy Mapperel pattern, decoder, loader rows, board set test) and D-14
  (one file per board).
- `.planning/phases/08-mmc1-and-battery-saves/08-CONTEXT.md`: D-04 (save
  chain), D-06 and D-07 (RAM layout and sizing), D-08 (row style), D-10
  (NULL pages for disabled RAM), D-11 and D-12 (save API and generation),
  D-16 (zeroed power-on).

### Hardware
- `.planning/preparation/README.md`: index; open what it points to.
- `.planning/preparation/NES-HARDWARE-PPU-CARTRIDGE.md`: MMC3 A12 notes
  (line 47), the verification table (line 66), the submapper notes (line 96)
  and HWP.12/HWP.20 sources.
- `.planning/preparation/ARCHITECTURE.md`: time base and the mapper seam.
- `.planning/research/PITFALLS.md`: pitfalls 7 and 8 (A12 filter, `$A000`
  polarity).
- NESdev "MMC3" (oldid 24268), "NES 2.0 submappers" (004), "Mirroring".
- Holy Mapperel v0.02 source, Zlib: `src/mmc3drivers.s`
  (`mmc3_test_wram_protection`), `src/wram.s`, `src/main.s`.
- blargg `mmc3_test_2` readme and sources at `95d8f62` (read only; no text
  copied).

### Code
- `src/mapper.h`: the seam, watch bits and the `ppu_a12` hook contract.
- `src/ppu.c`: `set_bus` A12 reporting and the empty-slot sprite fetches.
- `src/bus.c`: the 24-tick NTSC cycle and the M2 edges.
- `src/internal.h`: `cpu_cycle` (region-correct), `nesturbator__irq_update`
  and `nesturbator__map_cpu_read`.
- `src/mapper_mmc1.c`: the board pattern to follow.
- `src/cartridge.c`: `board_profile_ok`, `validate_image` and
  `nesturbator__mapper_ops_for`.
- `runner/main.c`: the rejection message and the exit statuses.
- `tests/holymapperel/roms.cmake` and `decode.c`: the ROM list and the
  decoder.
- `tests/mapper_test.h`, `tests/ines.h` and `tests/ppu/test_fetch.c`: test
  helpers.
- `.github/workflows/nightly.yml`, `tests/cmake/fetch_vectors.cmake`,
  `fetch_guard.cmake`, `nightly_workflow_policy.cmake` and
  `CMakePresets.json`: the nightly lane pattern.

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `ppu_a12(nes, level, tick)` hook and `NESTURBATOR_WATCH_PPU_A12`: already
  wired. The PPU reports every A12 change unfiltered, including `$2006` and
  `$2007` with rendering off.
- Empty sprite slots already fetch tile `$FF` (`$0FF0`/`$1FF0` in 8x8,
  `$1FE0+` in 8x16), checked by `tests/ppu/test_fetch.c`.
- The Phase 8 RAM allocation, the save span, `save_generation` and the
  runner `--save-dir` chain carry over unchanged.
- The Holy Mapperel decoder with the row-6 reader and the `:save` entry form.
- The `fetch_vectors.cmake` pin/fetch/guard machinery for the nightly lane.

### Established Patterns
- A zeroed register block is power-on, `init` writes only `map.watch`, and
  the cartridge keeps its registers across `nesturbator_reset`.
- `rebuild` leaves pages NULL for open bus and dropped writes. There is no
  bus branch per board.
- Test-only helpers may include `src/` (`tests/CMakeLists.txt`
  `${PROJECT_SOURCE_DIR}/src`).
- The nightly lanes are separate jobs feeding the one `report` issue; there
  is no cache and no skip 77.

### Integration Points
- `src/cartridge.c`: the mapper 4 row, the RAM-guard change and the ops
  switch.
- `runner/main.c`: `describe_rejection`.
- `tests/holymapperel/roms.cmake`: the M4 entries plus the derived-ROM
  entries.
- `nightly.yml` and `CMakePresets.json`: the `mmc3-oracle` lane.

</code_context>

<specifics>
## Specific Ideas

- The owner's standing preference: owned code over dependencies, one
  decisive recommendation, and automated proof with nothing waiting on a
  person.
- The Holy Mapperel build toolchain (ca65, Python, PIL) is not added to CI.
  The derived-header copy replaces building W8K/S8K ROMs.

</specifics>

<deferred>
## Deferred Ideas

- MMC6 (004:1), MC-ACC (004:3), T9552 (004:5), hard-wired mirroring (004:2),
  four-screen VRAM (LIB-04), TxSROM/TQROM (118/119) and 206: next tier.
- A region constant for PAL/Dendy M2 timing. D-05 already uses the
  region-correct cycle count, so only the MMC3 tests' tick values would
  need a second set.
- A public rejection-reason API for library and libretro hosts. Add it when
  a host asks.
- iNES 1 header repair or a database for Low G Man-style no-RAM games.

</deferred>

---

*Phase: 09-mmc3*
*Context gathered: 2026-10-10*
