# Phase 21 hosted evidence ledger

## Plan 21-09 production acceptance (2026-10-29)

The canonical Phase 21 hosted collection completed with `verdict=hosted-pass`.
The production collector accepted the required run, job, artifact, campaign,
performance, conformance, host and source identities. `MODE=validate` then
revalidated the immutable bundle against the current checkout. No hosted run,
PR, branch, workflow, merge or delivery metadata was changed. The draft PR,
INT-01, INT-02, KEEP-01 and phase completion remain open for independent phase
verification and parent-owned lifecycle updates.

## Source and delivery identities

| Identity | Accepted value |
|---|---|
| Repository / PR | `szTheory/nesturbator`, PR `61` (open, draft, unmerged) |
| Delivered candidate | `54c7db29c73a66bb552e71b0887529249a41a072` |
| Actual tested synthetic merge | `faf1b9159ce8dae83cd2c08c155fc9b77fac32de` |
| Protected base | `a64c23fd3ca7caed7e3ad102ff1a9beaa29fa562` |
| CI | run `40929011059`, attempt `1`, `pull_request`, completed success, 18/18 jobs |
| Nightly | run `40929010997`, attempt `1`, `pull_request`, completed success; seven work jobs passed, the PR-only report job was skipped by its declared condition |
| Effective source | 428 inputs, SHA-256 `004e963df11ad77f080e422ea0b87dab5c060117d38ff9b43d330116b974479e` |
| Hosted collection | retrieved `2026-10-29T19:06:59Z`; `manifest.txt` SHA-256 `2ae6a3d6878e99d310e52621e8653df4269d8825e122f62a5452504641dc49a5`; `verdict.txt` is `hosted-pass` |

The delivery snapshot is `build/phase21-hosted-evidence/availability/availability.txt`
(SHA-256 `890082b1dac1b056b1b376a9f520947c2cdf604935cd62eea07db1d07a9e2629`).
It binds PR, candidate, tested merge, base, run/attempts, all six build jobs,
the 16 selected artifact IDs and 428-input digest. Its 20-file API metadata
manifest is `1d5cb3c5657ad787addcc12bd4a1b1c6dce344b9bb13aa61cc2d4d5f89453777`.
The corresponding retained run, job, artifact, PR and tree snapshots are under
`build/phase21-hosted-evidence/provenance/`; their bounded identity digests are
in `manifest.txt` and the files are listed in
`provenance/evidence-files.sha256` (SHA-256
`a1cf7640fb0b4f2816043b80e2a76e68f903d65f953cecf834a72d1ed400b260`).

The independent 428-row Git-mode/blob list is
`build/phase21-hosted-evidence/effective-inputs.txt` (SHA-256
`d62c2f7e3f5b27653f2edcd9f9fef77d272bef6465b8afb161f8b76186899fe3`). It
matches all 428 effective inputs in the candidate and tested-merge Git trees.
An independent `phase21_source_identity.cmake` observation also matched the
current checkout's HEAD, index and worktree and the existing absolute clean
source checkout's HEAD, index and worktree, all 428/428:

| Checkout | Source-identity manifest | SHA-256 |
|---|---|---|
| Current checkout | `source-modes-blobs-current-accepted.tsv` | `74fff8838bfd15feeb85cf8f6e4d920916c7535d0c393270751e440535a6d0b8` |
| Clean source at `build/phase21-clean-source` | `source-modes-blobs-clean-accepted.tsv` | `b11cb67a1a8e84c045a661cecfb7999bdeed3bcd25bed493cd49ed7762182e7b` |

The tracked `.planning/**` tree, including the preserved `.planning/config.json`,
is excluded from the effective hosted input inventory. The configuration file
still has its required SHA-256 `62a22f6862efb7c74ee31baf2f22bae32d453e79aaec6726dbcf3b7213d47167`.
No effective production, test, pin, workflow or collector input changed during
collection or evidence closeout.

The exact source snapshots have matching Git trees: the tested merge has the
protected base and candidate as its two parents and its tree equals the
candidate tree. The complete API run/job/artifact records remain available in
`provenance/ci-run.json`, `ci-jobs.json`, `ci-artifacts.json`,
`nightly-run.json`, `nightly-jobs.json`, `nightly-artifacts.json`, and
`pr.json`; their hashes are recorded in `manifest.txt`. The six build job IDs
are also bound in the availability snapshot and the per-job source records
`provenance/build-*-job.txt`.

## Collection and validation

1. Read-only availability returned `available` from the retained
   `publication/check-availability.cmake` helper before artifact collection.
2. The public `MODE=collect` entry point rejects the sibling clean-source and
   canonical bundle paths because its existing root guard only permits a root
   under `${SOURCE_REF}/build` or `/tmp`. To preserve the canonical bundle root
   and the absolute clean-source mitigation, the same production
   `nesturbator_phase20_collect_evidence()` function was called through the
   ignored scratch helper `publication/collect-production.cmake`. No
   production collector file or acceptance check changed.
3. `XDG_CACHE_HOME` was pointed at writable temporary storage for the `gh run
   view --log` cache. The default user cache location is read-only in this
   sandbox; GitHub authentication/configuration was unchanged.
4. The production current-checkout command exited zero:
   `cmake -DMODE=validate -DROOT="$PWD/build/phase21-hosted-evidence" -DSOURCE_REF="$PWD" -P tests/cmake/phase21_hosted_evidence.cmake`.

The validated bundle records source digest/count, PR/run/attempt/artifact
identities, all selected content digests and every evidence-file digest.
`provenance/effective-inputs.tsv` hashes to the same accepted effective source
digest; `provenance/source-identity.txt` records the seven-field delivery
identity and hosted retrieval time.

## Hosted results

| CI platform | Build job ID | Hash artifact | AccuracyCoin artifact | Phase 21 evidence controls |
|---|---:|---:|---:|---:|
| Linux x64 | `123642730325` | `12921817792` | `12922122127` | pass, 36.02 s |
| Linux arm64 | `123642730792` | `12922760907` | `12922720606` | pass, 32.96 s |
| macOS x64 | `123642730371` | `12923111750` | `12923235886` | pass, 104.99 s |
| macOS arm64 | `123642730311` | `12922965566` | `12921592932` | pass, 46.03 s |
| Windows x64 | `123642730079` | `12922895516` | `12921439506` | pass, 49.51 s |
| Windows arm64 | `123642730207` | `12920514938` | `12922631506` | pass, 63.31 s |

All six Phase 21 control suites finished under the unchanged 120-second limit.
Each lane's source-bound AccuracyCoin evidence retained the 114 freshly
authenticated protected-main PASS keys and all eleven exact v5 target rows.
The six platform hash files each contain the complete 114-row frame, PCM,
APU-transition and state inventory and are byte-identical to
`tests/runner/hashes.txt`, SHA-256
`76f999668c5bf584074a37f050a19cb4c31f9c63b6671b06b85388e9950be850`. The
actual `hash-equality` and `CI required` jobs both succeeded. Holy Mapperel's
`holymapperel.m2.decode` and `runner.boot_to_play` passed on all six build
lanes.

The clean Linux x64 campaign reports `source_revision=faf1b9159ce8`,
`source_dirty=0`, ROM pin `6bee919529289c555803b612b8c9e4f7eaaeeabb`,
library `0.1.11`, behavior revision 9 and the unchanged 1800-frame execution
cap. It completed all 22 pages and 146/146 scored rows: **119 PASS / 27 FAIL**,
zero skip/not-run/timeout and five inventory-only DRAW rows. All 114 starting
protected keys remain PASS, and all eleven named v5 targets pass. Its report
SHA-256 is `56ebd01adb97b3916fde36f8739602284d951a1fd00d84285bb4ef441fb463a3`;
the 56-input campaign fingerprint is
`bf09fe6ebb319fbb668abb65f70e5142bc1477a0dc70f391a305d7f3e97c136a`.

The unchanged hosted performance gate passed with the original zero-spread
base controls and +1% per-ROM limit. The real `phase20-perf-40929011059-1`
artifact (ID `12920863587`, ZIP SHA-256
`e989d141819124cb704f65b1ffc267cf582506902275c002084d2595ab5933cf`) binds
base `a64c23f` to tested head `faf1b91` and recomputes from the retained raw
60/360-frame Valgrind samples:

| ROM | Mapper | Base Ir/frame | Head Ir/frame | Delta |
|---|---:|---:|---:|---:|
| nesteroids | 0 | 23,914,932 | 24,103,968 | +0.79% |
| runner | 1 | 22,203,492 | 22,392,202 | +0.84% |
| M4_P256K_C256K | 4 | 21,998,505 | 22,215,976 | +0.98% |

The performance artifact records GCC 14.2.0 and Valgrind 3.22.0; every ROM
passes with the existing margins and no `n/a`, single-run substitute or
threshold adjustment.

Nightly full-vector JUnit/inventory records **258/258 passed, zero failed or
skipped** (artifact `12922705132`); MMC3 Oracle records **13/13 passed, zero
failed or skipped** (artifact `12920713402`). Both artifacts bind the same
tested source and Nightly run/attempt. The required Linux no-FP, ASAN, host
session, title, hygiene, RetroArch end-to-end, vector-inventory, performance,
hash-equality and rollup jobs all passed within the 18/18 CI result.

The actual RetroArch marker artifact `retroarch-marker-progression` (ID
`12922506555`) binds the tested source to RetroArch 1.22.2. It records the
intended sequence reaching 16 boundaries and `VERDICT_OK`, plus passing
drop-advance, double-advance and wrong-origin controls with their expected
rejection reasons. The hosted marker is `complete`; its bounded artifact is
`artifacts/retroarch/hosted-evidence.txt`.

## Raw required artifact archives

All selected archive downloads are retained under
`build/phase21-hosted-evidence/downloads/`. Each SHA-256 below matches the
GitHub API digest and the corresponding `provenance/artifact-<ID>.sha256` file;
run/attempt binding is captured in the artifact JSON and availability record.

| Run | Artifact | ID | Download SHA-256 |
|---|---|---:|---|
| CI 40929011059/1 | `hashes-linux-x64` | `12921817792` | `d50660d9de034ebb3e32d8d16371e3e34450fada6f6e632f8e31619cc03d0e3d` |
| CI 40929011059/1 | `accuracycoin-evidence-linux-x64-40929011059-1` | `12922122127` | `2839894c05cec1f7cbd21518cf69f5d2a06f8bb00702ac5ccdfde3ac35e148ea` |
| CI 40929011059/1 | `hashes-linux-arm64` | `12922760907` | `c482536127b66a323073b030bc9620f0422270eefe200d4d36a8c78b9d55e339` |
| CI 40929011059/1 | `accuracycoin-evidence-linux-arm64-40929011059-1` | `12922720606` | `b62a28ab997a4271b6799e0fa1d6fac1f71ad4872fb1aa7427d4e05e692ffdd2` |
| CI 40929011059/1 | `hashes-macos-x64` | `12923111750` | `3cc8f7e2df492b06e945a73772fad7d5813dec81bccfb9e46f8ae73d4a61c60b` |
| CI 40929011059/1 | `accuracycoin-evidence-macos-x64-40929011059-1` | `12923235886` | `d46c836c828df77186e7189c6b72ea0f9f1587b55c42da01e20331f50cd05a77` |
| CI 40929011059/1 | `hashes-macos-arm64` | `12922965566` | `23db7d11296959a5abf11b105e9bc77ca0162465e4710ebdcd89d3d5401adb76` |
| CI 40929011059/1 | `accuracycoin-evidence-macos-arm64-40929011059-1` | `12921592932` | `3241a0a652608fe984d588a9120030e1d12c1ac360fa6001696e7f3acce5779e` |
| CI 40929011059/1 | `hashes-windows-x64` | `12922895516` | `af3eaebcf4068d3c86eb3488450875e2cf0af7f1928fdd7ec9581c205b827787` |
| CI 40929011059/1 | `accuracycoin-evidence-windows-x64-40929011059-1` | `12921439506` | `663d810c7a0f818af028b04f1b7629929291a800a45c6328f703c5a78890455d` |
| CI 40929011059/1 | `hashes-windows-arm64` | `12920514938` | `4815423981948ab8bfeef2ce808bb2452224801d72dc2960f033ffb6346df45d` |
| CI 40929011059/1 | `accuracycoin-evidence-windows-arm64-40929011059-1` | `12922631506` | `820b89db07a1d70d91233fa81f2bc21f99aa8ee1dfda0dbd13634fd4f484f7e7` |
| CI 40929011059/1 | `phase20-perf-40929011059-1` | `12920863587` | `e989d141819124cb704f65b1ffc267cf582506902275c002084d2595ab5933cf` |
| CI 40929011059/1 | `retroarch-marker-progression` | `12922506555` | `6a175837016d18f10ef34310f3923efb493c847920e3e5e68f5322993eebb7f6` |
| Nightly 40929010997/1 | `vectors-full-evidence-40929010997` | `12922705132` | `406e124ca081e924e5ba7b736aa9bf31fd5c7ba6d35e7e1058830f40103a5509` |
| Nightly 40929010997/1 | `mmc3-oracle-evidence-40929010997` | `12920713402` | `4d7ece6bd7f710ef1106e9f59808b89cc4917abbd9d568484fc4785655ff8968` |

## Interrupt replay and host evidence on the accepted source

The source-bound local native replay matrix reaches 20 production entry
cutpoints: BRK C2–C7, IRQ C1–C7 and NMI C1–C7. Public native save/load
continuation compares remaining production bus operations and cycle times,
PC/status/stack, request consumption, canonical saved bytes, guest markers,
pixels, valid stereo PCM, frame metadata and APU transition history. Fixtures
retain nonzero synthesis, filter and sample history; interleaved instances have
distinct histories. Omitted-history, skipped-load and wrong-origin controls
diverge at their expected bus locations while clean siblings match.

The actual dynamically loaded libretro module replays the same 20 cutpoint
classes through `retro_unserialize`/`retro_run`, at runahead depths 1–3 and
under both existing input scripts (60 class/depth and 120 script/depth cases).
It checks committed/speculative state and ticks, converted video, stereo PCM,
frame metadata and guest markers. Four adapter controls reach the intended
comparison verdicts: stale state (`0x0E`), skipped unserialize (`0x1E`), omitted
entry history (`0x1C`) and wrong origin (`0x0E`). The held/released inhibited-
NMI extension adds four active-C6 / idle-after-C7 histories across scripts and
depths (24 cases), requires initial serialization to echo every byte, and
checks the first-frame IRQ/NMI marker outcomes. These are test-runner
observations on the accepted effective source, not physical zero-margin tie
measurements.

The hosted `retroarch-marker-progression` artifact independently records the
actual RetroArch 1.22.2 session reaching all 16 intended boundaries and
`VERDICT_OK`; dropped-advance, double-advance and wrong-origin faults produce
their expected rejections. Hosted ASAN and both Linux no-float jobs are among
the passing CI jobs; local final-source workflows separately passed ASAN
686/686. Full-vector and MMC3 Nightly inventories pass 258/258 and 13/13.

## Causal history and acceptance limits

The earlier published candidate `5cf4c9cf056682f79daeafc7e35a38215903e930`
is historical and failed its actual CI attempt: performance regressed by
2.88%, 3.10% and 3.19%, and three platform control jobs reached the unchanged
120-second cap. The core/helper repair `319916fb3868aa0343d887419a22ad96cd551e8a`
restored performance and reduced the source-identity control's measured Git
process count from 1285 to seven per inventory. Its Windows run exposed a
CRLF-sensitive unresolved-index fixture. The final fixture-only candidate
`54c7db29c73a66bb552e71b0887529249a41a072` writes explicit LF and verifies raw
bytes and constructed stages; its actual CI and Nightly attempts are the only
hosted acceptance recorded above. The failed and canceled attempts remain
historical; no source-point or earlier-run substitution was used.

The accepted source retains state format 3, behavior revision 9, ABI 1 and the
existing interrupt-history byte. Final frame, PCM, APU-transition and state
inventory digests are unchanged from the pinned 114-row inventory; candidate
`54c7db2` changes only the evidence fixture, so no state-layout or CPU/APU
timing change is attributed to that delivery correction. The final campaign
has no incidental row movement from its immediately preceding measured
campaign: report SHA-256 remains
`56ebd01adb97b3916fde36f8739602284d951a1fd00d84285bb4ef441fb463a3`, with
119 PASS, 27 FAIL, five inventory-only DRAW and all 146 scored rows terminal.

Finite-margin source ordering constrains IRQ/NMI takeover and coherent vector
pairs. The global integer equal-timestamp policy is deterministic model
behavior only; the physical zero-margin CPU/PPU tie winner remains unmeasured.
The clean-room GPL/LGPL prohibition remains flagged-unverified judgment-tier
intent; no additional human sign-off is claimed. Collector issues CR-01
(recurring mode enforcement), WR-03 (recursive scan/relative clean-source
handling) and WR-04 (duplicate validation) remain open. This run's independent
428-row mode/blob comparison, absolute clean-source collection and
current-checkout validation mitigate this evidence instance only and do not
repair those debts. The PR remains open, draft and unmerged; INT-01, INT-02,
KEEP-01 and phase completion are left for independent parent verification.

