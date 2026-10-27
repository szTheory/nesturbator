# Phase 20 local evidence ledger

This ledger records local evidence through Plan 20-09. It does not claim hosted
six-platform completion or close KEEP-02/KEEP-03.

## Campaign and protected PASS evidence

The single final full AccuracyCoin campaign used the existing Linux full-tier
wiring:

```sh
cmake --preset ci -DNESTURBATOR_ACCURACYCOIN_FULL=ON
cmake --build --preset ci
ctest --preset ci -R '^accuracycoin\.campaign$' --no-tests=error --output-on-failure
```

It completed 22/22 pages and all 146 scored rows: 114 pass, 32 fail, 0 skip,
0 not-run, 0 timeout, and 5 DRAW. The canonical ROM is
`6bee919529289c555803b612b8c9e4f7eaaeeabb`, report SHA-256
`29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac`, and the
56-input fingerprint is
`3465f9abcc455f5698f7c444417b496e617af9021a8c648ed5945346754ee3ed`.
Metadata records `source_revision=92db8e2a40a0`, `source_dirty=1`,
`host_elapsed_guest_ns=7758848000`, behavior revision 8, and the unchanged
32/5/1800/41503 frame caps. The dirty measurement is local and retains the
pre-existing `.planning/config.json` edit; it is not a clean-source claim.

The original baseline had all six required rows failing. At that pin they were
`$2002 flag timing` fail `0x06`/521 frames; `NMI Control` fail `0x22`/1463;
and `NMI Timing`, `NMI Suppression`, `NMI at VBlank end`, and `NMI disabled at
VBlank` each fail `0x06`/1463. The final rows all pass at default alignment:

| Full key | Final result | Frames | Page attempt | Owned proof |
|---|---:|---:|---|---|
| `accuracycoin/NMI Control` | pass `0x01` | 1418 | `page-18-cold-01` | Control 8/9 guest markers, real `$2000` writes and instruction-specific poll oracle |
| `accuracycoin/NMI Timing` | pass `0x01` | 1418 | `page-18-cold-01` | Complete ten-cell source vector plus circuit phase projection |
| `accuracycoin/NMI Suppression` | pass `0x01` | 1418 | `page-18-cold-01` | Complete stored vector and source-selected checked/ignored cells |
| `accuracycoin/NMI at VBlank end` | pass `0x01` | 1418 | `page-18-cold-01` | Both executed seven-cell source sequences and captured/polled state |
| `accuracycoin/NMI disabled at VBlank` | pass `0x01` | 1418 | `page-18-cold-01` | Complete stored vector with only source cell x=3 ignored |
| `accuracycoin/$2002 flag timing` | pass `0x01` | 537 | `page-19-cold-01` | Split `$2002` samples, real bus side effects, set/clear dot witnesses |

The exact-key gate, `accuracycoin.nmi_targets`, passed in the final CI workflow.
Its scratch control rejects changed, absent, duplicated, and mistyped full
keys; a failed status reports the specific target row. The full AccuracyCoin
activation, cold protected-main observation, protected verdict, and 108-key
no-lost-pass checks also passed locally. The candidate scoreboard remains
separate from the fetched protected-main snapshot and protects all 108 starting
PASS keys, including the sprite-0 keys. No candidate scoreboard replaced the
fetched authority.

## Executed source sequences and timing controls

The production AccuracyCoin run stored these complete values. Values are
recorded from the guest's result RAM, separately from C comparator controls:

| Source table | Executed values | Source acceptance |
|---|---|---|
| `$2002` clear at PC `$94a0` | `[e0,80,80,00]` | Whole-array alternatives `[e0,e0,80,00]` or `[e0,80,80,00]` |
| `$2002` set at `$53..$55` | Raw `[18,58,78]`; masked `[00,40,60]` | `$53=00`, `$54=00` or `40`, `$55=60` |
| NMI Timing at `$50..$59` | `[03,02,02,02,02,02,02,01,01,01]` | One of the two complete ten-cell vectors, selected by the first sample; the other is `[02,02,02,02,02,02,01,01,01,01]` |
| NMI Suppression at `$50..$59` | `[00,02,02,02,00,01,01,03,03,03]` | Compare x=1,2,4,5,7,8,9; ignore x=3 and x=6 |
| NMI at VBlank end, sequence A, `$50..$56`, PC `$b49b` | `[01,01,01,00,00,00,00]` | Exact seven-cell vector |
| NMI at VBlank end, sequence B, `$60..$66`, PC `$b4bf` | `[01,01,01,00,00,00,00]` | Exact seven-cell vector |
| NMI disabled at VBlank, `$50..$56`, PC `$b4fd` | `[00,00,00,00,01,01,01]` | Compare every cell except x=3 |

Owned test oracles reject mixed per-sample shifts, the wrong `$2002` bit mask,
wrong suppression values, either altered VBlank-end sequence, and a changed
non-ignored disabled cell. Selected production-path wrong-model controls reject
set sampling one dot late, clear sampling one dot early, merging all `$2002`
bits at M2 rise, revoking a captured request when the line falls, inventing an
edge from an unchanged `$2000` enable, dropping the second RMW write, or
servicing NMI before the Control 8 `LDX`. Pre-close transient and widened-M2-only
pulses are also missed by the production close sampler as required.

The independent finite-margin circuit cases retain all six outcomes:

| Waveform | Close samples | Captured request | Poll result | Classification |
|---|---|---:|---:|---|
| `before-fall` | asserted then released | 1 | serviced | One retained capture |
| `after-fall-phi1-short` | low, low | 0 | none | No capture |
| `phi2-short-released-before-close` | low, low | 0 | none | Transient ended before close; no capture |
| `widened-m2-only` | low, low | 0 | none | Pulse overlaps M2 window only; no capture |
| `held-through-next-close` | low then asserted | 1 | retained for following poll | One capture at the next close |
| `no-pulse` | low, low | 0 | none | No capture |

The circuit inputs are recorded finite-margin waveforms. The owned C test feeds
their pin levels to production bus-cycle close samples; it does not replay the
full external bus or analog waveform, model NMIP transistor/output propagation,
or prove exact paired-chip setup/hold precedence. The PPU status edge at M2
rise and later `$2002` status composition at M2 fall are separately witnessed.
The final global integer-timestamp convention is addressed bus side effect,
detector capture, then tied PPU transition; CPU poll reads captured history at
phi2 open. This is the selected deterministic model policy, not an observed
physical CPU/PPU tie winner or an accuracy waiver. No per-test phase offset was
applied and no VBlank dot was moved. The earlier Plan 20-02 ordering and its
first divergences remain historical in `tests/accuracy/measurements.md`.

## Exact state and public host replay

The Plan 20-04 executed internal cutpoints are reached from separately labeled
constructed origins using complete production CPU instructions:

| Cutpoint | Constructed origin → executed endpoint | Recorded state and result |
|---|---|---|
| Enabled VBlank set | `240:333 → 241:1`, one JMP | Status/control `80/80`; previous/captured/polled `0/0/0`; APU phase 25,344 and synth level 11,540; canonical save/load/save and future guest, video, tick, PCM, and native-state equality |
| Enabled VBlank clear | `260:333 → 261:1`, one JMP | Status/control `00/80`; history `1/0/0`; same nonzero audio history; canonical state and continuation equality |
| Control 8 set before poll | `240:331 → 241:2`, `STA $2000` | PC `$8003`; status/control `80/80`; history `1/1/0`; APU phase 33,792 and synth level 11,540; following LDX reaches the expected NMI guest marker |
| Control 8 clear before poll | `260:330 → 261:1`, `STA $2000` | Status/control `00/80`; history `1/1/0`; captured request survives the low line; restored continuation reaches the expected marker |

The bounded Control 8/9 sweep covers set and clear transitions at offsets
-2 through +2, all 20 trials. Every set trial reaches its expected marker;
only clear offset -2 records a captured-before-poll request and then reaches
the marker. Control 9 retains both `$2000` RMW writes and its six-cycle `INC`;
at offsets -1 through +2 its handler-entry X is 0, while at -2 it is 16. A
CRC-correct omitted-pending control changes the guest marker from 1 to 0 and
loading it over an actually pending instance clears that stale latch. Format 1
is refused; malformed histories reject atomically. The disabled-NMI
constructed-origin probe remains limited evidence, not a claim that reset
reaches those constructed coordinates.

Plan 20-05 separately verifies public frame-boundary save/load. Core fork and
run-ahead cover frame 0 through successive NMI frames, independent instances,
changed future inputs, and speculative depths 1–3. Runner roundtrip and wrong-
load cases compare suffix frame, tick, PCM, transition, state, and guest marker
results. Libretro saves/restores at each displayed boundary at depths 1–3 in
both future-input modes, with visible frame and PCM checks. These host-boundary
results do not substitute for the exact instruction-boundary race cutpoints.

## Local verification

| Command | Result |
|---|---|
| `ctest --preset ci -R '^accuracycoin\.nmi_targets$' --no-tests=error --output-on-failure` | 1/1 passed, including four mutated scoreboard controls |
| `ctest --preset ci -R '^(runner\.write_hashes(\.content)?|hash\.inventory\.self_test|state\.format_pin)$' --no-tests=error --output-on-failure` | 4/4 passed; generated inventory equals checked-in inventory |
| `ctest --preset ci -R '^(accuracycoin\.(baseline_fetch|regression_controls)|hash\.inventory\.self_test|state\.format_pin)$' --no-tests=error --output-on-failure` | Baseline-fetch initially rejected an invalid provenance value containing `=`; the value was corrected. Rerun: baseline-fetch 1/1 passed; regression-controls, protected observation, inventory self-test, and format pin passed in the initial invocation. |
| `cmake --workflow --preset ci` | 683/683 passed; library, runner, and libretro packages produced |
| `cmake --workflow --preset asan` | 675/675 passed |

CI ran with `NESTURBATOR_ACCURACYCOIN_FULL=OFF`; the final measured campaign was
not duplicated by either workflow. Tests exercised CPU vectors, MMC3,
Holy Mapperel, boot-to-play, state/JAM matrices, host guards, and protected-pass
controls in their existing placements.

## Hosted obligations still pending

No hosted run, job, or artifact identity was obtained in this local plan. The
following evidence remains required at the exact final commit before KEEP-02,
KEEP-03, or full Phase 20 completion can be claimed:

- All six supported platform jobs must compare the same complete frame,
  PCM, transition, and state hash inventory and retain protected PASS keys.
- The existing per-ROM performance lane must show no regression above +1%,
  with its zero-spread controls and named-ROM comparison; local timing is not
  hosted performance evidence.
- Existing hosted CPU-vector, MMC3, Holy Mapperel, boot-to-play, and real-host
  checks must pass where the repository configures them. The Nightly
  `vectors-full` fetch remains unresolved: STATE records three upstream-fetch
  failures on 2026-10-26, with no confirming successful run/artifact recorded.
- A clean-source full campaign and identified exact-head artifacts remain
  outstanding. The measured local candidate remains `source_dirty=1`.

The phase remains in progress. All four Phase 20 requirements remain pending
for independent phase-level re-verification; KEEP-02 and KEEP-03 also remain
unchecked.

## Phase 20 Plan 07: split sprite-status sample witness

The owned `ppu.nmi_races` test starts after the ninth in-range sprite Y read at
scanline 0 dot 129 with eight secondary-OAM slots occupied. Independent PPU
advances witness overflow clear at dot 130 at both +9 M2 rise and +12 phi2
open, then set at dot 131 before the +24 M2 fall. Each actual decoder read
begins from that same dot-129 fixture and reaches dot 132 in one 24-tick CPU
cycle. Through both `$2002` and `$3FFA`, it returns `0x35`: early vblank is
clear, late sprite overflow is set, and low I/O bits are `0x15`. The test
checks the address-latch reset, open bus, I/O bus and age, and exactly one
cycle. Separate eighth-slot and ninth-Y-out-of-range controls stay clear.

The bounded wrong-sampler experiment temporarily captured `PPUSTATUS & 0x60`
at M2 rise and supplied those bits to the close-time status result while
preserving bit 7, the one-read side effects and the cycle count. The test
rejected the mutant only at the intended assertion for each alias:
`value == 0x35u` failed with `0x15 != 0x35`; no setup, compile, crash, or
unrelated assertion failed. The temporary `src/bus.c` edit was restored, and
`git diff --exit-code -- src/bus.c` passed before the restored focused tests.

The one full-enabled campaign completed 22/22 pages and 146/146 scored rows:
114 pass, 32 fail, no skip/not-run/timeout, and 5 DRAW. Measured metadata is
`source_revision=af268f348619`, `source_dirty=1`,
`host_elapsed_guest_ns=8625836000`, fingerprint
`152511afaec8cc129d7259eb49e5a72a11fb0e94adf9ed1757fc9526f0e77d84` over 56
inputs, and report SHA-256
`29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac`. Dirty
provenance includes the preserved `.planning/config.json` and `.planning/STATE.md`
edits; the local candidate is not clean-source evidence. The exact six target
keys pass, and fresh protected observation/verdict plus the 108-key no-lost-pass
check passed. `runner.write_hashes.content` passed with no hash inventory change.

| Command | Outcome |
|---|---|
| `cmake --build --preset ci && ctest --preset ci -R '^(ppu\.nmi_races|ppu\.registers|ppu\.sprites|core\.nmi_edge|vectors\.api_policy)$' --no-tests=error --output-on-failure` | 5/5 passed before the task commit and again after restoring the production source. |
| Full-enabled exact AccuracyCoin regex | Campaign, six-target gate, protected observation/verdict, 108-key no-lost-pass, and hash-content checks passed; activation initially rejected the stale committed fingerprint, as expected until this refresh. |
| Full-enabled campaign | 22/22 pages completed and trustworthy metadata/report retained under `build/ci/accuracycoin/full/`. |
| AccuracyCoin regex after metadata refresh with full option off | Activation, exact-six target, protected PASS, no-lost-pass, and content checks passed without another campaign invocation. |
| `ctest --preset ci -R '^accuracycoin\.baseline_fetch$' --no-tests=error --output-on-failure` | 1/1 passed after replacing the forbidden `=` in `source_dirty_reason` with “dirty bit 1”; measured provenance stayed unchanged. |
| `cmake --workflow --preset ci` | Passed 683/683 after that correction, then packaged the library, runner, and libretro adapter. The preceding full run had one `accuracycoin.baseline_fetch` parser failure; the isolated rerun and complete workflow passed after the metadata fix. |

The documentation corrects WR-01: `$2000` disable and pre-render clear lower
the live line without revoking a captured request. The changing-bit witness and
bounded mutant close WR-02 locally. No public API, state format, behavior
revision, ROM, or scoreboard key changed. Equal-time behavior remains the
selected deterministic model order and does not establish physical exact-tie
precedence. Hosted exact-head/cross-platform/performance evidence, Nightly
`vectors-full`, and the clean-room audit remain unresolved; KEEP-02/KEEP-03 and
all four project requirement checkboxes remain pending.

## Phase 20 Plan 08: current local backstop evidence

The single full-enabled local campaign for Plan 20-08 completed 22/22 pages and
all 146 scored rows: 114 pass, 32 fail, 0 skip, 0 not-run, 0 timeout, and 5
DRAW. The six required target keys passed. Generated metadata records
`source_revision=df10aafd2099`, `source_dirty=1`,
`host_elapsed_guest_ns=8570616000`, a 56-input fingerprint of
`1d324c83a1aac5a69939eecc149cf4e0e7b86b7175f1da26064969fa7cd26c0c`, and
report SHA-256
`29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac`. Dirty
provenance retains the uncommitted README/header comments and pre-existing
`.planning/config.json` edit. Native hashes, campaign caps, behavior revision
8, state format 2, protected-main authority, and the exact six target set were
not repinned or widened.

| KEEP-02 fallback backstop | Current local evidence and scope |
|---|---|
| Adjacency: equal-tick frame/audio/state identities remain distinct | `hash.inventory.self_test` accepted the six-platform synthetic fixture with `dabg/p0/frame 60`, `dabg/p1/frame 60`, and `state/dabg 60` all at 42,886,728 ticks, each with a different digest. PCM and transition full keys also remained distinct. An all-six duplicate retained row count and identical artifact bytes but was rejected by the expected full-key inventory; all-six and single-platform missing-key controls rejected for their expected inventory/artifact reasons. This proves local validator behavior only, not observed hosted equality. |
| Frame 0 and empty PCM retain canonical state and nonzero saved audio history | `core.nmi_state`'s `test_frame_zero_empty_pcm_preserves_history` saves the same frame-0/tick-72 state at the executed `(241,1)` endpoint with sample phase 25,344, mixed level 11,540, no PCM, and null output pointer. Poisoned-destination load/save was canonical; both empty drains returned zero without touching the sentinel or state. Restored continuation matched guest marker, state, frame/ticks, video, and nonzero 798-sample PCM. Resetting synth history on a third loaded instance was detected in saved state and diverged at frame 1 sample 0. The internal constructed origin is not reset reachability or a completed public frame. |
| Deterministic sorted inventory rejects equal keys | The full native 114-row sorted inventory and existing duplicate/missing controls remain intact. The new six-platform fixture specifically proves expected full-key rejection even when every artifact has equal count and identical bytes. |

`cmake --workflow --preset ci` is the final local behavior gate. It runs with
the full campaign option disabled so the already measured campaign is not
repeated. Neither the synthetic fixture nor local workflow establishes hosted
six-platform equality, hosted performance, clean-source exact-head, or physical
tie results. KEEP-02/KEEP-03 and all four project requirement checkboxes remain
pending for independent phase re-verification.

## Phase 20 Plan 09: hosted evidence preparation

The collector in `tests/cmake/phase20_hosted_evidence.cmake` accepts
`MODE=collect|validate`, `ROOT`, and `SOURCE_REF`. For an authorized delivery,
`ROOT/delivery.json` contains exactly the repository identity, CI and Nightly
run IDs/attempts, candidate head SHA, and pull-request number. Collection reads
GitHub run/job/artifact metadata and logs, downloads retained content, and
validates that content through the same validator as the offline fixture
controls. It writes raw metadata, retrieved artifact checksums, source
identities, a complete input manifest, and the resulting verdict under ROOT.
The collect operation issues only read-only `gh api`, `gh run view --attempt
--log`, and artifact-download requests. Missing or invalid delivery details
produce `pending_delivery`; fixtures cannot produce hosted provenance. The
current task has no authorized delivery IDs and therefore has no hosted-pass
manifest.

The local effective-source digest is
`684085be7b2ecccc55f30eb885387a111aecfb622950da3184a84bce1d489c43` over 415
files. The digest sorts the Git-index plus nonignored working-tree paths,
hashes regular file contents and symlink targets, and excludes only
`.planning/**` and `tests/accuracy/measurements.md`. This includes the complete
effective source/configuration/test/workflow set and is separate from the
narrower 56-input AccuracyCoin fingerprint. The measured campaign's causal
source revision is `f85e1b15f176`, with `source_dirty=1`; the measured effective
source tree after local preparation is commit `392b8f5995e4992dcc55f818d6e6b25d8ae9b9df`.
The full digest and count are the values Plan 20-10 must match against
GitHub API trees before accepting runs.

GitHub pull-request builds use a synthetic merge commit. A successful hosted
identity must bind that tested SHA to exactly the candidate head and protected
base as its parents, and verify full effective source content across all six
build lanes and companion CI/Nightly evidence. It must validate each exact NMI
key, the original 108 protected PASS keys on every lane, complete native hash
inventories, actual `CI required`/hash-equality/RetroArch marker evidence, all
three raw base/head performance rows with zero base spread and no regression
above 1%, and same-source Nightly vectors-full/MMC3 inventories and JUnit.
Push/dispatch runs, `n/a` performance rows, and fixture self-comparisons do not
establish those claims. Existing physical exact-tie limitations, descriptor-less
clean-room prohibition, unclassified NMI assumptions, local dirty provenance,
and Nightly fetch debt stay visible; the hosted source set remains pending
Plan 20-10.

The single full-enabled local campaign completed 22/22 pages and 146 scored
rows: 114 pass, 32 fail, no skip/not-run/timeout, and 5 DRAW. It used fingerprint
`ebdaef6447e2ad737de5367bc85eec6fca365d8070953a3b58663c6d5262d864` over 56
inputs, report SHA-256
`29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac`, and
`host_elapsed_guest_ns=5958989000`. After metadata refresh with the full option
off, the focused acceptance run passed 9/9, including all six target rows, the
cold protected-main observation, the 108-key no-lost-pass check, and unchanged
native hashes. `cmake --workflow --preset ci` passed 684/684 and generated all
three local packages. These are local preparation results, not hosted results.
All four requirements, KEEP-02/KEEP-03 and Phase 20 remain pending.

## Phase 20 Plan 11: collector repair and local campaign preparation

Plan 20-11 repairs the production collector's supported `gh api` usage,
validates actual per-job workflow-execution results, preserves raw archive and
log provenance while normalizing complete root or single-prefix archives, and
adds metadata-only `MODE=availability` through shared run/job/artifact/source
validators. Offline fake-gh adapter and availability tests traverse the real
production entrypoint. The controls assert intended refusals for incomplete
inventories, missing matrix jobs, missing/expired/duplicate/wrong-run/old-
attempt artifacts, absent freshness, wrong run/workflow/candidate/base/merge
parents, truncated or changed trees, local source drift, substituted job
attempts, archive/log duplicates and ambiguity, unsafe or missing content,
registration-only/empty/failed/skipped results, unsupported CLI/auth errors,
and stale-success retries. Availability fixtures reject archive/log calls and
produce no content acceptance files.

The refreshed local AccuracyCoin campaign used causal source revision
`ac8456b60272` with `source_dirty=1` for the preserved planning config and
execution-state edits. All 22 pages and 146 rows completed: 114 PASS, 32 FAIL,
zero skip/not-run/timeout, and 5 DRAW. Each of the six Phase 20 NMI target keys
passed. The campaign report SHA-256 is
`29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac`; the
56-input fingerprint is
`b2cdcdd15785b590a415dac911187b8abfa0d025a1cc448d412650d1716f1240`, and
host elapsed guest time is 5,017,232,000 ns. The measured effective source
digest is
`b07b42ac26cd92f698eec7a643bc77d5c3c4193ae9432b6f80032affdfabdede` over 416
files. The focused Task 1 gate passed 28/28 and the Task 2 gate passed 12/12,
including the protected 108-key observation/no-lost-pass checks and unchanged
native hash inventory.

The full `cmake --workflow --preset ci` passed 686/686 tests in 394.85 seconds
and generated library, runner, and libretro packages. This includes the 20-11
controls, adapter and availability CTests. These are local controls and
measurements, not hosted acceptance. The new effective tree needs fresh
clean-source hosted runs; previous green CI and Nightly identities remain
historical availability for their earlier candidate and digest. All four
requirements, KEEP-02/KEEP-03, and Phase 20 remain pending independent
verification. No 20-10 or 20-12 execution occurred.
