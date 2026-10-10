# Phase 6: Mapper seam and PPU fetch pipeline - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-10-10
**Phase:** 6-mapper-seam-and-ppu-fetch-pipeline
**Areas discussed:** Fetch-bus fidelity, Split-scroll oracle, A12 / address hook, Seam-plan gate

The owner selected all four areas and attached a generic fan-out brief. The
brief asked for each area to be researched through all relevant role lenses,
with primary sources and an adversarial pass, followed by coherent one-shot
recommendations to adopt without further questions. One advisor researcher ran
per area, under the clean-room rule (documentation and test behaviour only). A
cross-area adversarial pass followed.

---

## Fetch-bus fidelity

| Option | Description | Selected |
|--------|-------------|----------|
| Two-dot access, literal bus | Address/ALE dot, then read dot; read dot rebuilds high bits from live state | ✓ |
| One access per read dot | Address and data on one dot | |

**User's choice:** fan-out recommendation adopted (two-dot).
**Notes:** Matches DEC.10 and NESdev "PPU rendering". It is the only model in which the AccuracyCoin "Hybrid Addresses" and "ALE + Read" tests can pass. Sub-dot ALE/RD and the multiplexer output latches are deliberately not modelled. What would change it: a CTest showing the high bits do not follow a mid-access `v` change.

## Split-scroll oracle

| Option | Description | Selected |
|--------|-------------|----------|
| Self-checking synthetic frame | Direct register pokes at dots; hand-derived per-line origins; plane sampler; ambiguity self-check | ✓ |
| 6502 program + golden hash | End-to-end but circular and brittle | |

**User's choice:** fan-out recommendation adopted (self-checking).
**Notes:** Exact dot edges are tested by the `v` trace cases, not by the frame test.

## A12 / address hook (mapper interface)

| Option | Description | Selected |
|--------|-------------|----------|
| Const ops table + watch mask + A12 level-change event | Board applies its own filter; derived page tables rebuilt, never serialised | ✓ |
| Inline switch, MMC3 filter in core | Fewer indirect calls, but core edited per board | |

**User's choice:** fan-out recommendation adopted (ops table).
**Notes:** What would change it: more than about 3% frame-time cost on the slowest runner. Dispatch would then become a `switch` in a `static inline` helper, with the semantics unchanged.

## Seam-plan gate

| Option | Description | Selected |
|--------|-------------|----------|
| Reuse existing gates + git checks in plan/verification | `hashes.txt` compare, hash-equality job, protected scoreboard baseline, revision assert | ✓ |
| New repin-policy CTest | Lasting revision coupling, more ceremony | |

**User's choice:** fan-out recommendation adopted (reuse).
**Notes:** Push the seam commit and wait for green CI before pushing plan 2, so `cancel-in-progress` does not cancel the seam run.

## Claude's Discretion

- File split within ppu.c, helper names, the trace hook's form.
- Whether to include the `$2007`-during-rendering increment.
- A performance record before and after plan 2. This is not a gate.

## Deferred Ideas

- The repin-policy CTest.
- The `$2006` copy delay, sub-dot ALE/RD, multiplexer output latches.
- The `PPU_READ` and `CPU_CLOCK` hooks.
- Lazy PPU catch-up.
