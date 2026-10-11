# Phase 9: MMC3 - Research

**Researched:** 2026-10-10
**Domain:** MMC3 (mapper 4) board, PPU A12 clocking through the M2 filter, Holy Mapperel and blargg oracles, loader and runner rejection rules, C17 NES emulator core
**Confidence:** HIGH on the seams, the loader work, the oracle ROM pins and the register semantics. MEDIUM on the filter off-by-one and on IRQ-to-CPU latency, which no primary source or test ROM discriminates (see Open Questions).

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

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

### Deferred Ideas (OUT OF SCOPE)

- MMC6 (004:1), MC-ACC (004:3), T9552 (004:5), hard-wired mirroring (004:2),
  four-screen VRAM (LIB-04), TxSROM/TQROM (118/119) and 206: next tier.
- A region constant for PAL/Dendy M2 timing. D-05 already uses the
  region-correct cycle count, so only the MMC3 tests' tick values would
  need a second set.
- A public rejection-reason API for library and libretro hosts. Add it when
  a host asks.
- iNES 1 header repair or a database for Low G Man-style no-RAM games.

</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| BOARD-03 | MMC3 games run: IRQ clocked by A12 rises after three M2 falls, sprite fetches clock even for empty slots; Holy Mapperel mapper 4 passes incl. `$A001`; synthetic NEC test on submapper 4; nightly blargg job passes Sharp tests, `6-MMC3_alt` unsupported | Sections 1 (seams), 2 (board design), 3 (hardware semantics), 4 (Holy Mapperel), 5 (nightly lane), Validation Architecture |
| MAP-03 | Loader accepts mappers 0, 1, 2, 3, 4, 7; rejects others, MMC6 and four-screen with a message and non-zero exit; fuzz corpus and cartridge tests reflect the rules | Sections 6 (loader), 7 (runner), 8 (fuzz seeds) |
</phase_requirements>

## Summary

Everything the MMC3 needs already exists in the core. The `ppu_a12` hook, its watch bit and the unfiltered A12 reporting are wired and tested; empty sprite slots already fetch tile `$FF`; `nes->cpu_cycle` is the M2 clock; RAM gating needs no bus branch (NULL pages). The phase is therefore one new board file (`src/mapper_mmc3.c`, about 200 lines), one new `reg.mmc3` union member, one loader row plus a guard edit, and a large test and CI surface. No new library, no new public API, and no core change beyond `mapper.h` and `cartridge.c`.

Three findings change or sharpen the locked decisions and the planner must act on them (none re-litigates a decision, they are implementation facts):

1. **CMake script mode cannot patch the Holy Mapperel header** (D-02). CMake strings cannot hold NUL bytes; I ran `string(ASCII 0 b)` and it fails with "Character with code 0 does not exist". The repo already records this (`tests/runner/save_rom.c`: "CMake strings cannot hold NUL bytes"). The derived ROMs need a tiny test-only C tool (precedent: `runner.save_rom`, `holymapperel-decode`) driven by a `cmake -P` wrapper that does the sha256 check. Generate them at build time, not in a ctest fixture, because CI runs `write_hashes.cmake` outside ctest.
2. **D-09's "72-tick gap spans only two falls" does not hold under the D-05 derivation.** With the hook reading `nes->cpu_cycle`, `cpu_cycle` at a hook equals the number of M2 falls strictly before that dot, three dots per CPU cycle, so any 72-tick gap is exactly 3 falls and clocks under "at least three". Express the boundary tests in CPU cycles (2 filtered, 3 clocks, 4 clocks), as PITFALLS pitfall 7 already says ("hold A12 low for 2, 3 and 4 M2 cycles"). Rendering is unaffected: sprite-fetch low windows are 4 dots (at most 2 cycles) and BG-to-sprite windows are 12 dots or more (at least 4 cycles).
3. **`bus.unit` compiles the board sources by hand** (`tests/CMakeLists.txt:700-710`), so `src/mapper_mmc3.c` must be added to both source lists or `bus.unit` fails to link.

**Primary recommendation:** build in this order: loader row and `reg.mmc3` struct, then the board (registers, banks, RAM gating), then the A12 counter, then hook-level tests, then through-PPU tests, then Holy Mapperel entries and the derive tool, then runner `describe_rejection`, then fuzz seeds, then the nightly lane last (it needs measured frame budgets from the finished board).

## Architectural Responsibility Map

This is a C library, not a multi-tier web app. Tiers here are the emulator's own layers.

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| A12 level reporting (every edge, unfiltered) | PPU (`src/ppu.c` `set_bus`) | — | Already done; the core applies no filter (`mapper.h` contract) |
| M2 filter, counter, IRQ line | Board (`src/mapper_mmc3.c`) | — | The filter is a cartridge-chip circuit; the board owns it (D-04, D-05) |
| M2 fall clock | Bus (`nes->cpu_cycle`, `src/bus.c`) | Board reads it | `cpu_cycle` is incremented once per `cycle()` |
| PRG/CHR/mirroring maps | Board `rebuild` into `nes->map` | Core page helpers | Page tables are the only bus-visible state |
| PRG-RAM enable/protect | Board `rebuild` via NULL pages | Bus (no branch) | NULL read page is open bus, NULL write page drops the write |
| Save span and `save_generation` | Bus write path (existing) | Loader layout (existing) | Phase 8, reused unchanged (D-13) |
| Which images load | `cartridge.c` `board_profile_ok` | — | One row per mapper |
| Rejection wording | Runner (`describe_rejection`) | — | Library keeps one status, `NESTURBATOR_ERR_CARTRIDGE` (D-14) |
| Oracle protocol reading | Test-only `tests/mmc3/oracle.c` | Public `nesturbator_peek_cpu_ram` | Shipped runner knows nothing of it (D-11) |

## Standard Stack

No external packages are installed or added. The project is C17 with CMake 3.25+ and Ninja; `libretro.h` is the only vendored file.

| Tool | Version in this environment | Use |
|------|-----------------------------|-----|
| CMake | 4.4.3 [VERIFIED: `cmake --version`, this session]; project minimum 3.25 [VERIFIED: `CMakePresets.json` `cmakeMinimumRequired` 3.25.0] | Presets, CTest, `-P` scripts |
| Ninja | 1.13.2 [VERIFIED: `ninja --version`] | Generator |
| Apple clang | 21.0.0 [VERIFIED: `cc --version`] | Local compiler |
| git | 2.41.0 [VERIFIED: `git --version`] | Sparse blobless fetch for the nightly lane |
| bsdtar | `/usr/bin/bsdtar` [VERIFIED: `command -v`] | Extract the Holy Mapperel `.7z` |
| clang-format | NOT installed locally [VERIFIED: `command -v` printed nothing] | `hygiene.format` runs in CI (clang-format 18) |

**Package Legitimacy Audit:** no external packages are recommended. Nothing to audit. Packages removed (SLOP): none. Packages flagged (SUS): none.

### Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Binary patching of the HM header | A CMake hex writer | Tiny C tool `tests/holymapperel/derive.c` | NUL bytes cannot be written from CMake [VERIFIED: ran `string(ASCII 0 ...)`, CMake errored] |
| sha256 of fetched or base files | A hash routine in the oracle | `file(SHA256 ...)` in CMake `-P` scripts | Already how `fetch_vectors.cmake` and `manifest_sha256.cmake` do it |
| Sparse/blobless fetch with guards | A new fetch design | Copy `tests/cmake/fetch_vectors.cmake` and generalise the 256-file list to the 12 pinned paths | Marker file, in-tree refusal, stall limits already solved |
| RAM disabled reads/writes | A bus branch | `nesturbator__map_prg_ram_8k(nes, NULL)` | NULL pages are open bus and dropped writes |
| 1 KiB bank modulo | Own wrap code | `nesturbator__map_prg`, `nesturbator__map_chr` | True modulo against validated size (handles 16 KiB PRG aliasing) |
| `$6000` text reading | Parse via the CPU | `nesturbator__map_cpu_read` | Pure page-table read, no bus side effects, no open-bus write |

## 1. Seams the MMC3 plugs into (all read this session)

**`src/mapper.h`.** Watch bits, verbatim [VERIFIED: src/mapper.h:23-25]:

```
#define NESTURBATOR_WATCH_CPU_WRITE 1u
#define NESTURBATOR_WATCH_BUS_CONFLICT 2u
#define NESTURBATOR_WATCH_PPU_A12 4u
```

The hook, verbatim [VERIFIED: src/mapper.h:38]: `void (*ppu_a12)(struct nesturbator *nes, uint8_t level, uint64_t tick);`. The write hook receives `cpu_cycle` as its fourth argument. The `ppu_a12` hook receives only the PPU tick, and the header says hooks "never read nes->ticks". D-05 has the board read `nes->cpu_cycle`; this is a new, different field. Amend the hook-rules comment in `mapper.h` in the same change (rule 6): "ppu_a12 may read nes->cpu_cycle, the count of completed CPU cycles; it is the M2 fall clock".

`reg` is a union of per-board structs with the rule "Zero is the power-on state"; the MMC1 member shows the idiom (`control_x` is Control XOR `$0C`). `map.watch`, `ops`, `cpu_r[48]`, `cpu_w[48]`, `chr_r[8]`, `chr_w[8]`, `nt[4]` and `a12` are derived and never serialised.

**`src/cartridge.c`.** Quoted verbatim [VERIFIED: src/cartridge.c:83-88, 128-129, 186]:

```
    const int chr_8k_ok = chr_is_ram || chr_size == 8192u;
    const int no_ram = !battery && ram_work == 0u && ram_nv == 0u && chr_nv == 0u;
    /* Boards without RAM rows keep refusing the battery bit and every RAM size. */
    if (mapper != 1u && !no_ram)
```
```
    default:
        return 0;
```
```
        if (mapper == 1u) {
```

Four-screen is rejected for every mapper before the mapper is decoded [VERIFIED: src/cartridge.c:147]: `if ((image[6] & 0x08u) != 0u || (image[7] & 0xf0u) != 0u)`. The NES 2.0 path already rejects declared CHR-RAM other than exactly 8 KiB with no CHR ROM [VERIFIED: src/cartridge.c:164-167], so `M4_P128K_CR32K` and any `mmc3-chr-ram-16k` die before the profile. The load order is `board_profile_ok`, then the total-size check, then the `nesturbator__mapper_ops_for` probe (src/cartridge.c:195-204). Because the profile runs before the size check, a reject seed can be a bare 16-byte header and still reach the new row.

The iNES 1 RAM block at lines 186-192 sets RAM only for `mapper == 1u`. Mapper 4 needs its own branch: always 8 KiB, `ram_nv` when `image[6] & 2u` else `ram_work`. The trainer rule (line 284, `TRAINER_RAM_SIZE`) already forces 8 KiB of work RAM.

**`src/internal.h`.** `nesturbator__map_prg(nes, bank_1k)` and `nesturbator__map_chr(nes, bank_1k)` take a true modulo against the validated size; `nesturbator__map_prg_ram_8k(nes, ram_or_NULL)` sets both `cpu_r` and `cpu_w` for indexes 8-15. It cannot express "readable but write-protected". The board must call it and then set `cpu_w[8..15] = NULL` itself (two lines). `nesturbator__map_cpu_read` is a pure page read (no `open_bus` write) and is what the oracle uses. `cpu_cycle` is documented as "CPU bus cycles run since load: kept by nesturbator_reset, zeroed by load and unload" [VERIFIED: src/internal.h:205-209].

**`src/bus.c`.** `cycle()` advances 9 ticks, catches the PPU up, advances 15 more, catches the PPU up, then `nes->cpu_cycle++` [VERIFIED: src/bus.c:14-33]. All OAM and DMC DMA cycles go through `cycle()`, so `cpu_cycle` counts every M2 cycle. Only 24-tick cycles exist today (no region code: grep of `src/*.c` and `include/*.h` finds no region logic), which is why `tick / 24` would work today but is locked out by D-05.

**`src/ppu.c`.** Every address goes through `set_bus`, which calls `ppu_a12(nes, level, nes->ppu.ppu_ticks)` on a level change when the watch bit is set [VERIFIED: src/ppu.c:11-20]. PPU dots run at `ppu_ticks += 8` inside `ppu_run_until` (src/ppu.c:472-473). Sprite slots fetch tile `$FF` when empty: `uint8_t tile = slot < ppu->eval_count ? ppu->secondary_oam[base + 1u] : 0xffu;` [VERIFIED: src/ppu.c, `sprite_pattern_address`], giving `$0FF0/$1FF0` in 8x8 and `$1FE0+` in 8x16. Rendering off or lines 240-260: `set_bus(nes, ppu->v)` every dot. `$2006` second write: `if (!rendering_active(nes)) set_bus(nes, nes->ppu.v);`. `$2007`: `advance_v_after_data_access` calls `set_bus(v)` when not rendering. Pre-existing tests in `tests/ppu/test_fetch.c` assert the BG-at-`$1000` edge dots. **No PPU change is needed.** The A12 rise at the first sprite pattern address is dot 261 (1-based), the MMC3 page's 260 (0-based) [CITED: src/ppu.c comment at set_bus].

**Timing identity the board depends on (derived, VERIFIED by reading `cycle()` and `ppu_run_until`).** `ppu_ticks` is a multiple of 8, so each CPU cycle `n` contains three dots at ticks `24n+8`, `24n+16`, `24n+24`, and every hook inside cycle `n`'s two catch-ups sees `nes->cpu_cycle == n` (the increment happens after both). M2 falls at the cycle end, tick `24(n+1)`, after the third dot. So at a hook, `cpu_cycle` is the number of M2 falls that have strictly preceded that dot, and for a low edge at `cpu_cycle == a` and a rise at `cpu_cycle == b`, the falls between them are exactly `b - a`. Rule: **clock iff `b - a >= 3`.** Table of consequences: a 4-dot low window (sprite fetches) gives `b - a` of 1 or 2 (never clocks); a 12-dot BG-to-sprite window gives 4 or more (always clocks); a gap of exactly 3 dots can be 1 or 2. This is [ASSUMED] to be the intended reading of "three falling edges" at the exact boundary (see A4).

**`runner/main.c`.** After the load, `free(bytes)` is at line 709 and the generic message is printed at line 712 (`"nesturbator-run: malformed or unsupported cartridge (status %d); supported mappers are 0, 1, 2, 3 and 7\n"`), then `return 1`. `bytes` and `length` are still in scope, so `describe_rejection(bytes, length)` fits between the load and the `free`. The top-of-file comment and `usage()` both say "mapper 0, 1, 2, 3 or 7" [VERIFIED: runner/main.c:7 and the `--rom` usage line].

## 2. Board design (src/mapper_mmc3.c)

Follow `src/mapper_mmc1.c`: header comment with sources, `init` sets only `map.watch`, `rebuild` derives everything from `reg.mmc3`, hooks read and write only `nes->mapper`. Add to `mapper.h`:

```c
        struct {
            uint8_t select;      /* last $8000 write: bits 2-0 register, 6 PRG mode, 7 CHR inversion */
            uint8_t r[8];        /* R0-R7, raw written bytes; masks applied in rebuild */
            uint8_t mirror;      /* last $A000 write, bit 0 */
            uint8_t a001_x;      /* last $A001 write XOR $80: zero is enabled and writable */
            uint8_t latch, counter, reload, irq_enable;
            uint8_t a12;         /* the board's own last A12 level */
            uint64_t low_cycle;  /* cpu_cycle at the last A12 fall; zero = low since load */
        } mmc3;
```

and `void nesturbator__mapper_mmc3_ops(struct nesturbator__mapper_ops *out);`, then a `case 4u:` in `nesturbator__mapper_ops_for`, `src/mapper_mmc3.c` in the root `CMakeLists.txt` library list (line 59 neighbourhood) **and** in the `bus.unit` source list (`tests/CMakeLists.txt`, after `mapper_axrom.c`).

**Register decode:** `switch (addr & 0xE001u)` for `addr >= 0x8000u`; addresses below `0x8000` return immediately (a `$6000` RAM write reaches the hook because the hook follows the store). All eight are in D-07. Source for the layout [CITED: nesdev.org/wiki/MMC3 fetched as `action=raw` this session; the page is oldid 24268 per the WebFetch of the same page].

**Rebuild (all pages, every write):**

| Item | Rule |
|------|------|
| PRG windows (pages `16+8w .. 16+8w+7`, w = 0..3 for `$8000/$A000/$C000/$E000`) | Mode bit 6 = 0: R6, R7, (n8-2), (n8-1). Mode 1: (n8-2), R7, R6, (n8-1). R6/R7 `& 0x3F`. `n8 = prg_size / 8192`. Page = `bank * 8 + i`, passed to `nesturbator__map_prg` (modulo makes 16 KiB PRG alias) |
| CHR (pages 0-7) | Inversion bit 7 = 0: R0 `$0000`, R1 `$0800`, R2 `$1000`, R3 `$1400`, R4 `$1800`, R5 `$1C00`. Bit 7 = 1: R2 `$0000`, R3 `$0400`, R4 `$0800`, R5 `$0C00`, R0 `$1000`, R1 `$1800`. R0/R1 `& 0xFE` and occupy two consecutive 1 KiB pages. Use `nesturbator__map_chr`; writable only for CHR-RAM (see `map_chr_4k` for the `chr_w` idiom) |
| Mirroring | bit 0 = 0: `nt = {0,1,0,1}`; bit 0 = 1: `nt = {0,0,1,1}` (see section 3 for why) |
| `$6000-$7FFF` | If `cart.prg_ram == NULL` or RAM disabled: `map_prg_ram_8k(nes, NULL)`. If enabled: `map_prg_ram_8k(nes, cart.prg_ram)` then, when write-protected, `cpu_w[8..15] = NULL` |

The RAM base is `cart.prg_ram` (one allocation, work first then NVRAM; the span is the battery part, D-13). For V=8 KiB or N=8 KiB the allocation is exactly 8 KiB, so the MMC3 uses `cart.prg_ram` directly. An iNES 1 trainer-only image gets 8 KiB work RAM.

**Power-on consequence (D-08):** a zeroed block gives PRG mode 0 with R6 = 0, so `$E000` is the last bank and the reset vector reads correctly, the same as NESdev's note that the vector must point into `$E000-$FFFF`. [CITED: nesdev.org/wiki/MMC3, "the reset vector must point into $E000-$FFFF"]. The header mirroring bit is ignored (power-on is vertical from the zero block).

**A12 hook (`mmc3_ppu_a12`), exact algorithm:**

```c
static void mmc3_ppu_a12(struct nesturbator *nes, uint8_t level, uint64_t tick)
{
    struct nesturbator__mapper *m = &nes->mapper;
    (void)tick;
    if (level == m->reg.mmc3.a12)
        return;                                   /* duplicate edge */
    m->reg.mmc3.a12 = level;
    if (level == 0u) {                            /* fall: remember the M2 fall index */
        m->reg.mmc3.low_cycle = nes->cpu_cycle;
        return;
    }
    if (nes->cpu_cycle - m->reg.mmc3.low_cycle < 3u)
        return;                                   /* filtered: not low for three falls */
    clock_counter(nes);
}
```

`clock_counter` is D-06 verbatim: `old = counter`, `reload_seen = reload`; `counter = (old == 0 || reload) ? latch : counter - 1`; `reload = 0`; fire if `counter == 0 && irq_enable && (!nec || old != 0 || reload_seen)`; firing sets `mapper.irq = 1` and calls `nesturbator__irq_update`. `nec = (nes->mapper.submapper == 4u)`.

Two traps: (1) `low_cycle` zero at load with `a12 == 0` means "low since load"; a first rise at `cpu_cycle < 3` is filtered. (2) `nesturbator_reset` must not clear the block (cartridge has no reset line); it does not, because `nesturbator_reset` touches only CPU, PPU and APU (src/instance.c).

## 3. Hardware semantics to pin (with sources)

Primary source: NESdev Wiki "MMC3" (oldid 24268) fetched as raw wikitext this session; NES 2.0 submappers page; blargg's `mmc3_test_2` and `mmc3_irq_tests` readmes at the pinned commit (documentation only, no emulator source opened).

| Topic | Fact | Source |
|-------|------|--------|
| A12 filter | "triggered on a rising edge after the line has remained low for three falling edges of M2" | [CITED: nesdev.org/wiki/MMC3, IRQ Specifics] |
| Clock rule | "if zero **or** the reload flag is true, it's reloaded with the IRQ latched value at $C000; otherwise, it decrements." "If the IRQ counter is zero and IRQs are enabled ($E001), an IRQ is triggered." | [CITED: nesdev.org/wiki/MMC3] |
| `$C001` | "clears the counter, and set reload flag to true. It will be reloaded on the NEXT rising edge of filtered A12." No IRQ from the clear itself | [CITED: nesdev.org/wiki/MMC3; mmc3_irq_tests readme "The IRQ flag is not set when the counter is cleared by writing to $C001"] |
| `$E000` / `$E001` | `$E000` disables and acknowledges, counter keeps running; `$E001` enables, counter untouched | [CITED: nesdev.org/wiki/MMC3] |
| Interval | N+1 scanlines between IRQs for latch N | [CITED: nesdev.org/wiki/MMC3] |
| Sharp (submapper 0) | latch 0: IRQ on every clock (fires when counter equals 0 after the step) | [CITED: nesdev.org/wiki/MMC3; NES 2.0 submappers "004: 0 Sharp MMC3 ... Normal"] |
| NEC (submapper 4) | "Loading the latch with 0 disables IRQ"; one IRQ when decremented to 0; a `$C001` with latch 0 gives one more | [CITED: NES 2.0 submappers "004: 4 NEC MMC3"; nesdev.org/wiki/MMC3 NEC paragraph] |
| NEC rule check | blargg Rev A: "IRQ should be set when reloading to 0 after clear" and "IRQ shouldn't occur when reloading after counter normally reaches 0". D-06's `old != 0 \|\| reload_seen` satisfies both | [CITED: mmc3_irq_tests readme, 5.MMC3 Rev A; D-06] |
| Sharp rule check | Rev B: "Should reload and set IRQ every clock when reload is 0"; "IRQ should be set when counter is 0 after reloading" | [CITED: mmc3_irq_tests readme, 6.MMC3 Rev B] |
| `$A000` polarity | Wiki wording: "Nametable arrangement (0: horizontal (A10); 1: vertical (A11))". Mirroring page: "A **horizontal arrangement** of the nametables results in **vertical mirroring** ... connect PPU A10 to CIRAM A10" and "A **vertical arrangement** ... results in **horizontal mirroring** ... connect PPU A11 to CIRAM A10". So bit 0 = 0 is vertical mirroring `{0,1,0,1}`, bit 0 = 1 is horizontal mirroring `{0,0,1,1}`. Matches D-07. The pre-existing PITFALLS warning about a garbled polarity was a summariser error | [CITED: nesdev.org/wiki/MMC3 and nesdev.org/wiki/Mirroring, raw wikitext, this session] |
| `$8000` | bits 2-0 register select; bit 6 PRG mode (0: `$8000` swappable, `$C000` fixed second-last; 1: swapped); bit 7 CHR A12 inversion; bit 5 "Nothing on the MMC3" | [CITED: nesdev.org/wiki/MMC3] |
| `$8001` | R0/R1 ignore bit 0; R6/R7 ignore top two bits | [CITED: nesdev.org/wiki/MMC3] |
| `$A001` | bit 7 chip enable (0 disable, 1 enable); bit 6 write protection (0 allow, 1 deny); disabled reads return open bus. Wiki says power-on state is not specified; D-08's enabled default is a project decision | [CITED: nesdev.org/wiki/MMC3] |
| Power-on | R6, R7, `$8000` unspecified at power-on | [CITED: nesdev.org/wiki/MMC3] |
| Submappers | 0 Sharp, 1 MMC6 (Sharp IRQ), 2 hard-wired mirroring, 3 MC-ACC (A12 fall, different clock), 4 NEC, 5 T9552 | [CITED: nesdev.org/wiki/NES_2.0_submappers, section 004] |
| Unmodelled | "$C001 written on consecutive scanlines" pathology: blargg: "probably not a good idea to implement this"; wiki: "emulators do not yet emulate it" | [CITED: mmc3_irq_tests readme; nesdev.org/wiki/MMC3] |
| 241 clocks/frame | `mmc3_irq_tests` 2.Details test 7 "Counter should be clocked 241 times in PPU frame" = pre-render + 240 lines with BG `$0000`, sprites `$1000` | [CITED: mmc3_irq_tests readme] |

**Concrete acceptance sequences for hook-level tests** (counter and flag effects are fully determined; use `$C000`, `$C001`, `$E001`, then n filtered clocks; "fires" means `mapper.irq == 1` and `cpu.irq_line == 1`):

| Revision | Setup | Clock 1 | Clock 2 | Clock 3 | Clock 4 | Clock 5 |
|----------|-------|---------|---------|---------|---------|---------|
| Sharp | latch 3, `$C001`, `$E001` | counter 3, no IRQ | 2 | 1 | 0, **IRQ** | reload 3, no IRQ |
| Sharp | latch 0, `$C001`, `$E001` | 0, **IRQ** | 0, **IRQ** | **IRQ** | **IRQ** | **IRQ** |
| NEC | latch 1, `$C001`, `$E001` | reload 1, no IRQ | 0, **IRQ** (old = 1) | reload 1, no IRQ | 0, **IRQ** | — |
| NEC | latch 0, `$C001`, `$E001` | 0, **IRQ** (flag set) | no IRQ (old 0, no flag) | no IRQ | no IRQ | — |
| NEC | then `$E000` ack, `$C001` again | 0, **IRQ** (flag set) | — | — | — | — |

Boundary tests in CPU cycles (set `nes->cpu_cycle` directly, then call the hook; `mapper_test.h` shows the struct is reachable via `internal.h`): fall at cycle a, rise at a+2 -> no clock; rise at a+3 -> clock; rise at a+4 -> clock; fall at a, rise at a+3 (clock), fall at a+4, rise at a+5 (high pulse restarted the count) -> no clock; two rises with no fall between -> second ignored; first rise at `cpu_cycle` 2 with zero block -> filtered; at 3 -> clocks.

## 4. Holy Mapperel wiring (D-01, D-02, D-03)

**Verified this session [VERIFIED: downloaded `holy-mapperel-bin-0.02.7z`, sha256 matches D-01; bsdtar extracted `testroms/`]:**

| File | Size | sha256 | Header (first 16 bytes) |
|------|------|--------|-------------------------|
| `M4_P128K_CR8K.nes` | 131088 | `edb301630dae50a8470c1fb914f436434a150c886d9537d072275b751a9b4125` | `4e45531a 08 00 40 08 00 00 00 07 00 00 00 00` |
| `M4_P256K_C256K.nes` | 524304 | `278041824af3a0530d6a1367fa1a1d3bb544d2dd1e97ad8372673f7fe0379b17` | `4e45531a 10 20 40 08 00 00 00 00 00 00 00 00` |
| `M4_P128K_CR32K.nes` | 131088 | `428387f3...` | byte 11 = `09` (32 KiB CHR-RAM), rejected by the NES 2.0 CHR-RAM rule |
| `M4_P128K.nes` | 131088 | `b145558f...` | same header as CR8K |

So for the derived copies: byte 6 is `0x40`, byte 10 is `0x00`. `m4w8k`: byte 10 = `0x07`. `m4tkrom`: byte 6 = `0x42`, byte 10 = `0x70`. The mapper-4 mirroring bit (byte 6 bit 0) is clear on the base; irrelevant to the MMC3.

**What Holy Mapperel does with `$A001`** [CITED: holy-mapperel v0.02 `src/mmc3drivers.s` and `src/global.inc`, Zlib, fetched raw this session]: constants `MMC3_WRAM_OFF = $00`, `MMC3_WRAM_RW = $80`, `MMC3_WRAM_RO = $C0`. `mmc3_test_wram_protection` returns 0 immediately when `has_wram` is 0, which is why the shipped ROMs pass vacuously (D-02). With RAM present it: writes `$A001 = $80`, `$6000 = $B6`, compares; writes `$A001 = $C0`, stores `$6B`, requires `$B6` still read; writes `$A001 = $00`, requires `$6000` not read `$B6`; writes `$A001 = $80`, writes `$E000`, requires `$6000` still `$B6`. The last write to `$6000` that sticks is `$B6`, so D-02's `.sav` byte 0 = `0xB6` is consistent with the driver [ASSUMED for the whole-ROM end state: depends on no later Holy Mapperel stage rewriting `$6000`; the executor confirms on the first run]. Failure code for no write-protect is `2xxx` ("Mapper 004 `2xxx`: Read-only mode not present", README). The IRQ stage: BG on, sprites off, `PPUCTRL = VBLANK_NMI|BG_0000|OBJ_1000`, latch 64, `$C001`, `$E001`, `cli`, then it counts IRQs for one frame and requires exactly 3 (`eor #3`). With 241 clocks per frame and a period of 65 clocks, the first frame after `$C001` yields IRQs at clocks 65, 130, 195, so exactly 3 follows from the 241-clock frame [derived from the driver source]. This makes the empty-slot sprite fetch (sprites off but BG on) a hard requirement for `0000` on every mapper 4 ROM.

**Least-new-code entry form.** Today an entry is `key:file:N:text[:save]`, and four files read the list: `tests/CMakeLists.txt` (lines 172-248), `tests/cmake/write_hashes.cmake` (`${source_dir}/tests/roms/hm/${file}`), `tests/cmake/hash_inventory.cmake` (keys only) and the manifest list. Proposal:

- `roms.cmake` gains `NESTURBATOR_HOLYMAPPEREL_DERIVED` entries `<key>:<base file>:<patch>[,<patch>]` with patches `set@10=07`, `or@6=02` (colons are the list separator, so use `@` and `=`), and ROM entries whose file field starts with `derived/` (e.g. `m4w8k:derived/m4w8k.nes:600:8K PRG RAM OK`).
- One resolution rule in the two readers: file starting `derived/` resolves to `${NESTURBATOR_HM_DIR}/derived/<name>` (`tests/CMakeLists.txt`) or `${BUILD}/tests/holymapperel/derived/<name>` (`write_hashes.cmake`; `-DBUILD` is the build root and `tests/` is `PROJECT_BINARY_DIR/tests`), otherwise `tests/roms/hm/`.
- `tests/holymapperel/derive.c` (about 40 lines, `fopen` with `_CRT_SECURE_NO_WARNINGS` like its neighbours): `holymapperel-derive BASE OUT set@10=07 or@6=02`; refuses a base shorter than 16 bytes and an offset above 15; writes the same length.
- `tests/cmake/hm_derive.cmake` (`-P`): `file(SHA256 BASE)` must equal the manifest hash for the base, run the tool, then require `file(SIZE OUT) == file(SIZE BASE)`. Wire it with `add_custom_command(OUTPUT ... COMMAND cmake -P hm_derive.cmake ... DEPENDS holymapperel-derive <base>)` plus `add_custom_target(holymapperel-derived ALL DEPENDS ...)`, so the files exist after any build. CI runs `write_hashes.cmake` as a separate step after `cmake --workflow --preset ci` (.github/workflows/ci.yml:112), so the derived ROMs must not depend on a ctest fixture having run. Make the dump tests `DEPENDS`-free: they only need the file to exist at ctest time.
- Derived entries stay in the one ROM list, so `write_hashes.cmake` and `hash_inventory.cmake` pick them up with no further change (keys `holymapperel/m4w8k/frame N` etc.; the expected-row formula `36 + hm_count + saved` is dynamic). Regenerate `tests/runner/hashes.txt`. Derived files get no manifest line; the base files get theirs, also in `NESTURBATOR_HOLYMAPPEREL_MANIFEST` (that list is compared against `manifest.txt` by `write_hashes.cmake`).
- The `:save` branch hard-codes `-DSIZE=32768 -DHEX=5341564544415441`. Add an optional sixth field for the span size (default 32768) and run `check_sav.cmake` twice for `m4tkrom`: size 8192, offset 256 hex `5341564544415441`, and offset 0 hex `b6`. `check_sav.cmake` already takes SIZE, OFFSET and HEX, so no script change.
- Save file name: the runner derives `<stem>` from the ROM path, so `derived/m4tkrom.nes` writes `m4tkrom.sav` and the existing `hm_stem` logic (`NAME_WE`) already yields it.

Entries (frame budget N starts at 600 per D-01 and is pinned by the Phase 7 D-08 procedure; M1 SUROM needed 200 and SXROM 400, a 256 KiB CHR ROM check is the long pole):

| Key | File | Text asserted by `.prgram` | Notes |
|-----|------|----------------------------|-------|
| `m4tnrom` | `M4_P128K_CR8K.nes` | `PRG RAM MISSING` | Open-bus proof, V = N = 0 |
| `m4txrom` | `M4_P256K_C256K.nes` | `PRG RAM MISSING` | Largest CHR |
| `m4w8k` | `derived/m4w8k.nes` | `8K PRG RAM OK` | Runs `mmc3_test_wram_protection` |
| `m4tkrom` | `derived/m4tkrom.nes` + `:save:8192` | run 1 `8K PRG RAM OK`, run 2 `8K PRG RAM OK + BATTERY` | Save chain, `.sav` 8192 bytes |

Also update: `THIRD-PARTY-NOTICES.md` (the Holy Mapperel section lists files and says "mapper 1, 2, 3 and 7"), `tests/roms/manifest.txt` (two lines, same source/pin/licence columns as the existing HM lines), `README.md` (HM paragraph near line 46, the test-count prose near 273-302 and the "exact 45-key" sentence at ~line 454).

## 5. Nightly lane (D-10, D-11)

**Pins verified this session [VERIFIED: sparse blobless depth-1 fetch of `christopherpow/nes-test-roms` at `95d8f621ae55cee0d09b91519a8989ae0e64753b`; `git rev-parse HEAD` returned that commit; `shasum -a 256` of all 12 files equals the D-10 table; sizes below]:**

| Directory | Files | Size each | Header |
|-----------|-------|-----------|--------|
| `mmc3_test_2/rom_singles/` | 6 `.nes` | 40976 | `4e45531a 02 01 41 00 ...`: iNES 1, 32 KiB PRG, 8 KiB CHR ROM, mapper 4, vertical, no battery |
| `mmc3_irq_tests/` | 6 `.nes` | 16400 | `4e45531a 01 00 40 00 ...`: iNES 1, 16 KiB PRG, CHR-RAM, mapper 4, horizontal, no battery |

The cone checkout of `mmc3_test_2` and `mmc3_irq_tests` yields exactly these 12 `.nes` files plus readmes and sources (verified by `find`), so "a failure on any missing or extra `.nes`" works with a directory glob. The checkout also adds root files (`status.txt`, `test_roms.xml`); do not copy them into the tree. Do not run `git ls-tree -l` or any size/blob command in the blobless tree: it fetches blobs one at a time (my own attempt stalled for minutes; the `fetch_vectors.cmake` header already warns about it). Take sizes after checkout with `file(SIZE)`.

**Copy pattern.** `fetch_vectors.cmake` (read in full) generalises directly: `repo_url`, `check_in_source`, the marker file, the in-tree refusal, the stall limits (`GIT_HTTP_LOW_SPEED_LIMIT 1000`, `..._TIME 60`), `sparse-checkout set --cone`, `fetch -q --depth 1 --filter=blob:none origin <commit>`, `checkout -q FETCH_HEAD`, `rev-parse HEAD` check, `.git` removal. Differences: pins file is `tests/mmc3/pins.txt`, header line `repo https://github.com/christopherpow/nes-test-roms commit <40 hex> path mmc3_test_2,mmc3_irq_tests licence none-stated`, file list is 12 explicit paths (line format `<sha256>  <path>  <size>`), cone paths are two directories, the extra-file check globs `*.nes` in each. `fetch_guard.cmake` calls the script with a nonexistent git; add the same three offline refusals (foreign directory, symlink into the source tree, upper-case spelling) for the new script, using its own marker name (`.nesturbator-mmc3`).

**CMake and presets.** Mirror `NESTURBATOR_VECTORS_FULL` (tests/CMakeLists.txt:873-922): `option(NESTURBATOR_MMC3_ORACLE ... OFF)`, a fetch test with `FIXTURES_SETUP mmc3_oracle`, `TIMEOUT 300`, `LABELS mmc3-oracle`, and one test per ROM with `FIXTURES_REQUIRED` and the same label. Presets (pattern verified in `CMakePresets.json`): configure `mmc3-oracle` inherits `ci` with `NESTURBATOR_MMC3_ORACLE=ON`; build preset; test preset with `filter.include.label = "mmc3-oracle"`, `execution.noTestsAction = "error"`, `output.outputJUnitFile = "${sourceDir}/build/mmc3-oracle/mmc3-oracle.junit.xml"`; workflow preset with configure, build, test. Keep two offline tests in `ci`: the pins file and `oracle.txt` agree on the 12 keys, and the new fetch guard.

**Oracle (`tests/mmc3/oracle.c`).** Test-only executable; it links the library and includes `src/` (the `holymapperel.test_decode` target shows the idiom). Protocol facts:

- `mmc3_test_2`: blargg's `$6000` protocol. Signature `DE B0 61` at `$6001-$6003` (the readme prints `$G1`, a typo for `$61`); `$80` = running, `$81` = needs a reset (not expected), `$00-$7F` = result; text from `$6004`, zero-terminated [CITED: mmc3_test_2 readme "Output at $6000"]. Read with `nesturbator__map_cpu_read`.
- `mmc3_irq_tests`: `result = $f8` (zero page) and `tests_passed` stores `1` [VERIFIED: `mmc3_irq_tests/source/validation.asm:1,36-37`]. Read with `nesturbator_peek_cpu_ram(inst, 0x00f8, &v)`. Requiring `$F8 == 1` at the budget and still 1 after 60 more frames is D-11.
- The oracle exits 0 for pass and a non-zero code per D-11 reasons for failure; a CMake wrapper (copy `vectors_full_run.cmake`) looks the key up in `tests/mmc3/oracle.txt` and requires: `pass` rows exit 0; `unsupported` rows exit with exactly the recorded code. Row format `key<TAB>status<TAB>code<TAB>frames<TAB>hash` per `CONFORMANCE.md` "Scoreboard design" ("`key status code frames hash`"). Keys follow the repo style `mmc3_test_2/5-MMC3` (suite and name).
- Expected outcome with iNES 1 images (submapper 0, Sharp): `5-MMC3` and `6.MMC3_rev_B` pass; `6-MMC3_alt` and `5.MMC3_rev_A` fail (NEC behaviour) and are recorded `unsupported`. This follows from the readmes: the two revision tests differ in exactly the latch-0 reload behaviour.

**Workflow.** Add an `mmc3-oracle` job to `.github/workflows/nightly.yml` by copying `vectors-full` (timeouts, pinned action SHAs `actions/checkout@3d3c42e5aac5ba805825da76410c181273ba90b1`, `actions/upload-artifact@cf430e030ddbb5b0abf93d22962f4752f3646cd9`, `permissions: contents: read`, GCC 14 step, inventory step, `Failed keys`, `Verify run evidence`, upload, `Record run identity`). Then:

- `report`: extend `needs`, the `RESULT` expression, and the issue body with the failed oracle keys.
- `pull_request.paths`: add `tests/mmc3/**`, `tests/cmake/fetch_blargg_mmc3.cmake`, `tests/cmake/mmc3_oracle_run.cmake` (or its chosen name), `src/mapper_mmc3.c`, `src/mapper.h`, `src/ppu.c`, `src/cartridge.c`, `src/bus.c`, `tests/holymapperel/**` is not needed.
- `tests/cmake/nightly_workflow_policy.cmake` (run by `hygiene.nightly_workflow_policy` and `.selftest`, `tests/hygiene/CMakeLists.txt:31-38`) needs generalising. Facts that will bite: it extracts only the `vectors-full` job text (`string(FIND "${code}" "  vectors-full:" ...)` up to `  report:`) so a job inserted between them is swallowed into `vectors_job`; the evidence checks use first-occurrence `string(FIND ...)` and `string(SUBSTRING ... 180)`, so they would silently validate only the first lane's steps; `issues: write` must appear once; and `clean_fixture` in the self-test must be extended or the self-test fails. Loop the per-job checks over both lanes using each job's own text, and add the second lane to the self-test fixture and mutations.

**Do not** put the new lane under `vectors.registration_policy` (it configures `../vectors-full` and expects exactly the 258 full-vector entries); that policy is unaffected because the oracle tests register only under `NESTURBATOR_MMC3_ORACLE`.

## 6. Loader (D-12, D-13, D-14)

Add to `board_profile_ok` (verbatim rule set in D-12). Concrete edits:

1. Guard: `if (mapper != 1u && !no_ram)` becomes `if (mapper != 1u && mapper != 4u && !no_ram)`.
2. `case 4u:` accepting exactly: `submapper == 0u || submapper == 4u`; `is_pow2(prg_size) && prg_size >= 16384u && prg_size <= 524288u`; CHR: `chr_is_ram` (already guaranteed 8 KiB) or `is_pow2(chr_size) && chr_size >= 8192u && chr_size <= 262144u`; `chr_nv == 0u`; `(ram_work == 0u && ram_nv == 0u) || (ram_work == 8192u && ram_nv == 0u) || (ram_work == 0u && ram_nv == 8192u)`; `battery == (ram_nv != 0u)`.
3. iNES 1 RAM: in `validate_image` next to `if (mapper == 1u)`, add the mapper 4 branch (always 8 KiB; `ram_nv` when `(image[6] & 2u) != 0u`, else `ram_work`). Without it, an iNES 1 mapper 4 image would fail the RAM row for having `battery` set and `ram_nv == 0`, or load with no RAM.
4. `nesturbator__mapper_ops_for`: `case 4u: nesturbator__mapper_mmc3_ops(out); return 1;`.
5. 118, 119, 206, 249: no case, rejected by the `default: return 0`, as today.

Tests that must change in the same commit [VERIFIED by grep this session]: `tests/core/test_cartridge.c:353` `{"mapper-4", 4, 0, 0, 1, 1, 0, 0, 0, 0, BAD_}` is an iNES 1 mapper 4 image with 16 KiB PRG and 8 KiB CHR; under D-12 it now loads. Change it to `OK_` (rename `mapper-4-ines1-16k`) and add the true rejects as new rows; `{"mapper-5", ...}` stays the "no board" reject. `test_board_switch_matches_profiles` (line 625-640): the `want` set gains `id == 4u` and `CHECK_EQ_U64(boarded, 5u)` becomes 6. No other test in `tests/`, `libretro/`, `host/` or `runner/` mentions mapper 4 [VERIFIED: grep]. The row struct fields are `label, mapper, nes2, submapper, prg_banks, chr_8k, or_idx, or_val, size_delta, diskdude, want` (src: tests/core/test_cartridge.c:316-328); MMC1-style rows (`mmc1_rows`) carry RAM nibbles and are the model for an `mmc3_rows` table.

Docs changed in the same commit (rule 6): `include/nesturbator.h` (status comment at line 70-71, load comment lines 240-271, RAM notes near 343); `README.md` mapper lists at lines 7, 11-12, 698-728, the RAM table at 613-619 (add mapper 4 rows, Low G Man gap); `runner/main.c` top comment and `usage()`.

## 7. Runner rejection (D-15)

Implementation sketch (about 35 lines in `runner/main.c`, static, no new file):

```c
static void describe_rejection(const uint8_t *b, size_t len)
{
    unsigned mapper, sub = 0u;
    if (len >= 16u && b[0] == 'N' && b[1] == 'E' && b[2] == 'S' && b[3] == 0x1au) { ... }
}
```

Use the loader's bit rule exactly: `nes2 = (b[7] & 0x0c) == 0x08`; `mapper = (b[6] >> 4) | (b[7] & 0xf0)` plus `(b[8] & 0x0f) << 8` and `sub = b[8] >> 4` only when `nes2`. Order per D-15: four-screen (`b[6] & 8`) first, then mapper 4 sub 1, then mapper not in {0,1,2,3,4,7}, else the generic line (updated to "0, 1, 2, 3, 4 and 7"). Move `free(bytes)` after the message (line 709). Exit status stays 1.

**Testing the text.** `expect_output.cmake` checks stdout and exit status only and prints stderr on failure. The messages go to stderr. Smallest addition: an optional `-DEXPECT_ERR=<text>` in `expect_output.cmake` (about 6 lines: fail if the substring is absent from `actual_err`) and an `EXPECT_ERR` keyword in `nesturbator_runner_test`. Tests then read: `runner.reject.four_screen`, `runner.reject.mmc6`, `runner.reject.mapper` (e.g. `mapper-118`, expecting "mapper 118 is not supported; supported mappers are 0, 1, 2, 3, 4 and 7"), `runner.reject.generic`, each feeding a committed fuzz seed (`mmc3-four-screen`, `mmc3-submapper-1`, `mapper-118`, `mmc3-prg-8k`) with `--rom`, `EXIT 1 IGNORE_STDOUT`. A message test only proves the wording if the seed really reaches that branch, so choose seeds whose header alone decides it (they are all full-length or header-only, `describe_rejection` reads the first 16 bytes either way).

## 8. Fuzz corpus (D-16)

The corpus is `tests/fuzz/corpus/*` globbed at configure time and replayed by `fuzz.regress`; every file needs a line in `tests/roms/manifest.txt` (path, repo URL, 40-hex commit pin, `MIT`, sha256) or `hygiene.tree` and `manifest.sha256` fail. Existing seeds use the main-branch commit current when they were added (for example `df9f7fbbfbcdd6ea0f2276047e022411ed599739`). **No existing seed decodes to mapper 4** [VERIFIED: Python scan of every `NES\x1a` seed, bytes 6-8 with the loader's NES 2.0 bit rule printed nothing for mapper 4], so `mapper-bits` (mapper 1) and `mapper-155` need no re-tagging; keep `mapper-155`.

Seed construction cannot use CMake (NUL bytes). Use a throwaway shell or C snippet outside the repo: `printf` the 16-byte header, then `head -c N /dev/zero`. Valid seeds must be full images with a reset vector (the repo's other valid seeds are full). Reject seeds only need to reach `board_profile_ok`, which runs before the size check, so the large ones (`mmc3-prg-1m`, `mmc3-chr-512k`, `mmc3-prg-exp-huge`) can be 16-byte headers to avoid committing megabytes, as long as the cartridge-test row (a full image, built in memory) carries the semantic proof. Each seed has a matching row so the corpus and `core.cartridge` agree. Suggested table of headers (iNES 1 unless "N2"): `valid-mmc3` N2 byte 4 = `08`, byte 5 = `10`, byte 6 = `0x42`, byte 7 = `0x08`, byte 10 = `0x70`; `valid-mmc3-sub4` N2 as `valid-mmc3` with byte 8 = `0x40`; `valid-mmc3-prg16k` iNES 1 bytes 4 = `01`, 5 = `01`, 6 = `0x40`; the rest follow D-16 directly.

## Architecture Patterns

### System Architecture Diagram

```
CPU bus write ($8000-$FFFF) --> bus.c bus_write --> map.ops.cpu_write --> mmc3: decode addr&$E001
        |                                                   |-- $8000/$8001/$A000/$A001 --> reg.mmc3 --> rebuild --> cpu_r/cpu_w/chr_r/nt
        |                                                   '-- $C000/$C001/$E000/$E001 --> latch/counter/reload/irq
        '-- cycle(): +9 ticks -> ppu_run_until -> +15 ticks -> ppu_run_until -> cpu_cycle++
                                  |
PPU dot (8 ticks) --> fetch_step / $2006 / $2007 --> set_bus(addr) --> A12 change? --> map.ops.ppu_a12(level, ppu_ticks)
                                                                                |
                           mmc3_ppu_a12: fall -> low_cycle = cpu_cycle ; rise -> (cpu_cycle - low_cycle >= 3)? --> clock_counter
                                                                                |
                           clock_counter: reload-or-decrement; fire? --> mapper.irq=1 --> irq_update --> cpu.irq_line
Loader: validate_image --> board_profile_ok(case 4) --> alloc [work|NVRAM] --> mapper_load --> init + rebuild
```

### Recommended file changes

```
src/mapper_mmc3.c             new (one file per board, Phase 7 D-14)
src/mapper.h                  reg.mmc3 member, mmc3 ops decl, ppu_a12 comment
src/cartridge.c               guard, case 4, iNES1 RAM, ops switch
CMakeLists.txt                add src/mapper_mmc3.c
runner/main.c                 describe_rejection, text updates
tests/core/test_mapper_mmc3.c new: hook-level + loader rows + RAM gating
tests/ppu/test_mmc3.c         new: through-the-PPU clocks
tests/holymapperel/derive.c   new: header patch tool
tests/mmc3/{pins.txt,oracle.txt,oracle.c}   new
tests/cmake/{fetch_blargg_mmc3,hm_derive,mmc3_oracle_run}.cmake  new
```

### Pattern: through-the-PPU fixture
`tests/ppu/ppu_fixture.h` loads an NROM image only (`spec.mapper` is zero). The MMC3 PPU tests need a mapper 4 image: add a sibling loader (or a `mapper` parameter) that builds `spec.mapper = 4u`, `nes2 = 1`, with the board's `init` setting the watch bit; tests then poke `nes->ppu.control/mask/scanline/dot` as `test_fetch.c` does (`start(...)`, `step_to(...)`). Count clocks by observing `nes->mapper.reg.mmc3.counter` or an IRQ latch value rather than by logging edges. `tests/mapper_test.h` installs a *test* board and cannot host the real one.

### Anti-patterns
- **Reading `tick / 24` for fall index**: NTSC-only; D-05 forbids it.
- **Relying on `map.a12`**: it is zeroed by `nesturbator__mapper_load` (`memset(&nes->map ...)`), while `reg.mmc3` survives a state load. Keep the board's own level.
- **Calling `nesturbator__map_prg_ram_8k` and expecting protect to work**: it maps writes too.
- **Writing the `.sav` check as a regex of the whole file**: `check_sav.cmake` takes size, offset and hex; reuse it.
- **A bus branch for RAM enable**: not needed and breaks "no bus branch per board".

## Common Pitfalls

### Pitfall 1: CMake cannot write NUL bytes
**What goes wrong:** D-02's "cmake -P patches the header" cannot be written. **Why:** CMake strings are NUL-terminated. **Avoid:** the C derive tool above. **Warning sign:** `file(WRITE)` of a header that is 4 bytes long.

### Pitfall 2: Derived ROMs must exist outside ctest
**What goes wrong:** CI runs `write_hashes.cmake` as a bare `cmake -P` after the workflow; if the derived files are made by a ctest fixture, a lane that ran ctest still has them, but a clean `-P` run or a reordered CI does not. **Avoid:** build-time custom command attached to `ALL`.

### Pitfall 3: `bus.unit` link
**What goes wrong:** `cartridge.c` references `nesturbator__mapper_mmc3_ops`; `bus.unit` lists sources by hand and fails to link. **Avoid:** add `src/mapper_mmc3.c` to `tests/CMakeLists.txt` next to `mapper_axrom.c`.

### Pitfall 4: D-09's 72-tick example
See Summary item 2: write boundary tests in CPU cycles. A test keyed to ticks would encode the wrong number.

### Pitfall 5: iNES 1 mapper 4 gets no RAM unless the loader says so
The `mapper == 1u` iNES block is the only place RAM is created for iNES 1. Without a mapper 4 branch, `$6000` is open bus and blargg's `$6000` protocol never shows signature bytes.

### Pitfall 6: `$A001` power-on
Real hardware leaves it unspecified; if the stored value is plain `$A001` (zero = disabled and protected), blargg's `$6000` output is dropped before the test ROM ever writes `$A001`. The XOR `$80` encoding is mandatory, not optional.

### Pitfall 7: Mirroring polarity
Bit 0 = 0 is vertical mirroring `{0,1,0,1}` (nametables arranged horizontally, PPU A10 to CIRAM A10). Test both values; a swapped table renders vertically scrolling games wrongly.

### Pitfall 8: The nightly policy script validates only the first lane
See Section 5. Extend it per job or the new lane can drop the evidence steps unnoticed.

### Pitfall 9: Blobless fetch commands
`git ls-tree -l` and `git cat-file -s` in a blobless clone fetch each blob alone. D-10 says "records the sizes with `git cat-file -s`"; do it after the checkout (the blobs are present then) or use `file(SIZE)`; never before.

### Pitfall 10: Odd-frame and 2C02 pre-render double clock
The wiki says BG-at-`$1000` games clock the pre-render line twice on alternate frames on real hardware (Wario's Woods). This deterministic model clocks once per line; no success criterion requires the double clock. STATE.md also records "odd frames with PPUCTRL bit 4 set log no A12 rise at line 0 dot 0"; with the filter that rise is filtered anyway (4-dot low window), so no board-side handling is needed.

## Code Examples

### Clocking a hook-level test

```c
/* Source: derived from tests/mapper_test.h and the timing identity in section 1 */
static void rise_after(struct nesturbator *nes, uint64_t low_cycle, uint64_t gap)
{
    nes->cpu_cycle = low_cycle;
    nes->map.ops.ppu_a12(nes, 0u, 8u * low_cycle * 3u);
    nes->cpu_cycle = low_cycle + gap;
    nes->map.ops.ppu_a12(nes, 1u, 8u * (low_cycle + gap) * 3u);
}
```
Load an mapper 4 image through the public loader first (as `test_mapper_mmc1.c`'s `load_board` does), so `map.ops` is the real board; start counters with hook-delivered `$C000/$C001/$E001` writes (`nes->map.ops.cpu_write(nes, addr, v, stamp)`).

### describe_rejection reads the header with the loader's rule
See section 7.

### Oracle `$6000` read

```c
/* Source: blargg mmc3_test_2 readme "Output at $6000" */
uint8_t sig[3] = { nesturbator__map_cpu_read(nes, 0x6001u), nesturbator__map_cpu_read(nes, 0x6002u),
                   nesturbator__map_cpu_read(nes, 0x6003u) };
/* pass: sig == DE B0 61, state seen == 0x80 earlier, final == 0, "Passed" in text at $6004 */
```

## State of the Art

| Old approach | Current approach | Impact |
|--------------|------------------|--------|
| `$A000` described as "mirroring (0 vertical, 1 horizontal)" | NESdev now says "arrangement (0: horizontal (A10); 1: vertical (A11))" | Same hardware, opposite word; code comment states both terms |
| One MMC3 behaviour | NES 2.0 submapper 0 Sharp, 4 NEC, 3 MC-ACC, 1 MMC6 | Submapper decides the IRQ rule |
| Emulators skip `$A001` to avoid MMC6 conflicts | NES 2.0 can name submapper 1 | This project rejects MMC6 and models `$A001` |

**Deprecated:** mapper 249 is T9552 (submapper 5), rejected here.

## Runtime State Inventory

Not a rename or migration phase. Omitted.

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | `.sav` byte 0 after the derived NVRAM run is `0xB6` | 4 | Test expectation wrong; executor sees it on the first run and fixes the hex (D-02 already flags this) |
| A2 | Per-ROM frame budgets for the 12 oracle ROMs (start `mmc3_test_2` at 1800 frames, `mmc3_irq_tests` at 1200, then measure) | 5 | Too small = false timeouts; D-11 says measure on the finished board |
| A3 | The mapper IRQ line needs no extra delay between the A12 hook and `cpu.irq_line` for `4-scanline_timing` / `4.Scanline_timing` to pass | 3 | Those two oracle rows fail by one CPU cycle; a fix would be a one-cycle assert delay in the board, to be decided with measured codes |
| A4 | "At least three M2 falls" means `cpu_cycle(rise) - cpu_cycle(low edge) >= 3` at the exact boundary | 1, 2 | Off-by-one only matters for CPU-driven `$2006` gaps of 4 to 11 dots; no test ROM or Holy Mapperel discriminates 3 versus 4 (PITFALLS pitfall 7 expects tests at 2, 3 and 4 cycles) |
| A5 | `mmc3_irq_tests` `$F8 == 1` means pass and any other value is a failure code | 5 | Verified `tests_passed` stores 1 [VERIFIED: validation.asm]; whether `$F8` is 1 before the final stage is unproven, hence D-11's "still 1 after 60 more frames" |
| A6 | Frame budget N = 600 is enough for M4 Holy Mapperel ROMs | 4 | Executor measures per Phase 7 D-08 |
| A7 | The 16 KiB `mmc3_irq_tests` boot correctly with true-modulo bank aliasing (fixed banks are bank 0 and bank 1 of two) | 2, 5 | Oracle rows fail; check reset vector and bank layout first |

## Open Questions (RESOLVED)

1. **Will `4-scanline_timing` / `4.Scanline_timing` pass with zero extra IRQ delay?**
   - Known: the A12 hook fires inside the PPU catch-up and `cycle()` samples `irq_line` into `poll_latch` at the next cycle's +9 ticks.
   - Unclear: whether real hardware's filtered-A12 propagation shifts the assertion by a cycle.
   - Recommendation: build without a delay, run the nightly lane locally once, and record the codes; only add a delay if both timing tests fail by one cycle.
   - **RESOLVED (2026-10-10, planning): a measured-at-execution decision with a fixed rule.** The board ships with no extra IRQ delay (09-01). 09-11 Task 2 runs both scanline-timing ROMs through the `mmc3-oracle` lane and records their codes. Rule: add a one-CPU-cycle assertion delay, pinned by a `core.mapper_mmc3` case, only if both `mmc3_test_2/4-scanline_timing` and `mmc3_irq_tests/4.Scanline_timing` fail and their codes show the IRQ one cycle early or late; any other failure pattern is diagnosed as a different defect (A7 bank aliasing first) and never patched with a delay, and a Sharp-revision row is never recorded unsupported. No question remains open at planning time; only the measured value is left to execution.
2. **D-09 tick wording.** Resolve in favour of CPU-cycle units (Summary item 2); keep D-05 as locked.
   - **RESOLVED (2026-10-10):** the hook-level boundary tests are written in CPU cycles (rise 2 cycles after the fall is filtered, 3 and 4 clock), implemented by 09-01 Task 3. D-09 in 09-CONTEXT.md carries a dated amendment recording the correction; D-05 is unchanged.
3. **`ppu_a12` reading `nes->cpu_cycle`.** The header rule says hooks do not read `nes->ticks`. Amend the comment, do not add a new argument (no ABI change).
   - **RESOLVED (2026-10-10):** 09-01 Task 3 amends the hook-rules comment in `src/mapper.h` to say `ppu_a12` may read `nes->cpu_cycle`, the count of completed CPU cycles, as its M2 fall clock; no hook argument is added.

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| CMake (>=3.25) | everything | yes | 4.4.3 | — |
| Ninja | presets | yes | 1.13.2 | — |
| C compiler | build | yes | Apple clang 21.0.0 | — |
| git | nightly fetch | yes | 2.41.0 | — |
| Network (github.com) | nightly lane, HM asset | yes (I fetched both this session) | — | Lane fails offline by design, never skips |
| bsdtar | HM asset extraction | yes | `/usr/bin/bsdtar` | — |
| clang-format 18 | `hygiene.format` | no (local) | — | CI runs it; new C files should be formatted by hand to `.clang-format` style or formatted in CI |

## Validation Architecture

`workflow.nyquist_validation` is `false` in `.planning/config.json`, but the orchestrator asked for this section explicitly, so it is included.

### Test Framework
| Property | Value |
|----------|-------|
| Framework | CTest with in-repo `tests/check.h` (`CHECK`, `CHECK_EQ_U64`, `CHECK_EQ_HEX`, `CHECK_DONE`) |
| Config | `tests/CMakeLists.txt`, `CMakePresets.json` |
| Quick run | `ctest --test-dir build/ci -R 'core.mapper_mmc3|ppu.mmc3|core.cartridge|holymapperel.m4'` |
| Full suite | `cmake --workflow --preset ci` (and `--preset asan`) |
| Nightly lane | `cmake --workflow --preset mmc3-oracle` (network) |

### Success Criterion to Test Map
| Criterion | Behaviour | Test (suggested name) | Command | Exists |
|-----------|-----------|-----------------------|---------|--------|
| 1 | A12 rise clocks only after 3 falls; boundary 2/3/4; duplicate rise; short high pulse; first rise after load | `core.mapper_mmc3` | `ctest -R core.mapper_mmc3` | Wave 0 |
| 1 | Sprite fetches clock incl. empty slots (8x8 `$1FF0`, 8x16 `$FF`); 241 clocks per frame; BG `$1000`/sprites `$0000` clock at dot 325; both `$0000` no clocks; `$2006`/`$2007` clock once; 4-dot windows do not clock; non-MMC3 board gets no `ppu_a12` | `ppu.mmc3` | `ctest -R ppu.mmc3` | Wave 0 |
| 1 | NEC on submapper 4 (table in section 3) and Sharp on submapper 0 / iNES 1 | `core.mapper_mmc3` | same | Wave 0 |
| 1 | Register decode, masks, `$A000` polarity, `$A001`, power-on zero block, APU IRQ unaffected by `$E000`, `save_generation` only on enabled writable writes | `core.mapper_mmc3`, `core.save` additions | same | Wave 0 |
| 2 | Holy Mapperel mapper 4 `0000` and row text | `holymapperel.m4tnrom.{dump,decode,prgram}`, `m4txrom.*`, `m4w8k.*` | `ctest -R 'holymapperel.m4'` | Wave 0 |
| 2 | `$A001` enable and protect run, `.sav` chain | `holymapperel.m4tkrom.run1.{dump,decode,prgram,sav}`, `.run2.{dump,decode,prgram}` | same | Wave 0 |
| 2 | Hash determinism across six platforms | `runner.write_hashes.content`, `hash.inventory.self_test` (and CI `hash-equality`) | `ctest -R 'runner.write_hashes|hash.inventory'` | Updates to `tests/runner/hashes.txt` |
| 3 | Pins, fetch guard, oracle inventory | `mmc3.pins`, `mmc3.fetch.guard`, `mmc3.oracle.inventory` (all offline, in `ci`) | `ctest -R '^mmc3\.'` | Wave 0 |
| 3 | 12 ROMs: 10 pass or unsupported-as-recorded | `mmc3.oracle.fetch`, `mmc3.oracle.<key>` (label `mmc3-oracle`) | `cmake --workflow --preset mmc3-oracle` | Wave 0 (needs measured budgets) |
| 3 | Nightly workflow stays cold, non-skippable, least privilege | `hygiene.nightly_workflow_policy`, `.selftest` | `cmake --workflow --preset hygiene` | Update |
| 4 | Loader accepts 0,1,2,3,4,7; rows for mapper 4 | `core.cartridge` (new `mmc3_rows`, flipped `mapper-4`, board-set count 6) | `ctest -R core.cartridge` | Update |
| 4 | Runner messages and exit 1 | `runner.reject.four_screen`, `.mmc6`, `.mapper`, `.generic` | `ctest -R runner.reject` | Wave 0 |
| 4 | Fuzz corpus | `fuzz.regress`, `manifest.sha256`, `hygiene.tree` | `ctest -R 'fuzz.regress|manifest.sha256'` | New seeds |
| all | `bus.unit` still links | `bus.unit` | `ctest -R bus.unit` | Source-list edit |

### Sampling Rate
- **Per task commit:** the quick command above.
- **Per wave merge:** `cmake --workflow --preset ci`.
- **Phase gate:** `ci`, `asan` (UBSan matters for the masked shifts and `uint64_t` subtraction), `hygiene`, then one manual run of `cmake --workflow --preset mmc3-oracle` before recording `oracle.txt`.

### Wave 0 Gaps
- [ ] `tests/core/test_mapper_mmc3.c`, `tests/ppu/test_mmc3.c` (plus an MMC3 loader in a PPU fixture)
- [ ] `tests/holymapperel/derive.c`, `tests/cmake/hm_derive.cmake`, `roms.cmake` entries and the optional sixth `:save` field
- [ ] `tests/mmc3/{pins.txt,oracle.txt,oracle.c}`, `tests/cmake/fetch_blargg_mmc3.cmake`, wrapper script, presets, nightly job and policy edits
- [ ] `EXPECT_ERR` in `expect_output.cmake` and `nesturbator_runner_test`

## Security Domain

`security_enforcement` is `false` in `.planning/config.json`; section omitted. Fetch hardening is covered by the guard tests in section 5 and the nightly policy script.

## Project Constraints (from CLAUDE.md)

- C17, no extensions; the core links only the C memory functions; integer-only and deterministic; all state in the instance (no mutable statics in `src/`).
- Three deliverables only; no window, audio-device or input-device code.
- No ROM or BIOS bytes beyond `tests/roms/manifest.txt`; the blargg ROMs are fetched, never committed; no personal paths, emails or names in tracked files; commits use the GitHub noreply identity.
- Clean room: never open GPL/LGPL emulator source. This research read only NESdev Wiki pages, Holy Mapperel (Zlib) driver sources, and blargg's readmes and the `mmc3_irq_tests` test-program validation file (test code, not emulator source).
- Every behaviour change lands with a test run by `cmake --workflow --preset ci`, with the README and public header comments updated in the same change; no check waits on a person.
- One GSD step per command; phase branch; merge by PR with a Conventional Commit title.
- Style: small modules, plain control flow, fixed-width types; hardware comments cite their source.
- GSD workflow enforcement: file-changing tools are for GSD commands only.

## Sources

### Primary (HIGH confidence)
- This repository, read this session: `src/mapper.h`, `src/mapper_mmc1.c`, `src/cartridge.c`, `src/internal.h`, `src/bus.c`, `src/ppu.c`, `src/frame.c`, `src/instance.c`, `runner/main.c`, `include/nesturbator.h`, `CMakeLists.txt`, `CMakePresets.json`, `tests/CMakeLists.txt`, `tests/holymapperel/roms.cmake`, `tests/cmake/{fetch_vectors,fetch_guard,write_hashes,check_sav,expect_output,nightly_workflow_policy,manifest_sha256,fuzz_registration}.cmake`, `.github/workflows/{nightly,ci}.yml`, `tests/core/test_cartridge.c`, `tests/core/test_mapper_mmc1.c`, `tests/ppu/{ppu_fixture.h,test_fetch.c}`, `tests/ines.h`, `tests/mapper_test.h`, `tests/runner/save_rom.c`, `tests/roms/manifest.txt`.
- NESdev Wiki "MMC3" (oldid 24268), "Mirroring", "NES 2.0 submappers": raw wikitext via `action=raw`, 2026-10-10.
- blargg `mmc3_test_2/readme.txt` and `mmc3_irq_tests/readme.txt` and `mmc3_irq_tests/source/validation.asm` at commit `95d8f621ae55cee0d09b91519a8989ae0e64753b` (fetched from raw.githubusercontent and by sparse git fetch).
- Holy Mapperel v0.02 release asset (sha256 verified), `README.md`, `src/mmc3drivers.s`, `src/global.inc` at tag `v0.02` (Zlib).

### Secondary (MEDIUM confidence)
- `.planning/research/PITFALLS.md` pitfalls 7 and 8 (project-internal).

### Tertiary (LOW confidence)
- None used.

## Metadata

**Confidence breakdown:**
- Standard stack / seams: HIGH, every file read and quoted this session.
- Hardware semantics: HIGH for registers and revision rules (wiki plus blargg readmes agree); MEDIUM for the exact filter boundary (A4).
- Oracle wiring: HIGH for pins, sizes, headers and protocols; MEDIUM for frame budgets (A2) and timing-ROM outcome (A3).
- Pitfalls: HIGH.

**Research date:** 2026-10-10
**Valid until:** 2026-11-10 (the pins are commits; the wiki revision is recorded)
