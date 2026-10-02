---
title: "NES emulator ecosystem: projects, lessons and user complaints"
summary: "Licence, status, test scores, timing models and speed of NES emulators, what users report, and the slot a permissive C core could fill."
read_when: "Choosing a timing architecture, ranking features, picking licence-safe references, or checking what a rival already does."
updated: 2026-10-02
---

# NES emulator ecosystem: projects, lessons and user complaints

## Why this file exists
Planning needs facts about existing NES emulators, not recollection. Every claim is tied to something fetched on 2026-10-02; scores and releases age fast, so Sources carries pins. GPL and LGPL emulator source was not opened; those projects are described from documentation, changelogs, issues and author posts. AC means AccuracyCoin, a test ROM whose pass count is today's public accuracy measure [ECO.03].

## 1. Landscape
Licence, language, status: [ECO.02]. AC: out of 140, leaderboard [ECO.01], unless marked. n/r = source not read; - = none; active = commit within 35 days.

| Emulator | Licence | Language | Status | AC | Timing model, architecture | Published speed |
|---|---|---|---|---|---|---|
| TriCNES | MIT | C# | v1.0.1 2025-04; active | 140 | One master clock per step; one 11.7k-line core file; no audio, TAS-file input only, NTSC, 8 mappers [ECO.03] | - |
| mame_nes | GPL, MAME fork | C++ | 1.9, 2026-09 | 140 | Rework inside MAME [ECO.04] | - |
| MesenCE | GPL-3.0 | C++ | 2.2.1, 2026-06; active | 137; a June PR build reportedly fails 1 [ECO.07] | n/r | ~250 fps (author, 2017); <400 (user, 2018) [ECO.11] |
| jemu | MIT | Java | active | 136 | Half cycles, half dots [ECO.05] | - |
| BizHawk NesHawk | MIT [ECO.21] | C# | 2.11.1, 2026-05 | 125 | n/r | - |
| ares | ISC | C++ | v148, 2026-05 | 119 (v147) | Cooperative threads [ECO.06] | - |
| BeesNES | MIT | C++ | active, no releases | 101 | PHI1/PHI2 tick table [ECO.08] | 90–144 fps with filters [ECO.08] |
| puNES | GPL-2.0 | C | v0.111, 2024-02 | 101 | n/r | 30–40 fps, Atom N550, 2017 [ECO.11] |
| Nestopia | GPL-2.0 | C++ | 2.0.0, 2026-09 | 97 (v1.40); 2.0.0 changelog claims 144/144 [ECO.10] | n/r | 60 fps at 40–45% CPU, same Atom [ECO.11] |
| fixNES | MIT | C | last commit 2020-11 | 97 | One CPU cycle per loop pass [ECO.12] | - |
| FCEUX | GPL-2.0 | C++ | 2.6.6, 2023-08 | 85 | n/r | ~1300 fps where Mesen ran <400 [ECO.11] |
| FCEUmm | GPL-2.0 | C | active | - | n/r | "very fast" [ECO.13] |
| QuickNES | LGPLv2.1+ in libretro info [ECO.13]; GPL-2.0 LICENSE file | C | push 2026-07 | -; TASVideos 108/156 [ECO.09] | n/r | "fastest" on libretro [ECO.13] |
| TetaNES | MIT OR Apache-2.0 | Rust | 0.17.0, 2026-09 | - | libretro core, deterministic states [ECO.16] | - |
| RustyNES | GPL-3.0-or-later | Rust | active | 144/144 self-reported | CPU half cycles | 3.95 ms/frame headless, i9-10850K [ECO.17] |

The leaderboard pins the 140-test AC commit of 2026-06-04; the ROM had 136 tests in March 2026 and 144 now [ECO.18][ECO.03], so a score needs its ROM commit. TASVideos' older list is saturated: Nestopia, Mesen and BizHawk pass 156/156 [ECO.09].

## 2. How the top scorers are built

| Project | Smallest step | PPU | DMA | Speed tactic |
|---|---|---|---|---|
| TriCNES [ECO.03] | One master clock: CPU every 12, PPU dot every 4, half-dot routine between; register accesses advance the clock mid-access | Dot and half-dot routines; delay counters on register writes | Get/put from an APU toggle; halt and alignment flags per DMA type; starts on a read cycle | None |
| jemu [ECO.05] | Half CPU cycle: PHI1, 3 half-dots, PHI2, 3 half-dots | Half-dot routine; separate fetch, shift, sprite-evaluation steps | Halt checked on PHI2; DMC steps dummy, alignment, get | Fixed interleave |
| BeesNES [ECO.08] | PHI1 and PHI2 ticks in a precomputed order | Function pointer per (scanline, dot) | RDY low aborts a read; state saved, cycle replayed | Branch-free bus decode |
| ares [ECO.06] | Whole CPU cycle per access; PPU per dot; threads resync each step | Scanline routine in dot steps | Get/put loop in the CPU read path | None: "half the amount of code, but slower" |
| RustyNES [ECO.17] | CPU half cycles; PPU, APU, DMA caught up to each | Per dot; fast dot path (−11%) | "one unified model" | Catch-up |

- MesenCE (source not read) polices speed in review: changes "may be rejected if they cause a drop in maximum FPS" [ECO.07].
- Cost of the finest step: AprNes (C#) logged 264 fps on a Ryzen 7 3700X stepping one CPU cycle then three PPU dots, and 87–102 fps after porting TriCNES's model (ROM and build type differ, so the ratio is rough). It ported because the coarser model passed 136/136 AC yet mis-rendered two PPU test ROMs [ECO.18].
- A NESdev developer passed all NMI tests "without hacks" only after splitting the CPU cycle into halves separated by PPU time, noting "there is no test ROM that verifies this" [ECO.11].
- Accuracy-class cores publish 250–400 fps on desktop CPUs; FCEUX did about 1300 [ECO.11][ECO.17][ECO.18]. RustyNES names its main costs: band-limited APU synthesis and the per-dot PPU loop [ECO.17].

Recommendation for the time step: half CPU cycles with PPU half-dots between them (jemu's shape), counted in half-master-clock ticks. Strongest alternative: TriCNES's one-master-clock step, the only permissive design scoring 140. What would change it: an AC test or hardware trace a half-cycle model cannot reproduce, or a C measurement putting master-clock stepping within about 20% of half-cycle speed.

## 3. Closest permissive prior art

| Project | Gets right | Gets wrong or lacks |
|---|---|---|
| fixNES (MIT, C) [ECO.12] | 97/140; 126 mappers; six expansion-audio chips; FDS, NSF, PAL | File-level static state; no save states (libretro adapter reports size 0), so no rewind, run-ahead or netplay; flagged experimental [ECO.13]; dormant since 2020 |
| binjnes (MIT, C) [ECO.19] | Context struct, save states, rewind, wasm build, hashed test harness; event scheduler with catch-up [ECO.09]; 114 of 127 listed test ROMs pass | No AC score; fails DMA-overlap and PPU open-bus tests; 49 mappers; one 251 KB source file |
| merton-nes (MIT, C) [ECO.20] | Opaque-context API (frame, state, config); README stresses "code clarity and readability"; mappers for 95.5% of licensed carts; bundles AC | No published score; DMA as delay counts; own core interface, not libretro |
| agnes, ObaraEmmanuel/NES, nesrev (MIT, C) [ECO.21] | Small cores; agnes is a two-file library | 4, 8 and 2 mappers; agnes has no APU |
| TetaNES core (Rust) [ECO.16] | Core crate, headless mode, libretro adapter | No AC score; formats may break before 1.0 |

Why no accuracy-class permissive C library exists yet:
- The single-number target is new: the AC repository dates from 2025-05; perfect scores by anyone but its author date from 2026 [ECO.02][ECO.04][ECO.10].
- The native accuracy leaders (MesenCE, Nestopia, mame_nes) are GPL; the permissive leaders are single-author C# and Java applications, and ares's ISC core is tied to its own framework [ECO.02][ECO.06].
- Borrowing gets noticed: RustyNES shipped under MIT/Apache until a NESdev reviewer showed ports of Mesen2, puNES and FCEUX code; it relicensed on 2026-08-04. Its postmortem: those emulators' source sat in the workspace as the accuracy target and an AI model ported from it [ECO.17].

## 4. Lessons
- **Mesen, MesenCE.** NESdev Wiki: "Excellent debugger" [ECO.09]. libretro: most accurate, but "not a great choice for weak mobile devices" [ECO.13]; RetroPie-Setup PR #3129: "too slow for a stock Pi 3B" [ECO.15]. One-author risk: Mesen and Mesen2 are archived; MesenCE has 12 or more contributors [ECO.02]. GitHub issues are off because they "generate a lot of extra work" [ECO.07]. Its libretro port trailed upstream for years (libretro/Mesen #35, #48) [ECO.15].
- **Nestopia.** An APU struct change broke save states in 2018: "No one wants to choose between losing all their save states or never updating" (0ldsk00l/nestopia #300). The libretro core bakes in its cartridge database, which blocked a user adding a homebrew entry (libretro/nestopia #25) [ECO.15].
- **FCEUX, FCEUmm.** A 2017 poll (FCEUX first, 17 of 35) praised its "debugging tools" and said it "sucks at emulating accurately" [ECO.11]; its README says "you might like Mesen more" [ECO.21]. FCEUmm: state size changes with a sound option (#204), Game Genie disables states and so run-ahead (#617), the option count overflowed a fixed array (#647) [ECO.15].
- **puNES.** States from 0.110 or earlier "are no longer compatible" [ECO.21].
- **mame_nes.** After 140, its author reported mapper changes that kept breaking other games [ECO.04].

## 5. What users praise and complain about
Ranked by count over all 304 issues in the four libretro NES core trackers, sorted by title [ECO.14].
1. **core** Mapper, board and ROM-format coverage (59); Mesen's known-problems thread has 100 comments.
2. **core** (binding is **host**) Peripherals: Zapper, paddle, Power Pad, microphone, turbo (42).
3. **core** Ports and crashes on consoles, big-endian, old compilers (38).
4. **core** Game-specific bugs on supported boards (32).
5. **core** Audio: pops, pitch, expansion audio, quality options (24).
6. **core** Palette, overscan, aspect defaults (22); Nestopia masks 8 px top and bottom, FCEUmm crops vertically [ECO.09].
7. **core** FDS disk swapping (17).
8. **core** Save states, battery saves, run-ahead, netplay (15).
9. **core** Cheats, achievement memory maps (11); overclock (5); region (4); input latency (2); weak-device speed (1); option count (1).

Low count, high heat [ECO.15]:
- **core** Input latency: FCEUmm showed 3 frames in Mega Man 2 against Nestopia's 2 until its frame boundary moved, a one-line fix (libretro-fceumm #45); puNES #78 on input delay drew 40 comments. libretro's guide: run-ahead works only with save states that are "clean and fast enough" [ECO.09].
- **core** States across versions: "Known working save states created in 2016 are not loading", answered "this is not a bug" (libretro-fceumm #215); RetroArch #18033 calls it "a frequent source of confusion and frustration".
- Praise: Nestopia "cycle accurate" and light [ECO.11]; FCEUmm "very fast", many mappers, "runahead and rollback netplay" [ECO.13].

**host** (keyword pass over 2,271 issue titles: UI 192, packaging 274, debugger tools 147 [ECO.14]): shaders, filters; fullscreen, vsync, frame pacing; menus, file dialogs; hotkeys, controller mapping; translations; packaging; debugger windows.
**out of scope**: ROM and BIOS supply, HD packs, GUI platform ports.

## 6. The open slot
Missing: an embeddable permissive C library that is accuracy-class (136 or more at a pinned AC commit), has complete deterministic save states and publishes its speed.
- Of ten NES core info files in libretro, every one not flagged experimental is GPL or LGPL, except an ISC core at version 0.0.1 with no options [ECO.13].
- Permissive projects at 101 or above are C#, Java or C++ applications; the best MIT C core scores 97, lacks save states and stopped in 2020 [ECO.01][ECO.12].
- No accuracy-class project publishes more than about 400 fps (section 2).
- Fast headless cores are wanted: quickerNES targets "headless re-recording for TASing and botting" [ECO.21].

Strongest reason this is wrong: MesenCE states interest in "potentially relicensing the emulator under a more permissive license, such as MIT", already takes contributions under MIT, and bars AI-generated code [ECO.07]. Next: TetaNES ships a permissive libretro core today [ECO.16], and binjnes and merton-nes have never been scored. A MesenCE relicence, or a leaderboard entry above 130 for any of those three, would change this section.

## 7. Names and trademarks
Most projects put "NES" in the name (Nestopia, puNES, TetaNES); Nintendulator and Nintaco echo "Nintendo" [ECO.01]. None of 19 emulator READMEs fetched carries a trademark or non-affiliation notice; BizHawk's has a legality note about dumping [ECO.21]. libretro labels cores "Nintendo - NES / Famicom (name)" [ECO.13]. puNES replaced a controller image bearing Nintendo's logo after a 2018 issue (#63) [ECO.15]; a NESdev thread advises against anything resembling Nintendo's quality emblem or red logotype [ECO.11]. US nominative use covers naming a product that cannot be identified otherwise, using only as much of the mark as needed, with nothing suggesting sponsorship [ECO.09]. Practice, not legal advice.

## Open questions
- AC scores of binjnes, merton-nes and TetaNES, and whether the self-reported perfect scores (Nestopia 2.0.0, RustyNES, AprNes) reproduce: run the pinned ROM in each.
- Headless fps of MesenCE 2.2.1 and Nestopia 2.0.0 on one named CPU: measure them; published figures are from 2017–18.
- MesenCE's intra-cycle timing model: an author post or design note would settle it.
- Status of the "NES" word mark: a USPTO lookup.
- Reddit and the Emulation General wiki were blocked here, so section 5 rests on issue trackers and forums: read both in a browser.

## Sources
| ID | Tier | Source | Pin or access date | Supports |
|---|---|---|---|---|
| ECO.01 | T2 | doclic.eu/neslead.htm | Last-Modified 2026-09-19; AC commit feafd85 | Scores |
| ECO.02 | T1 | api.github.com/repos/OWNER/NAME per repo named; gitlab.com/api/v4/projects/jgemu%2Fnestopia | 2026-10-02 | Licence, language, status |
| ECO.03 | T1 | github.com/100thCoin: TriCNES README, Emulator.cs; AccuracyCoin README | 94f1b11; 673ef55 | Limits, clock, DMA; test count |
| ECO.04 | T1 | github.com/theklap/mame_nes README, release notes | 31520b7 | Scope, regressions |
| ECO.05 | T1 | github.com/ArkoSammy12/jemu README, core/nes | dev acd4b26 | Half-cycle model |
| ECO.06 | T1 | github.com/ares-emulator/ares README, LICENSE, ares/fc | 4cb8d92 | Threads, DMA |
| ECO.07 | T1 | github.com/nesdev-org/MesenCE README, CONTRIBUTING.md, PR #189 text | a60e79f | Relicensing, FPS rule, issues |
| ECO.08 | T1 | github.com/L-Spiro/BeesNES README, Src/System, Cpu, Ppu | 96f6c98 | Tick table, claims |
| ECO.09 | T2 | docs.libretro.com (library/fceumm, library/nestopia, guides/runahead); tasvideos.org/EmulatorResources/NESAccuracyTests; nesdev.org/wiki/Catch-up, /wiki/Emulators; en.wikipedia.org/wiki/Nominative_use | 2026-10-02 | Curated references |
| ECO.10 | T1 | gitlab.com/jgemu/nestopia ChangeLog | fe23e4b0 | 2.0.0 claims |
| ECO.11 | T4 | forums.nesdev.org/viewtopic.php?t= 15532, 13844&start=270, 15491, 24603, 18074 | 2026-10-02 | Speed, poll, half cycles, logos |
| ECO.12 | T1 | github.com/FIX94/fixNES README, main.c, cpu.c, libretro/libretro.c | 156fcac | Loop, statics, no states |
| ECO.13 | T1 | github.com/libretro/libretro-super dist/info/*.info | a705405 | Core licences, flags, descriptions |
| ECO.14 | T4 | Own tally of issue titles in libretro/libretro-fceumm, /nestopia, /Mesen, /QuickNES_Core (304), plus SourMesen/Mesen, TASEmulators/fceux, punesemu/puNES, 0ldsk00l/nestopia (2,271 in all) | 2026-10-02 | Ranking |
| ECO.15 | T4 | github.com/OWNER/REPO/issues/N as named inline: those repos, RetroPie/RetroPie-Setup, libretro/RetroArch | 2026-10-02 | Quotes |
| ECO.16 | T1 | github.com/lukexor/tetanes README, Cargo.toml | a0a6b17 | Licence, libretro core |
| ECO.17 | T1 | api.github.com/repositories/1119119554 (RustyNES) README, docs on performance and relicensing | 81c09c2 | Model, speed, relicensing |
| ECO.18 | T1 | github.com/erspicu/AprNes README, MD/Performance*; forums.nesdev.org/viewtopic.php?t=26533 | 9f62449 | Port, fps logs |
| ECO.19 | T1 | github.com/binji/binjnes README, test_results.md, src/emulator.c | e2f5871 | Scheduler, tests |
| ECO.20 | T1 | github.com/snowcone-ltd/merton-nes README, src/nes.h, src/sys.c | b34b02b | API, DMA |
| ECO.21 | T1 | github.com READMEs: kgabis/agnes, ObaraEmmanuel/NES, justinbax/nesrev, TASEmulators/fceux, punesemu/puNES, TASEmulators/BizHawk (+LICENSE), SergioMartin86/quickerNES | 2026-10-02 | Scope, self-descriptions |
