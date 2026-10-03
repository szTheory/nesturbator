# Phase 2: The CPU matches the public vectors - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-10-03
**Phase:** 02-the-cpu-matches-the-public-vectors
**Areas discussed:** Vector converter, CPU–bus seam, JAM and unstable ops, Nightly full-set run

Advisor mode (calibration `minimal_decisive`). The owner asked for a broad fan-out per area across every relevant role, with an adversarial pass and primary sources preferred; four researchers ran in parallel and measured against the pinned upstream data.

---

## Vector converter

| Option | Description | Selected |
|--------|-------------|----------|
| Owned C vecconv + N65V | One committed 1.68 MB blob; one reader and one compare loop for sample and nightly; nightly re-derives the sample byte for byte | ✓ |
| Read JSON at test time | About 14 MB of JSON prefixes committed; one parser, no binary format | |

**User's choice:** Owned C vecconv + N65V (recommended)
**Notes:** CMake `string(JSON)` ruled out (quadratic, cannot write NUL). Measured: files are JSON arrays, 25 use a compact layout, 100th test ends ≤ 54,565 B.

---

## CPU–bus seam

| Option | Description | Selected |
|--------|-------------|----------|
| Link-time seam | cpu.c calls only bus_read/bus_write; src/bus.c vs tests/cpu/vector_bus.c; cpu.c as an OBJECT library so the tested object ships | ✓ |
| Function-pointer bus | Callbacks plus user pointer in the CPU struct | |

**User's choice:** Link-time seam (recommended)
**Notes:** Test mode inside the real bus and a single-TU include were rejected by the researcher (test code in release; i-cache bloat with the Phase 3 bus).

---

## JAM and unstable ops

| Option | Description | Selected |
|--------|-------------|----------|
| Accept all | JAM 11 reads then jammed flag; ANE/LXA two $EE fields; SHx vector model in one helper; binary ADC/SBC; P rules; JSR order; no waiver list | ✓ |
| One shared ANE/LXA field | Same, with one constant | |

**User's choice:** Accept all (recommended)

---

## Nightly full-set run

| Option | Description | Selected |
|--------|-------------|----------|
| Accept all | Git sparse fetch by SHA + pins.txt; no cache; ubuntu-24.04 only; one rolling issue; vectors-full preset; offline fails | ✓ |
| Accept, but cache N65V | Also cache 162 MB of binaries | |
| Accept, add macos-15 | Also run on Apple Silicon | |

**User's choice:** Accept all (recommended)
**Notes:** Converter and nightly researchers disagreed on fetch (256 downloads vs git) and caching; resolved to git (measured 89 s, 193 MB vs 1.08 GB) and no cache. Revises ENGINEERING §5.

---

## Claude's Discretion

- Opcode dispatch shape and file split in cpu.c
- Vector failure-report format
- Whether the scoreboard file starts now or in Phase 3
- Profile struct layout
- README / header comment wording and commit title

## Deferred Ideas

- Jam stop reason and runner exit code (Phase 3)
- LXA $EE vs $FF check with instr_test-v5 (Phase 3)
- SHx RDY branch (DMA phase)
- Frame tick carry when the CPU drives frames (Phase 3)
- Nightly cache / macos-15 nightly (conditional)
- 60-day scheduled-workflow disable
- Fuzzing the N65V reader
