# Phase 15 perf gate: zero-spread proof (D-15)

Date: 2026-10-13
Head SHA of both runs: af7fc66ee36327c217639ab7b11363ab378f07db (branch gsd/phase-15-speed-and-close-out)

## Rule

D-15: arm the gate only if, in at least two separate `ci.yml` workflow_dispatch runs with `control: true`, every ROM shows delta exactly 0 and the two base measurements agree (base_a == base_b; a mismatch shows as an UNSTABLE row). A control run builds the same commit twice and compares it with itself.

Gate-mode rule for missing sides (D-10, D-14): once armed, only the head side fails when missing. A base that cannot be built or run shows `n/a` and does not fail the pull request. An UNSTABLE base still fails.

## Method notes

Both binaries run from equal-length paths (`bin/base`, `bin/head`) under `env -i PATH=/usr/bin:/bin`, so instruction counts do not depend on path or environment size (research finding 1). Counts are measured at 60 and 360 frames and reported per frame.

## Run 1

- Run: https://github.com/szTheory/nesturbator/actions/runs/38342366027 (perf job: success)
- Runner image: ubuntu-24.04, version 20261004.327.1
- valgrind: valgrind-3.22.0
- gcc: gcc-14 (Ubuntu 14.2.0-4ubuntu2~24.04.1) 14.2.0

| ROM | base Ir/frame | head Ir/frame | delta | status |
|---|---|---|---|---|
| nesteroids | 23743914 | 23743914 | 0 | ok |
| runner | 24651894 | 24651894 | 0 | ok |
| M4_P256K_C256K | 23319733 | 23319733 | 0 | ok |

## Run 2

- Run: https://github.com/szTheory/nesturbator/actions/runs/38343146470 (perf job: success)
- Runner image: ubuntu-24.04, version 20261004.327.1
- valgrind: valgrind-3.22.0
- gcc: gcc-14 (Ubuntu 14.2.0-4ubuntu2~24.04.1) 14.2.0

| ROM | base Ir/frame | head Ir/frame | delta | status |
|---|---|---|---|---|
| nesteroids | 23743914 | 23743914 | 0 | ok |
| runner | 24651894 | 24651894 | 0 | ok |
| M4_P256K_C256K | 23319733 | 23319733 | 0 | ok |

The counts are identical across the two runs as well. No UNSTABLE row appeared in either run.

Verdict: zero spread
