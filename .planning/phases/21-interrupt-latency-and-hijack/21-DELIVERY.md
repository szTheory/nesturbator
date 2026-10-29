# Phase 21 hosted delivery, 2026-10-29

## Execution and acceptance closeout

Plan 21-09 subsequently completed production collection and current-checkout
validation for the repaired delivery below. All 16 required artifact ZIPs match
their API digests; candidate, tested merge and both checkouts match all 428
effective modes/blobs, including a separate actual-filesystem mode check.
Independent phase verification passes 32/32, and INT-01/INT-02/KEEP-01 are
complete. Fresh parent CI passes 695/695 in 382.16 seconds and packages all three
deliverables. See `21-EVIDENCE.md`, `21-EXECUTION-EVIDENCE.md`,
`21-VERIFICATION.md` and the review disposition for accepted facts and limits.

The execution closeout remains local. PR #61 stays draft and unmerged; no push,
ready transition, workflow dispatch or next lifecycle command ran. Next command:
`$gsd-ship 21`. The debug/preparation statuses and next-command instructions
below are historical. WR-01 and Phase 20 CR-01/WR-03/WR-04 remain deferred;
physical exact-tie and judgment-tier provenance limits remain visible.

## Hosted regression repair qualification

The debug repair is published on the same draft PR #61 as
`54c7db29c73a66bb552e71b0887529249a41a072`. Actual final pull-request attempts
are [CI `40929011059/1`](https://github.com/szTheory/nesturbator/actions/runs/40929011059)
and [Nightly `40929010997/1`](https://github.com/szTheory/nesturbator/actions/runs/40929010997),
both completed successfully. CI passed all 18 jobs; Nightly passed all seven
work jobs, with its report job skipped by the declared pull-request condition.
The tested merge is
`faf1b9159ce8dae83cd2c08c155fc9b77fac32de`, with protected base `a64c23fd3ca7`
and the repair candidate as its two parents; its full tree equals the candidate,
`e27d0dd1cabf913f3c76c36c3fa59906ecb6e14d`.

The core/helper repair is `319916fb3868aa0343d887419a22ad96cd551e8a`.
Its actual hosted performance gate passed, but both Windows builds exposed
native CRLF in the new synthetic Git conflict fixture. Followup `54c7db2`
changes only that fixture to explicit LF and verifies its bytes and exact
constructed stages; an explicit CRLF mutant is rejected locally. Earlier
repair CI `40927374281/1` was canceled after those two test failures when the
correction was published. Its raw facts and performance success are retained,
and its Nightly `40927374013/1` remains historical. No failed or canceled
attempt supplies final acceptance.

The enlarged cycle had prevented GCC 14 from specializing dynamic PPUSTATUS
work and folding an unobserved timestamp. Constant ordinary/status cycle
arguments, one remaining 15-tick advance, transition-first NMI handling and
operation-specific IRQ acceptance remove that overhead while preserving the
interrupt and state histories. The evidence helper batches its independent
committed/index/raw-worktree inventory instead of launching Git three times
per path. Its measured work budget rejects the old helper (1285 launches versus
seven), and independent semantic controls cover divergent blobs, index modes,
staged-only paths, spaces/raw CRLF, symlink target bytes, missing inputs and
unresolved stages. Neither the +1% performance threshold nor the 120-second
control timeout changed.

Final corrected-source local CI passes 695/695 in 392.75 seconds with all
three packages. Full CPU vectors pass 258/258 in 8.65 seconds, MMC3 passes
13/13 in 1.72 seconds, hygiene passes 13/13 in 45.49 seconds, and ASAN passes
686/686 in 187.20 seconds.
The full CI invocation uses fresh protected-main authority
with exactly 114 PASS keys. Its campaign and eleven-target gates preserve
119 PASS / 27 FAIL, all 146 scored rows terminal and five DRAW rows. The report
digest remains `56ebd01adb97b3916fde36f8739602284d951a1fd00d84285bb4ef441fb463a3`;
the refreshed 56-input fingerprint is
`bf09fe6ebb319fbb668abb65f70e5142bc1477a0dc70f391a305d7f3e97c136a`.

Exact local Linux GCC14/Valgrind 60/360-frame comparisons pass the original
gate at +0.79045%, +0.84991% and +0.98857%. Reverting only constant status
dispatch in a scratch object/archive makes M4 fail at +2.200265%; reapplying
restores +0.988571% exactly. The canonical source and CI/ASAN builds were
untouched during that causal experiment. Independent read-only source review
reports no findings.

The final corrected-source hosted performance job also passes with these exact
counts. Raw artifact `12920863587`, `phase20-perf-40929011059-1`, has downloaded
ZIP SHA-256
`e989d141819124cb704f65b1ffc267cf582506902275c002084d2595ab5933cf`, matching
GitHub's digest; its identity binds the final candidate, tested merge, base and
attempt. The unchanged gate revalidates its raw samples locally. Final Windows
x64/ARM64 controls pass in 49.51/63.31 seconds under the original 120-second cap;
all 679 tests pass on each. Linux x64/ARM64 controls pass in 36.02/32.96 seconds,
and macOS ARM64 in 46.03 seconds. macOS Intel passes all 691 tests, with controls
in 104.99 seconds. The original three timeout environments now all pass.

The clean hosted Linux campaign records `source_dirty=0`, the final tested
merge, 119 PASS / 27 FAIL / five DRAW, all 114 authenticated protected-main PASS
keys and all eleven exact targets. Its report/fingerprint match the local
values above. All six platform native hash artifacts are byte-identical
to the existing pinned inventory, SHA-256
`76f999668c5bf584074a37f050a19cb4c31f9c63b6671b06b85388e9950be850`.
The actual six-platform hash-equality job and CI-required rollup both pass.

The new semantic-control script increases the effective tracked inventory to
428 inputs. Canonical and absolute no-hardlink clean clone manifests match
byte-for-byte, SHA-256
`b11cb67a1a8e84c045a661cecfb7999bdeed3bcd25bed493cd49ed7762182e7b`.
Independent candidate/tested-merge mode/blob inventories also match all 428
paths, SHA-256
`d62c2f7e3f5b27653f2edcd9f9fef77d272bef6465b8afb161f8b76186899fe3`.
The clean source checkout is `build/phase21-repair-clean-source/`; the new
delivery preparation root is `build/phase21-repair-hosted-evidence/`, with
bare-SHA `candidate.txt`, the exact seven-field `delivery.json`, and raw
run/PR snapshots. Earlier failed delivery bytes remain in the original bundle.

All 16 required raw artifact ZIPs are retained with API-matching SHA-256 and
source-bound identities in `raw-artifacts/inventory.json` under that new root.
The production read-only availability function returns `available` for the
final attempts and 428-input effective digest
`004e963df11ad77f080e422ea0b87dab5c060117d38ff9b43d330116b974479e`.
No `MODE=collect` or content-acceptance ledger step ran.

The canonical Plan 21-09 handoff now selects this repaired delivery:
`build/phase21-hosted-evidence/candidate.txt` contains the supported bare final
SHA, its `delivery.json` contains the exact new seven-field identity, and
`effective-inputs.txt` records all 428 tracked modes/blobs. The existing
`build/phase21-clean-source/` clone was updated through normal fetch/detached
checkout to the final candidate, with empty status; no checkout was recreated.
Its manifest matches the canonical checkout byte-for-byte at qualification.
Canonical read-only availability also passes; its ready record is
`build/phase21-hosted-evidence/availability/availability.txt`. The supported
library-entry probe is
`build/phase21-hosted-evidence/publication/check-availability.cmake`; it retains
the absolute clean-source mitigation without changing collector debt.
Original failed selectors, availability snapshot and rejection probe are
archived under `publication/failed-5cf-selectors/`. The first-repair `319916f`
selectors are separately retained in the new root's `publication/source-319916f/`.

This debug delivery supersedes earlier preparation source pins for the next
step; it does not execute acceptance. Next command: `$gsd-execute-phase 21`
to resume Plan 21-09 using these canonical inputs. INT-01, INT-02 and KEEP-01,
the phase, and the draft PR remain open. Final delivery metadata commits remain
local and source-equivalent to the qualified published candidate, avoiding
unnecessary workflow retriggers during this debug step.

State format 3, revision 9, ABI 1, preserved execution history, config edit,
physical-tie limits and collector CR-01/WR-03/WR-04 remain unchanged. No merge,
ready transition, Plan 21-09 execution, phase completion or other lifecycle
step is part of this debug step. The historical initial delivery follows.

## Historical initial delivery

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
