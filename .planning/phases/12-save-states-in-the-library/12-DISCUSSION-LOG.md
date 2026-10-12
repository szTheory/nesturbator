# Phase 12: Save states in the library - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-10-11
**Phase:** 12-save-states-in-the-library
**Areas discussed:** Rejection status codes, Frame numbers after a load, Save/load buffer contract, Format version policy

The owner selected all four areas. They asked for a broad fan-out on each, covering every stakeholder lens with an adversarial pass, followed by one coherent recommendation that is adopted without further questions. Four advisor researchers ran in parallel, and their reports were reconciled into CONTEXT.md.

---

## Rejection status codes

| Option | Description | Selected |
|--------|-------------|----------|
| Three classed codes (FORMAT, CARTRIDGE, CORRUPT) | Researcher's pick. Truncation, flipped byte and out-of-range all give CORRUPT; STATE-04 would be reworded. | |
| One code per check (~7) | Every internal check gets its own code; the enum grows with the format. | |
| Five codes, one per STATE-04 class (FORMAT, CARTRIDGE, SIZE, CHECKSUM, INVALID) | Keeps STATE-04 as written; the set is closed, and each code means a different host message. | ✓ |
| Reuse ERR_ARGUMENT for every refusal | The buffer-contract researcher's side suggestion; conflicts with STATE-04. | |

**User's choice:** Adopt the synthesized recommendation (five codes).
**Notes:** The choice widened the researcher's three classes to five, so the requirement did not need changing. The check order puts identity before length, so another game's state reports "wrong game". JAM is a saved latch, never a save or load status.

---

## Frame numbers after a load

| Option | Description | Selected |
|--------|-------------|----------|
| Restore frame_number and ticks; runner indices absolute | A state is a point on one timeline; `--frames` is an end frame. | ✓ |
| Reset on load; runner indices relative | `--frames N` means N more frames; this breaks the STATE-03 tick comparison and counters. | |

**User's choice:** Adopt the synthesized recommendation.
**Notes:** One change from the research: `--state-at-frame` and `--hash-state` also accept N = K, the loaded frame, for a cheap round-trip check. `--movie` with `--load-state` exits 2 until STATE-06.

---

## Save/load buffer contract

| Option | Description | Selected |
|--------|-------------|----------|
| size_t getter; save capacity at or above the size; load exact length | Matches libretro and the header idioms; no NULL query mode. | ✓ |
| snprintf-style save with a NULL size query and a written out-parameter | One entry point; clashes with the NULL means ERR_ARGUMENT rule; redundant with a fixed size. | |

**User's choice:** Adopt the synthesized recommendation.

---

## Format version policy

| Option | Description | Selected |
|--------|-------------|----------|
| Integer version, exact match, public macro, pinned guard | Bump on any byte change after the first release; free churn before it. | ✓ |
| Tagged skippable sections (PNG/EBML style) | Forward-compatible, but contradicts "refuse, never convert" and risks a silent desync. | |

**User's choice:** Adopt the synthesized recommendation.
**Notes:** A format bump ships as `feat:`, not as breaking. The CI guard is a `state_format` line in `tests/runner/hashes.txt` that must equal the macro.

---

## Claude's Discretion

- Header field order, widths and magic bytes; the section layout.
- Stderr wording per rejection class.
- Test file names and how the test executables are split.
- Whether the version struct also reports the format version.

## Deferred Ideas

- A movie that starts from a state (STATE-06).
- A hygiene check that ties state pins to the format macro.
- A zero-tail tolerance in `retro_unserialize` (Phase 13).
