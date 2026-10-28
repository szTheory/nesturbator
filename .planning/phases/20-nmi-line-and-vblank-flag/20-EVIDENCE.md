# Phase 20 evidence ledger

## Plan 20-10: accepted hosted content (2026-10-28)

Production `MODE=collect` completed with `verdict=hosted-pass`, followed by
successful `MODE=validate` against the current working checkout. All content
and source gates passed without changing effective inputs, thresholds, pins,
inventories, workflows or runtime. Independent phase re-verification still owns
NMI-01, NMI-02, KEEP-02, KEEP-03 and Phase 20 completion.

### Immutable identities and retained evidence

- Repository: `szTheory/nesturbator`; existing draft PR #59, no remote mutation.
- Candidate: `fe70d619d6a3cf7b80ed1867ff9faf4c7c9053b1`.
- Actual tested synthetic merge: `ef6d9e5ec2c94ddfd431c1d7162f620722a145df`.
- Protected base: `ab9c2e6eb822c06f0dfb0f08a0ca42ec541358be`.
- [CI 40700458574/1](https://github.com/szTheory/nesturbator/actions/runs/40700458574/attempts/1): all 18 jobs successful.
- [Nightly 40700458462/1](https://github.com/szTheory/nesturbator/actions/runs/40700458462/attempts/1): seven test jobs successful; the report job is intentionally skipped for the PR event.
- Effective source SHA-256: `a08da046cedeecbdd28227cc53d54e3d3e7dff2a93ee88aab9ead67da47bf61c`, 417 files. Candidate, merge and current checkout match this entire input manifest.
- Retrieval: `2026-10-28T14:47:48Z`; raw bundle retained at `build/phase20-hosted-evidence/`.
- `manifest.txt` SHA-256: `55857090681bef15f44671c8a0ef5a39fddda2c23b479b45d4802871e83a2386`.
- `verdict.txt` SHA-256: `84867fe8bfc1292323e0f0cc88e9b906b1b66eb947dbd9290f934b1943c68f22` (`verdict=hosted-pass`).
- `provenance/evidence-files.sha256`: `715e1b27b157ab117934bb3d4766ee9fd603e0b981eee442631e0a0a91130e54`, covering 114 content/provenance files. Raw archive/log checksums and normalization mappings are preserved separately.

The read-only precondition checked live exact run attempts, jobs, PR mapping,
unexpired required artifacts and current effective-source equality before
collection or document edits. Initial collection attempts exposed a blocked
GitHub CLI cache path and the collector's relative-path/build-fixture scan
limitations. The successful invocation used `XDG_CACHE_HOME` in build scratch,
an absolute `SOURCE_REF` pointing to a clean local clone at evidence-record
base `445e7e5cb6c266721d355c5bb927c4be7cb5eff8`, and a sibling scratch evidence
root. Its effective bytes match the candidate and tested merge. The accepted
bundle was copied to the path above and revalidated against this working
checkout using an absolute `SOURCE_REF`; failed attempts remain scratch-only.
No collector/source edit, new hosted run, delivery authorization or acceptance
waiver was needed.

### Six-platform content and protected PASS retention

Every lane's selected successful workflow step was rederived from its raw
per-job log and API step time bounds. All required configured sample-vector,
Holy Mapperel and `runner.boot_to_play` CTests actually passed. Fresh
`accuracycoin.protected_observe`, `accuracycoin.protected_verdict` and
`accuracycoin.no_lost_pass` terminals confirm all 108 original protected PASS
keys on each lane. The complete frame/PCM/transition/state inventories contain
114 rows each and are byte-identical, SHA-256
`1a06cbabea1fee522e023a405af4474726ad24ef798d067b5e27575cf505bc62`.

| Platform | Build job | Hash artifact | AccuracyCoin artifact | Protected keys / hash rows |
|---|---|---|---|---|
| linux-x64 | 122859486545 | 12822221056 | 12821766998 | 108/108; 114/114 |
| linux-arm64 | 122859486510 | 12821512434 | 12820934637 | 108/108; 114/114 |
| macos-x64 | 122859486834 | 12822684951 | 12823877579 | 108/108; 114/114 |
| macos-arm64 | 122859486484 | 12823010815 | 12822567333 | 108/108; 114/114 |
| windows-x64 | 122859486522 | 12822670660 | 12821767334 | 108/108; 114/114 |
| windows-arm64 | 122859486841 | 12822735013 | 12821847585 | 108/108; 114/114 |

The clean Linux x64 campaign records `source_revision=ef6d9e5ec2c9`,
`source_dirty=0`, 22/22 pages, 146 scored rows, 114 PASS / 32 FAIL,
zero skipped/not-run/timeout and five DRAW. All six exact NMI target keys pass.
Its report SHA-256 remains
`29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac`;
the 56-input fingerprint is
`9b4d2199c8d6464c16d16f51766174aa64d633fb0a9d8b2968bac55013113993`.
The measured `host_elapsed_guest_ns` is `16457386307`. This independently
confirms the recorded dirty local campaign within its stated scope; local
`source_dirty=1` metadata and causal old/new hashes remain unchanged.

### Performance, conformance and real host

The actual performance artifact `12821532905`, job `122859328349`, binds raw
base/head measurements to the protected base and tested merge above. Production
recomputation agrees with the hosted summary; both base controls have zero
spread for every ROM. There is no n/a, skipped/self-only result, allowlisted
slowdown or threshold change. Tools: Valgrind 3.22.0 and GCC 14.2.0.

| ROM | Mapper | Base Ir/frame | Head Ir/frame | Delta | Base spread |
|---|---|---|---|---|---|
| nesteroids | 0 | 23791573 | 23914932 | +0.51% | 0 |
| runner | 1 | 24699006 | 22203492 | -10.10% | 0 |
| M4_P256K_C256K | 4 | 23367379 | 21998505 | -5.85% | 0 |

- `hash-equality` job `122878420493` and `CI required` job `122878507828` succeeded.
- Nightly `vectors-full` job `122858544021`, artifact `12819969669`: registration inventory and JUnit establish 258 successful tests, including cold fetch, all 256 opcodes and sample-match; zero failed/skipped.
- Nightly `mmc3-oracle` job `122858543964`, artifact `12820139927`: 13 successful JUnit tests, including fetch and all configured oracle tests; zero failed/skipped.
- Real `retroarch-e2e` job `122859328006`, artifact `12822541503`: pinned RetroArch 1.22.2; intended progression reaches 16 boundaries and `VERDICT_OK`, with save/load, rewind and run-ahead commands. Drop/double-advance controls reject with `ADVANCE_NOT_EXACTLY_ONE`; wrong-origin rejects with `ORIGIN`. The real-host source is the same tested merge.

These results resolve the identified current-source six-platform, protected-PASS,
clean campaign, per-ROM performance and configured hosted conformance/host debt,
including the previously unconfirmed vectors-full/MMC3 fetch outcomes. Historical
failed attempts and unrelated debt remain. The exact physical paired-chip tie
and the flagged judgment-tier clean-room prohibition retain their prior bounded
dispositions; no manual test or provenance sign-off is invented.

### Immutable artifact downloads

Each ID below was retrieved through its GitHub artifact ZIP endpoint after
run/attempt/freshness checks. Download SHA-256 equals the API artifact digest;
full metadata, raw bytes and copy mappings remain in the checksummed bundle.

| Artifact ID | Download SHA-256 |
|---|---|
| 12819969669 | `1173245ff3a1c8e23d11b6d696e46bf438a6072d15f1447b3da8624e7f14662b` |
| 12820139927 | `8b76bad7a20108bd7d42b2b3fb77799ea537ecd919083c0b44d5a1e20df007c1` |
| 12820934637 | `5e5d18eee301c4f1b82abedc7a378bd4c13c3045c54a2fe1657ec713f446ab12` |
| 12821512434 | `2d82731eac7171333c737bbac95d1898f60a4df2fa189e9985a8a39ec9299229` |
| 12821532905 | `16f84c41b9078a781576f40f0c82c4d3382908b708ab2a8c38238f12550b9ef2` |
| 12821766998 | `3d4fa259b38278d1527b7e7531ac55424c0c5d1e61512c8329bf4ac57b5e9dbc` |
| 12821767334 | `ca239b5b484314c2fb02cd8a648a10ed5ef85086e1cd32d59ab47a353c9626fb` |
| 12821847585 | `c0922222e3342bb87a6d921f61838ac75c9b5743995a5a6f323b7482ad989aa8` |
| 12822221056 | `6b2d85f5144999bf75f1b8bb36cecab2e15cc775849bea048b388e2f0c0f3138` |
| 12822541503 | `ba43a010518c3235033d6b756bd5d92f70f18c3a52bd83f8d2042d470db7ed29` |
| 12822567333 | `c9d5b0f68eb4dfe9d3f0dc45b53e62d708262b9d16a903a08661e8561219b8e6` |
| 12822670660 | `2517f54fa41ca45abc9e25957aa4e63f4396bfe6aedd16573fee625915cb007d` |
| 12822684951 | `34f333abbf402fa4987fb9c0f0c3d29250614fc28cce04622d51bbdd4413e142` |
| 12822735013 | `feda00f440f467740b38568ccef4900cc8391ea624670cdc35f43e4f2d56e5e2` |
| 12823010815 | `56d1c6dec8f5b14eb7bd32055309152e27c0e1f5e6683f0960d1b36dedd77367` |
| 12823877579 | `f9bade263cbc274062d0f747e1f07eb5f730313c018deffc2336180cfe033e0c` |

Only `.planning/**` and `tests/accuracy/measurements.md` are excluded by the
existing effective-input contract. The evidence commits record these immutable
runs; they are not new tested source identities. Any later effective-input
change invalidates this acceptance and requires fresh hosted evidence.

# Historical local evidence through Plan 20-09

The sections below preserve their original local observations and pending
obligations. The Plan 20-10 acceptance above supersedes their hosted-pending
statements for the identified delivered source; it does not rewrite history.

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

## Historical hosted obligations before Plan 20-10

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

### Post-review correction: actual GitHub job-log response

Pre-publication review of the official
[GitHub workflow jobs logs API](https://docs.github.com/en/rest/actions/workflow-jobs#download-job-logs-for-a-workflow-run)
and old job `122224137324` from CI `40514060876/1` showed that the job `logs`
endpoint returns a UTF-8-BOM timestamped plain-text job log, not a ZIP. The
retained old response was inspected as data only; its successful prior CI and
Nightly run identities remain historical availability and do not certify the
repaired source. The collector now preserves the raw job log, binds its SHA to
the selected job/run/attempt and successful step metadata, selects the unique
expected step group within its `started_at`/`completed_at` bounds, and
recomputes the normalized selected-step digest during validation. ANSI CSI
sequences and line endings are normalized only in the derived step file. The
exact terminal parser was replayed against the captured historical response;
this was a parser check, not acceptance evidence. Artifact ZIP handling keeps
the path, duplicate-member, traversal and symlink checks. Raw log and ZIP
requests use the resolved `gh` executable and feature-detect
`--allow-escape-sequences`.

Correction commits: RED fixture `8202159` and implementation
`f3b56e0`. The focused Task 1 gate passed 28/28 with exact-reason controls for
other-step-only CTests, missing/near-match/skipped/failed/duplicate and
contradictory results, plus unsupported old-CLI rejection before a raw
request. Task 2 passed 12/12 with the cold protected observation, all 108
protected PASS keys, exact six NMI targets, native inventory and policy
controls. The full workflow passed 686/686 in 389.05 seconds and packaged
library, runner and libretro outputs.

Two full-enabled local campaigns completed after the correction. The second
was an unintended fixture-triggered repeat while the Task 2 focused gate ran
before full mode was disabled; both runs agree on the measured result and
fingerprint, and the second run supplies the current baseline metadata. It
records source revision `f3b56e003dc8`, `source_dirty=1` from the preserved
config edit, `host_elapsed_guest_ns=4768711000`, report SHA-256
`29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac`, and
56-input fingerprint
`b2cdcdd15785b590a415dac911187b8abfa0d025a1cc448d412650d1716f1240`. All 22
pages and 146 scored rows completed: 114 PASS, 32 FAIL, no skip/not-run/timeout,
five DRAW, and all six NMI targets pass. After baseline refresh, the effective
source digest is
`d1d4a456b40332a78678fbcf1ba1bf948a82641f20dfec5ecafbe0f2f678c69e` over 416
files. No new hosted collection or acceptance manifest exists. New
source-bound hosted CI/Nightly and actual content acceptance remain pending;
all four Phase 20 requirements and KEEP-02/KEEP-03 remain open.
