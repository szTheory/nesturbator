# Phase 15: Speed and close-out - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-10-12
**Phase:** 15-speed-and-close-out
**Areas discussed:** Bench output and clock, Cachegrind job shape, Gate arming and allow file, Close-out sweep breadth
**Mode:** advisor (calibration `minimal_decisive`). The owner selected all four areas and asked
for a research fan-out per area and one coherent recommendation each, followed automatically.

---

## Bench output and clock

| Option | Description | Selected |
|--------|-------------|----------|
| Monotonic clock per platform | `clock_gettime(CLOCK_MONOTONIC)` / QPC in a runner module | ✓ |
| C17 `timespec_get` | Only `TIME_UTC`, which can step | |
| Nearest-rank percentiles | Integer, always a measured value | ✓ |
| Interpolated percentiles | Needs float, reports values never observed | |
| stderr `key=value` lines | stdout stays hashes only | ✓ |
| stdout output | Breaks the stdout-is-hashes rule | |

**Reconciled by Claude:**
- The researcher proposed a fixed warm-up and `--frames` conflicting with `--bench` while
  still tying hash frames to `--frames`. This was replaced with `--bench N` as a run mode over
  frames 1..N, with no warm-up, so frame numbering matches `--frames`.
- A save/load failure is exit 1, not 5.
- Decimal places are fixed at three for µs and two for fps.

## Cachegrind job shape

| Option | Description | Selected |
|--------|-------------|----------|
| Difference of two run lengths | `(Ir(360) - Ir(60)) / 300`, no source change | ✓ |
| `--toggle-collect=<fn>` | Fragile under inlining and renames | |
| Client requests | Needs vendored `valgrind.h`; violates rule 1 | |

**ROMs:** nesteroids (NROM), runner.nes (MMC1), hm/M4_P256K_C256K (MMC3). The base is the
first parent of the test-merge commit.

## Gate arming and allow file

| Option | Description | Selected |
|--------|-------------|----------|
| Arm in Phase 15 after recorded zero-spread proof, plus per-PR base repeat | Closes criterion 3 safely | ✓ |
| Report only, arm later | Leaves PERF-02 criterion 3 unmet | |

**Reconciled by Claude:**
- The proof builds the same commit in two directories, covering "identical builds", not only
  "same binary twice".
- The script is read from head, not from the base checkout. This avoids a bootstrap problem,
  and review plus the self-test cover the risk.
- `CI required` must tolerate `perf` skipped on non-PR events (push, `workflow_call`).

## Close-out sweep breadth

| Option | Description | Selected |
|--------|-------------|----------|
| Measure all unmeasured timeouts + hygiene check | Same dispatch runs; prevents drift | ✓ |
| suite-flake only | Leaves "initial" timeouts at release | |
| Fix WR-01..03, defer IN-01..04 | Serves close-out without scope creep | ✓ |
| Confirm-and-record pins/labels; no AccuracyCoin bump | Bump re-baselines hashes | ✓ |

**Reconciled by Claude:** for WR-01 the researcher suggested `sigaction`. The Phase 14 decision
(C17 `signal()`, no `sigaction`) stands. The fix restores an inherited `SIG_IGN` and saves
`errno` instead.

## Claude's Discretion

- Module and file names, the comparison script language, and the plan split.

## Deferred Ideas

- Bump AccuracyCoin.
- Phase 14 IN-01..04.
- `--bench` with `--load-state` or `--movie`.
- Machine-readable stdout bench output.
- Lazy PPU schedule (SEED-004).
