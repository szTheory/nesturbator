# Phase 2: The CPU matches the public vectors - Research

**Researched:** 2026-10-03
**Domain:** 2A03 (6502 without decimal) instruction-level CPU in C17; binary test-vector pipeline; CTest/CMake/GitHub Actions nightly
**Confidence:** HIGH

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

Everything in `.planning/preparation/DECISIONS.md` (DEC.01–DEC.39) and Phase 1's `01-CONTEXT.md` (D-01–D-20) stays settled. In particular: instruction-level CPU, straight-line code per instruction, one bus call per cycle with dummy accesses (DEC.09); ANE/LXA constant is a machine-profile field (DEC.18); sample = first 100 tests per opcode, full set never committed (DEC.20, DEC.21); labels `vectors` / `vectors-full`, `nightly.yml` outside `CI required` (ENGINEERING §3, §5).

#### Vector converter and sample
- **D-01:** An owned C host tool `tools/vecconv/vecconv.c` (palgen-style: built by CMake in every preset, not installed, not linked into the core, integer-only) converts upstream JSON to N65V. Usage `vecconv <in.json> <out.n65v> <opcode> [--first N]`; with `--first N` it stops after N objects and accepts a truncated tail (a range prefix); without it the file must close with `]` and hold exactly 10,000 tests. After writing it re-reads its output through the shared reader. CMake `string(JSON)` is rejected (quadratic re-parse, cannot emit NUL bytes); no Python.
- **D-02:** The JSON tokenizer is schema-specific and layout-independent: any JSON whitespace, keys matched by name, unsigned decimal integers only; exit 1 with a byte offset on a float, sign, exponent, out-of-range value (>65535 / >255), count >255, unknown kind, unknown or missing key. Measured at the pin: each file is a JSON array (not JSON Lines); 25 files (02 0c 12 1c 22 32 3c 42 52 5c 62 72 6b 7c 92 93 9b 9c 9e 9f b2 d2 dc f2 fc) use a compact layout (`[{`, `[[`); the 100th object ends at byte 54,565 at most (CONFORMANCE's 54,433 came from 18 files — correct it in the same change); `name` is not unique and is dropped.
- **D-03:** N65V v1 keeps the CONFORMANCE layout with these rules fixed: little-endian, decoded byte by byte (never memcpy into a struct); version byte 1, any other rejected; per-test index u16, strictly increasing (failures name the upstream test as `a9.json[i]`); kind byte 0 = read, 1 = write, other rejected; counts u8, header count u16; no in-file checksum. — **Reversibility:** costly — the committed blob, the reader, the converter and the nightly all depend on the layout; a change means regenerating and re-manifesting the blob.
- **D-04:** The committed sample is **one file** `tests/vectors/65x02-sample.n65v` (256 per-opcode chunks concatenated in opcode order, 100 tests each; 1,677,602 B measured), with one line in `tests/roms/manifest.txt` (path, `https://github.com/SingleStepTests/65x02`, the pin, `MIT`, sha256). `.gitattributes` marks `*.n65v binary` and the JSON fixtures `eol=lf`. `THIRD-PARTY-NOTICES.md` gains the upstream MIT notice verbatim from the pinned `LICENSE` and its "one file comes from elsewhere" sentence is corrected; `PROVENANCE.md` lists the blob.
- **D-05:** The reader `tests/vectors/n65v.{c,h}` works on a buffer and length, checks bounds before every field (`n > len - off`, never `off + n`), takes the expected chunk count, opcodes and tests per chunk (256×100 for the sample, 1×10,000 per nightly file) and requires the file to end exactly after the last chunk. One reader and one comparison loop serve both tiers.
- **D-06:** `ci` tests for the converter and reader: crafted buffers through `check.h` (truncated at every field boundary, bad magic, version 2, kind 2, opcode mismatch, short count, trailing bytes); small committed JSON fixtures (first 3 tests of `a9` spaced layout and `02` compact layout, one truncated tail, negative cases that must exit 1) converted with `--first 3` and run through the vector program; each fixture carries an MIT attribution comment.
- **D-07:** A pin bump regenerates the sample locally from 256 × 64 KiB HTTP range prefixes (`file(DOWNLOAD … RANGE_START/RANGE_END)`, CMake ≥ 3.24), about 16 MB, writing the blob and printing the manifest line. A pin bump is its own change (CONFORMANCE).

#### CPU–bus seam
- **D-08:** **Link-time seam.** `src/cpu.c` references exactly two external symbols, declared in `src/internal.h`: `uint8_t nesturbator__bus_read(struct nesturbator *nes, uint16_t addr)` and `void nesturbator__bus_write(struct nesturbator *nes, uint16_t addr, uint8_t value)`. `src/bus.c` defines them for the library; `tests/cpu/vector_bus.c` defines them for the vector test (flat 64 KiB RAM plus a fixed-capacity cycle log; overflow fails the test). No function pointers in the instance. — **Reversibility:** costly — Phase 3's bus, DMA and PPU catch-up are built behind this seam; moving to a run-time seam touches every bus call site and the save-state design.
- **D-09:** CMake: `add_library(nesturbator_cpu OBJECT src/cpu.c)` with the core's include dirs, warnings, `C_VISIBILITY_PRESET hidden`, `POSITION_INDEPENDENT_CODE ON` and the nofp flags (one helper applies the core flags to both targets); folded into the library with `target_sources(nesturbator PRIVATE $<TARGET_OBJECTS:nesturbator_cpu>)` — not `target_link_libraries`, which breaks `install(EXPORT)`. The test is `add_executable(cpu.vectors tests/cpu/test_vectors.c tests/cpu/vector_bus.c tests/vectors/n65v.c $<TARGET_OBJECTS:nesturbator_cpu>)` with private `src`/`include` includes and never links `nesturbator::nesturbator` (duplicate bus symbols). The tested object is the shipped object.
- **D-10:** CPU state `nesturbator__cpu { uint16_t pc; uint8_t a, x, y, s, p; uint8_t nmi_prev, nmi_pending, irq_line, poll_latch; uint8_t halted_in_read; uint8_t jammed; }` is the `cpu` member of `struct nesturbator`. Interrupt fields exist now, unused until Phase 3. Bus-side state (RAM, open-bus latch, DMA) goes on the bus side of the instance, not in the CPU struct. Code accesses `nes->cpu.*` directly; the bus never reads CPU registers other than `halted_in_read` and the interrupt fields.
- **D-11:** Internal hook `void nesturbator__cpu_step(struct nesturbator *nes)` runs exactly one instruction, opcode fetch to last cycle. The vector test zero-initialises a `struct vector_machine { struct nesturbator nes; uint8_t ram[65536]; log…; }` (first-member conversion, C17 6.7.2.1p15), sets `nes.cpu.*` and RAM, calls `cpu_step` once, then compares registers, RAM and every logged cycle. It does not call `nesturbator_create`.
- **D-12:** `src/bus.c` has its final shape in Phase 2: per call, advance `nes->ticks` 9 to M2 rise, empty PPU catch-up, 15 to M2 fall, empty catch-up, then the access (NTSC M2 high 15/24, NES-HARDWARE-CPU-APU §2.1). Phase 2 decode: 2 KiB RAM mirrored over `$0000–$1FFF`, open-bus latch elsewhere. Interrupt sampling will live in the bus call during phi2; `cpu.c` only reads the latch at poll points. `bus.c` lands in the same change as `cpu.c` (else `abi.undefined_symbols` fails). `run_frame` keeps producing the test card; the CPU does not drive frames until Phase 3.

#### JAM and unstable opcodes (checked against the pinned data)
- **D-13:** JAM (`$02 $12 $22 $32 $42 $52 $62 $72 $92 $B2 $D2 $F2`): exactly 11 reads — opcode at PC, PC+1 (discarded), then `$FFFF $FFFE $FFFE $FFFF $FFFF $FFFF $FFFF $FFFF $FFFF`; PC ends at PC+1 (16-bit wrap); no write, no register or flag change; sets `jammed`. While jammed each `cpu_step` is one read of `$FFFF`, NMI and IRQ are not taken, only reset or power clears it; the flag is instance state (save states include it later). The vector harness compares the opcode's own 11 cycles and never relies on the jammed loop. Cite NESdev Wiki "CPU unofficial opcodes" rev 23975 and 65x02 commit b7ed828 / issue #1.
- **D-14:** Two profile fields `ane_magic` and `lxa_magic`, both `$EE` for NTSC RP2A03G. ANE: `A = (A | ane_magic) & X & imm`; LXA: `A = X = (A | lxa_magic) & imm`; both set N, Z. The vector test sets both to `$EE` itself; a unit test pins the profile defaults. The profile lives in the instance; no public option this phase.
- **D-15:** SHY/SHX/SHA (`$9F`, `$93`)/TAS: `value = reg & (H + 1)` with reg = Y, X, or A & X (H for `$93` is the zero-page pointer's high byte); target `((cross ? value : H) << 8) | ((L + idx) & $FF)`; TAS first sets `S = A & X`. The `& (H+1)` mask sits in one helper so the RDY ("halted in this read") case is one condition later. No RDY branch now.
- **D-16:** Decimal: D is stored and restored (SED, CLD, PHP, PLP, RTI; BRK does not clear it) but ADC/SBC are always binary — hardware comment cites the nes6502 README. P held with bit 5 = 1, bit 4 = 0; PHP and BRK push `P | $30`; PLP and RTI load `(v & ~$10) | $20`; later IRQ/NMI pushes use `(P & ~$10) | $20`.
- **D-17:** JSR order: read ADL, dummy stack read, push PCH, push PCL, read ADH (upstream issue #18); the sample must keep a test where code overlaps the stack page, or a unit test covers it.
- **D-18:** Match the pinned vectors with no exclusion list; documented chip differences go through profile fields only. A waiver list (test name + cited source, read as `unsupported`) is added only if a vector is shown wrong. None is known at the pin.

#### Nightly full-set run
- **D-19:** Fetch by git, the platform tool: into `<dir>/src`, `git init`, `git sparse-checkout set --cone nes6502/v1`, `git -c gc.auto=0 fetch --depth 1 --filter=blob:none origin <sha>`, `git checkout FETCH_HEAD`, with `GIT_HTTP_LOW_SPEED_LIMIT=1000` and `GIT_HTTP_LOW_SPEED_TIME=60`; then every file is checked with `file(SHA256)` against the committed list, `.git` is deleted, and any mismatch or missing file fails. Measured: about 193 MB transferred, 89 s. Upstream has no Git LFS. Never run size/log/diff commands in the blobless tree (each lazily fetches blobs). Script `tests/cmake/fetch_vectors.cmake` (`cmake -P`), run at test time as a fixture, never at configure time; it does nothing when all 256 files already verify.
- **D-20:** `tests/vectors/pins.txt` (text): line 1 `repo … commit … path nes6502/v1 licence MIT`, then 256 sorted lines `<sha256>  nes6502/v1/xx.json  <size>` in sha256sum layout. An offline `ci` test checks 256 lines, sorted, sizes summing to 1,081,529,097, and the commit equal to the sample's manifest pin.
- **D-21:** **No cache** in the nightly: a run every night keeps a cache warm forever, so "fetched at its pin" would stop being tested. This revises ENGINEERING §5 ("nightly vector cache keyed by upstream commit"); update it in the same change. Never cache the raw JSON. Revisit only if a measured cold run exceeds about 10 minutes (then cache the N65V output keyed on `pins.txt` and the converter sources).
- **D-22:** CTest: option `NESTURBATOR_VECTORS_FULL` (default OFF; `vectors-full` tests registered only when ON, so `ci` never touches the network) and cache variable `NESTURBATOR_VECTORS_DIR` (default `${CMAKE_BINARY_DIR}/vectors-full`; the script refuses a path inside the source tree outside `build/`). Tests: `cpu.vectors-full.fetch` (`FIXTURES_SETUP vectors_full`); `cpu.vectors-full.sample-match` (first 100 tests of each converted file byte-equal the committed sample — the provenance proof); `cpu.vectors-full.00`–`.ff` (`FIXTURES_REQUIRED`), each reporting its failing-vector count. Offline fails; never return code 77 or `SKIP_RETURN_CODE` for this label.
- **D-23:** Workflow preset `vectors-full` (configure inherits `ci` plus `NESTURBATOR_VECTORS_FULL=ON`; test preset filters label `vectors-full`, `noTestsAction: error`) — DEC.28's single entrypoint, locally and in CI. The owner may point `NESTURBATOR_VECTORS_DIR` at a user cache dir through the git-ignored `CMakeUserPresets.json`; no tracked file carries such a path.
- **D-24:** `.github/workflows/nightly.yml`: triggers `schedule` (`17 4 * * *`), `workflow_dispatch`, and `pull_request` path-filtered to `nightly.yml`, `tests/vectors/**`, `tests/cmake/fetch_vectors.cmake` (safe: outside `CI required`; it is how the phase branch proves the workflow, since `schedule` runs only on the default branch). `permissions: {}` at top; job `vectors-full` on `ubuntu-24.04` only, `contents: read`, checkout at the `ci.yml` SHA pin with `persist-credentials: false`, `cmake --workflow --preset vectors-full`, `timeout-minutes` twice a measured cold run with the run recorded in a comment. Job `report` (`needs`, `if: always() && github.event_name == 'schedule'`, `issues: write`, no checkout) uses `gh` to open or edit one rolling issue labelled `nightly` with the run URL and failing `65x02/xx` keys, and closes it on success. No commits from the nightly. Every `uses:` SHA-pinned (covered by `action_pins.cmake`); no secrets.

### Claude's Discretion
- Opcode dispatch shape inside `cpu.c` (one switch, per-addressing-mode helpers, macros) within D-08 and "straight-line code per instruction"; file split if `cpu.c` grows large.
- Exact failure-report format of the vector program (first mismatching field, cycle diff), as long as it names `xx.json[i]`.
- Whether the scoreboard file (DEC.22) is created now with `65x02/xx` keys or in Phase 3 with AccuracyCoin; the phase bar is that every vector passes.
- Profile struct layout and where the RP2A03G default is defined.
- README and public-header comment wording for the CPU landing (rule 6), and the Conventional Commit title.

### Deferred Ideas (OUT OF SCOPE)
- Jam surfaced to hosts as a status bit or stop reason; runner exits with its own code on a jam — Phase 3 (run API).
- Run instr_test-v5 `03-immediate` to settle LXA `$EE` vs `$FF`; change only `lxa_magic` — Phase 3.
- SHx RDY branch and AccuracyCoin unofficial-opcode RDY sub-tests (codes 7–C) — the phase that adds DMC DMA.
- Ticks overshooting a frame once the CPU drives `run_frame` (instruction boundaries) — plan the carry and runner hash updates in Phase 3.
- Caching converted N65V in the nightly — only if a cold run exceeds about 10 minutes.
- Adding `macos-15` to the nightly — only if a nightly ever disagrees with a local Mac run.
- Scheduled workflows are disabled after 60 days of repository inactivity — note for quiet periods between milestones.
- libFuzzer target for the N65V reader — optional, later (inputs are committed or self-generated).
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| CPU-01 | `ctest -L vectors` runs the committed sample of the 65x02 vectors through the CPU, and every opcode matches on final state and on every bus cycle. | Per-cycle table below matched all 25,600 sample tests in a scratch model; P-bit finding (Pitfall 1); expected blob size and SHA-256 (Code Examples); CMake wiring (Pattern 2, Pitfalls 4–7); hygiene and manifest fit (Pitfall 9). |
| CPU-02 | `ctest -L vectors-full` runs the full 65x02 vector set, fetched at its pinned commit, with every test matching; CI runs it nightly. | Pin is still upstream HEAD; 256 files, 1,081,529,097 B checked against the git tree; fetch recipe tested (needs `git remote add`, Pitfall 10); 380,000 full-file tests in 38 files matched the same table; label permission and `release-as` findings (Pitfalls 11, 12). |
</phase_requirements>

## Summary

The decisions in CONTEXT.md hold up against the pinned data, with two corrections and one missing prerequisite. I wrote a throwaway Python model of the per-cycle behaviour in the scratchpad. It used only 64doc and the D-13 to D-17 rules, and nothing goes into the repo. It matched **all 25,600 sample tests** (the first 100 of every opcode) and **all 380,000 tests in 38 full files** (official, unofficial, JAM, unstable, branches, RMW, JSR, RTS, RTI, BRK, PHP and PLP) on registers, RAM and every bus cycle. So the per-cycle table in this document is proven against the vectors. The C implementation can copy it one-for-one. [VERIFIED: scratch model run against `nes6502/v1` at 2f6980a2]

**Correction 1 (P register, high impact).** In 25 files, every test starts with P bits 4 and 5 **both set** (`p & 0x30 == 0x30`) and expects them unchanged at the end. These are the 12 JAM files plus `0c 1c 3c 5c 6b 7c 93 9b 9c 9e 9f dc fc`. In the other 231 files, bit 5 is always set and bit 4 always clear. D-16's "P held with bit 5 = 1, bit 4 = 0" stays true for the core's own state. But the CPU must **never re-normalise P** on entry or when it updates flags. Each flag write may only touch its own bits (N V D I Z C). Only PLP and RTI write bits 4 and 5. A CPU that rebuilds P from separate flags, or forces `| 0x20 & ~0x10` each step, fails those 25 files. That includes ARR (`6b`), which changes flags. [VERIFIED: analysis of 256 range prefixes and full `02 93 9b 9c 9e 9f`]

**Correction 2 (sample blob size).** With the D-03 layout (8-byte chunk header: `N65V`, version, opcode, u16 count), the sample is **1,678,114 B**, not 1,677,602 B. The difference is exactly 256 × 2 B. My scratch encoder gives SHA-256 `0c318cec0bd4031396964ca9a0de0daf25c2651c28f34a0d0616a9294430e109`. It is an independent cross-check for vecconv's output. **Corrections to D-17:** no test in the full `nes6502/v1/20.json` has the stack overlapping the ADH operand. All 41 JSR tests with code in page 1 come after index 100. So the overlap case **must be a unit test**, using the issue #18 case (from `6502/v1/20.json`). **Prerequisite:** `release-please-config.json:10` still holds `"release-as": "0.1.0"`, and STATE.md lists removing it as Phase 2's first task. Unless it is removed, success criterion 3 (merging publishes a release) gives 0.1.0 again.

**Primary recommendation:** Build `cpu.c` as one `switch (opcode)`. Each case calls small `static` addressing-mode helpers that return the effective address after doing their exact dummy accesses, plus `static` operation helpers that touch only their own P bits. Do not use a function-pointer opcode table: under PIC it lands in `.data.rel.ro`, which `abi.global_symbols` rejects.

## Project Constraints (from CLAUDE.md)

- Own code only; `libretro.h` is the only vendored file; core links only C memory functions (allowlist in `tests/cmake/undefined_symbols.cmake:18-22`: `memcpy memmove memset memcmp` `malloc free` `__memcpy_chk __memmove_chk __memset_chk` `__stack_chk_fail __stack_chk_guard`, plus `bzero` on Apple and `_GLOBAL_OFFSET_TABLE_`). [VERIFIED: tests/cmake/undefined_symbols.cmake:18-32]
- Three deliverables only; no window/audio/input code.
- Clean tree: no ROM/BIOS bytes except files listed in `tests/roms/manifest.txt`; no personal paths, emails, names; commits use the GitHub noreply identity; `scripts/hygiene.sh` runs on commit and push.
- Clean room: never open GPL/LGPL emulator source. The vectors are MIT test data, and their README and LICENSE are fine to read.
- Deterministic, integer-only core; all state in the instance; no mutable statics/globals (checked by `abi.global_symbols`).
- Every behaviour change lands with a test in `cmake --workflow --preset ci`, and the README and public-header comments are updated in the same change. Nothing waits on a person.
- One GSD step per command; phase branch; PR with a Conventional Commit title.
- Style: small modules, plain control flow, fixed-width types; hardware comments cite sources; ponytail ladder (remove, reuse, stdlib, platform, then minimum new code).
- C17, extensions off; warnings `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Wstrict-prototypes -Wmissing-prototypes -Wvla` (GCC adds `-Wduplicated-cond -Wlogical-op`; MSVC `/W4`), fatal in presets. [VERIFIED: CMakeLists.txt:17-22]

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| Instruction execution, P handling, unstable opcodes | Core library (`src/cpu.c`, object lib) | — | Shipped object; same object is tested (D-09) |
| Bus timing (ticks), RAM mirror, open-bus latch | Core library (`src/bus.c`) | — | Final shape now (D-12); Phase 3 adds PPU/DMA behind the seam |
| Flat 64 KiB RAM + cycle log for vectors | Test code (`tests/cpu/vector_bus.c`) | — | Link-time replacement of the two bus symbols (D-08) |
| JSON → N65V conversion | Host tool (`tools/vecconv/`) | CMake scripts | Never linked into core; palgen pattern |
| N65V parsing/bounds | Test code (`tests/vectors/n65v.c`) | vecconv (re-read check) | One reader for both tiers (D-05) |
| Full-set fetch + SHA-256 verification | CMake script (`tests/cmake/fetch_vectors.cmake`) | git | Test-time fixture, never configure time (D-19) |
| Nightly orchestration and reporting | GitHub Actions (`nightly.yml`) | `gh` | Outside `CI required` (D-24) |
| Release on merge | release.yml + release-please (existing) | — | Needs the `release-as` pin removed |

## Standard Stack

No new external packages. Everything is in-repo C plus platform tools that are already required.

| Tool | Version (local / CI) | Purpose | Why |
|------|----------------------|---------|-----|
| CMake | 4.4.3 local; 3.25 minimum; ≥3.31.6 on images | build, `cmake -P` scripts, `file(DOWNLOAD RANGE_*)`, `file(SHA256)`, `file(READ … OFFSET LIMIT HEX)`, `cmake -E cat`, `cmake -E sha256sum` | `RANGE_START/RANGE_END` "Added in version 3.24" [CITED: cmake.org/cmake/help/latest/command/file.html] |
| git | 2.41.0 local | sparse, blobless, depth-1 fetch by SHA | recipe tested this session (below) |
| CTest | with CMake | labels, fixtures | existing pattern (`FIXTURES_SETUP testframe_ppm`) [VERIFIED: tests/CMakeLists.txt] |
| gh | present locally; on GitHub images | nightly `report` job | already used in ci.yml `title` job |

**Installation:** none.

## Package Legitimacy Audit

No external packages are installed in this phase. Not applicable.
**Packages removed due to [SLOP] verdict:** none. **Packages flagged [SUS]:** none.

## Upstream facts verified this session (pin `2f6980a2d95757486c7bee24355c360e40e2a224`)

| Fact | Evidence |
|------|----------|
| Pin = upstream `main` HEAD today; commit date 2025-05-10, "Merge pull request #17 … Clarified expectations regarding RAM accesses" | `gh api repos/SingleStepTests/65x02/commits/main` [VERIFIED] |
| `nes6502/v1` last changed by b7ed828 (2024-07-27, "Correct JAM/KIL/HLT bus activity.", touched exactly the 12 JAM files); imported a86fc68 (2024-05-11) | `gh api …/commits?path=nes6502/v1` [VERIFIED] |
| 256 files `00.json`–`ff.json`, total 1,081,529,097 B; tree not truncated; no `.gitattributes`/LFS | git trees API, recursive [VERIFIED] |
| LICENSE: MIT, `Copyright (c) 2024 Thomas Harte et al` | raw LICENSE at pin [VERIFIED] |
| nes6502 README: "This version of the 6502 disabled the binary-coded decimal mode."; "Within each `.json` file, there is a JSON array of 10,000 test scenarios."; "Any memory address not included in a test's `ram` lists must not be accessed during that test."; "ensure only **one** bus operation occurs per cycle." | raw `nes6502/README.md` at pin [VERIFIED] |
| Keys used: `name initial final cycles pc s a x y p ram`; kinds only `read`/`write`; no CR, tab, non-ASCII, sign or float in any prefix; `name` values are hex digits and spaces only | scan of 256 × 64 KiB prefixes [VERIFIED] |
| File tails: `a9.json` ends `}\n]` and `02.json` ends `}]`, neither with a trailing newline | `od -c` [VERIFIED] |
| 25 compact files exactly as D-02 lists; 100th object ends ≤ 54,565 B | prefix scan [VERIFIED] |
| Max 11 cycles per test (JAM); max 9 RAM entries per state in the sample | prefix scan [VERIFIED] |
| Raw GitHub honours Range; `RANGE_END` is inclusive (0..65535 → 65,536 B, sha identical to curl) | `cmake -P` test [VERIFIED] |
| ADC/SBC with D set are binary (4,948 D-set tests in `69.json`, all binary) | full-file analysis [VERIFIED] |
| ANE `(A|$EE)&X&imm` and LXA `(A|$EE)&imm` fit all 10,000 tests each; SHY/SHX/SHA/TAS formula of D-15 fits all 10,000 of each of `9c 9e 9f 93 9b` | full-file analysis [VERIFIED] |
| JSR cycle order: opcode, ADL, read `$0100+S`, write PCH, write PCL, read ADH, in all 10,000 tests; **zero** stack/ADH overlap tests in nes6502 `20.json`; 41 page-1 tests, first at index 166 | full-file analysis [VERIFIED] |
| Issue #18 overlap case (from `6502/v1/20.json` line 4046): initial pc 379, s 125, ram `[379,32] [380,85] [381,19] [341,173]`; cycles `[379,32,r] [380,85,r] [381,19,r] [381,1,w] [380,125,w] [381,1,r]`; final pc 341, s 123, ram `[380,125] [381,1]` | issue #18 body [VERIFIED: gh api issues/18] |
| Issue #2 (LXA $EE vs $FF) is still open | gh api [VERIFIED] |

## Architecture Patterns

### System Architecture Diagram

```
 per-PR (ci, 6 platforms)                         nightly (ubuntu-24.04)
 ------------------------                         ----------------------
 tests/vectors/65x02-sample.n65v                  git sparse/blobless fetch @pin
          |                                         -> verify 256 x SHA-256 vs pins.txt
          v                                                  |
 n65v reader (bounds-checked) --chunk xx-->         vecconv xx.json -> xx.n65v (10,000)
          |                                          |               \--first 100--> sample-match
          v                                          v                               (byte-equal to
 test_vectors: for each test                    n65v reader (1 x 10,000)              committed chunk)
   set nes.cpu.* + RAM ---------------------------------+
   nesturbator__cpu_step(nes)  <-- cpu.o (the shipped object)
          |   every cycle: nesturbator__bus_read/write
          v
   vector_bus.c: flat RAM + cycle log (cap, overflow=fail)
          |
   compare pc s a x y p, final RAM pairs, cycle list
          -> report "xx.json[i]: <first mismatch>"; exit = failures != 0

 library build:  cpu.o + bus.o (+ instance/frame/...) -> libnesturbator.a
                 bus.o: ticks += 9, catch-up(empty), += 15, catch-up(empty), access
                        RAM $0000-$1FFF mirrored 2 KiB; else open-bus latch
```

### Recommended file layout

```
src/cpu.c                 # nesturbator__cpu_step; switch over 256 opcodes; static helpers
src/bus.c                 # nesturbator__bus_read / _write (library definitions)
src/internal.h            # + struct nesturbator__cpu, profile, bus state, the 3 prototypes
tools/vecconv/vecconv.c   # JSON -> N65V; links tests/vectors/n65v.c for its re-read
tools/vecconv/CMakeLists.txt
tests/vectors/n65v.{c,h}  # reader
tests/vectors/65x02-sample.n65v
tests/vectors/pins.txt
tests/vectors/fixtures/*.json (+ README.md for attribution, see Pitfall 8)
tests/cpu/test_vectors.c  # the vector program (sample and full)
tests/cpu/vector_bus.c    # test bus
tests/cpu/test_cpu_unit.c # JSR overlap, jammed loop, profile defaults (links cpu.o + vector_bus.c)
tests/cpu/test_bus.c      # links src/bus.c directly: mirror, latch, 24 ticks per call
tests/vectors/test_n65v.c # crafted-buffer reader tests
tests/cmake/fetch_vectors.cmake, vectors_sample_match.cmake, vectors_regen.cmake, pins_check.cmake
.github/workflows/nightly.yml
```

### Pattern 1: switch dispatch with addressing-mode helpers (recommended for the discretion area)

**What:** `switch (op)` with one `case` per opcode. Each case is one or two lines calling `static` helpers. Helpers: `fetch()` (read PC, PC++), `ea_zp`, `ea_zpx`, `ea_zpy`, `ea_abs`, `ea_absi(idx, always_dummy)`, `ea_izx`, `ea_izy(always_dummy)`, plus `rmw(ea, op)` (read, write old, write new), `branch(cond)`, `push/pull`, and the flag helpers `set_nz`, `set_flag(mask, cond)`.
**Why:** It meets "straight-line code per instruction" with no data-relocated pointer table. `abi.global_symbols` rejects `.data.rel.ro` pointer tables: "a file-scope const table of pointers (to functions or strings) is placed there, and nm reports it as d. So the core keeps no such table." [VERIFIED: tests/cmake/global_symbols.cmake:8-14]. A `const uint8_t` table is allowed ("const lookup tables hold integers only"), but the switch needs none.
**Rule:** one bus access per statement (ENGINEERING §1: "operand order is unspecified and unwarned").

### Pattern 2: one helper for core flags (D-09)

The existing nofp flag is attached to the `nesturbator` target only (`CMakeLists.txt:44-50`, `target_compile_options(nesturbator PRIVATE -mgeneral-regs-only)`). Write `function(nesturbator_core_flags target)` that applies the include dirs, `nesturbator_warnings`, the visibility/PIC properties and, when `NESTURBATOR_NOFP` is set, `-mgeneral-regs-only`. Call it for `nesturbator` and `nesturbator_cpu`. The `option(NESTURBATOR_NOFP …)` must come before the first call. The OBJECT library must be defined **before** `if(NOT PROJECT_IS_TOP_LEVEL) return()` (lines 84-86), because embedders need it. vecconv, the tests and the nightly wiring go after that line.

### Verified per-cycle table (matched all sample tests and 380,000 full-file tests)

Notation: `R a` is a read, `W a v` a write. PC means the value after the preceding fetches. "dummy" means the value is discarded. Indexed low-byte sums wrap within the page.

| Group | Cycles after opcode fetch `R PC` |
|-------|-------------------------------|
| Implied/accumulator (incl. NOP `1A 3A 5A 7A DA FA EA`) | `R PC` (dummy, no PC++) |
| Immediate (incl. `80 82 89 C2 E2 EB 0B 2B 4B 6B CB 8B AB`) | `R PC` operand, PC++ |
| zp read / write | `R PC`→b; `R b` / `W b` |
| zp,X / zp,Y | `R PC`→b; `R b` (dummy); `R/W (b+i)&FF` |
| abs (incl. `0C` NOP) | `R lo`; `R hi`; `R/W ea` |
| abs,X/Y **read** | `R lo`; `R hi`; if `lo+i>FF`: `R (hi<<8)|((lo+i)&FF)` dummy; `R ea` |
| abs,X/Y **write/RMW/SHx** | always the dummy read at the uncorrected address, then the access |
| (zp,X) | `R PC`→b; `R b` dummy; `R (b+X)&FF`→lo; `R (b+X+1)&FF`→hi; access |
| (zp),Y | `R PC`→p; `R p`→lo; `R (p+1)&FF`→hi; dummy read at uncorrected address on cross (read) or always (write/RMW/SHA `93`); access |
| RMW (any mode, official and SLO/RLA/SRE/RRA/DCP/ISC) | …; `R ea`→v; `W ea v` (old value); `W ea result` |
| Branch | `R PC` offset, PC++; not taken: done (2 cycles). Taken: `R PC` dummy; if the page changes: `R (oldPCH<<8)|newPCL` dummy |
| JMP abs | `R lo`; `R hi` (3 cycles) |
| JMP (ind) | `R lo`; `R hi`; `R hi:lo`; `R hi:((lo+1)&FF)` |
| JSR | `R PC` ADL, PC++; `R $100+S` dummy; `W $100+S PCH`, S--; `W $100+S PCL`, S--; `R PC` ADH (after the pushes) |
| RTS | `R PC` dummy; `R $100+S` dummy; pull PCL; pull PCH; `R PC` dummy; PC++ |
| RTI | `R PC` dummy; `R $100+S` dummy; pull P→`(v&~$10)|$20`; pull PCL; pull PCH |
| BRK | `R PC` dummy, PC++; push PCH; push PCL; push `P|$30`; I=1 (D unchanged); `R FFFE`; `R FFFF` |
| PHA/PHP | `R PC` dummy; push A / `P|$30` |
| PLA/PLP | `R PC` dummy; `R $100+S` dummy; pull (PLA sets N Z; PLP `(v&~$10)|$20`) |
| JAM (12 opcodes) | `R PC` (no PC++); `R FFFF FFFE FFFE FFFF FFFF FFFF FFFF FFFF FFFF`; PC ends at opcode+1 |
| SHY `9C` (abs,X), SHX `9E`, SHA `9F`, TAS `9B` (abs,Y), SHA `93` ((zp),Y) | address cycles with the dummy read always; TAS sets S=A&X first; `v = reg & ((H+1)&FF)`; `W ((cross?v:H)<<8)|((L+i)&FF) v` |

Unofficial operation semantics confirmed: ANC: `A&=imm`, then N, Z, C=N. ALR: `t=A&imm; C=t&1; A=t>>1`. ARR: `t=A&imm; r=(t>>1)|(C<<7)`; N Z from r; C=bit 6; V=bit6^bit5. SBX: `t=A&X; C=(t>=imm); X=t-imm`, N Z. LAS: `r=mem&S; S=A=X=r`. `EB` = SBC immediate. LAX and SAX use the listed modes (`A7 B7 AF BF A3 B3`; `87 97 8F 83`). [VERIFIED: scratch model]

### Anti-Patterns to Avoid
- **Rebuilding P from flag bits, or normalising P each step.** This fails 25 files (Pitfall 1).
- **Function-pointer opcode table.** It fails `abi.global_symbols`.
- **`memcpy` of N65V records into a struct.** D-03 forbids it, and padding and endianness vary.
- **Clearing all 64 KiB per test in the nightly.** 10,000 × 64 KiB per file is wasteful. Zero only the addresses in the test's `initial`/`final` lists after each test. The README guarantees that no other address is touched, and the cycle compare enforces it.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Concatenating 256 binary chunks (D-07 regen) | a C cat tool | `execute_process(COMMAND ${CMAKE_COMMAND} -E cat … OUTPUT_FILE …)` | Byte-exact on macOS (tested: the SHA matched after split and join) |
| Comparing a converted chunk with the committed blob (sample-match) | a second binary diff program | `file(SIZE)` + `file(READ <blob> <var> OFFSET o LIMIT n HEX)` vs `file(READ chunk <var> HEX)`, advancing `o` | Tested: works on 1.6 MB (3,356,228 hex chars) |
| SHA-256 lists for `pins.txt` | the runner's sha256.c | `file(SHA256)` / `cmake -E sha256sum` (sha256sum layout) + `file(SIZE)` | Platform tool |
| Range download | curl in a script | `file(DOWNLOAD … RANGE_START 0 RANGE_END 65535 TLS_VERIFY ON STATUS st)` | `RANGE_END` is inclusive |
| Hex test names 00–ff in CMake | `math(… OUTPUT_FORMAT HEXADECIMAL)` (gives `0x0`, unpadded) | nested `foreach(h 0 1 … f)` × `foreach(l 0 1 … f)` | exact two-digit lowercase |
| JSON parsing | a generic JSON library | D-02 schema tokenizer | locked |

## Common Pitfalls

### Pitfall 1: P bits 4 and 5 are not normalised in 25 files
**What goes wrong:** All tests in `02 12 22 32 42 52 62 72 92 b2 d2 f2 0c 1c 3c 5c 6b 7c 93 9b 9c 9e 9f dc fc` start with `p & 0x30 == 0x30`, and `final.p` keeps it. In the full `93 9c 9e 9f`, `final.p == initial.p` in all 10,000.
**How to avoid:** The harness loads `p` raw and compares raw. CPU flag writes are masked (`p = (p & ~mask) | bits`). Only PLP and RTI assign bits 4 and 5. Pushes OR in `$30` without changing P.
**Warning sign:** Only the compact-layout files fail, all on `p`.

### Pitfall 2: The sample has no JSR stack-overlap case
Add a unit test from the issue #18 values (see the facts table). Expect final pc 341 (`$0155`), S 123, and RAM `$017C=125`, `$017D=1`. Expect exactly the 6 cycles listed. Cite issue #18 and 64doc. Data from `6502/v1` is MIT at the same pin.

### Pitfall 3: Float scan trips on comments
`abi.float_scan` scans `src/` and `include/` and counts comments. It flags `[0-9]\.[0-9]` and `[^A-Za-z0-9_][0-9]+[eE][+-]?[0-9]` after removing `0x` hex [VERIFIED: tests/cmake/float_scan.cmake:33-38]. "§2.1", "rev 1.2", "C17 6.7.2.1p15" and `$`-hex such as `$0E00` or `$1E0` in a `src/` comment all fail. Write hex as `0x…` in core comments and section numbers as words ("section 2, part 1"), as Phase 1 did (STATE: "Core comments state numbers as integers or words"). The C17 citation belongs in `tests/cpu/`, which is not scanned.

### Pitfall 4: `-Wconversion` and MSVC `/W4` on 8-bit arithmetic
`uint8_t r = a + b;` warns (int → uint8_t), and presets make warnings fatal. Cast every narrowing explicitly: `(uint8_t)(a + b)`, `(uint16_t)((hi << 8) | lo)`. Cast to `uint32_t` before `<<` or `*` (ENGINEERING §1, UBSan). The test sources need `_CRT_SECURE_NO_WARNINGS` on MSVC for `fopen`, as `libretro.host` does [VERIFIED: tests/CMakeLists.txt].

### Pitfall 5: The OBJECT library and the embed and install checks
Use `target_sources(nesturbator PRIVATE $<TARGET_OBJECTS:nesturbator_cpu>)` (D-09). `tests/embed/CMakeLists.txt` forbids only `nesturbator_host nesturbator-run nesturbator_libretro palgen` in an embedder, so `nesturbator_cpu` is allowed, but `vecconv` must stay below the top-level `return()`. Add `vecconv` to that forbidden list so the check grows with the tool.

### Pitfall 6: Duplicate bus symbols
`cpu.vectors` and the CPU unit tests must not link `nesturbator::nesturbator` (D-09). `test_bus.c` compiles `src/bus.c` directly and needs no CPU.

### Pitfall 7: `nofp` builds everything but runs only `abi`
The vector tests are not run under `nofp`, which is fine. The flag must still reach `cpu.o`, because `abi.undefined_symbols` and `abi.global_symbols` read `libnesturbator.a`, which now contains `cpu.o` and `bus.o`.

### Pitfall 8: JSON fixtures cannot carry a comment
D-06 says "each fixture carries an MIT attribution comment", but JSON has no comments, and the D-02 tokenizer must reject anything unknown. Put the attribution in `tests/vectors/fixtures/README.md` and give each fixture a manifest line. ASSET_POLICY: "Test ROMs and test data whose licence permits redistribution, each listed in `tests/roms/manifest.txt` with its source, pin, licence and SHA-256." [VERIFIED: ASSET_POLICY.md:25-26]. `* text=auto eol=lf` [VERIFIED: .gitattributes:1] keeps their SHA-256 stable on Windows. The planner should confirm this reading with the owner, or treat it as a correction to D-06.

### Pitfall 9: Manifest format and hygiene
Manifest lines are tab-separated `path<TAB>source<TAB>pin<TAB>licence<TAB>sha256` [VERIFIED: tests/roms/manifest.txt:7-15]. `hygiene.sh` treats any file with a NUL as binary, and a binary file needs a manifest line (`in_manifest` matches field 1) [VERIFIED: scripts/hygiene.sh:46,63-69]. The N65V blob holds NULs, so the line is mandatory, and `--history` checks the blob too. Nothing currently verifies the manifest's SHA-256 column. Add a `ci` test (`manifest.sha256`, `cmake -P`) that recomputes each listed file's hash. Without it, a regenerated blob with a stale manifest line goes unnoticed.

### Pitfall 10: The git fetch recipe needs a configured remote
D-19 lists `git init … fetch origin <sha>` with no `git remote add`. Tested this session: `git init`, `git remote add origin https://github.com/SingleStepTests/65x02.git`, `git sparse-checkout set --cone <dir>`, `git -c gc.auto=0 fetch --depth 1 --filter=blob:none origin <sha>`, `git checkout FETCH_HEAD` works on git 2.41. The fetch sets `remote.origin.promisor=true` and `partialclonefilter=blob:none`, and HEAD equals the pin. Cone mode also checks out the root `LICENSE` and `README.md` (and `nes6502/README.md`), so the script can verify `LICENSE` as well. Pass `-c advice.detachedHead=false` to keep logs quiet.

### Pitfall 11: The `nightly` label must exist
`gh issue create --label nightly` fails if the label does not exist. Run `gh label create nightly --force` in the `report` job. `POST /repos/{owner}/{repo}/labels` needs Issues write [CITED: docs.github.com/en/rest/authentication/permissions-required-for-fine-grained-personal-access-tokens], which `issues: write` grants. Pass `--repo "$GITHUB_REPOSITORY"`, because the job has no checkout.

### Pitfall 12: The `release-as` pin blocks the phase's release
`release-please-config.json:10` is `"release-as": "0.1.0",` [VERIFIED: file read]. release-please docs: "once the release PR is merged you should either remove this or update it to a higher version. Otherwise subsequent `manifest-pr` runs will continue to use this version" [CITED: github.com/googleapis/release-please/blob/main/docs/manifest-releaser.md]. STATE.md already records removing it as the first task of Phase 2, with an automated check (WR-08). With `bump-patch-for-minor-pre-major: true` (line 7), a `feat:` title then releases **0.1.1**, not 0.2.0 ("feat commits bump semver patch instead of minor if version < 1.0.0") [CITED: same doc].

### Pitfall 13: TLS verification defaults
`file(DOWNLOAD)` verifies TLS by default only since CMake 3.31 [CITED: cmake.org file docs]. The project minimum is 3.25, so pass `TLS_VERIFY ON` explicitly in the regen script.

## Code Examples

### P-preserving flag helpers (pattern; names illustrative)
```c
/* Flag bits of P. Bits 4 and 5 are never written here: PLP and RTI set them,
   and the pinned vectors start 25 opcodes with both set (02-RESEARCH). */
static void set_nz(struct nesturbator *nes, uint8_t v)
{
    nes->cpu.p = (uint8_t)((nes->cpu.p & ~0x82u) | (v & 0x80u) | (v == 0u ? 0x02u : 0u));
}
```

### SHx helper (D-15, verified 50,000 tests)
```c
static void store_sh(struct nesturbator *nes, uint8_t reg, uint8_t lo, uint8_t hi, uint8_t idx)
{
    uint16_t sum = (uint16_t)(lo + idx);
    uint8_t value = (uint8_t)(reg & (uint8_t)(hi + 1u)); /* RDY case joins here later */
    uint8_t high = (sum > 0xFFu) ? value : hi;
    nesturbator__bus_read(nes, (uint16_t)(((uint32_t)hi << 8) | (sum & 0xFFu)));
    nesturbator__bus_write(nes, (uint16_t)(((uint32_t)high << 8) | (sum & 0xFFu)), value);
}
```

### Expected sample artefact (independent cross-check)
Layout D-03, chunks in opcode order, RAM pairs in JSON order, index = 0-based position in the file:
- size **1,678,114 B**; SHA-256 `0c318cec0bd4031396964ca9a0de0daf25c2651c28f34a0d0616a9294430e109`; first 12 bytes `4e 36 35 56 01 00 64 00 00 00 81 e8`.
If vecconv's output differs, find out why before committing. Do not adopt either number blindly.

### Fixture sizes
The first 3 objects of `a9.json` end at byte 984; those of `02.json` end at byte 1,592. The fixtures stay small text files.

## State of the Art

| Old (prep docs) | Current (measured) | Impact |
|-----------------|--------------------|--------|
| CONFORMANCE: "10,000 tests, one per line" | JSON array; 25 compact files | The tokenizer must be layout-independent (D-02) |
| CONFORMANCE: 54,433 B bound | 54,565 B over all 256 files | Correct it in the same change |
| CONFORMANCE: "63.3 B per test, 1.6 MB" | 65.55 B per test; 1,678,114 B | Correct D-04's 1,677,602 |
| ENGINEERING §5: nightly cache keyed by commit | no cache (D-21) | Revise §5 |

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | The JSON fixtures should be manifest-listed, with attribution in a README, because JSON cannot hold the D-06 comment | Pitfall 8 | Low: hygiene does not require text files in the manifest; a policy reading only |
| A2 | `cmake -E cat` is byte-exact on Linux as well (tested on macOS only; nightly and regen do not run on Windows) | Don't Hand-Roll | Low: sample-match would catch a corrupted concatenation |
| A3 | A full nightly (fetch ~89 s per D-19 plus 2.56 M tests in C) fits well under 10 minutes; the first run sets `timeout-minutes` | D-24 | Low: measured on first run |
| A4 | `gh label create --force` works with the job's `GITHUB_TOKEN` under `issues: write` | Pitfall 11 | Low: a first scheduled or dispatched run shows it |

## Open Questions

1. **Release version for this phase.** A `feat:` merge after removing `release-as` gives 0.1.1. If the owner wants 0.2.0 per phase, set `"release-as": "0.2.0"`, which re-arms the trap for the next phase. Recommendation: accept 0.1.1 and keep the automated check that `release-as` is absent.
2. **Scoreboard now or in Phase 3 (discretion).** Recommendation: Phase 3. The phase bar is "every vector passes", and the sample has nothing to "improve".
3. **Measured nightly duration.** Unknown until the first `pull_request`-triggered run on the phase branch. Set `timeout-minutes` from that run, as the existing jobs do.

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| CMake ≥3.25 (3.24 for RANGE) | everything | ✓ | 4.4.3 | — |
| Ninja | presets | ✓ | 1.13.2 | — |
| C compiler | build | ✓ | Apple clang 21 (CI: GCC 14, clang 18, MSVC) | — |
| git with sparse-checkout and partial clone | nightly fetch, regen | ✓ | 2.41.0 | — |
| clang-format 18 | hygiene | ✓ | /opt/homebrew/opt/llvm@18 | — |
| gh | nightly report | ✓ (local and images) | — | — |
| Network to github.com / raw.githubusercontent.com | regen (local), nightly | ✓ (tested) | — | none; offline fails by design (D-22) |

Missing dependencies: none.

## Validation Architecture

Skipped (`workflow.nyquist_validation: false`). The test map is in `<phase_requirements>`. Unit tests to plan beyond the vectors: reader crafted buffers; vecconv fixtures (positive and negative); JSR overlap; the jammed loop (each later `cpu_step` is one `R $FFFF`, PC unchanged); profile defaults `$EE` through `nesturbator_create` plus an internal cast; `bus.c` (mirror at `$0800/$1000/$1800`, latch on unmapped reads, `ticks += 24` per call); `pins.txt` offline check; manifest SHA-256 check; `release-as` absent.

## Security Domain

Omitted (`security_enforcement: false`). Relevant hygiene only: the nightly has `permissions: {}`, `persist-credentials: false`, SHA-pinned actions and no secrets (D-24). The parsers fail closed with bounds checks (D-02, D-05).

## Sources

### Primary (HIGH)
- github.com/SingleStepTests/65x02 @ 2f6980a2: `LICENSE`, `README.md`, `nes6502/README.md`, git tree, 256 range prefixes, 38 full files; commits b7ed828 and a86fc68; issues #1, #2, #8, #13, #18 (gh api)
- nesdev.org/6502_cpu.txt (64doc): JSR, RTS, RTI, BRK, PHA/PHP, PLA/PLP, abs,X RMW and branch tables
- In-repo: `CMakeLists.txt`, `CMakePresets.json`, `tests/CMakeLists.txt`, `tests/abi/CMakeLists.txt`, `tests/cmake/{undefined_symbols,global_symbols,float_scan,action_pins,palette_regen,format_check,check_archives}.cmake`, `tests/embed/CMakeLists.txt`, `scripts/hygiene.sh`, `tests/roms/manifest.txt`, `.gitattributes`, `.github/workflows/{ci,release}.yml`, `release-please-config.json`, `src/internal.h`, `src/instance.c`, `src/frame.c`, `ASSET_POLICY.md`, `PROVENANCE.md`, `THIRD-PARTY-NOTICES.md`, `.planning/STATE.md`

### Secondary (MEDIUM/CITED)
- cmake.org/cmake/help/latest/command/file.html (`RANGE_*` 3.24, `TLS_VERIFY` default since 3.31)
- release-please `docs/manifest-releaser.md` (`release-as`, `bump-patch-for-minor-pre-major`)
- docs.github.com fine-grained permissions (labels: Issues write)

## Metadata

**Confidence breakdown:**
- Per-cycle behaviour and opcode semantics: HIGH. An executable model matched 405,600 vectors.
- Build and CI wiring: HIGH. Grounded in the files read and in the tests run here.
- Nightly runtime and label behaviour: MEDIUM. Confirmed on the first run.

**Research date:** 2026-10-03
**Valid until:** 2026-11-02, or until upstream `main` moves past the pin.
