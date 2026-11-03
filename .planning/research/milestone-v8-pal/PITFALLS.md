# Domain Pitfalls: PAL Region Support

**Domain:** Deterministic NES core adding licensed PAL 2A07/2C07 behavior  
**Researched:** 2026-11-01  
**Overall confidence:** MEDIUM for timing/APU tables and host-interface contracts; LOW for the exact 2A07 DMA halt mechanism and any claim of canonical 2C07 analog colors.

## Scope and Evidence Boundary

This research covers the v8 scope in [PROJECT.md](../../PROJECT.md) and the existing PAL stack guidance in [STACK.md](STACK.md): NTSC and licensed PAL profiles, six existing mapper families, a modeled 2C07 palette, PAL 2A07 DMA behavior with an explicit evidence limit, native state identity, and the existing library, runner, and libretro deliverables. Dendy, new boards, Playstead/libfuturo integration, and consumer-host work remain outside scope.

The preparation index defines source tiers: **T1** primary material, official documentation, circuit analysis, or measurements made here; **T2** curated community hardware references such as NESdev Wiki; **T3** implementation behavior; **T4** forum or issue discussion. This research did not read emulator implementation source. The PAL hardware pages below are mostly T2 and do not become hardware proof merely because the project has pinned a prior wiki revision. Web findings were cross-checked against the project’s curated notes; the research confidence seam classifies those verified web findings as MEDIUM. A low-confidence statement below is an evidence limit, not an authoritative hardware conclusion.

Project scope is authoritative about what v8 must do. In particular, the product policy is: resolve unambiguous single-region NES 2.0 declarations; default ambiguous and legacy content to NTSC; permit host selection for multi-region and legacy iNES inputs; reject Dendy. This is a project decision, distinct from claims about what hardware does.

## Established Facts and Evidence Levels

| Finding | Evidence and source tier | Confidence and limit |
|---|---|---|
| Official PAL timing uses a 26.601712 MHz master clock, a CPU divider of 16, five master clocks per PPU dot, and 106,392 dots per frame. The project derives the exact frame rate as `322445/6448` Hz (about 50.007 Hz). | Curated [CPU/APU notes, §2.1 and §6](../../preparation/NES-HARDWARE-CPU-APU.md) cite HWC.03/HWC.04 (T2); [PPU/cartridge notes, §§4 and 8](../../preparation/NES-HARDWARE-PPU-CARTRIDGE.md) cite HWP.10 (T2). Current [CPU variants](https://www.nesdev.org/wiki/CPU_variants) and [cycle reference](https://www.nesdev.org/wiki/Cycle_reference_chart) pages were checked 2026-11-01. | MEDIUM for the documented model. `322445/6448` is derived in project notes; preserve that exact fraction and its derivation, not just the rounded decimal. |
| PAL APU behavior includes PAL-specific frame-counter intervals and separate noise and DMC period tables. Project notes list frame-counter events at 4156, 8313, 12469, and 16626 CPU cycles; the live [noise](https://www.nesdev.org/wiki/APU_Noise) and [DMC](https://www.nesdev.org/wiki/APU_DMC) references expose distinct PAL tables. | HWC.08-HWC.10, T2, in [CPU/APU notes, §§4 and 6](../../preparation/NES-HARDWARE-CPU-APU.md); current NESdev APU pages checked 2026-11-01. | MEDIUM for table provenance, not a claim that every PAL CPU revision or clone shares the official RP2A07 tables. Keep timer and frame-sequencer phase semantics with the tables. |
| The NES 2.0 timing field distinguishes NTSC (`0`), licensed PAL (`1`), multiple-region (`2`), and Dendy (`3`). Multiple-region describes software intended to adapt between NTSC and PAL/Dendy timing; it does not pick the player’s preferred hardware. | [NES 2.0 specification](https://www.nesdev.org/wiki/NES_2.0), current page checked 2026-11-01; HWP.14 (T2) in [PPU/cartridge notes](../../preparation/NES-HARDWARE-PPU-CARTRIDGE.md). | MEDIUM for the encoding. Product precedence and Dendy refusal come from the project scope, not the format page. |
| NESdev describes 2A07 as avoiding NTSC DMC extra-read behavior by attempting to halt on the first CPU instruction cycle, but says the mechanism is not understood. Its DMA page also records experimental observations of a read during the halt cycle and the following opcode appearing on open bus in a particular OAM-DMA case. | Current [DMA](https://www.nesdev.org/wiki/DMA), [DPCM conflict](https://www.nesdev.org/wiki/DPCM_conflict), and [CPU variants](https://www.nesdev.org/wiki/CPU_variants) pages (T2), checked 2026-11-01; HWC.01/HWC.10 (T2) in [CPU/APU notes, §§2.2 and 6](../../preparation/NES-HARDWARE-CPU-APU.md). | MEDIUM that this is the published community model and that its evidence is explicitly incomplete; LOW that the proposed first-cycle rule is the exact physical mechanism. The preparation notes explicitly preserve the conflict and call for a PAL hardware test. |
| The 2C07 produces composite PAL video. The NESdev 2C07 palette page describes a table generated with pally v0.24.1 and specific modeling parameters; PAL also swaps the red/green emphasis interpretation noted in the project preparation. | [PPU palettes](https://www.nesdev.org/wiki/PPU_palettes), checked 2026-11-01; HWP.07/HWP.08 (T2) in [PPU/cartridge notes, §4](../../preparation/NES-HARDWARE-PPU-CARTRIDGE.md). The preparation file also identifies a decap write-up as T1, but says only its clock material and README were read; it is not evidence for exact palette values here. | MEDIUM for reproducing that named model; LOW for any claim that it is a unique, measured, or universally accurate analog palette. |
| `retro_get_system_av_info()` may depend on loaded content and is queried after a valid load; accurate FPS and audio sample-rate metadata matter for synchronization. Libretro calls `retro_run()` once per video frame. | Official [libretro core development documentation](https://docs.libretro.com/development/cores/developing-cores/), T1, checked 2026-11-01; [libretro preparation notes, §§1-2](../../preparation/LIBRETRO-AND-RUNNER.md) cite the pinned header for `retro_get_region()` values (0 NTSC, 1 PAL). | MEDIUM under the provider confidence tier, with a primary interface source. This establishes the host contract, not the correctness of PAL clock constants. |

## Critical Pitfalls

### 1. Mixing PAL cadence with NTSC clocks, or rounding the rate

**What goes wrong:** A PAL frame is treated as exactly 20 ms or its rate is rounded to 50 Hz; CPU/PPU divider and dot counts are mixed with NTSC values; or audio generation is still driven by NTSC cycles. Games then run at the wrong speed, frame and sample totals drift, and notes or DMC samples change pitch.

**Why it happens:** The visible “PAL is 50 Hz” shorthand hides a 50.007 Hz frame rate and distinct CPU/APU clocks. A core can draw the correct number of lines while still advancing CPU timers at the wrong rate.

**Prevention:** Keep region clock/divider/frame values as integer or rational constants in the instance profile. Preserve `322445/6448` as the frame-rate fraction at the native API boundary; use integer phase/remainder accounting for samples and timer schedules. Generate PAL PCM at the existing output sample rate while advancing APU units at PAL clocks. Do not add a floating-point clock accumulator to the deterministic core.

**Acceptance:** Check one PAL frame’s elapsed ticks and frame count; verify long-run frame and sample totals against the exact fractions without cumulative drift; measure representative pulse/triangle/DMC pitch in output samples; and verify the libretro FPS is the fraction converted only at the adapter boundary. Compare the same checks on multiple platforms.

### 2. Updating the CPU clock but retaining NTSC APU tables or phase rules

**What goes wrong:** Pulse pitch shifts with the PAL CPU rate, but noise and DMC pitches do not match 2A07 behavior; frame-counter clocks occur at NTSC intervals; or a shared table edit changes NTSC hashes.

**Why it happens:** Timer tables and the frame sequencer are easy to mistake for a consequence of the master clock. The reference notes document explicit PAL noise/DMC tables and PAL frame-counter steps, so scaling only the master clock is incomplete.

**Prevention:** Make every region-dependent table and interval explicit and selected from the resolved region. Preserve the current event ordering, get/put phase, delayed `$4017` behavior, and interrupt semantics when adapting the PAL frame-counter schedule. Do not “correct” the PAL DMC table’s documented tuning irregularities by substituting NTSC values or an idealized note table.

**Acceptance:** Add table-level assertions for all PAL noise and DMC indices, frame-counter boundary/race fixtures for each supported mode, pitch checks from rendered samples, and wrong-table controls that fail. Treat the existing NTSC frame/audio hash inventory as a protected baseline on every platform; do not regenerate expected hashes to accept timing drift. Keep the NTSC tables unchanged unless a separate, justified NTSC correction is in scope.

### 3. Claiming the 2A07 DMA rule as proven hardware behavior

**What goes wrong:** The implementation or README states that PAL DMA is fully understood or universally accurate, and the selected first-instruction-cycle halt rule is treated as settled. The project then hides whether it follows a documented model or a physical measurement; a wrong halt edge can still corrupt observable reads or open bus.

**Why it happens:** NESdev describes a user-visible fix for NTSC extra reads, while its DMA page separately says the mechanism is unknown and lists experimental observations. The curated notes preserve this as an open question, including conflict between sources.

**Prevention:** Implement only the best source-backed candidate chosen by the phase, label the exact modeled behavior and evidence tier, and retain the hardware-proof limit in documentation. Keep the distinction between the halt-cycle read, repeated/parked reads, register side effects, low-address decode, and open-bus value. Do not infer that all NTSC DMA rules disappear on PAL just because one extra-read problem is fixed.

**Acceptance:** Use a small owned bus fixture that observes the halt address/data, side effects, controller reads, and OAM-DMA `$4000`-page open bus where relevant. Include a negative control for the competing halt edge/model. Require deterministic results and continuation after the transfer. Report this as conformance to the selected documented model, not proof against physical silicon. Hardware capture or a suitably scoped physical test is required to raise confidence.

### 4. Letting metadata and overrides select different regions in different deliverables

**What goes wrong:** Library, runner, and libretro resolve one cartridge differently; an override silently replaces an explicit single-region declaration; “multiple” is treated as NTSC-only or PAL-only without host choice; legacy iNES behavior varies by frontend; or unsupported Dendy content falls through to PAL.

**Why it happens:** The NES 2.0 field describes intended timing, while a multi-region image deliberately has no single preferred system. Legacy iNES lacks equivalent certainty. A CLI flag, core option, or public API can accidentally create separate precedence logic.

**Prevention:** Define one resolver and a precedence table before implementation: single-region NES 2.0 values select their declared profile; multi-region and legacy/ambiguous inputs default to NTSC and may use a host override; Dendy is refused. Keep malformed/reserved declarations explicit rather than silently treating them as a valid PAL request. Resolve the profile at content load and make all three deliverables use the same decision. Do not infer region from filename or a bundled ROM database.

**Acceptance:** Test all four NES 2.0 timing values, legacy iNES, ambiguous/unknown input, each allowed override, disallowed override of explicit single-region metadata, and Dendy refusal. Run the cases through library, runner, and adapter; assert the same resolved region, AV metadata, and failure status. Ensure failed load or refusal leaves any prior live instance intact.

### 5. Adding PAL behavior without making region part of state identity

**What goes wrong:** A state saved under PAL is accepted by an NTSC instance, or a refused cross-region load partially changes CPU/APU/PPU state. Rewind, replay, or native continuation can then diverge even when the ROM hash matches.

**Why it happens:** Cartridge identity alone does not identify machine timing. The project already versions ABI, state format, behavior revision, and battery-RAM layout separately; a version bump is not a substitute for region validation. The stack notes a region byte in the native state format, but its current semantics must be confirmed before relying on it.

**Prevention:** Bind the resolved region into machine-state identity and validate it alongside cartridge identity before applying any field. Keep canonical byte-wise encoding and fixed state size. Bump state format only if serialized layout/meaning changes; use behavior revision for its documented purpose and do not turn it into an implicit loader gate. If the region byte already exists, verify it is populated and checked for both profiles.

**Acceptance:** For both profiles, save and resume to identical future frame/audio hashes. Attempt a PAL-to-NTSC and NTSC-to-PAL load; require refusal and prove atomicity by comparing the complete pre/post state bytes and subsequent output. Also cover malformed region bytes, wrong cartridge identity, and the existing state compatibility contract.

## Moderate Pitfalls

### 6. Treating the modeled 2C07 palette as exact color truth

**What goes wrong:** A deterministic generated RGB table is advertised as the exact 2C07 analog output, or PAL output is produced with NTSC emphasis ordering. The project’s intended caveat disappears and palette changes unexpectedly affect frame hashes.

**Prevention:** Call it a modeled 2C07 palette and name the source/model parameters. Preserve raw native palette index and emphasis in the core’s frame representation; perform RGB conversion at the existing palette boundary. Use the PAL model’s red/green emphasis mapping. Keep the NTSC table byte-identical and document that composite PAL decoding, display, and hardware variation are outside this table’s claim.

**Acceptance:** Check all 64 palette indices across the 8 emphasis combinations through the palette API, plus greyscale/backdrop handling. Assert region selects the intended table, repeated lookups are identical, raw native frame hashes remain independent of RGB conversion, and all NTSC palette results remain unchanged.

### 7. Publishing stale or rounded AV metadata to a host

**What goes wrong:** The core runs PAL timing but reports NTSC FPS, rounds to 50.0, reports stale timing after loading different content, or returns the wrong `retro_get_region()` value. Hosts can then pace frames/audio incorrectly even when internal hashes are deterministic.

**Prevention:** Derive native AV rates from the single resolved profile. Let the library expose exact numerator/denominator values and convert to libretro’s floating fields only in the adapter. Return PAL region `1` and its measured model rate after successful PAL load, NTSC region `0` and existing NTSC metadata otherwise. Keep the audio output sample rate at the documented native contract and avoid metadata changes that are not reflected in core timing.

**Acceptance:** Query AV metadata and region after NTSC, PAL, override, unload, failed load, and subsequent content load. Check exact native fractions, expected host doubles within a defined conversion tolerance, stable output sample rate, and one video callback per `retro_run()`.

### 8. Proving only NROM or only the runner path

**What goes wrong:** PAL works for the easiest board but mapper IRQ/banking behavior or adapter metadata diverges on MMC1, UxROM, CNROM, MMC3, or AxROM. A test ROM that boots on NROM does not establish coverage of the shared six-board surface.

**Prevention:** Reuse owned in-memory C fixtures for mapper families 0, 1, 2, 3, 4, and 7. Exercise shared region selection and timing through the library and runner, and verify adapter parity for the same frames, samples, and state spans. Keep this as regression coverage for existing boards; do not add boards or third-party emulator code.

**Acceptance:** Every supported mapper has at least a PAL load/execute/state case and an NTSC control. MMC3’s region-sensitive scanline/CPU synchronization receives an explicit observable fixture. A mapper failure must identify the resolved profile and not be hidden by the global NTSC hashes.

### 9. Committing ROM bytes or test data without clear redistribution rights

**What goes wrong:** PAL-specific test coverage introduces a copyrighted commercial image, an unlicensed public test ROM, or a fixture whose source pin/license differs from the repository manifest.

**Prevention:** Prefer small owned C-generated bus, metadata, and mapper fixtures. If a ROM file is needed, require an explicit permissive license, exact source pin, and entry in `tests/roms/manifest.txt`; otherwise follow the project’s pinned-fetch/checksum policy for tests that are not redistributable. Never include commercial ROM bytes or rely on “widely used” as a license.

**Acceptance:** Hygiene passes; every added ROM byte is listed in the manifest with verified rights; generated fixtures are deterministic and contain no saved state, BIOS, or private material.

### 10. Expanding PAL into a consumer or emulator-port project

**What goes wrong:** Scope absorbs Playstead/libfuturo integration, a host UI/audio device, Dendy, new mapper families, or behavior without a confirmed core diagnosis. The region feature stops being reviewable against its current goal.

**Prevention:** Keep the change inside the C library, headless runner, and libretro adapter. No new emulator dependency or source, no device code, no Dendy profile, no new boards, and no consumer integration. Preserve the clean-room rule; public hardware descriptions and owned tests are the design basis.

**Acceptance:** Review changed files against the approved v8 scope; no new vendored files beyond the existing `libretro.h`, no new package dependency, and no changes in Playstead/libfuturo or unrelated host repositories.

## Phase-Specific Acceptance Map

| Phase topic | Main pitfall | Minimum prevention/evidence |
|---|---|---|
| Region resolution | Resolver drift or override precedence ambiguity | One shared precedence table; library/runner/libretro matrix; Dendy refusal and failed-load atomicity |
| CPU/PPU/APU timebase | Rounded cadence, mixed clock domains, wrong APU tables | Integer/rational profile; PAL frame/timer/table fixtures; pitch and long-run sample-count checks |
| 2A07 DMA | Provisional wiki model presented as measured truth | Owned bus fixture plus competing-model control; report T2 evidence and unresolved physical edge |
| 2C07 palette/API | Modeled RGB claimed as canonical; NTSC palette/hash changes | 512 lookup checks, explicit emphasis behavior/caveat, unchanged NTSC table and native pixel hashes |
| Mapper regression | NROM-only proof | PAL and NTSC controls across mapper IDs 0, 1, 2, 3, 4, 7; explicit MMC3 timing fixture |
| State compatibility | Cross-region load mutates instance or format version is misused | Same-region future continuation; cross-region rejection with full pre/post identity and output equality |
| Host metadata | Stale or rounded AV information | Check native fractions, adapter conversion, region enum, content-load lifecycle, callback counts |
| Tests and docs | Rights or clean-room boundary violation | Owned generated fixtures or manifest-approved ROMs; update README and public API comments with behavior |

## Open Unknowns to Carry Into Phase Research

- **2A07 DMA halt edge:** the curated sources explicitly disagree or stop short of a mechanism. The accepted implementation may follow the best documented candidate, but physical correctness remains unproven until PAL hardware evidence exists. Do not expand that investigation into unrelated DMA behavior.
- **2C07 color reference:** the table’s source is a named simulation/model recipe, not an authoritative measured RGB output for every display path. The requested modeled table is feasible; stronger color-accuracy claims need a measurement-backed reference and a separately approved scope.
- **Existing state region-byte semantics:** STACK.md reports a region byte, but the phase must establish whether it is already part of load identity and whether state layout/meaning changes. Do not assume its presence alone guarantees cross-region refusal.
- **Override activation lifecycle:** the project fixes which inputs may be overridden, while the interface evidence says AV metadata is content-dependent. Decide whether a libretro option is latched at content load or can be changed live; if live change is supported, define state/replay identity and the host metadata update path before implementing it. Do not silently swap timing while retaining stale AV information.
- **PAL test ROM availability:** the preparation references establish licensing rules, but this research did not identify a required PAL ROM with both rights-cleared bytes and an independent hardware oracle. Owned synthetic fixtures should cover the core contract unless a qualifying fixture is found.

## Sources

- Project goal and boundaries: [PROJECT.md](../../PROJECT.md), direct project authority.
- Existing recommendations: [STACK.md](STACK.md), especially region precedence, exact frame-rate fraction, state identity, palette caveat, six-board coverage, and 2A07 evidence limit.
- Hardware source map and pinned claims: [NES-HARDWARE-CPU-APU.md](../../preparation/NES-HARDWARE-CPU-APU.md), HWC.01, .03-.04, .08-.10, .13; NESdev Wiki, T2, revisions recorded in that file (accessed there 2026-10-02).
- PPU/frame-rate/format/palette claims: [NES-HARDWARE-PPU-CARTRIDGE.md](../../preparation/NES-HARDWARE-PPU-CARTRIDGE.md), HWP.07, .08, .10, .14; NESdev Wiki, T2, revisions recorded in that file (accessed there 2026-10-02).
- Current community references checked 2026-11-01: [DMA](https://www.nesdev.org/wiki/DMA), [DPCM conflict](https://www.nesdev.org/wiki/DPCM_conflict), [CPU variants](https://www.nesdev.org/wiki/CPU_variants), [APU noise](https://www.nesdev.org/wiki/APU_Noise), [APU DMC](https://www.nesdev.org/wiki/APU_DMC), [NES 2.0](https://www.nesdev.org/wiki/NES_2.0), and [PPU palettes](https://www.nesdev.org/wiki/PPU_palettes). These support community documentation, not direct hardware proof.
- Host interface: official [libretro core development documentation](https://docs.libretro.com/development/cores/developing-cores/), T1, checked 2026-11-01; local [LIBRETRO-AND-RUNNER.md](../../preparation/LIBRETRO-AND-RUNNER.md), LR.01 and LR.06.
- Rights-cleared fixtures: [CONFORMANCE.md](../../preparation/CONFORMANCE.md), especially “ROMs that can be committed or released”; [DECISIONS.md](../../preparation/DECISIONS.md), DEC.20-21.
