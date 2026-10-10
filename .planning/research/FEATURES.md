# Feature Landscape

**Domain:** NES cartridge mappers (MMC1, MMC3, UxROM, CNROM, AxROM), battery saves and soft reset in an embeddable C core, plus the proofs that show them working
**Milestone:** v2 "Most of the library plays"
**Researched:** 2026-10-09
**Overall confidence:** MEDIUM-HIGH. Board facts are from the NESdev Wiki (curated community reference, cross-checked against Holy Mapperel's README). The libretro save-RAM flush rules and the complaint ranking are partly from recall or older tracker counts and are marked.

Builds on `.planning/preparation/NES-HARDWARE-PPU-CARTRIDGE.md` sections 2 and 5 (shares, bus conflicts, submappers) and `CONFORMANCE.md` (Holy Mapperel, MMC3 suites, committable ROMs). Those facts are not repeated here except where a requirement needs them.

## What exists today (dependency anchors)

| Existing piece | State at v1 | v2 consequence |
|---|---|---|
| Loader (iNES and NES 2.0, trainer, hostile-size checks) | Accepts mapper 0 only; trainer lands in PRG RAM | Must parse battery bit, NES 2.0 PRG-NVRAM size, submapper, CHR-RAM size; mapper numbers 1, 2, 3, 4, 7 become accepted |
| Page-table bus (1 KiB pages, per-access hooks planned) | NROM uses it | MMC3 needs a PPU-address observer (A12); MMC1 needs a CPU-write observer with cycle info |
| `retro_get_memory_data/size` | Returns NULL / 0 | Must return the save buffer for battery carts only |
| `retro_reset()` | Empty body; header says "test card has nothing to reset" | Becomes a real soft reset for a loaded game |
| Runner | Frame and audio hashes, movies, AccuracyCoin RAM read | Needs `.sav` load and write, and (for proofs) a way to read the nametable text and RAM |
| Header comment in `include/nesturbator.h` | No battery or save API | Public API grows; README and header comments change in the same PR (CLAUDE.md rule 6) |

## Board variants and which ones matter

Shares are North American licensed dumps (705) from the prep file; "all licensed" in brackets. These decide what is table stakes.

| Family | Share | Variants that matter | Variants that do not (this milestone) |
|---|---|---|---|
| MMC1 (mapper 1) | 33.9% [30.0%] | SNROM (8 KiB CHR-RAM or CHR-ROM, 8 KiB PRG-RAM; CHR A16 doubles as RAM enable), SKROM/SLROM/SGROM (no RAM or CHR-RAM, 256 KiB PRG), SOROM (16 KiB RAM in two banks), SUROM (512 KiB PRG via outer-bank bit, 8 KiB RAM), SXROM (32 KiB RAM in four banks plus 512 KiB PRG), SZROM (16 KiB RAM, 64 KiB CHR), SEROM/SHROM/SH1ROM (32 KiB fixed PRG, submapper 5) | MMC1A (mapper 155; RAM always on, a few Japanese titles), 2ME (submapper 6, Famicom Network System) |
| MMC3 (mapper 4) | 29.5% [25.0%] | MMC3B/C with Sharp IRQ behaviour (the common chip), MMC3A / NEC (submapper 4: latch 0 stops IRQs), 8 KiB PRG-RAM with protect bits, 4-screen carts (Gauntlet, Rad Racer II) | MMC6 (submapper 1; only StarTropics and StarTropics II, 1 KiB RAM, different protect scheme), MC-ACC (submapper 3), T9552 (submapper 5), TFROM hard-wired mirroring (submapper 2) |
| UxROM (mapper 2) | 14.0% [11.4%] | UNROM (3 bank bits), UOROM (4 bank bits), 8 KiB CHR-RAM | UN1ROM (mapper 94), mapper 180 (Crazy Climber), mapper 71 (no conflicts, Codemasters), UNROM 512 homebrew |
| CNROM (mapper 3) | 8.8% [6.7%] | 8 KiB CHR banks, 2 bits used by licensed games | Mapper 185 (CHR copy-protect; not mapper 3), 128 KiB oversize (Family Trainer: Jogging Race only), security diodes (Japanese; emulators need not model) |
| AxROM (mapper 7) | 5.1% [2.7%] | ANROM / AN1ROM (no conflicts), AMROM / AOROM (conflict wiring varies), 8 KiB CHR-RAM, single-screen select bit | BNROM (mapper 34), mapper 470, 512 KiB bit-3 variant (Hot Dance 2000) |

Battery-bearing licensed dumps: 364 of 2,216 (60 of 705 North American). Nearly all are MMC1 or MMC3 with 8 KiB RAM; a few use 32 KiB.

## Table Stakes

Missing any of these means a first-wave game in the family fails, or a user sees a save vanish.

### MMC1 (mapper 1)

| Feature | Why Expected | Complexity | Notes |
|---|---|---|---|
| 5-bit serial shift register on writes to $8000-$FFFF; register chosen by address bits 14-13 | Defines the chip | Low | Control, CHR 0, CHR 1, PRG |
| Bit 7 write resets the shift register and ORs $0C into control | Games boot with it; Shinsenden crashes if the reset is skipped on a repeated write | Low | Reset never ignored |
| Ignore all writes after the first on consecutive CPU cycles | Bill & Ted's Excellent Adventure depends on it (INC / DEC / ASL read-modify-write double write) | Med | Needs a cycle stamp on each mapper write; the existing bus-cycle model gives it. This is a hard-case index entry in the prep file |
| Control modes: PRG 32 KiB / fixed first / fixed last, CHR 8 KiB / 4 KiB, four mirroring values including both one-screen | Basic banking | Low | Power-on: treat control as $0C (last bank fixed). The wiki gives no fixed power-on value but commercial reset vectors assume it |
| PRG-RAM at $6000-$7FFF with enable bit ($E000 bit 4) | Holy Mapperel checks it (WRAM digit); Air Fortress needs RAM enabled at power-up | Low | MMC1B semantics as default: enabled at power-on, bit 4 set disables |
| CHR bits reused for RAM bank and PRG outer bank (SOROM, SUROM, SXROM, SZROM) | Without it, 16, 32 and 512 KiB boards break | Med | Derive from sizes, not submappers 1, 2, 4 (those are deprecated). Rule: PRG > 256 KiB means CHR A16 is the outer bank; RAM > 8 KiB means bits 3-2 select the RAM bank. In 4 KiB CHR mode both CHR registers should carry matching high bits |
| 8 KiB CHR-RAM when the header declares no CHR-ROM | SNROM-class and many SxROM games | Low | NES 2.0 byte 11 gives the size; iNES zero CHR banks means 8 KiB RAM |
| Default 32 KiB PRG-RAM when the header is iNES (no NES 2.0 size) and the board is mapper 1 | Wiki: "32 KiB covers all known titles" | Low | Volatile size and save size are different decisions; see Battery |
| Submapper 5 (fixed 32 KiB PRG: SEROM / SHROM / SH1ROM) | Cheap | Low | Treat as a normal MMC1 with 32 KiB PRG; no extra logic |

### MMC3 (mapper 4)

| Feature | Why Expected | Complexity | Notes |
|---|---|---|---|
| Bank select ($8000), bank data ($8001), R0-R7, PRG mode and CHR inversion bits | Defines the chip | Med | 8 KiB PRG pages, 1 and 2 KiB CHR pages: the existing 1 KiB page table covers it |
| Mirroring ($A000) with the 4-screen exception | Gauntlet and Rad Racer II use 4-screen; a header bit forces it | Low | $A000 has no effect on 4-screen carts |
| PRG-RAM enable and write-protect ($A001) | Holy Mapperel checks "read-only mode present" on mapper 4 (WRAM digit 2) | Low | Implement the Sharp/MMC3 bits; do not make mapper 4 depend on them for MMC6 titles. Many emulators leave protect off for compatibility; Holy Mapperel says otherwise, so the CI check decides |
| IRQ counter: latch ($C000), reload request ($C001), disable and acknowledge ($E000), enable ($E001) | Scanline split games (SMB3, Kirby, Mega Man 3-6, Crystalis, Gargoyle's Quest II) | High | Counter clocks on filtered PPU A12 rising edge; IRQ period is latch + 1 lines; $C000 does not change the live counter |
| A12 filter: the rising edge counts only after A12 has been low for three M2 falling edges | Without it, sprite and background fetch patterns double-clock; also makes `$2006` writes clock the counter with rendering off (documented behaviour; some games rely on it) | High | The existing half-master-clock model and per-dot PPU make this feasible; it needs the PPU to publish its bus address per dot and a mapper "M2 low run" counter. This is the single riskiest item in the milestone |
| Sprite fetches clock the counter when either BG or sprites render | G.I. Joe and Mickey in Letterland turn sprites off while BG stays on and still need IRQs | Med | Follows from clocking on fetch addresses, not on a scanline timer |
| Default revision = Sharp MMC3B/C ("new" behaviour; latch 0 gives an IRQ on every line) | Most shipped carts and the mmc3_test_2 suite's baseline tests | Med | Revision is a mapper-instance setting chosen at load |
| Revision switch for MMC3A / NEC (submapper 4 and a header-less default) | Astyanax and a few others differ by ASIC; mmc3_test_2 ends with revision-specific ROMs | Med | Cheap once the counter is correct: one flag changes the "latch 0" and "reload after 0" rule. Which games need the old behaviour is not established in the sources read; test ROM, not game, is the proof |
| Mapper IRQ line wired into the CPU IRQ with correct acknowledge | Existing CPU already polls IRQ | Low | The APU frame IRQ already exercises it |

### UxROM, CNROM, AxROM

| Feature | Why Expected | Complexity | Notes |
|---|---|---|---|
| UxROM: switchable 16 KiB at $8000, last bank fixed at $C000, 8 KiB CHR-RAM, mirroring from header | 14% of NA library | Low | Bank register is the full written value (masked by PRG size); hardware uses 3 or 4 bits |
| CNROM: 8 KiB CHR bank from the low bits of the write, CHR-ROM only | 8.8% | Low | Writes to CHR-ROM must be ignored (B-Wings, Fantasy Zone II, Krusty's Fun House write there and expect nothing) |
| AxROM: 32 KiB PRG bank (bits 2-0), one-screen select (bit 4), 8 KiB CHR-RAM | 5.1% | Low | Single-screen mirroring support in the PPU/nametable layer; check it exists for MMC1 too (shared requirement) |
| Bus conflicts as a board-level flag: value written = CPU value AND ROM byte at that address | UNROM, UOROM, AMROM, CNROM originals; Cybernoid relies on them | Low | A 1-line mapper input rule once the mapper can read the ROM byte at the written address |
| Conflict policy driven by NES 2.0 submapper (0 = unspecified, 1 = none, 2 = AND) with defaults: UxROM and CNROM submapper 0 behaves as AND (the real board; all licensed dumps in the database are submapper 2), AxROM submapper 0 behaves as none | Wiki: AOROM / ANROM games glitch if conflicts are emulated (Double Dare, Wheel of Fortune), while UxROM / CNROM licensed games are written around them | Low | Different default per mapper is deliberate, because the three boards were wired differently. The wiki's own advice for mapper 7 submapper 0 is "do not enforce". A mapper-2 default of "none" is what several emulators do, but it hides real bugs; AND is the safe hardware-true default because well-behaved licensed games work either way |
| AxROM submapper 2 and UxROM submapper 2 enforce AND | NES 2.0 homebrew and test ROMs | Low | |

### Battery saves and soft reset

| Feature | Why Expected | Complexity | Notes |
|---|---|---|---|
| Loader reads the battery bit (byte 6 bit 1) and NES 2.0 PRG-NVRAM size (byte 10 high nibble); separates NVRAM from volatile RAM | Without it, save RAM and work RAM are conflated and users lose or leak data | Low-Med | Battery bit without NES 2.0 size: 8 KiB default (see decision below). NES 2.0 sizes come through the hostile-header guards already in the loader |
| Library API: expose the NVRAM buffer pointer and size for a loaded battery cart, and a way to load bytes into it before the first frame | Both hosts need it; the sibling Neo Geo core uses the same API conventions | Med | Public header comment and README updated together. Size is constant for the lifetime of the loaded game |
| Raw bytes: the save is the NVRAM contents, byte for byte, no header or checksum | The milestone requires compatibility with existing `.sav` files from other cores | Low | A shorter file is accepted (copied to the front, the rest left at power-on value); a longer file is truncated, not rejected |
| libretro: return the NVRAM pointer for `RETRO_MEMORY_SAVE_RAM` (size constant, pointer stable while the game is loaded) and NULL/0 for carts without a battery; keep RTC and system RAM NULL | The frontend reads the region at unload and (if enabled) at an autosave interval, and loads it back before the first `retro_run` | Low | libretro.h (vendored) says the region is "regular save RAM... usually backed up by a battery". The frontend, not the core, picks the file name (RetroArch uses `.srm`); the runner picks `.sav` |
| libretro: save buffer must survive `retro_reset()` | Hardware keeps battery RAM across the reset button | Low | Do not clear or reallocate the buffer in reset |
| Runner: `--save <path>` (or default beside the ROM) loads the file if present and writes it when the run ends | Playstead and scripts need a deterministic, headless path | Low-Med | Write only for battery carts, only if the bytes changed, via temp file plus rename so a kill mid-write never destroys the old save |
| Never write a save file for a cart without the battery bit; never overwrite an existing file when loading it failed | Common loss path in other cores | Low | Test: a corrupt or unreadable `.sav` leaves the file untouched and the run reports the failure |
| Power-on RAM contents are a fixed pattern (zero) for volatile RAM and for NVRAM that had no file | Determinism rule: same inputs, same hashes | Low | Hardware power-on is random; zero fill is the deterministic choice and what Holy Mapperel's "SAVEDATA at $6100" check expects on first run |
| Real `retro_reset()` for a loaded game: soft reset (reset-button semantics) | Header comment: cores "should treat this as a soft reset, but hard resets are acceptable". The v1 audit lists it as debt | Med | See "Reset semantics" below |
| Public API call for soft reset (and, for hosts that want it, a separate power cycle) | The runner needs it for blargg `cpu_reset` / `apu_reset` style tests and Playstead for restarts | Low-Med | Names and ordering to be settled in the phase |

### Reset semantics (what hardware does, what to implement)

| State | Reset button | Power cycle |
|---|---|---|
| CPU registers | PC from the reset vector, I set, SP decremented by 3, A X Y unchanged; no stack writes | All power-on defaults |
| CPU RAM | Kept | Power-on pattern |
| PPU | Registers $2000 / $2001 cleared; writes to $2000, $2001, $2005, $2006 ignored for about 29,658 CPU cycles on the 2C02; VRAM, OAM, palette kept | Power-on defaults |
| APU | Channels silenced ($4015 cleared), frame counter reset, DMC and IRQ flags cleared | Power-on defaults |
| Mapper registers | Not touched by the console reset line on MMC1 / MMC3 / UxROM / CNROM / AxROM; games recover because the reset vector lives in the fixed bank | Unspecified at power-on; pick a fixed value (MMC1 control $0C; MMC3 R6/R7 per bank rule) |
| MMC1 shift register | Not reset by the console; a game writes $80 early | Empty |
| PRG-RAM / NVRAM | Kept | Kept for battery carts, pattern for non-battery |

Source: NESdev Wiki CPU / PPU / APU power-up and reset pages (the prep file already points at the `$6000` protocol suites `cpu_reset`, `apu_reset` and the PPU register-ignore window). Confidence MEDIUM; the exact list is in the phase's reset plan, not in this file.

Holy Mapperel's README states that pressing Reset gives an incorrect result in its battery test, so that ROM cannot double as the reset proof.

## Differentiators

Valued by embedders and by Playstead; not demanded by a casual player.

| Feature | Value Proposition | Complexity | Notes |
|---|---|---|---|
| Dirty flag or write counter on NVRAM so a host can flush only when it changed | Playstead needs "safe battery saves"; avoids flash wear and pointless writes; makes autosave cheap | Low | A single "written since last query" bit set in the PRG-RAM write path |
| Runner flush on a signal or a periodic interval, not just at normal exit | A killed headless run does not lose progress; libretro hosts only flush at unload unless the user turns on the autosave interval | Med | Use a plain `--save-every <frames>` option instead of signal handlers if portability on Windows is a concern |
| Mapper registers and IRQ state included in determinism hashes (state hash helper) | Makes bank-switch bugs visible as a one-line hash diff | Low | Not a save state; a read-only digest. Save states are SEED-002 and stay out of scope |
| Per-board frame-hash table in CI (Holy Mapperel ROMs, one game per board) | The milestone's own acceptance route | Med | See Proof section |
| Synthetic ROM fixtures generated in C test code (tiny programs that exercise MMC1 serial writes, double-write ignore, bus-conflict AND, CNROM CHR-ROM write ignore, battery persist, reset persistence) | Needs no licence, no manifest entry, deterministic, small; each behaviour gets a pass/fail test the CI can run | Med | Hand-assembled byte arrays are test code, not ROM files; confirm `scripts/hygiene.sh` treats generated arrays in `.c` files as source, not as committed ROM bytes |
| NES 2.0 submapper honoured for mapper 4 (0, 1, 4 at least) and recognised-but-unsupported (2, 3, 5) rejected with a clear error | Honest behaviour; avoids silent misemulation | Low | Reject at load with a distinct error code; the fuzzer corpus gets new entries |
| MMC3 revision auto-pick by test: run the same IRQ test ROM under each revision flag and record which ROM variant passes | Documents what the core claims instead of guessing games | Low | Goes in the scoreboard as separate keys |
| Zero-copy NVRAM view through the library API (host owns the file, core owns the bytes) | Embeds cleanly; no allocation in the core | Low | Matches "core links only the C memory functions" |

## Anti-Features

| Anti-Feature | Why Avoid | What to Do Instead |
|---|---|---|
| A bundled game database to guess battery bit, mapper, mirroring or submapper | Licensing of nes20db is unstated; EU database-right risk; the repo is MIT | Header-only decisions; a host-supplied override structure stays a later item |
| Mapper variants outside the six families "while we are in there" (MMC2/4 for Punch-Out!!, MMC5, VRC, mapper 34/66/71/94/180/185) | Scope creep; each has its own test needs | Put them in the next-tier seed; reject cleanly with an error naming the mapper |
| MMC6 (StarTropics) and MC-ACC, T9552 | Two licensed titles total for MMC6; different protect scheme; no test ROM committable | Accept `mapper 4 submapper 1` only after Holy Mapperel and mmc3 tests pass; record as a known gap in README |
| Modelling the MMC3 IRQ at scanline granularity ("count a line at dot 260") | Fails games that turn sprites off, use `$2006` clocking, or put both layers on one pattern table; Mega Man 3 pause screen, G.I. Joe, Wario's Woods | Clock from real A12 edges on the PPU address bus with the M2 filter |
| Always applying the CPU value on bus conflicts ("CPU wins") | Hides game bugs; wiki notes several early iNES dumps only work because of this, and Cybernoid needs the AND | Implement the AND rule by board default and submapper |
| Always enforcing bus conflicts on AxROM | Double Dare and Wheel of Fortune glitch | Default off for mapper 7 submapper 0 |
| Heuristic "fixes" keyed on ROM hash (per-game patches) | Not clean-room friendly; hides core bugs; same objection as a database | Fix the mechanism and cover it by a test ROM |
| Writing `.sav` on every frame or every NVRAM byte | Flash wear on the host, Windows file-lock errors, slow CI | Dirty flag; write at end of run, on interval, or on demand |
| A save file format wrapper (header, checksum, compression) | Breaks the "raw bytes compatible with `.sav`" requirement | Raw bytes only; any metadata goes in the host's own file |
| Writing the save file from inside the libretro core | The frontend owns the file and its name (`.srm`); a second writer races with it | Expose the region and let RetroArch persist it |
| Emulating security diodes in Japanese CNROM carts | Wiki: not needed | Skip |
| Rewinding or save states to prove reset or battery behaviour | SEED-002 territory | Prove with two runner invocations and file comparison |
| Hard reset reached through `retro_reset()` (reload the ROM) | Allowed by libretro, but it clears RAM and mapper state and masks reset bugs | Implement a soft reset; keep reload for "load game again" |

## Feature Dependencies

```
Loader: battery bit, NES 2.0 sizes, submapper
  -> Library NVRAM buffer API
       -> Runner .sav load / write
       -> libretro RETRO_MEMORY_SAVE_RAM (pointer stable, size constant)
  -> Mapper selection (1, 2, 3, 4, 7)
       -> Bus-conflict flag (UxROM, CNROM, AxROM)
       -> Single-screen mirroring in the nametable layer (MMC1, AxROM)
       -> CHR-RAM sizes from header (SNROM, UxROM, AxROM)

MMC1 consecutive-write ignore  <- cycle stamp on mapper writes (bus already counts cycles)
MMC3 IRQ  <- PPU address-bus observer (per dot, A12) + M2-low filter + CPU IRQ line
MMC3 revision flag  <- MMC3 IRQ working first

Soft reset in library
  -> retro_reset() in libretro
  -> runner reset test hooks (frame N)
  -> uses NVRAM kept, mapper registers kept, PPU ignore window

Tune-up phase (CI faster, flake-free, exact release-policy check, trainer host path) <- independent of mappers; must land first so the new per-board hashes are not run on a slow or flaky matrix

Holy Mapperel per-board hash  <- all five mappers + battery persistence (for the SAVEDATA check) + synthetic fixtures
Boot-to-play game proof  <- the game's mapper + (battery if the game saves) + audio + movie replay
```

## Proof and test hooks (written so a command can check each)

These are what the roadmap's success criteria should name.

| Behaviour | Command-checkable proof | Source / licence | Notes |
|---|---|---|---|
| Mapper basics for 1, 2, 3, 4, 7 and WRAM / CHR-RAM sizes | Holy Mapperel ROMs run N frames in the runner; the 4-digit code (WRAM, PRG, IRQ, CHR digits) read from the nametable equals 0000; frame hash pinned per ROM | zlib, committable; ROMs are built from source or taken from the v0.02 release | No memory channel for results: read tiles from the nametable (ASCII digits) or pin the hash. A per-board hash alone does not tell pass from fail; combine with a decoded-code check |
| Holy Mapperel is one test per board size (e.g. SNROM, SUROM 512 KiB, SXROM) | Run all 40 and pin each | Same | Names are not on the project page; list them at build time from the archive. Mapper 1 digit meanings: WRAM digit 1 = $E000 bit 4 not disabling, 4 = $A000 bit 4 mis-handled; mapper 4 digit 2 = no read-only mode |
| Battery persistence | Run 1: Holy Mapperel writes `SAVEDATA` at $6100 and exits; the runner writes the `.sav`; Run 2 loads the file and the code is still 0000. A shorter and a longer file are loaded without error | Holy Mapperel README | Reset during the run gives a wrong code, so do not use this ROM for reset |
| MMC1 serial / double-write, MMC1 variants | Synthetic fixtures; Bill & Ted behaviour verified by a fixture, not the game | Written here | Named test: `mapper.mmc1.consecutive_write_ignored` |
| MMC3 IRQ counter | mmc3_test_2 (6 ROMs) and mmc3_irq_tests (6 ROMs) via the $6000 protocol and $F8 byte; the last two ROMs of each suite target different chips | nes-test-roms, no licence stated: fetched nightly at a pin with SHA-256, not committed | Revision A vs B variants are different ROMs; the scoreboard records them as separate keys. Per-PR coverage still needs a committed proof: Holy Mapperel's gross IRQ check |
| Bus conflicts | Synthetic fixtures with a mapper write whose value differs from the ROM byte; assert resulting bank under submapper 0, 1, 2; plus Holy Mapperel mapper 2, 7 codes | Written here | Cover the per-mapper default (AND for 2 and 3, none for 7) |
| CNROM CHR-ROM write ignore | Fixture writing to $0000-$1FFF CHR space via PPU data; assert no change | Written here | |
| Soft reset | Fixture: set RAM and NVRAM bytes, request reset, assert RAM and NVRAM kept, CPU PC at the vector, SP decreased by 3, APU silent, PPU register writes ignored for the window | Written here; blargg `cpu_reset`, `apu_reset` as nightly extra | libretro host test: call `retro_reset()` after N frames and compare the following frames with the runner path |
| Trainer through the host path | Runner and libretro host both run a committed trainer image and match the core's bytes | Debt item | Needs a tiny synthetic mapper-0 image with a trainer; done by the same fixture builder |
| One game, boot to interactive play | Pick a game from the committed, redistributable set whose mapper falls in the six families (from CONFORMANCE: nes-runner v1, MMC1, MIT). Define the game-state change as a RAM-address predicate plus a nametable text, scripted input movie, a before/after frame hash that differs on input, and non-silent audio (nonzero PCM and a minimum spectral energy) | nes-runner is MIT, game, mapper 1 | Explicit: not "title screen reached". The seed says to define the observable first. Whether nes-runner reaches interactive play without extra hardware is not yet checked; confirm it runs before committing to it, and keep Nesteroids (MIT, mapper 0) as fallback evidence only |
| Hosted RetroArch capture of the same game | Extend the hosted `retroarch-e2e` job | Existing job | Local self-skipping tests: run or retire |

## What users complain about for these mappers

Evidence is a mix of NESdev Wiki, the wiki's own "Tricky-to-emulate games" page, libretro trackers counted in `NES-ECOSYSTEM.md` (mapper coverage is the largest group, 59 of 304 libretro NES issues; save-related 15), and recall. Items marked (recall) were not re-fetched.

| Complaint | Mapper | Cause | Our response |
|---|---|---|---|
| Bill & Ted's Excellent Adventure resets or garbles | MMC1 | Double-write ignore missing | Table stakes; fixture test |
| Shinsenden crashes | MMC1 | Reset bit skipped on repeated write | Table stakes |
| Air Fortress hangs or shows bad colour | MMC1 | PRG-RAM disabled at power-up | RAM enabled at power-on |
| Wrong RAM bank on SOROM/SXROM; large ROMs (512 KiB) black screen | MMC1 | CHR bits not reused for RAM bank / outer bank | Derive from sizes |
| Save file missing or empty | MMC1 / MMC3 | Battery bit unset in a header (libretro forum report found a Famicom game losing saves until the bit was set), or core writes only at clean exit | Honour the header bit; log "battery-backed: yes/no" clearly; flush on unload and optionally on an interval. The bit being wrong is not our bug, the log line is how users find out |
| Save does not load after switching cores | MMC1 | Different save size | Accept any length: pad short files, truncate long files |
| Status-bar jitter, wobbling title (Jurassic Park), moving seam (Crystalis), flicker (StarTropics), missing HUD frames (Astyanax) | MMC3 | IRQ clocked by scanline timer, not A12; revision differences; delay of one or two cycles | Real A12 filtering; revision flag; test ROMs |
| Mega Man 3 pause or boss select glitches | MMC3 / PPU | Wiki threads point at BG and sprites on the same pattern table, and at `$2006` write timing, not the IRQ itself | Do not chase these in the mapper; they belong to the PPU hard-case list already covered |
| Games with 4-screen VRAM show wrong layout | MMC3 | Header bit ignored | Honour the bit |
| StarTropics saves broken | MMC6 | Protect scheme differs | Out of scope; document |
| Games glitch with bus conflicts on (Double Dare, Wheel of Fortune) | AxROM | AOROM boards have no effective conflict | Default off |
| Games break without bus conflicts (Cybernoid, Shanghai II, Pescatore) | Class | Rely on AND (Cybernoid, listed on the tricky page) | Default AND for UxROM and CNROM; honour submapper |
| B-Wings, Fantasy Zone II, Krusty's Fun House garbage | CNROM | Writes into CHR-ROM take effect | Ignore CHR-ROM writes |
| Reset button does nothing or acts as power cycle | all | Soft reset not implemented, or reload used | Soft reset |
| Save lost on abnormal exit | all | Frontend writes SRAM only on unload unless the autosave interval is set (RetroArch default is 0; disabled) | Document; provide a dirty flag and interval-capable runner |
| Latency, speed, shaders | host | Out of scope | Not ours |

## Complexity ranking

| Item | Complexity | Dependencies on existing code | Risk |
|---|---|---|---|
| MMC3 IRQ with A12 filter and revisions | High | PPU must publish a per-dot address; CPU IRQ line | Timing sensitive; mmc3_test_2 is the judge |
| MMC1 including variants | Medium | Cycle stamp for double writes; single-screen mirroring | Variant derivation from sizes is subtle |
| Loader and save API (library, runner, libretro) | Medium | Loader hostile-input guards; public header | Public API change plus README plus header comments |
| Soft reset | Medium | CPU reset path exists for loaded images; PPU window | Many small states (APU, PPU ignore window) |
| UxROM, CNROM, AxROM with conflicts | Low-Medium | Mapper interface; single-screen | Default-by-mapper policy |
| Holy Mapperel and per-board hashes | Medium | Runner nametable read; manifest entries | 40 ROMs; hash plus decode, not hash alone |
| Boot-to-play proof | Medium | Movie replay; audio hash; chosen game's mapper | Game choice; defining the predicate |
| Tune-up debt | Medium | CI configuration; CTest scripts | Not mapper work; schedule first |

## MVP Recommendation

Order by dependency and risk (feeds the roadmap):

1. Tune-up phase first: faster, non-flaky CI, then the release-policy exactness check, trainer host path, `retro_reset()` shell and the local RetroArch decision. Every later phase adds many hashes; a flaky matrix would hide mapper regressions.
2. Loader and mapper interface growth plus UxROM, CNROM, AxROM with bus conflicts (cheapest family, exercises the mapper interface, submapper plumbing, single-screen mirroring and CHR-RAM before the harder chips). Add Holy Mapperel for mappers 2, 3, 7.
3. MMC1 with battery saves end to end (library API, runner `.sav`, libretro `RETRO_MEMORY_SAVE_RAM`), and the real soft reset. This is the second-largest family and the first battery user.
4. MMC3 with the scanline IRQ, then the revision flag, Holy Mapperel for mapper 4, mmc3_test_2 and mmc3_irq_tests in the nightly tier.
5. Per-board frame hashes and the boot-to-play proof last, on a game chosen from the committed, redistributable set.

Defer: MMC6, mapper 34, 71, 94, 155, 180, 185; PAL timing for any of these; any non-header guess about battery or submapper.

### Decisions the phases must still settle (flagged, not resolved here)

1. **Save size without NES 2.0 data.** Options: 8 KiB for every battery cart (Zelda, Metroid, most MMC3); expose 32 KiB for mapper 1 batteries to cover SXROM. Existing `.sav` files are normally 8 KiB; extra bytes in a longer file are harmless to other cores. Recommend 8 KiB by default, 32 KiB only when NES 2.0 says so or when mapper 1 with more than 256 KiB PRG and battery set, and verify the claim with a pad-and-truncate test. LOW-MEDIUM confidence on which licensed titles actually use more than 8 KiB.
2. **Which MMC3 revision is the default** when submapper is 0: Sharp (IRQ every line on latch 0) is the recommendation, since the prep file and wiki list it as the common chip and mmc3_test_2's first four ROMs pass on both.
3. **Whether to model MMC1 power-on control as $0C.** The wiki leaves it unspecified; commercial reset vectors assume the last bank is fixed.
4. **Autosave policy for the runner.** Exit-only is simplest; an interval option is a differentiator.

## Sources

- NESdev Wiki: MMC1, MMC3, UxROM, INES Mapper 003, AxROM, Bus conflict, NES 2.0 submappers, Tricky-to-emulate games (fetched 2026-10-09). Tier T2, MEDIUM-HIGH.
- Holy Mapperel README (github.com/pinobatch/holy-mapperel), fetched 2026-10-09. Tier T1, HIGH for its own behaviour: result digits, `SAVEDATA` at $6100, "Reset gives incorrect result".
- `libretro/libretro.h` (vendored in this repo): `RETRO_MEMORY_SAVE_RAM` comment, `retro_reset` comment "soft reset if possible, hard reset acceptable". Tier T1, HIGH.
- Web search results on RetroArch autosave: SRAM written at unload or clean exit by default; autosave interval off by default and writes only on change. MEDIUM (forum and wiki summaries; not read in the RetroArch source or manual).
- libretro forum thread on a missing battery bit causing lost saves (via search result; not opened). LOW-MEDIUM.
- `.planning/preparation/NES-HARDWARE-PPU-CARTRIDGE.md` sections 2 and 5, `CONFORMANCE.md`, `NES-ECOSYSTEM.md` section 5 (project files). HIGH for what they state.
- `.planning/milestones/v1-MILESTONE-AUDIT.md` (debt list); `SEED-001`, `SEED-261009-zs2`. HIGH.
- Clean-room note: no emulator source (GPL or LGPL) was opened. Emulator behaviour is cited only from wiki statements of what some emulators do.
