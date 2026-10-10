# Phase 7: UxROM, CNROM and AxROM - Discussion Log

> **Audit trail only.** Do not use this as input to planning, research or
> execution agents. The decisions are in CONTEXT.md; this log keeps the
> alternatives that were considered.

**Date:** 2026-10-10
**Phase:** 07-uxrom-cnrom-and-axrom
**Areas discussed:** Holy Mapperel in the tree, reading the 0000 code, loader
rules this phase. Board modules was added as Claude's call.

**Owner's instruction:** fan out on every decision point across all relevant
roles, run an adversarial pass, and then adopt one coherent set of
recommendations without further questions. Another copy-paste is better than
another dependency. Prefer primary sources.

**Mode:** advisor, `minimal_decisive` tier, four `gsd-advisor-researcher`
agents in parallel. The orchestrator ran the adversarial pass against the code.

---

## Holy Mapperel in the tree

| Option | Description | Selected |
|--------|-------------|----------|
| Commit 3 release ROMs raw, with manifest lines | Byte-identical files and a sha256 chain to the release. About 16 KB packed in git history. | ✓ |
| A packed or stripped form, a ca65 build, or a nightly-only fetch | Smaller tree, but needs a decoder or a toolchain, or breaks DEC.20. | |

**Choice:** the raw three ROMs, without the twins (D-01 to D-03).

## Reading the 0000 code

| Option | Description | Selected |
|--------|-------------|----------|
| A test-only decoder that reads the `--dump-frame` PPM | Glyph match in the test tree; the runner is unchanged. | ✓ |
| A `--holy-mapperel` runner mode | One command, but puts third-party ROM knowledge in the shipped runner. | |
| A frame-hash match as the oracle | Gives no diagnosis, and re-pinning could hide a failure. | |
| Reading the nametable or RAM through internals | Violates "from the frame". | |

**Choice:** the test decoder, with exit statuses 0, 1 and 2, plus a hash pin as
the regression check (D-04 to D-08).

## Loader rules this phase

| Option | Description | Selected |
|--------|-------------|----------|
| A per-board profile, accepting exactly {0, 2, 3, 7} | The real size and submapper shape of each board. Phases 8 and 9 add rows. | ✓ |
| A generic numeric range | Smallest diff, but mapper 1 or 4 would load with no board. | |

**Choice:** per-board profiles. The adversarial pass added:
- `mapper_load` fails loudly when no board matches (D-10).
- The power-on PC comes through the mapped pages. Checked in the code:
  `src/cartridge.c:218` reads the PC from the end of the file (D-11).

## Board modules (Claude's call)

| Option | Description | Selected |
|--------|-------------|----------|
| One file per board | The NROM pattern, with a small shared mirroring helper. | ✓ |
| One `mapper_discrete.c` that switches on the id | A switch inside each hook, which the seam was built to avoid. | |

**Correction:** a researcher cited another emulator's default. That citation was
dropped; the sources are NESdev and BOARD-01 only (clean-room rule 4).

## Claude's Discretion

- Test file and CTest names, where the PPM reader lives, and message wording.

## Deferred Ideas

- Oversize CNROM, 512 KiB AxROM, CNROM PRG-RAM, tolerance for DiskDude
  headers, mappers 180 and 185, and Phase 6's WR-01 and WR-03 (to be settled
  before Phases 8 and 9).
