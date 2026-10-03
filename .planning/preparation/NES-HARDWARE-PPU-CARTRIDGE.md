---
title: "NES hardware: PPU, video output, cartridges and file formats"
summary: "Where PPU, video, mapper and ROM-format behaviour is documented, what is hard, and which public tests check it."
read_when: "Designing or building the PPU, video output, cartridge and mapper layer, or ROM loader."
updated: 2026-10-02
---

# NES hardware: PPU, video output, cartridges and file formats

## Why this file exists
The PPU and cartridge bus are where a first design most often forces a rewrite, because mappers and CPU-visible registers observe PPU internals dot by dot. This file gives the best source per topic, the constraining facts and a public test per hard case; numbers were read on 2026-10-02.

## 1. Source map
"W:" = NESdev Wiki page (T2; URL rule in [HWP.01]). "Breaks" = emu-russia decap write-up of the 2C02G and 2C07 (T1) [HWP.18]; its .md file names are from the directory listing, only pclk and Readme were read.

| Topic | Sources, best first |
|---|---|
| Rendering pipeline, frame timing | Breaks fsm, hv_decoder, dataread, pclk; W: "Visual 2C02"; [HWP.02] [HWP.03] |
| Registers, v/t/x/w; open bus, decay | Breaks regs, scroll_regs; [HWP.04] [HWP.05] [HWP.09] |
| Sprite evaluation; OAM | Breaks obj_eval, oam; AccuracyCoin [HWP.19]; [HWP.06] [HWP.09] |
| NMI, vblank, $2002 races | ppu_vbl_nmi readme [HWP.20]; [HWP.03] |
| Palette, colour, emphasis | Breaks cram, video_out; [HWP.07] [HWP.08]; pally (via [HWP.07]) |
| PAL, Dendy; RGB PPUs | Breaks pal, umc6538, rgb; [HWP.10] [HWP.07]; W: "PAL video", "Vs. System" |
| Cartridge connector | [HWP.11]; W: "PPU pinout" |
| Mapper families | W: "Mapper"; [HWP.12] [HWP.13] [HWP.15]; Holy Mapperel (in [HWP.16]) |
| File formats | [HWP.14] [HWP.17] |
| Controllers, expansion | W: "Input devices", "Standard controller", "Expansion port" |

## 2. Facts that shape the architecture
"AC" = AccuracyCoin test name [HWP.19] (144 tests, one NROM cartridge, built for RP2A03G + RP2C02G; pages 17–22 are PPU).

**Model depth**
- Time: the NTSC master clock is 236.25/11 MHz; a CPU cycle is 12 master clocks, a dot 4; PAL uses 16 and 5 [HWP.10]. Four CPU/PPU clock alignments exist and can shift register effects by a dot [HWP.03]. ppu_vbl_nmi tests flag and NMI timing to one PPU clock [HWP.20].
- Bus: a PPU access takes two dots: address with an external latch of the low 8 bits, then the read [HWP.02]. AC "ALE + Read", "Hybrid Addresses" and "$2007 Stress Test" depend on it and on the dummy nametable fetches.
- State: AC "BG Serial In" draws the constant bits shifted into the background shifters [HWP.02]; "Stale BG Shift Registers" needs shifters frozen in blanking; "$2004 Stress Test", "Misaligned OAM Behavior" and "Misaligned OAM2 Address" expose both OAM address counters and the shared buffer [HWP.09].
- Half dots: the 2C02 uses both pixel-clock levels as two states per dot [HWP.18]; TriCNES, by AccuracyCoin's author, steps master clocks with full and half PPU steps [HWP.21].

Recommendation: integer time in half-master-clock ticks ([ARCHITECTURE.md](ARCHITECTURE.md), section 2); one dot per PPU step with two ordered half-dot phases; fetches as real bus accesses; shifters, latches and OAM counters as literal state. Alternative: no half-dot phases, enough for blargg's tests; an AC page 20–22 failure traced to ordering inside a dot decides. Scanline or tile-batch renderers lack the state these tests probe. Analog decay can wait (I/O bus 3–30 ms [HWP.04]; AC accepts under 1 s).

**CPU-visible timing**
- VBL sets at line 241 dot 1, clears at line 261 dot 1. A $2002 read one dot early returns 0 and cancels flag and NMI for the frame; on the set dot or one later it returns 1 and cancels NMI. /NMI is vblank_flag AND NMI_output [HWP.03].
- With rendering on, odd frames jump from line 261 dot 339 to line 0 dot 0 [HWP.03].
- $2001 rendering changes act about 3–4 dots after the write [HWP.04]. A second $2006 write copies t to v about 3 dots after it begins; a write first shows CPU open bus (usually $20) to the PPU, which corrupts scroll at dot 257 [HWP.09].
- While rendering, $2007 increments coarse X and Y together [HWP.05] and $2004 reads $FF in dots 1–64 [HWP.06].

**What mappers observe**
- PPU address bus (fetch addresses when rendering, v otherwise [HWP.02]): MMC3 counts A12 rises that follow three M2 falling edges with A12 low, so $2006 writes clock it with rendering off [HWP.12].
- MMC5 finds scanlines from three consecutive PPU reads of one $2xxx address, watches $2000/$2001 writes, and substitutes nametable, attribute and per-tile CHR data [HWP.12].
- MMC2 switches CHR when the PPU reads tiles $FD and $FE; FME-7 and Namco 163 IRQs count CPU cycles [HWP.15].
- Bus conflicts: the latch gets CPU value AND ROM value [HWP.13]. Unmapped CPU reads return the last bus value [HWP.04].
- The cartridge drives CIRAM /CE and A10 (nametable replacement), /IRQ, and on 60-pin systems the audio path [HWP.11].
- Saves: battery bit plus NES 2.0 sizes; Namco 163's 128-byte internal RAM lies outside them [HWP.14][HWP.15].

**Bank sizes**: licensed ASICs bank PRG down to 8 KiB and CHR down to 1 KiB (MMC3, MMC5, FME-7, Namco 163) [HWP.12][HWP.15]; mapper 31 and NSF use 4 KiB PRG, NES 2.0 mapper 257 uses 1 KiB [HWP.15][HWP.17]. Recommendation: 1 KiB pages on both buses plus per-access hooks for snooping mappers. Alternative: 8 KiB CPU pages with callbacks, if measurement shows the finer table slows CPU fetches.

## 3. Hard-case index
| Behaviour | Why hard | Documented | Public test (see [HWP.16]) |
|---|---|---|---|
| $2002, $2000, odd-frame skip at vblank | one-dot windows | [HWP.03] | ppu_vbl_nmi 02–10 [HWP.20]; AC "NMI Suppression" |
| Sprite overflow bug; sprite 0 hit edges | n and m both step after 8 sprites; no hit at x=255 | [HWP.06] | sprite_overflow_tests; ppu_sprite_hit; AC page 19 |
| $2004, $2007 while rendering | expose OAM buffer, concurrent fetch | [HWP.05][HWP.06] | AC "$2004 Stress Test", "$2007 Stress Test" |
| Mid-frame rendering toggle | delay, stale shifters, OAM row copy | [HWP.04][HWP.09] | AC "Stale BG Shift Registers", "OAM Corruption" |
| Multiplexed address bus | latched low byte, live high bits | [HWP.02] | AC "ALE + Read", "Hybrid Addresses" |
| Early writes; v increment with reload | open bus seen first; v, t become the AND | [HWP.09] | 2nd2006_next_level (forum); none found for the AND case |
| OAMADDR non-zero or misaligned at evaluation | 8-byte row copy on 2C02G/H; shifted sprite reads | [HWP.06] | oamtest3; AC "Misaligned OAM Behavior" |
| MMC3 IRQ | real A12 edges, M2 filter, Sharp vs NEC | [HWP.12] | mmc3_test_2 [HWP.20]; mmc3irqtest |
| MMC5 scanline IRQ, ExRAM | snoops PPU reads, CPU writes | [HWP.12] | mmc5test, mmc5test_v2, exram |
| MMC1 serial writes; bus conflicts | second write of a read-modify-write ignored; AND per board | [HWP.12][HWP.13] | Bill & Ted; 2_test, 3_test, 7_test |

## 4. Video output contract
- Area: the picture is 256×240; NTSC adds a backdrop border of 16 pixels left, 11 right, 2 below [HWP.02]. Emulators usually show lines 8–231; pixel aspect ratio is exactly 8:7 on NTSC, about 1.3862:1 on PAL [HWP.10].
- Frame rate: dot clock 59062500/11 Hz; 89,342 dots per frame, 89,341 on odd frames with rendering on [HWP.03]: 39375000/655171 Hz (≈60.0988) alternating, 29531250/491381 Hz (≈60.0985) without the skip; PAL 322445/6448 Hz (≈50.0070). Fractions derived here; decimals per [HWP.10].
- Emphasis: $2001 bits 5–7 are red, green, blue on NTSC; PAL and Dendy swap red and green; RGB PPUs force the channel to full brightness. The signal drops to about 0.816 on the active colour phases, except for $xE/$xF [HWP.08].
- NTSC decoding: a pixel is 8 samples of a 12-sample subcarrier cycle; a line is 227⅓ cycles; frames alternate 59,561⅓ and 59,560⅔ cycles, or repeat every 3 frames when the skip is suppressed; start phase is set at reset [HWP.08]. No single palette matches every title [HWP.07].

Recommendation: per pixel one 16-bit value: bits 0–5 palette entry after greyscale and backdrop override, bits 6–8 raw $2001 emphasis bits. Per frame: uncropped 256×240 buffer, PPU model, ticks elapsed, subcarrier phase (0–11) at line 0 dot 0. Frame hashes then depend on no palette; the libretro adapter converts through a 512-entry table generated offline by tools/palgen from cited NESdev Wiki facts (D-12; pally is not vendored and may serve as an offline cross-check). Alternative: XRGB8888 from the core, better only if no host will filter NTSC or pick an RGB-PPU palette.

## 5. Cartridges and mappers
Counts recomputed from the nes20db snapshot dated 2020-04-19 in merton-nes (4,404 entries; class = folder prefix of each name; every dump or revision counts once) [HWP.22].

| Mapper | Licensed North America (705) | All licensed (2,216) |
|---|---|---|
| 1 MMC1 | 239, 33.9% | 665, 30.0% |
| 4 MMC3 | 208, 29.5% | 553, 25.0% |
| 2 UxROM | 99, 14.0% | 253, 11.4% |
| 3 CNROM | 62, 8.8% | 149, 6.7% |
| 7 AxROM | 36, 5.1% | 59, 2.7% |
| 0 NROM | 34, 4.8% | 191, 8.6% |
| Six together | 96.2% | 84.4% |

Next by all-licensed dumps: 206 (36), 5 (26), 19 (23), 16 (16), 18 (16), 66 (15), 69 (12), 210 (12). Cross-check: NesCartDB game counts quoted in wiki infoboxes (undated) are MMC1 390, MMC3 300, UxROM 155, the same rank order [HWP.12][HWP.15]. A 2026-09-26 nes20db release was not fetchable by script [HWP.23].

Recommendation: NROM first (AccuracyCoin runs on it), then MMC1, MMC3, UxROM, CNROM, AxROM, then the next tier as listed. Alternative: discrete boards 0, 2, 3, 7 first (231 North American dumps, 32.8%), better if early game coverage matters more than the mapper interface.

- MMC1 [HWP.12]: SNROM, SOROM, SUROM, SXROM, SZROM reuse CHR bits for PRG-RAM enable, PRG-RAM bank or the 256 KiB PRG half; MMC1A (mapper 155) keeps PRG-RAM always on. Without NES 2.0, 32 KiB PRG-RAM covers all known titles.
- MMC3 [HWP.12][HWP.13]: Sharp chips IRQ every line when the latch is 0, NEC chips (submapper 4) do not; MMC6 (submapper 1) has 1 KiB RAM; MC-ACC (submapper 3) clocks on A12 falling edges.
- Bus conflicts [HWP.13]: the wiki category lists 12 boards. Submappers 1 and 2 of mappers 2, 3, 7 select no conflicts or AND conflicts; in the snapshot all licensed UxROM and CNROM dumps are submapper 2, AxROM splits 27 without to 31 with [HWP.22].
- Battery [HWP.22]: 364 of 2,216 licensed dumps set the bit (60 of 705 North American).

## 6. File formats
- Detection [HWP.14]: magic `NES` $1A. Byte 7 AND $0C: $08 with declared sizes that fit the file is NES 2.0; $04 archaic iNES; $00 with bytes 12–15 zero iNES; otherwise iNES 0.7 or archaic, where bytes 7–15 may hold text ("DiskDude!" adds 64 to the mapper number).
- Layout [HWP.14]: 16-byte header, optional 512-byte trainer, PRG, CHR, then in NES 2.0 a miscellaneous ROM area sized by what remains; iNES files may append PlayChoice data and a 127- or 128-byte title.
- Hostile input [HWP.14]: NES 2.0 ROM sizes are 12-bit unit counts (PRG to 62,898,176 bytes, CHR to 31,449,088) or, with high nibble $F, 2^E × (2M+1) with E up to 63, beyond 64 bits; four RAM fields each allow 64 << n, n up to 15 (2 MiB). A loader that allocates from header fields before comparing them with the file length can be driven to huge allocations. Largest PRG in the snapshot: 64 MiB; largest licensed PRG 640 KiB, CHR 512 KiB [HWP.22].
- NES 2.0 CHR-RAM exists only if byte 11 declares it [HWP.14]. nes20db lookups hash each file twice: over the declared length and over the whole file minus header [HWP.23].
- Later [HWP.17]: FDS images have an optional 16-byte header and 65,500 bytes per side; the hardware needs an 8 KiB BIOS only the host can supply. NSF uses 4 KiB banks. UNIF is deprecated.

## 7. Header database licensing
- nes20db: neither the first post (2020-04-17) nor the 2026-09-26 release post states a licence [HWP.23]; merton-nes bundles a copy under a credit line (practice, not permission) [HWP.22].
- NesCartDB returned HTTP 403 to curl and the fetch tool (terms not read). No-Intro shows no licence text on its front page or DAT-o-MATIC download page [HWP.24].
- Law: the United States does not protect uncreative collections of facts; the EU database right restricts extracting a substantial part of a database built with substantial investment [HWP.25].

Recommendation: host-supplied override only: the library takes an optional header-override structure, the command-line runner a path to a user-provided database, and the repository carries no table. Reason: no upstream grants terms, a bulk hash-to-header copy is plausibly a substantial extraction under EU law, and an MIT licence cannot pass on rights the project lacks. Alternative: a bundled table, reasonable once the nes20db author grants terms in writing.

## 8. Regions, variants and peripherals
| | NTSC 2C02 | PAL 2C07 | Dendy UA6538 | RGB 2C03/4/5 |
|---|---|---|---|---|
| Master clocks per dot / CPU cycle | 4 / 12 | 5 / 16 | 5 / 15 | 4 / 12 |
| Dots per frame | 89,341.5 mean | 106,392 | 106,392 | 89,342 |
| Post-render lines; vblank lines | 1; 20 | 1; 70 | 51; 20 | 1; 20 |

From [HWP.10]. 2C04s scramble the palette and 2C05s swap $2000/$2001 [HWP.07][HWP.14]. NTSC-first defers PAL and Dendy timing, RGB PPUs with Vs. System and PlayChoice hardware, FDS, expansion audio, expansion-port devices and PPU revisions other than G. Header byte 15 names the default input device [HWP.14].

## Open questions
- Does any AccuracyCoin test need half-dot ordering? Its assembly comments for pages 20–22 [HWP.19] would settle it.
- Which CPU/PPU alignment do reference results assume? ppu_vbl_nmi notes that some resets differ [HWP.20]; AccuracyCoin's per-test comments would settle it.
- Current shares and database terms: nes20db 2026-09-26 downloaded by hand [HWP.23]; a reply from its author; a manual visit to nescartdb.com.

## Sources
r = wiki revision id; repository pins are commit prefixes.

| ID | Tier | Source | Pin or access date | Supports |
|---|---|---|---|---|
| HWP.01 | T2 | NESdev Wiki; page "X Y" is https://www.nesdev.org/wiki/X_Y | 2026-10-02 | pointers |
| HWP.02 | T2 | W: PPU rendering | r23300 | pipeline |
| HWP.03 | T2 | W: PPU frame timing; NMI | r19961; r23420 | vblank |
| HWP.04 | T2 | W: PPU registers; Open bus behavior | r22850; r23814 | registers |
| HWP.05 | T2 | W: PPU scrolling | r23139 | v/t/x/w |
| HWP.06 | T2 | W: PPU sprite evaluation; PPU OAM | r22442; r23161 | sprites |
| HWP.07 | T2 | W: PPU palettes | r24257 | palettes |
| HWP.08 | T2 | W: NTSC video; Colour emphasis | r24244; r23220 | signal |
| HWP.09 | T2 | W: Errata; PPU glitches | r24163; r23827 | glitches |
| HWP.10 | T2 | W: Cycle reference chart; Overscan | r22030; r22760 | clocks |
| HWP.11 | T2 | W: Cartridge connector | r24187 | signals |
| HWP.12 | T2 | W: MMC1; MMC3; MMC5 | r24091; r24268; r24329 | ASICs |
| HWP.13 | T2 | W: Bus conflict (+ category); NES 2.0 submappers | r24010; r23708 | variants |
| HWP.14 | T2 | W: INES; NES 2.0 | r23194; r24342 | headers |
| HWP.15 | T2 | W: UxROM; MMC2; Sunsoft FME-7; INES Mapper 019; INES Mapper 031; NES 2.0 Mapper 257 | r24147; 22483; 24262; 24233; 19421; 23118 | banks |
| HWP.16 | T2 | W: Emulator tests; Tricky-to-emulate games | r23774; r23875 | tests |
| HWP.17 | T2 | W: FDS file format; Family Computer Disk System; NSF; UNIF | r22826; 23901; 23612; 23877 | formats |
| HWP.18 | T1 | https://github.com/emu-russia/breaks BreakingNESWiki_DeepL/PPU | 5a55bac | circuits |
| HWP.19 | T1 | https://github.com/100thCoin/AccuracyCoin README.md | 673ef55 | tests |
| HWP.20 | T1 | https://github.com/christopherpow/nes-test-roms ppu_vbl_nmi, mmc3_test_2 readmes | 95d8f62 | tests |
| HWP.21 | T3 | https://github.com/100thCoin/TriCNES Emulator.cs | 94f1b11 | stepping |
| HWP.22 | T1 | https://github.com/snowcone-ltd/merton-nes assets/db/nes20db.xml | b34b02b | counts |
| HWP.23 | T4 | https://forums.nesdev.org/viewtopic.php?t=19940 and post p310787 | 2026-10-02 | nes20db |
| HWP.24 | T2 | https://no-intro.org/ ; https://datomatic.no-intro.org/index.php?page=download | 2026-10-02 | No-Intro |
| HWP.25 | T2 | https://en.wikipedia.org/wiki/Sui_generis_database_right | 2026-10-02 | law |
