# Phase 20 Re-verification Evidence

**Verified:** 2026-10-28

This note records a fresh source-bound evidence pass after the prior report's
417-file bundle. It supplements the historical evidence in `20-EVIDENCE.md`;
the previous runs and bundle were not reused as proof for the current source.

## Current source and hosted runs

| Item | Verified value |
|---|---|
| Candidate | `7848cc8980e8e96a1fc6f5ec223a9783fb7ef19d` |
| Tested merge | `716e8d49c80602e3524b9348881ddf171fe33a2f` |
| Protected base | `ab9c2e6eb822c06f0dfb0f08a0ca42ec541358be` |
| PR | #59, open and ready for review |
| CI | Run `40748417351`, attempt 1: completed successfully, 18/18 jobs |
| Nightly | Run `40740261760`, attempt 1: completed successfully; seven work jobs passed and the PR-only report job was skipped |
| Effective source | 418 files, SHA-256 `87ee34cf7b5ba45d87e748ab69b69f929a7fe7be96c02ed044bf9b1394f93202` |
| Hosted manifest | SHA-256 `0d9a5bd74f43b2546a2d01534387e2167615a8b1ade84a705edffad87c2478e3` |
| Hosted verdict | `hosted-pass`, SHA-256 `84867fe8bfc1292323e0f0cc88e9b906b1b66eb947dbd9290f934b1943c68f22` |
| Evidence file inventory | SHA-256 `06606c6c57bc90e0a2524278b0863eaa732a56c9f309c6a4025f7ac08169577e` |
| CI run metadata | SHA-256 `b9987afb0fecbaf6d97b2d02d25148c6a6de67d7b0d16ae1e8309187695fc5fa` |
| CI jobs metadata | SHA-256 `48116e8dcd76d4501e4f24c7480d2e2429f7bef5d24ed0e9184a6fd38ba0802d` |
| CI artifacts metadata | SHA-256 `0df02df1ae4f4944e9678a376e721585022f5ce09fc1483c67e091a3f4fbe4bc` |

The production collector completed from the clean checkout at
`/tmp/nesturbator-phase20-refresh-source` using a new, single-attempt evidence
root at `/tmp/nesturbator-phase20-refresh-single-attempt-evidence`. Its
`manifest.txt` records `source_count=418`, matching
`source_inputs_sha256=87ee34cf7b5ba45d87e748ab69b69f929a7fe7be96c02ed044bf9b1394f93202`.
The accepted bundle was copied to `build/phase20-refresh-hosted-evidence/`;
`MODE=validate` against the absolute working-tree path exited 0 with
“hosted phase20 evidence revalidated”.

The accepted run's AccuracyCoin artifacts are:

| Platform | Artifact ID |
|---|---:|
| Linux arm64 | `12841464168` |
| Linux x64 | `12841499565` |
| macOS arm64 | `12843551846` |
| macOS x64 | `12842179196` |
| Windows arm64 | `12842034373` |
| Windows x64 | `12842707464` |

Other relevant CI artifacts are the six platform hash files (`12843481085`,
`12842433368`, `12842859228`, `12842064459`, `12842523212`, `12842658184`),
performance measurements `12841204824`, vector inventory `12842881189`, and
RetroArch progression/frame evidence `12842802455` and `12842163317`. The
complete run/attempt/artifact mapping and each downloaded archive digest are
retained under `build/phase20-refresh-hosted-evidence/provenance/`.

Nightly run `40740261760/1` supplied vector-full artifact `12838279254`,
MMC3-oracle artifact `12837969461`, and execution-fuzz artifact `12839246719`.
The Nightly run/job/artifact JSON and raw logs are retained beside the CI
records.

## Source identity and behavior evidence

`build/phase20-refresh-source-modes.tsv` contains 418 `(path, mode, blob SHA)`
entries and has SHA-256
`98f1978ae7a3ead9f8ce22b62a4f6d5950165abeddb03523e57bfe696e4fce7f`. All 418
entries matched the current Git index and the candidate, CI-tested, and
Nightly-tested Git trees. The full trees and index contain 1,238 blobs; this
check covers every entry in the collector's 418-file effective-source set.

The fresh candidate scoreboard has these required target rows passing:

```text
accuracycoin/$2002 flag timing        pass
accuracycoin/NMI Control              pass
accuracycoin/NMI Suppression           pass
accuracycoin/NMI Timing                pass
accuracycoin/NMI at VBlank end         pass
accuracycoin/NMI disabled at VBlank    pass
```

The clean hosted campaign records 22/22 pages and 146/146 scored rows, with
114 pass, 32 fail, zero skipped/not-run/timeout, and five draw. It is bound to
tested revision `716e8d49c806`; its `source_dirty=0`, 56-input fingerprint is
`424db30d3e712a95c6ebacd7785b710ca67beff078269dd9346c654d390ecc84`, and its
report SHA-256 is
`29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac`.

Each of the six Linux/macOS/Windows x64/arm64 hash artifacts contains 114 rows
and has SHA-256
`1a06cbabea1fee522e023a405af4474726ad24ef798d067b5e27575cf505bc62`. Each
lane's protected-main record has 108/108 PASS rows. The production performance
recomputation passes the unchanged +1% limit (largest increase +0.51%). Fresh
Nightly records 258/258 full-vector tests and 13/13 MMC3 tests passing. Real
RetroArch 1.22.2 reaches 16 intended frame boundaries with
`VERDICT_OK`; drop/double-advance and wrong-origin controls reject as expected.

Fresh local validation used `cmake --workflow --preset ci`; the retained log
`build/phase20-refresh-regression.log` records 688/688 tests passing in 434.48
seconds and generates the library, runner, and libretro packages. Direct named
checks passed for `ppu.nmi_races`, `core.nmi_poll`, `core.nmi_state`,
`core.state_fork.nmi`, `runner.state.nmi_roundtrip`,
`runner.state.nmi_idempotent`, `runner.state.nmi_wrong_load`,
`core.state_reject`, `state.format_pin`, `hash.inventory.self_test`, and
`accuracycoin.nmi_targets` (runner CTest fixture setup passed where selected).
The independent circuit probe also exited 0 with six fresh-state finite-margin
pulse cases passing and no exact ties claimed. `libretro.runahead` exceeded the
10-second bounded spot-check window, so that isolated invocation was stopped;
the same fresh full-workflow log records the named test passing in 35.19
seconds. Its successful run, plus the fresh hosted RetroArch
save/load/rewind/runahead evidence, verifies the libretro replay path.

Source inspection confirmed the production `$2002` read composes the VBlank
bit sampled at M2 rise with the other status bits at M2 fall, keeps read side
effects in the production decoder, and recognizes mirrored status addresses.
The NMI line, edge history, pending request, and instruction poll are distinct
per-instance fields; CPU service consumes the polled request without erasing
an edge captured during interrupt entry. State serialization validates and
restores that history. Format 2 / behavior revision 8 remain pinned, and the
state rejection tests refuse format 1. The direct named checks above passed;
the full workflow also passed the libretro/state replay tests.

All 12 `verify.artifacts` queries passed: 41/41 declared plan artifacts were
present and substantive. The key-link query's text matcher reported misses for
C/CMake call edges; direct call-site and test tracing confirmed the production
bus-to-PPU-to-CPU path, state serializer/loader, runner and libretro save/load
paths, CTest registration, and hosted collection/validation path are wired.
The full suite and exact hosted bundle verify the data flows from guest
execution, production save-state serialization, native runner hashes, and
GitHub run artifacts rather than static or mock values. Decision coverage is
24/24. The four Phase 20 requirements are covered by the verified roadmap
truths; no Phase 20 requirement is orphaned.

## Retained failed collection and bounded limitations

The earlier mixed-attempt collection remains at
`/tmp/nesturbator-phase20-refresh-evidence/`; its raw log remains
`build/phase20-refresh-collect.log`. It was rejected because CI artifact
`accuracycoin-evidence-windows-x64-40740261875-1` predates the supplied run
attempt. The failed collection was not reused or rewritten. A fresh CI attempt
with matching artifact identities was collected and accepted above; no retry
budget, collector rule, or acceptance policy was relaxed.

The review gate found no new reproducible defect in the nine reviewed files.
CR-01 (recurring collector comparison does not enforce Git modes), WR-03
(recursive source-root scan can see ignored build output), and WR-04 (duplicate
Nightly function definition) remain explicitly deferred maintenance items.
The current 418-entry mode/blob comparison supplies run-specific evidence for
this accepted bundle; the TSV and review/disposition are retained for
independent audit. WR-01 and WR-02 retain their fixed identities.

The exact paired-chip physical tie winner remains unmeasured; the documented
deterministic tie policy does not claim otherwise. The clean-room prohibition
against GPL/LGPL source or unmanifested ROM/BIOS bytes remains an explicitly
flagged judgment-tier limitation. The automated hygiene and provenance checks
passed, but this evidence does not silently claim a complete provenance proof.

The workspace's `.planning/config.json` still has SHA-256
`62a22f6862efb7c74ee31baf2f22bae32d453e79aaec6726dbcf3b7213d47167`.
