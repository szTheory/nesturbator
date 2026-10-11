# Phase 10: Close-out and boot-to-play - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-10-11
**Phase:** 10-close-out-and-boot-to-play
**Areas discussed:** nes-runner intake + movie, RAM byte observation, thresholds + negative controls, BOARD-04 close-out scope

---

The owner selected all four areas. They asked for a fan-out across every
relevant lens, an adversarial pass, and then one coherent set of
recommendations to be followed without further questions. That fan-out was
four advisor researchers, run in advisor mode with the minimal_decisive tier.

## Research outcome that reshaped the phase

Three researchers ran nes-runner v1 on this core. All three found that it never
reaches gameplay:
- the title screen is garbled;
- input has no effect;
- the stack wanders.

The cause is in `src/ppu.c:632`. A `$2000` write with bit 7 set during vblank
retriggers NMI even when bit 7 was already set. The orchestrator confirmed this
in the code. A scratch fix restored play, and nes-runner then also writes its
save. This became D-01 to D-03.

## nes-runner intake + movie

| Option | Description | Selected |
|--------|-------------|----------|
| Committed ROM + C NMOVIE1 writer with pinned jump list | Reuses the `game_movie.c` pattern; deterministic | ✓ |
| Closed-loop RAM-reading bot | Robust to timing drift, but more code and could hide regressions | |
| Build from source / fetch at test time | Needs cc65, Go and a gitignored CSV / network flakiness | |

**Owner question:** "Should nes-runner v1's ROM be committed byte-for-byte?"

| Option | Description | Selected |
|--------|-------------|----------|
| Commit it | MIT, citing licence commit e2d70cf, with FamiTracker driver attribution | ✓ |
| Fetch, don't commit | Like blargg: PR CI needs the network, or the check moves to nightly | |

**User's choice:** Commit it.

## RAM byte observation

| Option | Description | Selected |
|--------|-------------|----------|
| Library-linked C test using `nesturbator_peek_cpu_ram` | No public CLI change; all runs in one process | ✓ |
| Runner `--peek-ram FRAME:ADDR` flag | New public surface and parser; nobody has asked for it | |

Bytes: `$3E` game state and `$5D`–`$60` score, with a layout canary.

## Thresholds + negative controls

| Option | Description | Selected |
|--------|-------------|----------|
| One C driver with a `judge()` bitmask, exact expected masks per control, synthetic mutants | Integer predicates P1 (RAM), P2 (changed pixels), P3 (audio floor + A/B) | ✓ |
| Runner flags + CMake -P script | Widens the CLI; fragile parsing | |

Reconciled: the attract control is menu navigation into Credits, because the
game has no attract mode and an idle title would duplicate the no-input
control. The failed-load control runs in the library, because the runner exits
1 and never falls back to the test frame.

## BOARD-04 close-out scope

| Option | Description | Selected |
|--------|-------------|----------|
| Data-driven: Holy Mapperel audio rows, shared key module, ROM drift guard, per-ROM libretro parity CTests, fold 07-WR-02 | Meets the criterion literally; failures name the ROM | ✓ |
| Audio rows + one looping libretro test | Hides which ROM failed; state bleed; can pass on an empty list | |

05-CR-01 was also folded in, because the reset path breaks "the same hashes
everywhere" and the parity sentinel would expose it.

## Claude's Discretion

- The file split for the check.
- The exact checkpoint frames, K, F and window counts, pinned by procedure.
- An optional libretro PCM comparison.

## Deferred Ideas

- A runner RAM-peek flag.
- A closed-loop movie bot.
- Stallar as a second real-game check.
- The remaining open review items.
- Backlog 999.1.
