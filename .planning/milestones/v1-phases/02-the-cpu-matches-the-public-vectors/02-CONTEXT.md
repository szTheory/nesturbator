# Phase 2: The CPU matches the public vectors - Context

**Gathered:** 2026-10-03
**Status:** Ready for planning

<domain>
## Phase Boundary

A 6502 core (the 2A03's CPU, no decimal mode) in the library that matches the SingleStepTests 65x02 `nes6502/v1` vectors at commit `2f6980a2d95757486c7bee24355c360e40e2a224` on final state and on every bus cycle, for all 256 opcodes. The committed sample (first 100 tests per opcode) runs under `ctest -L vectors` in `cmake --workflow --preset ci` on six platforms; the full set is fetched at the pin and runs under `ctest -L vectors-full`, nightly in CI. The `ci`, `asan`, `nofp` and `hygiene` presets keep passing and merging publishes a release. Requirements: CPU-01, CPU-02.

Not in this phase: interrupts taken, reset sequence, DMA, PPU, APU, controllers, cartridge loading, any public API change. The CPU is reached only through internal hooks (Phase 1 D-10).

</domain>

<decisions>
## Implementation Decisions

Everything in `.planning/preparation/DECISIONS.md` (DEC.01–DEC.39) and Phase 1's `01-CONTEXT.md` (D-01–D-20) stays settled. In particular: instruction-level CPU, straight-line code per instruction, one bus call per cycle with dummy accesses (DEC.09); ANE/LXA constant is a machine-profile field (DEC.18); sample = first 100 tests per opcode, full set never committed (DEC.20, DEC.21); labels `vectors` / `vectors-full`, `nightly.yml` outside `CI required` (ENGINEERING §3, §5).

### Vector converter and sample
- **D-01:** An owned C host tool `tools/vecconv/vecconv.c` (palgen-style: built by CMake in every preset, not installed, not linked into the core, integer-only) converts upstream JSON to N65V. Usage `vecconv <in.json> <out.n65v> <opcode> [--first N]`; with `--first N` it stops after N objects and accepts a truncated tail (a range prefix); without it the file must close with `]` and hold exactly 10,000 tests. After writing it re-reads its output through the shared reader. CMake `string(JSON)` is rejected (quadratic re-parse, cannot emit NUL bytes); no Python.
- **D-02:** The JSON tokenizer is schema-specific and layout-independent: any JSON whitespace, keys matched by name, unsigned decimal integers only; exit 1 with a byte offset on a float, sign, exponent, out-of-range value (>65535 / >255), count >255, unknown kind, unknown or missing key. Measured at the pin: each file is a JSON array (not JSON Lines); 25 files (02 0c 12 1c 22 32 3c 42 52 5c 62 72 6b 7c 92 93 9b 9c 9e 9f b2 d2 dc f2 fc) use a compact layout (`[{`, `[[`); the 100th object ends at byte 54,565 at most (CONFORMANCE's 54,433 came from 18 files — correct it in the same change); `name` is not unique and is dropped.
- **D-03:** N65V v1 keeps the CONFORMANCE layout with these rules fixed: little-endian, decoded byte by byte (never memcpy into a struct); version byte 1, any other rejected; per-test index u16, strictly increasing (failures name the upstream test as `a9.json[i]`); kind byte 0 = read, 1 = write, other rejected; counts u8, header count u16; no in-file checksum. — **Reversibility:** costly — the committed blob, the reader, the converter and the nightly all depend on the layout; a change means regenerating and re-manifesting the blob.
- **D-04:** The committed sample is **one file** `tests/vectors/65x02-sample.n65v` (256 per-opcode chunks concatenated in opcode order, 100 tests each; 1,677,602 B measured), with one line in `tests/roms/manifest.txt` (path, `https://github.com/SingleStepTests/65x02`, the pin, `MIT`, sha256). `.gitattributes` marks `*.n65v binary` and the JSON fixtures `eol=lf`. `THIRD-PARTY-NOTICES.md` gains the upstream MIT notice verbatim from the pinned `LICENSE` and its "one file comes from elsewhere" sentence is corrected; `PROVENANCE.md` lists the blob.
- **D-05:** The reader `tests/vectors/n65v.{c,h}` works on a buffer and length, checks bounds before every field (`n > len - off`, never `off + n`), takes the expected chunk count, opcodes and tests per chunk (256×100 for the sample, 1×10,000 per nightly file) and requires the file to end exactly after the last chunk. One reader and one comparison loop serve both tiers.
- **D-06:** `ci` tests for the converter and reader: crafted buffers through `check.h` (truncated at every field boundary, bad magic, version 2, kind 2, opcode mismatch, short count, trailing bytes); small committed JSON fixtures (first 3 tests of `a9` spaced layout and `02` compact layout, one truncated tail, negative cases that must exit 1) converted with `--first 3` and run through the vector program; each fixture carries an MIT attribution comment.
- **D-07:** A pin bump regenerates the sample locally from 256 × 64 KiB HTTP range prefixes (`file(DOWNLOAD … RANGE_START/RANGE_END)`, CMake ≥ 3.24), about 16 MB, writing the blob and printing the manifest line. A pin bump is its own change (CONFORMANCE).

### CPU–bus seam
- **D-08:** **Link-time seam.** `src/cpu.c` references exactly two external symbols, declared in `src/internal.h`: `uint8_t nesturbator__bus_read(struct nesturbator *nes, uint16_t addr)` and `void nesturbator__bus_write(struct nesturbator *nes, uint16_t addr, uint8_t value)`. `src/bus.c` defines them for the library; `tests/cpu/vector_bus.c` defines them for the vector test (flat 64 KiB RAM plus a fixed-capacity cycle log; overflow fails the test). No function pointers in the instance. — **Reversibility:** costly — Phase 3's bus, DMA and PPU catch-up are built behind this seam; moving to a run-time seam touches every bus call site and the save-state design.
- **D-09:** CMake: `add_library(nesturbator_cpu OBJECT src/cpu.c)` with the core's include dirs, warnings, `C_VISIBILITY_PRESET hidden`, `POSITION_INDEPENDENT_CODE ON` and the nofp flags (one helper applies the core flags to both targets); folded into the library with `target_sources(nesturbator PRIVATE $<TARGET_OBJECTS:nesturbator_cpu>)` — not `target_link_libraries`, which breaks `install(EXPORT)`. The test is `add_executable(cpu.vectors tests/cpu/test_vectors.c tests/cpu/vector_bus.c tests/vectors/n65v.c $<TARGET_OBJECTS:nesturbator_cpu>)` with private `src`/`include` includes and never links `nesturbator::nesturbator` (duplicate bus symbols). The tested object is the shipped object.
- **D-10:** CPU state `nesturbator__cpu { uint16_t pc; uint8_t a, x, y, s, p; uint8_t nmi_prev, nmi_pending, irq_line, poll_latch; uint8_t halted_in_read; uint8_t jammed; }` is the `cpu` member of `struct nesturbator`. Interrupt fields exist now, unused until Phase 3. Bus-side state (RAM, open-bus latch, DMA) goes on the bus side of the instance, not in the CPU struct. Code accesses `nes->cpu.*` directly; the bus never reads CPU registers other than `halted_in_read` and the interrupt fields.
- **D-11:** Internal hook `void nesturbator__cpu_step(struct nesturbator *nes)` runs exactly one instruction, opcode fetch to last cycle. The vector test zero-initialises a `struct vector_machine { struct nesturbator nes; uint8_t ram[65536]; log…; }` (first-member conversion, C17 6.7.2.1p15), sets `nes.cpu.*` and RAM, calls `cpu_step` once, then compares registers, RAM and every logged cycle. It does not call `nesturbator_create`.
- **D-12:** `src/bus.c` has its final shape in Phase 2: per call, advance `nes->ticks` 9 to M2 rise, empty PPU catch-up, 15 to M2 fall, empty catch-up, then the access (NTSC M2 high 15/24, NES-HARDWARE-CPU-APU §2.1). Phase 2 decode: 2 KiB RAM mirrored over `$0000–$1FFF`, open-bus latch elsewhere. Interrupt sampling will live in the bus call during phi2; `cpu.c` only reads the latch at poll points. `bus.c` lands in the same change as `cpu.c` (else `abi.undefined_symbols` fails). `run_frame` keeps producing the test card; the CPU does not drive frames until Phase 3.

### JAM and unstable opcodes (checked against the pinned data)
- **D-13:** JAM (`$02 $12 $22 $32 $42 $52 $62 $72 $92 $B2 $D2 $F2`): exactly 11 reads — opcode at PC, PC+1 (discarded), then `$FFFF $FFFE $FFFE $FFFF $FFFF $FFFF $FFFF $FFFF $FFFF`; PC ends at PC+1 (16-bit wrap); no write, no register or flag change; sets `jammed`. While jammed each `cpu_step` is one read of `$FFFF`, NMI and IRQ are not taken, only reset or power clears it; the flag is instance state (save states include it later). The vector harness compares the opcode's own 11 cycles and never relies on the jammed loop. Cite NESdev Wiki "CPU unofficial opcodes" rev 23975 and 65x02 commit b7ed828 / issue #1.
- **D-14:** Two profile fields `ane_magic` and `lxa_magic`, both `$EE` for NTSC RP2A03G. ANE: `A = (A | ane_magic) & X & imm`; LXA: `A = X = (A | lxa_magic) & imm`; both set N, Z. The vector test sets both to `$EE` itself; a unit test pins the profile defaults. The profile lives in the instance; no public option this phase.
- **D-15:** SHY/SHX/SHA (`$9F`, `$93`)/TAS: `value = reg & (H + 1)` with reg = Y, X, or A & X (H for `$93` is the zero-page pointer's high byte); target `((cross ? value : H) << 8) | ((L + idx) & $FF)`; TAS first sets `S = A & X`. The `& (H+1)` mask sits in one helper so the RDY ("halted in this read") case is one condition later. No RDY branch now.
- **D-16:** Decimal: D is stored and restored (SED, CLD, PHP, PLP, RTI; BRK does not clear it) but ADC/SBC are always binary — hardware comment cites the nes6502 README. P held with bit 5 = 1, bit 4 = 0; PHP and BRK push `P | $30`; PLP and RTI load `(v & ~$10) | $20`; later IRQ/NMI pushes use `(P & ~$10) | $20`.
- **D-17:** JSR order: read ADL, dummy stack read, push PCH, push PCL, read ADH (upstream issue #18); the sample must keep a test where code overlaps the stack page, or a unit test covers it.
- **D-18:** Match the pinned vectors with no exclusion list; documented chip differences go through profile fields only. A waiver list (test name + cited source, read as `unsupported`) is added only if a vector is shown wrong. None is known at the pin.

### Nightly full-set run
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

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Settled choices
- `.planning/preparation/DECISIONS.md` — DEC.01–DEC.39
- `.planning/phases/01-a-test-frame-in-retroarch/01-CONTEXT.md` — Phase 1 decisions, header conventions (D-06–D-11)
- `.planning/preparation/README.md` — index of preparation files

### CPU behaviour
- `.planning/preparation/NES-HARDWARE-CPU-APU.md` §2.1–2.4 (time, DMA, polling, mid-instruction stops), §3 (hard cases), §7 (unofficial/unstable opcodes), §8 (power-on)
- `.planning/preparation/ARCHITECTURE.md` §2 (time), §3 (CPU and bus), §7 (API principles)
- NESdev Wiki "CPU unofficial opcodes" rev 23975, "Status flags" rev 22145; 64doc (https://www.nesdev.org/6502_cpu.txt) — per-cycle tables
- SingleStepTests/65x02 at `2f6980a2d95757486c7bee24355c360e40e2a224`: `README.md`, `nes6502/README.md`, `LICENSE`; issues #1, #2, #13, #18; commit b7ed828

### Tests, CI, provenance
- `.planning/preparation/CONFORMANCE.md` "The 65x02 vectors", "Scoreboard design", "Where each test runs" — update the "one per line" wording and 54,433 bound per D-02
- `.planning/preparation/ENGINEERING.md` §1 (C rules), §2 (presets), §3 (labels), §5 (CI; revise the cache line per D-21), §7 (hygiene)
- `tests/roms/manifest.txt`, `scripts/hygiene.sh`, `ASSET_POLICY.md`, `PROVENANCE.md`, `THIRD-PARTY-NOTICES.md`

### Project rules
- `CLAUDE.md`, `AGENTS.md` — clean room (no GPL/LGPL emulator source), owned code, tests and docs together

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `tools/palgen/` + `tests/cmake/palette_regen.cmake`: the pattern for an offline owned generator with a byte-compare regeneration test — `vecconv` and `sample-match` follow it.
- `tests/check.h`: comparison macros for the reader and CPU unit tests.
- `tests/cmake/action_pins.cmake`: already globs workflows, so `nightly.yml` pins are checked.
- `tests/cmake/undefined_symbols.cmake`, `global_symbols.cmake`, `float_scan.cmake`: the nofp/symbol checks the CPU object must pass.

### Established Patterns
- All state in `struct nesturbator` (`src/internal.h`); internal symbols prefixed `nesturbator__`; hidden visibility; hardware comments cite sources.
- Tests are plain C executables under CTest with labels; test presets use `noTestsAction: error`.
- Tracked binaries need a manifest line; hygiene runs in hooks and CI.

### Integration Points
- `CMakeLists.txt` core target gains `$<TARGET_OBJECTS:nesturbator_cpu>` and `src/bus.c`.
- `tests/CMakeLists.txt` gains `cpu.vectors.00`–`.ff` (label `vectors`), converter/reader tests, and the gated `vectors-full` block.
- `CMakePresets.json` gains the `vectors-full` quartet; `.github/workflows/nightly.yml` is new.
- No change to `include/nesturbator.h` beyond comments.

</code_context>

<specifics>
## Specific Ideas

- Owner's rule throughout: "another copy and paste is better than another dep" — owned converter and reader, git and `cmake -E`/`file()` as the platform tools, no Python, no JSON library.
- Fail loudly, never skip: a red nightly is the only acceptable signal when the network or upstream breaks, because a skipped nightly silently drops the provenance check.
- Prefer primary sources: every opcode quirk above was checked against the pinned vector data, not only documentation.

</specifics>

<deferred>
## Deferred Ideas

- Jam surfaced to hosts as a status bit or stop reason; runner exits with its own code on a jam — Phase 3 (run API).
- Run instr_test-v5 `03-immediate` to settle LXA `$EE` vs `$FF`; change only `lxa_magic` — Phase 3.
- SHx RDY branch and AccuracyCoin unofficial-opcode RDY sub-tests (codes 7–C) — the phase that adds DMC DMA.
- Ticks overshooting a frame once the CPU drives `run_frame` (instruction boundaries) — plan the carry and runner hash updates in Phase 3.
- Caching converted N65V in the nightly — only if a cold run exceeds about 10 minutes.
- Adding `macos-15` to the nightly — only if a nightly ever disagrees with a local Mac run.
- Scheduled workflows are disabled after 60 days of repository inactivity — note for quiet periods between milestones.
- libFuzzer target for the N65V reader — optional, later (inputs are committed or self-generated).

</deferred>

---

*Phase: 02-the-cpu-matches-the-public-vectors*
*Context gathered: 2026-10-03*
