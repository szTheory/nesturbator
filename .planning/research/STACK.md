# Technology Stack: v5 reference sources, test ROMs and tooling

**Project:** nesturbator, milestone v5 "NMI, VBlank and sprite-0 timing"
**Researched:** 2026-10-25
**Scope:** only what the 11 AccuracyCoin rows need. Nothing in the language, build or dependency stack changes.

## Verdict

v5 needs no new dependency, no new committed ROM and no new runner flag. The rows are fixed against three kinds of reference (the NESdev pages the preparation files already cite, the MIT AccuracyCoin source at the current pin, and the circuit-level Visual 6502 findings mirrored on NESdev) and are proved by the scoreboard that already exists. The one optional addition is a nightly, network-fetched blargg oracle lane built by copying the existing `mmc3-oracle` lane. Confidence: HIGH on what exists in the repo (read directly), MEDIUM on blargg licensing (the project's own 2026-10-02 survey, [CF.06], not re-fetched today).

## Authoritative expected behaviour, per row

All pages below are the NESdev Wiki (https://www.nesdev.org/wiki/, community-maintained, T2) unless marked. The preparation files already pin revisions; cite those when writing plans.

| Row(s) | Primary reference | Why this one |
|---|---|---|
| `$2002 flag timing`, `NMI Timing`, `NMI Suppression`, `NMI at VBlank end`, `NMI disabled at VBlank`, `NMI Control` | PPU frame timing and NMI pages (HWP.03, r19961 and r23420): flag set at line 241 dot 1, cleared at line 261 dot 1, a `$2002` read one dot early cancels flag and NMI, on or after the set dot returns 1 and cancels NMI, /NMI is vblank_flag AND NMI_output | Gives the exact one-dot windows; `NMI Control` and `NMI disabled at VBlank` follow from the AND-gate and the edge detector on `$2000` bit 7 |
| `Interrupt flag latency`, `NMI Overlap BRK`, `NMI Overlap IRQ` | CPU interrupts page (HWC.02, r21632) and the Visual 6502 "Interrupt Recognition Stages and Tolerances" mirror (HWC.17, r19267): lines sampled during phi2, polled in the last cycle, so the second-to-last cycle's line state decides; I-flag change is seen one instruction late; vector chosen after cycle 4 of 7, so an NMI in the first four cycles of BRK or IRQ hijacks | The circuit-level source is the tiebreaker where wiki prose is ambiguous |
| `Sprite 0 Hit behavior`, `Arbitrary Sprite zero` | PPU sprite evaluation and OAM pages (HWP.06): no hit at x=255, hit needs opaque BG and sprite pixels, flag set at the pixel, cleared at pre-render | Wiki states the rules; the ROM's per-case comments state which edge each case probes |
| All 11 | `AccuracyCoin.asm` and README at pin `6bee919529289c555803b612b8c9e4f7eaaeeabb`, MIT (https://github.com/100thCoin/AccuracyCoin, [CF.14]) | The test is the specification here: each test's code and comments say what hardware result it expects and what error code each failing step returns. Read the exact test routine for each failing code in `scoreboard.txt` (e.g. NMI Control `0x22`, others `0x06` or `0x0a`) to learn which sub-check fails first |

How to use the error codes: `tests/accuracy/scoreboard.txt` records the raw result byte per row (`0x06`, `0x0a`, `0x22`, timeout `0x03`). In AccuracyCoin the high bits are the failing sub-test number, so each byte names the first failing step; the assembly shows what that step does. This is cheaper than guessing from the wiki and is permitted because the source is MIT.

## Permitted test ROMs

| ROM | Licence | Redistribute / commit? | Role in v5 |
|---|---|---|---|
| `tests/roms/accuracycoin.nes` (pin 6bee919) | MIT | Already in `tests/roms/manifest.txt` | The acceptance gate: all 11 rows must flip to `pass` in `tests/accuracy/scoreboard.txt`; the 673ef copy stays as the old-pin comparison |
| blargg `ppu_vbl_nmi` (11), `cpu_interrupts_v2` (5 relevant), `sprite_hit_tests_2005.10.05` (11), `vbl_nmi_timing` (7) in https://github.com/christopherpow/nes-test-roms @ `95d8f621ae55cee0d09b91519a8989ae0e64753b` | No licence stated in any blargg package or in the repository ([CF.06], project survey 2026-10-02) | **No.** Not in the manifest and `scripts/hygiene.sh` rejects them. Fetch-only, as `mmc3_test_2` already is | Independent, differently-authored second opinion (see below) |
| Holy Mapperel, DABG, Nesteroids, RHDE, nes-runner | Zlib / MIT / FSFAP | Already committed with hashes | Regression guard: their frame and audio hashes must still match, or be re-pinned with a documented reason, because NMI timing changes can shift game frames |

`cpu_interrupts_v2` ROM 4 is a DMA test: out of v5 scope (DMA is the next cluster). The blargg ROMs use the `$6000` text protocol (ppu_vbl_nmi, cpu_interrupts_v2) or the `$F8` zero-page result (sprite_hit_tests, vbl_nmi_timing), both of which `tests/mmc3/oracle.c` already judges ([CF.06]; README section on `mmc3-oracle`).

### Recommended optional addition: a nightly `vbl-oracle` lane (reuse, not new machinery)

Value: AccuracyCoin was written against its author's own emulator and shares its common-mode risk ([CF.03]); blargg's ROMs were verified on hardware and give an independent check, and ppu_vbl_nmi tests flag and NMI timing "to one PPU clock" (HWP.20). They also give finer diagnostics than AccuracyCoin's single byte for this cluster (ppu_vbl_nmi splits into 01-vbl_basics through 10-even_odd_timing).

How, at near-zero cost, following the repository's own ladder (reuse what is here):
- Copy `tests/mmc3/pins.txt` and `tests/cmake/fetch_blargg_mmc3.cmake` to a second pin list for these four directories: a sparse, blobless, depth-1 fetch at the same commit, size and SHA-256 check, marker-guarded delete. Same `repo ... licence none-stated` header line.
- Reuse `tests/mmc3/oracle.c` (it already handles both protocols); add rows to a sibling of `oracle.txt` with `pass` or `unsupported` and the expected exit code. Record currently failing ROMs as `unsupported` first, then flip them as v5 phases land; this makes progress visible in the nightly issue without gating merges ("outside CI required").
- Do not read the blargg ROM source or disassemblies for the repo; the pass/fail text from `$6000` is test behaviour, which clean room permits.
- Hardware-dependent outcomes: ppu_vbl_nmi notes that some CPU/PPU alignments differ (HWP.20). If a ROM passes only on one alignment, record the accepted code as the existing manifest convention allows rather than fudging the core.

This lane is a recommendation, not a requirement for the 11 rows. If the roadmap wants minimum scope, defer it; the acceptance gate stands without it.

## Owned reproducers (the permitted "extra test ROMs")

The project writes its own tests. For each fix, the strongest evidence is a small owned test that pins the single edge being changed, because AccuracyCoin only reports a coarse byte.

| Where | What | Why |
|---|---|---|
| `tests/ppu/` (fixture `ppu_fixture.h`, `test_registers.c`) | Dot-exact unit tests: `$2002` read at dot 0, 1, 2 of line 241 and at 261 dot 0, 1 and the returned bit plus whether NMI is delivered; `$2000` bit 7 toggled 0 to 1 while flag is set and 1 to 0 at dot 1 to 3 | Pins the one-dot windows of HWP.03 without a whole ROM; these run in `ci` in milliseconds |
| `tests/cpu/` (`test_cpu_unit.c`, `test_bus.c`) | Hand-assembled in-memory programs for CLI/SEI/PLP one-instruction latency, branch-poll quirk, NMI landing in cycles 1 to 4 vs 5 to 7 of BRK and IRQ | Pins HWC.02/HWC.17 behaviours; the 65x02 vectors exclude interrupts, so nothing else covers them |
| `tests/ppu/test_sprites.c` | Sprite-0 cases: x=255, clipped left 8 pixels, transparent pixels, hit cleared at pre-render, `Arbitrary Sprite zero` placements at several Y and X | The timeout row is a hang in the guest's wait loop; an owned test with a bounded frame budget turns a hang into a clear failure |
| `tests/accuracy/` | Update `scoreboard.txt` rows for the 11 keys to `pass 0x01`, keep every other row unchanged | `verify_scoreboard_regression.cmake` already fails on any lost PASS, which is the "no regression" gate |
| `tests/libretro/`, `tests/core/test_state_runahead.c` | Run-ahead-across-JAM test; save-state coverage for any new PPU/CPU field (NMI pending/edge latch, delayed `$2002` state), with a state-format pin update | Rule 5 and the state-format pin (`state_format_pin.cmake`) require this when state grows |

## In-repo tooling that suffices (verified in `runner/main.c` and `tests/`)

| Need | Existing tool |
|---|---|
| Run one AccuracyCoin page strictly | `nesturbator-run --accuracycoin-page N --rom FILE --scoreboard FILE` (pages 2, 14, 17 only) |
| Observe any page without gating, with evidence files | `--accuracycoin-observe PAGE --frames N --accuracycoin-output DIR` and `--accuracycoin-collect DIR` for a whole 22-page campaign |
| Inspect which rows exist | `--accuracycoin-inventory` |
| Caps for boot, navigation, execution | `--accuracycoin-boot-cap`, `--accuracycoin-navigation-cap`, `--accuracycoin-execution-cap` |
| Classify (pass, fail, timeout, not-run, draw, invalid, skip) | `accuracycoin_classify` in `tests/accuracy/test_conformance.c`; the JAM tune-up extends this classifier with a new cause, it does not need a new tool |
| Frame and audio hash regression | `--hash-frame`, `--hash-audio`, `tests/runner/hashes.txt`; re-pin only with a stated reason |
| Compare against a changed baseline | `prepare_scoreboard_baseline.cmake`, `verify_scoreboard_regression.cmake`, `scoreboard-main.txt` |
| Gate | `cmake --workflow --preset ci` (main suite), `asan` (same under sanitizers), `nofp` (integer-only check) |

JAM tune-up: the AccuracyCoin harness already reads guest state per frame; attribution needs the library to expose a "CPU halted by JAM" fact (or the runner to read the existing status) and the classifier to map `observed result stuck + JAM` to a distinct cause string like the existing `active-test-budget-exhausted`. This is an in-repo change, not a new source or tool.

## What NOT to add

| Do not add | Why |
|---|---|
| blargg, bisqwit or kevtris ROMs to `tests/roms/` or the manifest | No stated licence, cannot be redistributed; fetch-only if used at all |
| GPL or LGPL emulator source (Mesen/MesenCE, FCEUX, Nestopia, puNES, etc.) | Clean-room rule 4. MesenCE may be used as a released binary only (comparison of screens), never opened |
| TriCNES source reading as a spec | Source is MIT so it is permitted by licence ([CF.11]), but it shares its author with AccuracyCoin (common-mode, [CF.03]) and CLAUDE.md asks for hardware documentation and test behaviour. Prefer AccuracyCoin asm comments and NESdev; if TriCNES is ever consulted, record it in `PROVENANCE.md` |
| A TriCNES Windows runner, a MesenCE oracle lane, or a leaderboard submission | Open owner questions in CONFORMANCE.md; not needed to pass these 11 rows |
| Another test framework, a cycle-trace library, a logic-analyser or waveform tool | `check.h` and the existing tracer suffice |
| New runner flags such as `--nmi-trace` | Observation flags already give per-page evidence; a debug trace belongs in a test-only program, not the public runner |
| Floating point, wall-clock time or host state in any new timing code | Rule 5; `nofp` preset enforces |
| PAL timing constants or DMA cluster work | Next milestones (DMA preferred, then PAL); keep v5 NTSC-only |
| A newer AccuracyCoin pin | The row list and byte codes are pinned at 6bee919; bumping would move targets mid-milestone |

## Integration order the sources imply

1. Unit tests for `$2002`/NMI one-dot windows first (HWP.03), then fix the PPU flag and NMI-line model; `$2002 flag timing`, `NMI Timing`, `NMI Suppression`, `NMI at VBlank end`, `NMI disabled at VBlank` and `NMI Control` share this code.
2. CPU poll and hijack timing (HWC.02, HWC.17) for the three CPU rows; these depend on step 1's NMI edge being right at the half-cycle.
3. Sprite-0 behaviour last; `Arbitrary Sprite zero` timeout may clear as a side effect of correct `$2002` timing, otherwise it is its own fix.
4. Scoreboard flip and hash re-pins after each step, never batched at the end.

## Sources

- `.planning/preparation/CONFORMANCE.md`, `NES-HARDWARE-CPU-APU.md`, `NES-HARDWARE-PPU-CARTRIDGE.md` (citations HWP.03, HWP.06, HWP.20, HWC.02, HWC.17, CF.03, CF.06, CF.11, CF.14), read in this repo. HIGH.
- `tests/roms/manifest.txt`, `tests/accuracy/scoreboard.txt`, `tests/mmc3/pins.txt`, `README.md` (mmc3-oracle section), `runner/main.c` usage text, read in this repo. HIGH.
- https://github.com/100thCoin/AccuracyCoin (MIT) at 6bee919529289c555803b612b8c9e4f7eaaeeabb. HIGH (project pin; not re-fetched this session).
- https://github.com/christopherpow/nes-test-roms at 95d8f621ae55cee0d09b91519a8989ae0e64753b, no licence stated. MEDIUM (project survey of 2026-10-02).
- https://www.nesdev.org/wiki/PPU_frame_timing, https://www.nesdev.org/wiki/NMI, https://www.nesdev.org/wiki/CPU_interrupts, https://www.nesdev.org/wiki/PPU_sprite_evaluation (revisions as pinned in the preparation files). MEDIUM (T2, curated community reference).
