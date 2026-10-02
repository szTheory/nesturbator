---
title: "Conformance tests, oracles and the scoreboard"
summary: "Public NES test suites and reference emulators - what each proves, how it reports headlessly, its licence and its pin."
read_when: "Choosing test ROMs; writing the headless runner, pin manifest, scoreboard or CI tiers."
updated: 2026-10-02
---

# Conformance tests, oracles and the scoreboard

## Why this file exists
nesturbator's correctness claims rest on outside tests. They differ in what they prove, in whether expected results come from hardware or an emulator, and in redistribution rights. Facts here were fetched or measured on 2026-10-02.

## What each suite proves

| Suite | Licence | Size | Proves; does not prove | Pin | Headless protocol | Results from |
|---|---|---|---|---|---|---|
| AccuracyCoin | MIT | 1 ROM, 40,976 B, mapper 0 | 144 tests of CPU, unofficial opcodes, interrupts, DMA, APU, PPU and sprite timing on NTSC RP2A03G + RP2C02G; no mappers, audio output, PAL | commit [CF.01] | one Start press; result bytes in RAM $0403-$0495 | author's consoles and emulator [CF.01][CF.03] |
| 65x02 `nes6502/v1` | MIT | 256 files, 1,081,529,097 B | state and per-cycle bus activity of single instructions, all opcodes; no interrupts, DMA, NES memory map | commit [CF.04] | compare final state and bus list | an emulator [CF.04] |
| Holy Mapperel | zlib | 40 ROMs, 17,964 B 7z archive (over 8 MB unpacked) | banking, WRAM, CHR RAM, gross IRQ for mappers 0-4, 7, 9, 10, 11, 28, 34, 66, 69, 78.3, 118, 180; not exhaustive (README); NES 2.0 headers | v0.02 [CF.05] | no memory channel: 4-digit screen code (0000 normal), beeps | real boards: a PCB test [CF.05] |
| nes-test-roms | none | 290 ROMs, 14,885,408 B | per subsystem; see Classic suites | commit [CF.06] | $6000 protocol in 19 suites, else screen text | hardware, per most blargg readmes [CF.06] |

## AccuracyCoin in detail

**Running headless.** After power-on the cursor rests on the page index, where a new Start press (the menu's NMI handler reads controller 1 each frame) runs every test and draws a results table. $EC reads $0A once the menu is up. $35 is 1 during the run, then 0; $37 then holds the tests counted, $38 the passes, $3F the skips. Tests that read controllers fail while a button is held, so Start is released once the run begins. A hang leaves that test and all later ones at $00 [CF.01].

**Result bytes.** $00 not run, $FF skip. Otherwise bits 0-1: 1 pass, 2 fail, 3 running; bits 2-7: a code. On fail it is the error number in the README's list for that test; on pass, non-zero names the accepted hardware variant seen ($39: variant E, PPU revision E or earlier) [CF.01].

**Names.** The ROM holds its own table: at file offset $0110 (CPU $8100), little-endian page pointers ending where the first one points (22 pages). A page is a title, $FF, then entries of name, $FF, result address, routine address, closed by $FF. The pinned ROM yields 149 uniquely named entries: 144 tests and 5 DRAW entries (result address $03FF) that only print power-on state. Parsing it replaces a hand-kept address list [CF.01].

**Categories** (tests). CPU behaviour and addressing 15; unofficial instructions 66; interrupts 3; APU registers and DMA 10; APU 9; CPU behaviour 2: 5; PPU 27; advanced background and sprite evaluation 9 [CF.01].

**Pages.** 1 CPU behaviour; 2 addressing-mode wraparound; 3 to 11 unofficial instructions; 12 CPU interrupts; 13 APU registers and DMA; 14 APU tests (Length Counter, Length Table, Frame Counter IRQ, Frame Counter 4-step, Frame Counter 5-step, Delta Modulation Channel, APU Register Activation, Controller Strobing, Controller Clocking); 15 CPU behaviour 2; 16 power-on state (the DRAW entries); 17 PPU behaviour; 18 vblank timing; 19 sprite evaluation; 20 PPU miscellany; 21 and 22 advanced background and sprite evaluation. Names are as the ROM's own table spells them [CF.01].

**Change rate.** 376 commits from 2025-05-09 to 2026-09-23, 281 changing the ROM; 140 tests on 2026-06-04, 144 on 2026-09-03. Tests get removed, renamed and renumbered; no version label, no release [CF.01][CF.02].

**Hardware status.** No test is individually marked as verified on hardware; the README only names the design target. The author reports three tests failing on an Everdrive N8 Pro and two for analogue reasons; tests that failed on consoles were removed or made DRAW entries [CF.01][CF.02].

**Leaderboard.** Submission: an e-mail to the page's address with a PNG of the results table and both revisions. All 39 rows use the older commit feafd85e (140 tests): TriCNES and mame_nes 140, MesenCE 2.2.1 137, BizHawk 125, FCEUX 85 [CF.03].

## The 65x02 vectors

**Schema.** `00.json` to `ff.json`, each 10,000 tests, one per line: `name` (not unique), `initial` and `final` (`pc`, `s`, `a`, `x`, `y`, `p`, `ram` as [address, value] pairs), `cycles` as [address, value, "read" or "write"]. Memory is 64 KiB of flat RAM [CF.04].

**Coverage.** All 256 opcodes, including JAM and the unstable ones, where the vectors fix one emulator's choice: ANE and LXA fit only the constant $EE [CF.04]; AccuracyCoin tests those two only where the constant cannot matter [CF.01]. Absent: reset, NMI, IRQ, interrupt polling, DMA, open bus, registers, mirrors [CF.04].

**Per-pull-request sample.** Recommendation: the first 100 tests of each file (25,600), fixed by index order at the pin. They sit in the first 54,433 bytes of each measured file, so 256 range requests of 64 KiB rebuild the sample [CF.04]. Strongest alternative: every 100th test, which needs the full set. Nightly failures the sample misses would argue for a larger one.

**Binary form.** Recommendation, little-endian: header `N65V`, version u8, opcode u8, count u16; per test: index u16; `initial` and `final` each as pc u16, s a x y p u8, count u8, (address u16, value u8) pairs; count u8, (address u16, value u8, kind u8) cycle triples. Measured 63.3 B per test: 1.6 MB for the sample (committable as MIT data), 162 MB for the full set.

**Nightly full set.** 2,560,000 tests, about 11 million CPU cycles; the cost is download and parsing. The tree is 5.17 GB over five CPU variants; a sparse checkout of `nes6502/v1` fetches 1.08 GB. GitHub Actions caches default to 10 GB per repository and drop entries unused for 7 days [CF.04][CF.12].

## Classic suites

From the nes-test-roms pin [CF.06]; brackets are ROM counts.

**$6000 protocol.** $6000: $80 running, $81 press reset at least 100 ms later, $00-$7F final code (0 passed). $6001-$6003: $DE $B0 $61. Zero-terminated ASCII from $6004. It needs RAM at $6000-$7FFF though the mapper-0 ROMs declare none [CF.06]; AccuracyCoin's open-bus tests use only $4000-$5FFF, so that RAM does no harm there [CF.01].

**Screen text.** Prints PASSED or FAILED #n; some suites keep the code in zero page $F8 (1 passed; marked in the table). No end marker, so runs last a fixed frame count. Tiles are ASCII codes, readable from the nametable.

| Area | $6000 | Screen text |
|---|---|---|
| CPU: opcodes, cycle counts, dummy accesses, interrupts, reset | instr_test-v5 (18; skips KIL, $8B, $93, $9B, $9F, $BB), instr_misc (5), instr_timing (3), cpu_interrupts_v2 (6), cpu_reset (2), cpu_exec_space (2), cpu_dummy_writes (2) | nestest (entry $C000 leaves error bytes in $02, $03), cpu_timing_test6, branch_timing_tests (3, $F8), cpu_dummy_reads |
| PPU: VBL/NMI timing, OAM, $2007, sprite 0 hit, overflow | ppu_vbl_nmi (11), ppu_open_bus, oam_read, oam_stress, ppu_read_buffer | blargg_ppu_tests_2005.09.15b (5), sprite_hit_tests_2005.10.05 (11, $F8), sprite_overflow_tests (5, $F8), vbl_nmi_timing (7, $F8) |
| APU: length and frame counters, IRQ, DMC, reset | apu_test (9), apu_reset (6) | blargg_apu_2005.07.30 (11), dmc_tests (4), apu_mixer (4, by ear) |
| DMA: reads during DMC DMA | | dmc_dma_during_read4 (5, prints CRC), sprdma_and_dmc_dma (2), read_joy3 (4) |
| Mapper: MMC3 IRQ counter | mmc3_test_2 (6) | mmc3_irq_tests (6, $F8) |

Mapper 0 except multi-test ROMs (1), cpu_dummy_reads, read_joy3, ppu_read_buffer (3) and MMC3 suites (4). cpu_reset and apu_reset request a reset via $81.

Hardware-dependent results: `dma_2007_read` has two valid CRCs; `oam_stress` passes on one of four power-on alignments; the last two ROMs of each MMC3 suite target different chip revisions [CF.06]. On the saturated TASVideos list (156 tests) three emulators score 156, flash-cart consoles 123-132 [CF.08].

**Licence.** No blargg, bisqwit or kevtris package states one (589 source files and readmes checked), nor does the repository. They cannot be redistributed; the workable route is a fetch at the pinned commit checked against a SHA-256 manifest, nothing committed. `test_roms.xml` gives frame counts for 180 entries (168 NTSC, 40,300 frames); its verdicts are one emulator's recorded results [CF.06].

## ROMs that can be committed or released

Explicit licences only [CF.13]; m = mapper.

| Licence | ROMs (source and pin) |
|---|---|
| MIT | AccuracyCoin (`100thCoin/AccuracyCoin` @673ef550); Nesteroids (`battlelinegames/nesteroids` @ae65d780; game, m0); nes-runner (`zorchenhimer/nes-runner` v1; game, m1) |
| zlib | Holy Mapperel (`pinobatch/holy-mapperel` v0.02); Double Action Blaster Guys (`NovaSquirrel/DABG` v2; game, m0); Pently demo (`pinobatch/pently` v0.05wip11; music, m0) |
| all-permissive | RHDE (`pinobatch/rhde-nes` v0.07; game, m0) |

Excluded: Thwaite, Concentration Room (GPL-3.0), 240p Test Suite (GPL-2.0): copyleft. Nova the Squirrel: CC BY-NC-SA assets. Lizard: assets not redistributable. Super Tilt Bro (WTFPL): no release asset. nes15: fonts of unstated origin. spritecans: only "feel free to spread". tvpassfail: "do not distribute".

## Oracles

**MesenCE** (GPL-3.0). Pin: release 2.2.1; its API listing gives a SHA-256 per asset, including macOS ARM64 and Linux x64 zips, which need SDL2 [CF.09]. Headless use is documented for Mesen 0.9.9 as `--testrunner <rom> <script.lua>`, which runs at maximum speed until the script calls `emu.stop(exitCode)` and makes that the process exit code [CF.10]; on MesenCE 2.2.1 it is untested. The Lua reference lists `emu.read` with `emu.memType.nesDebug` (no side effects), `emu.setInput` in an `inputPolled` callback, the `endFrame` event, `emu.getScreenBuffer()` (ARGB) and `emu.takeScreenshot()` (PNG) [CF.09]. RAM power-on pattern and random CPU/PPU alignment are settings, so the settings file belongs to the pin [CF.10].

**TriCNES** (MIT). Pinned by commit. A .NET Framework 4.8 WinForms program that takes no arguments and documents no headless mode. README: no audio, input only from TAS movie files, NTSC only, mappers 0-4, 7, 9, 69. Its `Emulator` class exposes frame advance, memory observation and a 256x240 bitmap, so a small Windows console host is possible [CF.11].

**Recommendation.** MesenCE as the routine reference, comparing still result screens under one shared 64-colour palette (oracle ARGB depends on palette and frame boundary) or decoded nametable text. Strongest alternative: a TriCNES host on a Windows runner, justified by repeated disputes over tests MesenCE fails.

**Common-mode risk.** AccuracyCoin was written to test TriCNES and shares its author, so their agreement is one opinion [CF.03]. The 65x02 vectors are emulator-generated [CF.04]; nestest's reference log is Nintendulator's [CF.07]. MesenCE 2.2.1 fails 3 of 140 AccuracyCoin tests [CF.03]. An oracle difference locates a divergence; it does not say which side is right.

## Scoreboard design

Recommendation: one committed, sorted text file, one tab-separated line per test: `key status code frames hash`. `key` is suite and test name (`accuracycoin/NMI Overlap BRK`, `65x02/a9`). `status` is `pass`, `fail`, `unavailable` (ROM not fetched in this run) or `unsupported` (needs a feature the core does not claim). `code` is the suite's own result (AccuracyCoin byte, $6000 code, failing-vector count). `hash` is a SHA-256 prefix of the final frame as palette indices, or `-`.

Pin manifest, per fetched object: URL, commit, SHA-256, licence, protocol (`accuracycoin-ram`, `blargg-6000`, `blargg-f8`, `screen-text`, `vectors`, `frame-hash`), frame cap, input script, and accepted codes where hardware has several valid outcomes.

"May only improve" compares keys, never totals, and only between runs with identical pins: `pass` stays `pass`; `fail` to `pass` updates the file in the same change; `unavailable` is neither. A pin bump is its own change: added keys enter with the status they have, removed keys are deleted, a rename is a removal plus an addition.

Strongest alternative: JUnit XML per run, readable by dashboards but poor to diff; adopting a dashboard would favour generating it from this file.

## Where each test runs

Recommendation; costs are emulated time (host speed unmeasured).

| Tier | Contents | Cost |
|---|---|---|
| Per pull request | committed files only: 65x02 sample, AccuracyCoin, Holy Mapperel, open-licence games (frame and audio hashes) | 113,000 CPU cycles for the sample; AccuracyCoin at least 400 frames |
| Nightly | fetched at pins: full 65x02, classic suites, MesenCE comparison, AccuracyCoin HEAD report | 1.08 GB download; 40,300 frames (11 minutes) |
| Local only | private commercial ROMs: frame and audio hashes under scripted input, results kept out of the repository | depends on the set |

Strongest alternative: classics per pull request from a cache, justified by a warm run under a minute.

## Open questions
- AccuracyCoin run length in frames: one logged run in MesenCE's test runner.
- Per-test hardware status of AccuracyCoin at the pin: a console run log at that commit.
- Does MesenCE 2.2.1 still accept `--testrunner`, and does it run without a display server on Linux CI, and with which power-on settings: one trial run.
- Does instr_test-v5, which tests $AB, accept the vectors' LXA constant $EE (unverified): one run of 03-immediate.
- Result addresses in Holy Mapperel and the 2005 blargg APU/PPU ROMs: a linker map or disassembly.
- Owner: are GPL-licensed test ROMs fetched nightly or left out (they are not committed)? Windows runner for TriCNES? AccuracyCoin pin bumps manual or scheduled? Leaderboard submission (needs a PNG frame dump)?

## Sources
| ID | Tier | Source | Pin or access date | Supports |
|---|---|---|---|---|
| CF.01 | T1 | github.com/100thCoin/AccuracyCoin (README, source, ROM, history) | 673ef550db296136d52229961e7d39366116882a | tests |
| CF.02 | T4 | same: issues 9, 17, 20, 31, 62; discussion 11 | 2026-10-02 | hardware |
| CF.03 | T2 | doclic.eu/neslead.htm | 2026-10-02 | leaderboard |
| CF.04 | T1 | github.com/SingleStepTests/65x02 (READMEs, tree, 18 files measured) | 2f6980a2d95757486c7bee24355c360e40e2a224 | schema |
| CF.05 | T1 | github.com/pinobatch/holy-mapperel (README, release v0.02) | c022622274ca8b83d214dea97e4388a6b0e92d8a | boards |
| CF.06 | T1 | github.com/christopherpow/nes-test-roms (readmes, sources, test_roms.xml) | 95d8f621ae55cee0d09b91519a8989ae0e64753b | classics |
| CF.07 | T2 | nesdev.org/wiki/Emulator_tests | revision 23774 | nestest log |
| CF.08 | T2 | tasvideos.org/EmulatorResources/NESAccuracyTests | 2026-10-02 | scores |
| CF.09 | T1 | github.com/nesdev-org/MesenCE (release 2.2.1, UI/Debugger/Documentation/LuaDocumentation.json) | 20ba206cef5ba207c21203176d02cb9f43dda9fb | Lua API |
| CF.10 | T1 | mesen.ca/docs (apireference, configuration/emulation; v0.9.9) | 2026-10-02 | test runner |
| CF.11 | T1 | github.com/100thCoin/TriCNES (README, project file, sources) | 94f1b1178057e84c56e4c2410c8391eee4d78975 | limits |
| CF.12 | T1 | docs.github.com/en/actions/reference/workflows-and-actions/dependency-caching | 2026-10-02 | cache |
| CF.13 | T1 | github.com/ + each repository named under committable ROMs (LICENSE, README, releases) | 2026-10-02 | licences |
