# Phase 21 hosted delivery, 2026-10-29

The owner authorized publishing a draft PR, waiting for CI and Nightly, and preparing real delivery facts. This delivery is complete; hosted acceptance is blocked by actual CI failures. No merge or additional GSD lifecycle step ran. Plans 21-01 through 21-08 remain locally complete; Plan 21-09 and INT-01/INT-02/KEEP-01 remain incomplete.

## Publication and preserved history

- Draft PR: [#61](https://github.com/szTheory/nesturbator/pull/61).
- Branch: `gsd/phase-21-interrupt-latency-and-hijack`.
- Published candidate: `5cf4c9cf056682f79daeafc7e35a38215903e930`.
- Protected base: `a64c23fd3ca7caed7e3ad102ff1a9beaa29fa562`.
- Actual tested merge: `28b1eefd1184a8d303f2bab7f03c4ec0fc31e199`; its parents are the base and candidate above. Its Git tree is identical to the candidate, `48d72c86d10deeb94d302a44c59ad8bcf52bd292`.
- Current and absolute clean checkout inventories match all 427 tracked inputs. Manifest: `build/phase21-hosted-evidence/source-modes-blobs-publication.tsv`, SHA-256 `0b4c668cb4469f1a945e09d8cb0f2ee7dbe67d5d6cf90ef2cbc78f18a1c4b7b9`.

The first push was rejected by the normal pre-push formatter on 52 historical commit/file pairs. An isolated checkout formatted 19 owned paths with clang-format 18 and published one clean commit based on main. The complete original execution history remains locally reachable at `refs/phase21-history/execution-6c42607`, ending at `6c42607863c78a28b30abd2c053da6b429b3442d`. The working branch now tracks the published history; the user's existing config edit was preserved byte-for-byte. No hook was bypassed.

Fresh publication preflight passed hygiene 13/13 and focused interrupt/state/actual-adapter tests 77/77. The campaign still reports 119 PASS / 27 FAIL, all 146 scored rows terminal, and five DRAW inventory rows. Its report digest is unchanged; formatting changes its 56-input fingerprint to `cb1f1ce5de0bfe13b87e6754a8631a3a8d07365c7df1a62677ab4ef60257906c`. Earlier full local workflow results and original candidate manifests remain historical.

## Completed actual attempts

| Workflow | Run / attempt | Result |
|---|---|---|
| [CI](https://github.com/szTheory/nesturbator/actions/runs/40919233064) | `40919233064 / 1` | Failed: performance and three build-leg control timeouts; rollup failed and hash equality skipped |
| [Nightly](https://github.com/szTheory/nesturbator/actions/runs/40919233254) | `40919233254 / 1` | Success: all seven work jobs passed; PR-only report job skipped by its declared condition |

CI's ASAN, both Linux no-float jobs, Linux x64/arm64 and macOS arm64 builds, both session-control jobs, RetroArch end-to-end, hygiene, title and vector inventory passed. Full six-platform hash equality is not established.

## Repair targets

The measured performance gate failed with the original +1% threshold intact:

| ROM | Base Ir/frame | Head Ir/frame | Delta |
|---|---:|---:|---:|
| nesteroids | 23,914,932 | 24,603,738 | +2.88% |
| runner | 22,203,492 | 22,891,947 | +3.10% |
| M4_P256K_C256K | 21,998,505 | 22,701,479 | +3.19% |

`phase21.hosted_evidence.controls` timed out at 120.12s on macOS x64 (690/691 other tests passed), 120.01s on Windows x64 (678/679), and 120.02s on Windows arm64 (678/679). These are actual test failures, not successful or skipped qualification.

The failed performance artifact is ID `12918181011`, named `phase20-perf-40919233064-1`; downloaded ZIP SHA-256 `e8dc3b84234f204baec2622b875c3de3abaf67c05d5ec065f9106e1fe3211034`. Raw counts, summary and identity are retained under `build/phase21-hosted-evidence/publication/perf-artifact/`. The ZIP, all four failing job logs, raw run snapshots, job snapshots and artifact metadata are alongside it. No performance allowance or test threshold was changed.

## Delivery and next command

`build/phase21-hosted-evidence/delivery.json` contains the exact seven-field schema using the completed attempts above. The production read-only availability function rejects this failed CI run with `incomplete: CI provenance: ci run identity/status/workflow does not match the required source-bound pull-request run`. No Plan 21-09 task or success artifact was created.

`candidate.txt` uses the supported bare-SHA form. The prefixed form exposed a separate existing reader defect: substring offset 18 retains the equals sign. Using the bare form avoids that input-parser defect without changing the collector or accepting failed evidence. The read-only probe is retained at `build/phase21-hosted-evidence/publication/check-availability.cmake`.

Next command:

```text
$gsd-debug Phase 21 hosted CI performance regression and evidence-control timeouts; use .planning/phases/21-interrupt-latency-and-hijack/21-DELIVERY.md
```

Repair and requalify the actual source before resuming Plan 21-09. Preserve finite-margin/physical-tie limits, original causal history, state format 3/revision 9, all protected passes and targets, and the existing collector debt. Keep the PR draft until acceptance and independent phase verification pass.
