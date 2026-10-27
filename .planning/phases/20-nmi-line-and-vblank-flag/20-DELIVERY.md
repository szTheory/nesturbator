# Phase 20 hosted delivery

The owner authorized the delivery needed to unblock Phase 20 on 2026-10-27: push the prepared phase branch and open its evidence PR so PR CI and Nightly can run. This authorizes delivery and collection readiness; phase completion and merge still require their existing evidence gates.

## Formatting repair and original evidence

The first normal push was rejected by the existing pre-push formatter because unpublished C/header snapshots were unformatted. Clang-format 18 was applied to those snapshots without changing their intended behavior. Of 51 outgoing commits, 45 acquired new object identities; 15 distinct source/test files needed formatting. Commit subjects, original author/committer metadata, plan ordering and the published base were preserved. No force push is required. Normal hygiene and formatting hooks remain enabled.

`20-DELIVERY-COMMIT-MAP.tsv` maps every original outgoing commit to its formatted identity. Historical measurement records continue to name their actual original source SHA and fingerprint; the map does not relabel them as runs against formatted source. Original commit objects remain retained by a local backup ref. The source digest from 20-09 is superseded for delivery because formatting and the measured baseline metadata changed effective inputs.

## Remeasured delivery inputs

- Formatting-only source head before metadata refresh: `28092e52803ab73ae9b3537383ca4423b8632a78`.
- One full-enabled campaign: 22 pages, 146 scored rows, 114 PASS / 32 FAIL, zero skip/not-run/timeout, five DRAW.
- Campaign report SHA-256: `29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac` (unchanged).
- Measured input fingerprint: `b154c4a48e17134e1421a7852c52257fd6486c8ceb1a03beff0aa43fb606cab1` over 56 inputs.
- Campaign source revision `28092e52803a`, dirty bit 1; the preserved config edit and a temporary Python import cache contributed to the local dirty observation. That generated cache was removed after measurement. This is local evidence, not clean hosted evidence.
- Focused activation, exact six targets, cold protected observation, all 108 original PASS protections, baseline parser, native hashes and hosted-control checks: 10/10 passed.
- Effective delivery source digest: `cf314a6d3c2494cefba4eb88bd6a779bb93e3d464e408103aa079b320bbb650f` over 415 files. Exclusions remain only `.planning/**` and `tests/accuracy/measurements.md`.
- Full local `cmake --workflow --preset ci`: 684/684 passed in 377.45 seconds; library, runner and libretro packages generated.

## Hosted acceptance

The PR must run CI including actual base/head per-ROM performance, and Nightly on the same tested source. Record the candidate head, synthetic merge/base mapping, run IDs and attempts in `build/phase20-hosted-evidence/delivery.json` once identified. Only completed available results can pass 20-10's read-only precondition. Missing, failed or incompatible results remain pending; original dirty-source and Nightly fetch debt are preserved until actual results resolve their scope. No result is waived by opening the PR.
