---
phase: 15-speed-and-close-out
verified: 2026-10-13T00:00:00Z
status: passed
score: 4/4 must-haves verified
behavior_unverified: 0
gaps: []
warnings:
  - "WR-01 (open): perf_compare.cmake gate mode passes a missing head as n/a when the base is also missing; contradicts the documented rule. Does not defeat the goal truth (see below)."
  - "WR-02..WR-04 open, non-blocking."
---

# Phase 15: Speed and close-out Verification Report

Phase goal: speed is visible on every PR, a slowdown cannot land unseen, milestone closes with a measured nightly timeout and docs matching released code.

## Truths

| # | Truth | Status | Evidence |
|---|---|---|---|
| 1 | `--bench N` prints fps and p50/p90/p99/p99.9/max; `--bench-state` adds save/load; ci test shows no hash change | VERIFIED | Ran `nesturbator-run --rom ... --bench 30 --bench-state`: printed frames/fps, frame_us, save_us, load_us lines with all five percentiles. ctest `runner.bench.nohash` (compares plain vs bench hashes), `runner.bench.percentile`, and 20+ `runner.usage.bench.*` tests pass. README documents it (lines ~1061-1090). |
| 2 | Every PR runs a Cachegrind job comparing Ir/frame head vs merge base on NROM, MMC1, MMC3 ROMs, table in job summary | VERIFIED | `.github/workflows/ci.yml` job `perf` (line ~321): builds base and head, runs valgrind cachegrind at 60 and 360 frames, `perf_compare.cmake` writes the table, `tee -a $GITHUB_STEP_SUMMARY`. ROMs fixed in perf_compare.cmake (nesteroids m0, runner m1, M4_P256K_C256K m4). Rolled into `ci-required` via needs; perf skipped only on non-PR events (rollup test passes). The "report mode does not fail" clause is superseded by the armed gate, as SC3 intends. |
| 3 | +1% per-ROM gate armed only after zero spread proof; deliberate slowdown passes with a reviewed allow line | VERIFIED | `15-PERF-PROOF.md`: two control runs (38342366027, 38343146470), all deltas 0, base_a == base_b; ci.yml invokes `-DMODE=gate`. `tests/perf/allow.txt` (cap, expiry <= 60 days, reason). 15 `perf.gate_selftest.*` tests pass locally (allowed slowdown, over-cap, expired, malformed, UNSTABLE, missing head, etc.). |
| 4 | Nightly suite-flake timeout ~2x measured cold run; pins, runner labels, AccuracyCoin pin reviewed; README/header describe released code | VERIFIED | nightly.yml suite-flake `timeout-minutes: 38`; samples 871/1112/959 s from three cold dispatch runs, ceil(2*1112/60)=38. `15-CLOSEOUT.md` reviews all action pins (one behind, left to Dependabot, none open), six runner labels, AccuracyCoin pin kept with backlog 999.2. `hygiene.timeout_comments` test passes. README documents bench and gate. |

## Requirements coverage

| ID | Plans | Status |
|---|---|---|
| PERF-02 | 15-04, 15-05, 15-06 | SATISFIED |
| PERF-03 | 15-02, 15-03 | SATISFIED |
| TUNE-10 | 15-01, 15-07, 15-08 | SATISFIED |

All three IDs appear in plan frontmatter and in REQUIREMENTS.md (marked complete, mapped to Phase 15). No orphaned requirements.

## Checks run by the verifier

- Local ctest in `build/ci` for bench, perf, timeout_comments, rollup labels: 44/44 passed.
- Manual `--bench 30 --bench-state` run: correct output shape.
- Hosted CI: run 38357342480 (head 03cc5df) and 38345355652 (head 199e830) both conclusion success.
- Full 629/629 suite not re-run by me; relied on orchestrator report.

## WR-01 judgment (explicit)

Confirmed real in code: in `perf_compare.cmake` the `base_a == "" OR base_b == ""` branch precedes the `head == ""` branch, so in gate mode with base unmeasured and head unmeasured the ROM is `n/a` and exits 0. This contradicts the file header, ci.yml comment and README, and no selftest covers it.

It does NOT undermine the goal truth "a slowdown cannot land unseen":
- With the base unmeasured, the gate cannot compute any slowdown whatever the head is; that case passes by the documented design (D-10: no base means n/a, never fails), proven by the PERF-PROOF rule. So the head-missing hole adds no slowdown that would otherwise have been caught.
- The ordinary path (base measured, head missing) fails correctly (`missing_head` selftest passes) and a measured slowdown over +1% fails.
- The residual risk is narrow: a PR that breaks the runner under Cachegrind at the same time the merge base cannot be built or run. Build breakage would still fail the `build` jobs.

Classification: WARNING, not BLOCKER. It is a documentation/behavior mismatch and a small fail-open window. Recommend fixing before merge (move the head check first in gate mode, add `MISSING head:... base:...` selftest) as a cheap follow-up.

## Other open review warnings (non-blocking)

- WR-02: `;`/`[` in allow-file reasons break parsing (fails closed with a misleading message, hits PRs that need an exception).
- WR-03: inherited SIG_IGN behavior undocumented in README/stop.h (CLAUDE.md rule 6 documentation gap).
- WR-04: 1-minute timeouts on release-please/hygiene etc. sit near a single sample; flakiness risk, not goal failure.

## Anti-patterns / human verification

No debt markers checked beyond review findings; no human verification items. Status: passed, with warnings above.
