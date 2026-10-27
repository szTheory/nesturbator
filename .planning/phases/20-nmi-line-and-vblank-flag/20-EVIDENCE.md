# Phase 20 local evidence ledger

This ledger records the local result for Plan 20-06. It does not claim hosted
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

The phase remains in progress. NMI-01 and NMI-02 are the only requirements
closed by local plan evidence; KEEP-02 and KEEP-03 remain unchecked.
