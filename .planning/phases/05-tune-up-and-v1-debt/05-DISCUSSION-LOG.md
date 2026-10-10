# Phase 5: Tune-up and v1 debt - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-10-10
**Phase:** 05-tune-up-and-v1-debt
**Areas discussed:** Local RetroArch tests, Soft-reset semantics, Skip gate mechanism, Release-policy exactness

The owner selected all four areas. They asked for a multi-lens research pass on
each, with an adversarial review, and for the synthesized recommendations to be
adopted. Four advisor researchers ran in parallel.

---

## Local RetroArch tests

| Option | Description | Selected |
|--------|-------------|----------|
| Remove local registrations; driver required-only in hosted job | No self-skipping test; hosted job is the sole RetroArch check | ✓ |
| Fix the local run on the Mac | GUI-session/Metal abort, not scriptable; still skips on 5 runners | |
| Opt-in CMake option | Dormant second path that rots | |

**User's choice:** adopt research recommendation.

## Skip gate mechanism

| Option | Description | Selected |
|--------|-------------|----------|
| Ban skip properties via `ctest --show-only=json-v1` policy test | Runs inside the ci workflow, parallel-safe, no new step | ✓ |
| `--output-junit` + parse skipped | CI-only step, version-dependent semantics | |
| Parse LastTest logs | Fragile; failed log never lists skips | |
| `--no-tests=error` only | Already set; covers zero tests only | |

**User's choice:** adopt research recommendation.

## Release-policy exactness

| Option | Description | Selected |
|--------|-------------|----------|
| Scoped line extraction + exact equality + deny-list | One canonical string per line, no parser | ✓ |
| Golden copy of publish job | Churns with every step or pin edit | |
| Mini YAML parser in CMake | Bug farm for a one-line invariant | |
| Hash of the block | Opaque failures, blind regeneration | |

**User's choice:** adopt research recommendation.

## Soft-reset semantics

| Option | Description | Selected |
|--------|-------------|----------|
| PPU reset flag cleared at scanline 261 dot 1 | Matches nesdev; window depends on reset position | ✓ |
| Fixed 29,658-cycle window | Power-on figure, wrong for mid-frame reset | |
| Reset runs 7-cycle sequence through bus | One path for later board hooks; timeline monotonic | ✓ |
| Instant reset (set state directly) | Diverges from hardware timing | |

**User's choice:** adopt research recommendation. Synth history kept; revision stays 4; load/power-on unchanged.

## Claude's Discretion

Parallel job count, labels, nightly flake job placement, ccache measurement and
decision, action pin bumps, AccuracyCoin pin review, the synthetic iNES builder,
and API baseline updates.

## Deferred Ideas

Power-on write-ignore window and startup sequence; retroarch-e2e artifact reuse;
cpu_reset/apu_reset ROM rows; freezing publish permissions/environment.
