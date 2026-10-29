# AccuracyCoin bounded campaign measurements

## Final campaign identity

The final registered run used the canonical 6bee ROM and production collector:

- Pin: `6bee919529289c555803b612b8c9e4f7eaaeeabb`
- ROM SHA-256: `8ac8904a3cf26fbdfa6be501e414d45886ff4478d30dee8e58fa436cf5fd5adc`
- Inventory SHA-256: `7d022800d01ad81159109bf7457af94ab6f80e4a682fce716ec476f7c454a4a5`
- Retained comparison pin: `673ef550db296136d52229961e7d39366116882a`, ROM SHA-256 `4fe8c2bc9abc6f4d418da47b73f62cba89fcacd950fae763097a1681d650e839`, inventory SHA-256 `3271803cdc63c4b4a83cf65453b817dc66ec65ed97863ad19261b56f36821c84`
- Old inventory: 149 entries (144 scored, 5 DRAW); new inventory: 151 entries (146 scored, 5 DRAW)
- Report/inventory schema: 1
- Compiled source revision: `d4a8a4aaf123` (`source_dirty=1`). This pre-commit capture includes CR-01 enforcement of unique, complete final raw samples, malformed-final rejection and CRLF raw-header support; the measured campaign rows remain unchanged. The checkout also contains paired README/public-header contract notes and regression controls, plus the untouched pre-existing `.planning/config.json` edit; the planning file is excluded from the input fingerprint.
- Deterministic build-input fingerprint: `5aedbb3b04eca54a04d42e4e0ec7851e2e694cd72fd032d2ab1c29dfcbf2afe0` over 56 source/build input files. Generated reports are excluded. Campaign report SHA-256: `6d0a811960f6964e7fc66802c6e4ade0c4ffb8772c40d88d50cf79e1f8463652` (unchanged).
- Input policy: cold boot; one page-header A input; one-frame A press; one-frame release; execute the selected page independently.
- Accepted bounds: boot 32, navigation 5 per page switch, start 2 frames, execution 1800; page child 15 seconds, collector 10 seconds, CTest 370 seconds.
- The earlier hosted dispatch `40054252174` used the former 5-second common page deadline and failed to produce trustworthy campaign evidence. Its Linux x64 artifact records page 19 as `OUTER_TIMEOUT` at 5 seconds, after an otherwise source-qualified guest timeout observation reached 1800 frames in 6.335862879 seconds; page 18 took 4.290085045 seconds. All other page observations were at or below 2.463 seconds. The 15-second common host-child deadline is 2.36 times the measured maximum and preserves the existing 1800-frame guest cap. The old run remains attributed as failed infrastructure evidence, not a successful campaign.
- The finite campaign envelope is now 22 * 15 seconds per-page maximum + 10 seconds collection = 340 seconds, with a 370-second CTest bound leaving 30 seconds for orchestration and final output checks. The injected `accuracycoin.outer_failure` controls keep their 1-second child deadline. The workflow/job timeout remains unchanged.
- The latest local campaign measurement sums to 7.320463 seconds across page children; the slowest child was page 19 at 2.285672 seconds. All 22 children exited zero. `accuracycoin.campaign` passed on the current dirty source; full hosted CI remains the exact-head verification after the fix.
- Retained output: `build/ci/accuracycoin/full/` contains all 22 page directories, child stdout/stderr, raw snapshots, diagnostics, page reports/metadata, `outer-outcomes.tsv`, and the production `inventory.tsv`, `attempts.tsv`, `observations.tsv`, `report.tsv`, `summary.txt`, and campaign `metadata.txt`.

`outer-outcomes.tsv` records 22 `EXIT_ZERO` children, no outer timeout and no child failure. Campaign metadata says `trustworthy_observation=1` and `infrastructure_verdict=complete`.

## Phase 18 build-input provenance refresh

The Phase 18 fuzz-control, state-loader oracle and public ticks-comment changes altered the
selected AccuracyCoin build-input fingerprint, so the full campaign was run again during the
`ci` workflow before refreshing its provenance. This run used source revision `68c99c078dc2`
with a dirty working tree, fingerprint `a3a8ec4717db2600a98cb1a67dd0a098cd20305d6c88469c7b421f08cb961c12`
over 56 inputs, and `host_elapsed_guest_ns=7327020000`. It retained the canonical report SHA-256
`6d0a811960f6964e7fc66802c6e4ade0c4ffb8772c40d88d50cf79e1f8463652`; all 22 pages completed,
all 146 scored rows were observed, and the canonical counts stayed at 101 pass, 42 fail, 0 skip,
2 not-run, 1 timeout and 5 inventory-only DRAW rows. The source and measurement metadata were
refreshed from `build/ci/accuracycoin/full/metadata.txt`; the scoreboard and report rows were not
changed.

## Campaign outcome

| Measure | Actual |
|---|---:|
| Pages launched / attributed starts / completed | 22 / 22 / 22 |
| Scored inventory | 146 |
| Attempted / observed raw scored rows | 146 / 146 |
| Terminal classifications | 143 / 146 |
| Pass | 101 |
| Fail | 42 |
| Skip | 0 |
| Not run | 2 |
| Guest timeout | 1 |
| DRAW inventory-only rows | 5 |
| Actual guest frames, all attempts | 7153 |
| Maximum declared campaign guest-frame budget | 41503 |

The five DRAW entries are listed separately and never enter the scored denominator. Every scored row has an observed final byte. The two `not-run` classifications are the page-19 following row with a noncurrent `$03` marker and the untouched `$00` row; neither was invented as an active timeout. The page-19 `Arbitrary Sprite zero` row is the only guest timeout. No all-pass or hardware-conformance claim is made.

## Retained protected-pass measurements

All 17 protected old-pin passes were re-executed from the exact retained `673ef` ROM in fresh cold runs. Each mapped new-pin result is also backed by a fresh campaign observation. All 17 currently pass at both pins; the four source-reviewed changed-meaning rows are explicitly marked and do not broaden strict success to other rows.

The production protected observer now derives the needed new-pin pages from
the PASS keys in the same fetched protected-main baseline used by the verdict.
Before the pin transition that selects the 17 migrated targets; after protected
main adopts the new pin it selects all pages containing the 101 protected PASS
rows. The verdict freshly compares every such row's observed raw byte and
status on each current lane. A primary-Linux full-lane control builds the
post-merge 101-PASS baseline, obtains new cold page reports through this same
observer, accepts all 101 measured passes, and rejects an outside-page failure
and a formerly changed-meaning-key failure under same-pin policy.
The focused CTest fixture and regression-control run passed in 7.46 seconds.
Its future-main control produced 22 fresh page reports (three retained
old-pin pages plus the 19 new-pin pages containing protected PASS keys),
accounting for 6.042099 seconds of summed guest/page-observer time; the slowest
page took 1.591905 seconds. This is local placement evidence, not a hosted
post-merge measurement.

| Page | Protected keys | Old `673ef` measurement | New `6bee` measurement |
|---:|---|---|---|
| 2 | Absolute Indexed; Zero Page Indexed; Indirect; Indirect, X; Indirect, Y; Relative | All six pass, raw `$01`, 11 frames each | All six pass, raw `$01`, 12 frames each |
| 14 | Length Counter; Length Table; Delta Modulation Channel; Frame Counter IRQ*; Frame Counter 4-step*; Frame Counter 5-step* | All six pass, raw `$01`, 120 frames each | All six pass, raw `$01`, 120 frames each |
| 17 | CHR ROM is not writable; PPU Register Mirroring; PPU Register Open Bus*; PPU Read Buffer; Palette RAM Quirks | Four pass raw `$01` and one pass raw `$41`, all 151 frames | Four pass raw `$01` and one pass raw `$41`, all 159 frames |

`*` indicates a source-reviewed changed meaning: the three frame-counter tests changed their cycle synchronization sequence, and PPU Register Open Bus changed its initial setup. The assertions and test symbols were reviewed per pinned source in `migration.tsv`. PPU Read Buffer `$41` remains a source-defined accepted pass for that key only. `tests/accuracy/old-673ef-scoreboard.txt` preserves the old strict expectations; `migration.tsv` carries the exact old/new raw bytes, frame counts, attempts, source symbols and fresh observation references.

## Per-page measurements

| Page | Boot | Navigation | Start | Execution | Total frames | Verdict | Host ms |
|---:|---:|---:|---:|---:|---:|---|---:|
| 1 | 26 | 0 | 2 | 51 | 79 | complete | 80.007 |
| 2 | 26 | 5 | 2 | 12 | 45 | complete | 38.418 |
| 3 | 26 | 10 | 2 | 13 | 51 | complete | 46.838 |
| 4 | 26 | 15 | 2 | 13 | 56 | complete | 57.148 |
| 5 | 26 | 20 | 2 | 13 | 61 | complete | 53.230 |
| 6 | 26 | 25 | 2 | 13 | 66 | complete | 55.881 |
| 7 | 26 | 30 | 2 | 22 | 80 | complete | 73.222 |
| 8 | 26 | 35 | 2 | 15 | 78 | complete | 86.811 |
| 9 | 26 | 40 | 2 | 13 | 81 | complete | 68.037 |
| 10 | 26 | 45 | 2 | 14 | 87 | complete | 75.742 |
| 11 | 26 | 50 | 2 | 17 | 95 | complete | 81.037 |
| 12 | 26 | 55 | 2 | 138 | 221 | complete | 161.661 |
| 13 | 26 | 60 | 2 | 533 | 621 | complete | 657.384 |
| 14 | 26 | 65 | 2 | 120 | 213 | complete | 205.038 |
| 15 | 26 | 70 | 2 | 92 | 190 | complete | 184.258 |
| 16 | 26 | 75 | 2 | 15 | 118 | complete (DRAW-only) | 104.151 |
| 17 | 26 | 80 | 2 | 159 | 267 | complete | 249.146 |
| 18 | 26 | 85 | 2 | 1463 | 1576 | complete | 1387.206 |
| 19 | 26 | 90 | 2 | 1800 | 1918 | guest-timeout | 2285.672 |
| 20 | 26 | 95 | 2 | 503 | 626 | complete | 790.990 |
| 21 | 26 | 100 | 2 | 212 | 340 | complete | 321.706 |
| 22 | 26 | 105 | 2 | 151 | 284 | complete | 256.880 |

At page 16, the ROM's DRAW-only sequence reports no scored result transition. The start is attributed to the observed row-cursor transition and automation flag; completion is the later cursor return to the page header with automation cleared. All five DRAW rows remain inventory-only.

At page 19 under the final five-frame navigation policy, the final diagnostic sample has menu cursor `$05` (`menuCursorYPos`, CPU RAM `$0016`) and `AutomateTestSuite` `$01` (CPU RAM `$0034`). This qualifies inventory row 6, `Arbitrary Sprite zero`, as the active `$03` test at the execution cap. Row 7, `Misaligned OAM behavior`, also retains raw `$03`, but is not the current cursor row and is classified `not-run` with cause `noncurrent-running-marker`; row 8 is observed `$00`/`unstarted`. The observer and collector retain the bytes and cross-check the cursor/automation diagnostics before assigning the timeout.

## Separate navigation-cap-6 exploratory sample

This is one page-19 exploratory attempt, not part of the 22-page final campaign and not combined into its counts. With `boot=32`, `navigation=6`, and `execution=1800`, it used 108 navigation frames, reached the cap after 1936 total frames, and took 1.91 seconds externally. The final diagnostic cursor was `$06` with automation `$01`; the active key was row 7, `Misaligned OAM behavior`, raw `$03`, so row 7 timed out while row 6 was terminal fail `$06`. The per-page raw and diagnostic output is retained separately under `build/ci/accuracycoin/navigation-cap-6-page-19/`.

The difference demonstrates that this fixed frame sequence's navigation cap also changes guest timing: an extra neutral frame per page shift changes which test is active when page 19 reaches its execution cap. The final campaign therefore uses the measured five-frame input unit throughout; statuses from the six-frame probe are not interchangeable with its results. The adversarial report fixture separately proves that when two `$03` samples appear in one snapshot set, only the `$0016` cursor-qualified row can be timeout and the other raw marker remains visible as `not-run`.

## Cap rationale and CI placement input

- Boot: all 22 attempts reached menu-ready by frame 26. Cap 32 leaves 6 frames (23%) for startup variation without increasing the longer per-page deadline.
- Navigation: five frames per page change is the measured fixed sequence: one Right edge followed by four neutral frames. The six-frame probe changes guest timing, so it is a distinct policy rather than a free safety margin.
- Start/release: two frames, with A released on the second frame, were observed in all 22 attempts. Controller-sensitive page 14 also completed through the released-input path.
- Execution: cap 1800 leaves 337 frames above the longest completed execution (page 18 at 1463); page 19 timed out before Phase 19 and now completes (see "Phase 19 re-record"). The cap is unchanged.
- Wall bounds: hosted evidence from the former deadline showed page 19 at 6.336 seconds and page 18 at 4.290 seconds, so the former 5-second bound did not provide sufficient margin. The new common 15-second page deadline is 2.36 times the observed maximum; the 22 sequential page deadlines plus 10-second collection cap total 340 seconds, and the 370-second CTest timeout leaves 30 seconds for orchestration and final output checks.
- Isolated ASan driver bound: page 19 reached the same expected row-6 `$03` guest timeout after 1918 frames on hosted ASan, with `host_elapsed_ns=14393278785`; the prior 10-second subprocess deadline killed the otherwise valid observation. The page-19 test-only host deadline is now 30 seconds (about 2x the measured run); the 1800 guest-frame cap, 60-second driver test timeout and all campaign/CI job bounds are unchanged.
- Protected-page bound: each cold page observer has a 30-second host-child deadline. A protected baseline can select up to 22 new-pin pages in addition to the three retained old-pin pages; the setup CTest enclosure is `(22 + 3) * 30 + 10 = 760` seconds. The bound covers the hosted ASan page-19 observation at 22.454 seconds from successful run 40055745306 and leaves its guest-frame cap unchanged.
- Placement selected for Plan 17-04: one full campaign in the required Linux x64 build lane on each PR, not all six platform matrix legs. CI turns on `NESTURBATOR_ACCURACYCOIN_FULL` only in that lane; the other five still run decoder, classifier, activation, migration, protected-pass and parity checks. The full campaign CTest took 6.65 seconds locally within a 376.25-second full `ci` workflow. Each build lane uploads its `build/<preset>/accuracycoin/` evidence with an `always()` upload step and fails if the required directory is absent. The existing Linux build timeout remains 39 minutes: this repository sets it from sampled hosted runs, so the selected lane's actual hosted execution must be recorded before revisiting it. The same applies to the current timeout comment; local duration does not establish hosted performance.

No row-A fallback was run. The cold campaign observed raw bytes for every scored row, and the only nonterminal results retain a current active timeout, a source-disqualified running marker, and an unstarted `$00` row. A separately selected-row attempt would change the guest context, and no additional raw byte is missing from this baseline. The two honest `not-run` findings remain visible for later plans.

## Phase 19 re-record (sprite 0 and OAMADDR, behaviour revision 7)

Cause. The basic sprite-0 hit already worked (Phase 19 D-01/D-02). The `0x06` results on `Sprite 0 Hit behavior`, `Suddenly Resize Sprite`, `Misaligned OAM DMA` and `Arbitrary Sprite zero` were guest re-run artifacts: after `Misaligned OAM behavior` took its failure branch, the ROM re-ran later tests with OAMADDR stale, because the core never cleared OAMADDR during rendering (sprite tile loading, dots 257-320) and started sprite evaluation at slot 0 instead of OAMADDR. The fix clears OAMADDR during tile loading and starts evaluation at OAMADDR; no hit test was changed. The `Arbitrary Sprite zero` `0x03` timeout described under "Campaign outcome" above is superseded: it came from the same stale OAMADDR, not from a missing hit.

Full campaign after the fix (`build/ci/accuracycoin/full/`, 22 of 22 pages complete): 146 scored rows, 108 pass, 38 fail, 0 skip, 0 not-run, 0 timeout, 146 terminal. This matches the research prediction (108/38/0/0/146). The 1800-frame execution cap and the 32/5 boot and navigation caps are unchanged. Page 19 rows now report 521 frames and its `metadata.txt` has `verdict=complete`; the page-18 rows moved from 503 to 569 frames.

| Row | Before | After |
|---|---|---|
| Sprite 0 Hit behavior | fail 0x06 | pass 0x01 |
| Arbitrary Sprite zero | timeout 0x03 | pass 0x01 |
| Suddenly Resize Sprite | fail 0x06 | pass 0x01 |
| Misaligned OAM DMA | fail 0x06 | pass 0x01 |
| Misaligned OAM behavior | not-run 0x03 | fail 0x16 (terminal) |
| OAM Corruption | not-run 0x00 | fail 0x0a (terminal) |
| INC $4014 | fail 0x0a | pass 0x01 |
| Rendering Flag Behavior | fail 0x0a | pass 0x01 |
| $2007 read w/ rendering | fail 0x06 | pass 0x01 |

The last three are neighbour rows that started to pass; they are recorded with their cause (the same OAMADDR behaviour), not tuned or chased. No row that passed on main stopped passing. Remaining rows only changed their frame counts.

Guard. `accuracycoin.activation` now fails unless `Misaligned OAM behavior` is neither not-run nor raw `0x03`, `Sprite 0 Hit behavior` and `Arbitrary Sprite zero` are pass `0x01`, and no scored row times out; with the full campaign on it also fails on any page-19 timeout row or a page-19 `metadata.txt` without `verdict=complete`.

Hashes. No frame hash in `tests/runner/hashes.txt` moved. The 12 `state/*` rows were re-pinned in Plan 19-05 only because saved states carry the behaviour revision byte (now 7).

## Phase 20 pre-implementation NMI/VBlank baseline and source-derived oracles

The initial pre-phase capture was a trustworthy complete run at source revision
`6380cd844584`. After this plan's test registration changed the measured input
set, the pre-implementation campaign was run again and `baseline-metadata.txt`
was refreshed from the retained output. This pre-implementation run used AccuracyCoin pin
`6bee919529289c555803b612b8c9e4f7eaaeeabb`, ROM SHA-256
`8ac8904a3cf26fbdfa6be501e414d45886ff4478d30dee8e58fa436cf5fd5adc`, behaviour
revision 7, source revision `58607dbbd339`, `source_dirty=1`, and build-input
fingerprint `e252004f3b152187984df0a1ffd4e937dc86c0a6f37d41bad16e33e8ad96e16a`
(56 files). It completed 22/22 pages, observed all 146 scored rows, and reported
108 pass, 38 fail, and zero nonterminal results. Its report SHA-256 remained
`b73c7b255ef4c0fe091c2d50bf94685e5b666760aaead6e28985bc3931ceeb7c`; the
scoreboard SHA-256 is
`5693a671844469093afa74f8a728e94e1077ecdcdcf72e8519b4d4a940151c87`. These are
the measured ROM outputs from that campaign, not results produced by the local
table-comparator self-checks in `test_nmi_races.c`.

| Measured AccuracyCoin row | Raw result | Frames | Source case |
|---|---:|---:|---|
| `$2002 flag timing` | `$06` fail | 521 | `TEST_2002_VBLANK_FLAG` |
| `NMI Control` | `$22` fail | 1463 | `TEST_NMI_Control` |
| `NMI Suppression` | `$06` fail | 1463 | `TEST_NMI_Suppression` |
| `NMI Timing` | `$06` fail | 1463 | `TEST_NMI_Timing` |
| `NMI at VBlank end` | `$06` fail | 1463 | `TEST_NMI_At_VBlank_End` |
| `NMI disabled at VBlank` | `$06` fail | 1463 | `TEST_NMI_Disabled_At_VBlank` |

The local source-oracle checks encode the pinned assembly's whole accepted
vectors: `$2002` clear arrays `[E0,E0,80,00]` or `[E0,80,80,00]`; set samples
`$53=00`, `$54=00|40`, `$55=60` after the source mask; NMI Timing's two ten-cell
arrays `[03,02,02,02,02,02,02,01,01,01]` and
`[02,02,02,02,02,02,01,01,01,01]`; and NMI Suppression's
`[FF,02,02,FF,00,01,FF,03,03,03]` with source checks beginning at x=1 and
skipping x=3 and x=6. The NMI-at-VBlank-end test runs both source sequences and
expects `[01,01,01,00,00,00,00]` for each. Sequence A enables NMI and stores
the sampled value; sequence B is exactly `LDA #$80; STA $2000; NOP; NOP;
LDA #$00; STA $2000` before storing the sample. The disabled-at-VBlank row
expects `[00,00,00,FF,01,01,01]` and skips the x=3 cell. NMI Control case 8
checks the instruction after `STA $2000` before the handler's INX marker; case
9 polls between the two writes of `INC $2000`.

The source-derived table values and accepted alternatives above are copied from
the MIT AccuracyCoin `AccuracyCoin.asm` at the pinned commit (flag timing
around lines 1839–1900; NMI Control around 6521–6640; timing, suppression, and
VBlank-end cases around 6688–6886). The tests validate comparator fidelity and
reject mixed/incorrect vectors; they do not execute the AccuracyCoin ROM and do
not constitute production conformance evidence. At that pre-implementation
checkpoint, the owned NROM cases had not yet closed the production-path controls
or recorded the actual ROM tables. The next section records the later campaign,
production-path controls, and actual executed table observations.

## Plan 20-02 current capture

The full campaign was rerun after the status-read decoder fix and the format-2,
revision-8 implementation. Its retained output is
`build/accuracycoin-plan18-02-campaign/accuracycoin/full/`.

| Measure | Recorded value |
|---|---|
| AccuracyCoin pin / ROM SHA-256 | `6bee919529289c555803b612b8c9e4f7eaaeeabb` / `8ac8904a3cf26fbdfa6be501e414d45886ff4478d30dee8e58fa436cf5fd5adc` |
| Source revision / source dirty | `53b32c2baf65` / `1` |
| Behaviour revision / state format | 8 / 2 |
| Build-input fingerprint / files | `d3898ef935b97427518b9cdc1501074f44fee3f4a6c7caa7ff23a7baff16115b` / 56 |
| Report SHA-256 / scoreboard SHA-256 | `15f69db8a3c0c7037219bd27f930ae2607fba99b0f5013ef0b99c303a6d72dc7` / `27068b6791b5a4aca4f2bc22cf9e9cdcf9407dbc37ec0ff7f2ad654839f0001b` |
| Pages / scored rows observed / terminal | 22 / 146 / 146 |
| Pass / fail / skip / not-run / timeout / DRAW | 110 / 36 / 0 / 0 / 0 / 5 |
| Guest elapsed time | 7,068,439,000 ns |

The ROM pin, schema, 32/5/1800 caps, 146 scored denominator and five DRAW
entries are unchanged. `source_dirty=1` records that implementation and tests
were uncommitted during the local measurement. This output is not hosted
authority. The cold protected-main fetch still reads the separate
`scoreboard-main.txt` snapshot with its 108 PASS keys; the protected verdict
and no-lost-pass controls passed against fresh page observations. Nightly
vectors-fetch evidence remains unresolved.

The current target rows are:

| AccuracyCoin row | Result | Frames |
|---|---:|---:|
| `$2002 flag timing` | fail `0x0a` | 551 |
| `NMI Control` | pass `0x01` | 1437 |
| `NMI Timing` | fail `0x06` | 1437 |
| `NMI Suppression` | fail `0x06` | 1437 |
| `NMI at VBlank end` | fail `0x06` | 1437 |
| `NMI disabled at VBlank` | fail `0x06` | 1437 |
| `VBlank beginning` | pass `0x01` | 1437 |
| `VBlank end` | pass `0x01` | 1437 |

All 108 PASS keys from the tracked pre-plan scoreboard remain PASS. Two rows
also moved from failure to pass: `NMI Control` (`0x22` to `0x01`) and `Hybrid
Addresses` (`0x0a` to `0x01`). `$2002 flag timing` remains a failure, with its
error byte moving from `0x06` to `0x0a`; the actual table's first mismatch is
recorded below. No remaining polling failure is promoted to a pass or removed
from the Plan 20-03/20-06 follow-up.

### Actual source table observations

These values are from the MIT AccuracyCoin ROM executing through the production
runner, not from the owned C comparators. The log records writes to the ROM's
result RAM, including PC, half-master tick, PPU coordinate, status, control and
NMI line/history. The page-18 table stores are at the listed production PCs.

| Source sequence | Production stores and observed vector | Source comparison and first divergence |
|---|---|---|
| VBlank beginning | `$50..$56`, PC `$b29d`: `[02,02,02,02,00,01,01]` | Expected `[02,02,02,02,00,01,01]`; source skips alignment-dependent index 3. Pass. Stores run at 241:130–136; no divergence. |
| NMI Timing | `$50..$59`, PC `$b400`: `[02,02,02,01,01,01,01,01,01,01]` | First cell 2 selects the shifted expected sequence `[02,02,02,02,02,02,01,01,01,01]`. First divergence is x=3: expected 2, observed 1, at tick 404,483,760 / 241:94. |
| NMI Suppression | `$50..$59`, PC `$b454`: `[02,02,02,02,00,03,03,03,03,03]` | Source checks x=1,2,4,5,7,8,9 and skips x=3,6. First divergence is x=5: expected 1, observed 3, at tick 660,359,112 / 241:78. |
| NMI at VBlank end, sequence A | `$50..$56`, PC `$b49b`: `[00,00,00,00,00,00,00]` | Expected `[01,01,01,00,00,00,00]`. First divergence is x=0, expected 1 / observed 0, at tick 774,056,448 / 261:47. |
| NMI at VBlank end, sequence B | `$60..$66`, PC `$b4bf`: `[00,00,00,00,00,00,00]` | Same expected vector as sequence A. First divergence is x=0, expected 1 / observed 0, at tick 876,978,432 / 261:47. |
| NMI disabled at VBlank | `$50..$56`, PC `$b4fd`: `[01,01,01,01,01,01,01]` | Expected `[00,00,00,FF,01,01,01]`; source skips x=3. First divergence is x=0, expected 0 / observed 1, at tick 1,022,730,384 / 241:94. |

The two VBlank status-only rows also have actual stores. VBlank end writes
`$50..$56` as `[01,01,01,01,00,00,00]` at PC `$b2dc`, ticks
217,277,040–339,496,944 / 261:29–35, matching the pinned source vector. The
`$2002 flag timing` clear table at PC `$94a0` is `[e0,80,80,00]`, the permitted
shifted alternative. Its set table first diverges at `$53`: expected `00`,
observed `40`, at PC `$94d3`, tick 179,757,096 / 131:172. The page-19 final
result remains `fail 0x0a` at 551 frames.

### Bus decoder regression and production controls

The reproduced regression was a missing `$2000` lower bound in the status-read
predicate. A plain RAM read at `$00ea` has low three bits `010`; the old
predicate therefore ran PPUSTATUS effects before the RAM decoder returned its
value. The same mistake affected DMC's parked read. A shared predicate now
requires `$2000 <= address < $4000` and low bits `010` in both paths. Owned
checks cover `$00ea`, RAM mirrors and boundaries, a DMC parked `$0002` read
(five 24-tick cycles with status/latches retained), ordinary `$2002`, mirrored
`$3ffa`, and ordinary/mirrored PPUCTRL writes. The focused race test passed
after the fix.

Expected-red controls now execute the production bus/sample path and reject one
named wrong expectation each: set sampled one dot late, clear sampled one dot
early, all status bits merged at M2 rise, captured request revoked on line fall,
unchanged PPUCTRL enable making another edge, and a pre-close transient or
widened-M2-only pulse captured. These prove the intended source assertion
rejects each wrong result; they are not compiled implementation mutants. The
six circuit waveforms invoke `nesturbator__bus_read` at internal-phi2 close
endpoints. In the circuit reference cycle, M2 rises at 2437, internal phi2
opens at 2440, and both fall at 2452; relative to the cycle start at 2428,
these are +9, +12 and +24 half-master ticks. The projection samples at 2428
and 2452, so its second close is the reference close. This is an endpoint
projection, not a full bus or
transistor waveform replay. It does not model NMIP transitions, output
propagation, analog setup/hold, or prove a paired-chip winner at exact ties.

Plan 20-02 initially recorded PPU transition, CPU sample, then bus side effect
for equal integer timestamps. The source-run observations in that plan first
diverged at NMI Timing x=3, Suppression x=5, and the first sample of each
VBlank-end/disabled sequence; the complete values and coordinates are in the
table above. Plan 20-03 superseded that policy with one global order: addressed
bus side effect, NMI detector capture, then tied PPU transition. CPU polling
uses the captured request at phi2 open. The final campaign below passes the
source's complete arrays at default alignment without per-table phase offsets.
Finite-margin waveforms constrain phase endpoints, but they do not identify the
physical winner at an exact tie. The PPU owns flag, enable and suppression; a
falling line does not revoke a captured request; service consumes it and reset
clears both sampled history and pending request. State format 2 serializes
those fields and refuses format-1 states; behaviour revision is 8.

### Every changed campaign row

This table compares the tracked pre-plan scoreboard with the current full
campaign. Columns are status / raw error byte / frames. It lists every changed
full key, including frame-only changes; unchanged rows are absent.

| Full key | Before | Current |
|---|---|---|
| `accuracycoin/$0B   ANC immediate` | pass / `0x01` / 17 | pass / `0x01` / 18 |
| `accuracycoin/$2002 flag timing` | fail / `0x06` / 521 | fail / `0x0a` / 551 |
| `accuracycoin/$2004 Stress Test` | fail / `0x0a` / 569 | fail / `0x0a` / 557 |
| `accuracycoin/$2007 Stress Test` | fail / `0x0a` / 569 | fail / `0x0a` / 557 |
| `accuracycoin/$2007 read w/ rendering` | pass / `0x01` / 569 | pass / `0x01` / 557 |
| `accuracycoin/$2B   ANC immediate` | pass / `0x01` / 17 | pass / `0x01` / 18 |
| `accuracycoin/$43   SRE indirect,X` | pass / `0x01` / 13 | pass / `0x01` / 14 |
| `accuracycoin/$47   SRE zeropage` | pass / `0x01` / 13 | pass / `0x01` / 14 |
| `accuracycoin/$4B   ASR immediate` | pass / `0x01` / 17 | pass / `0x01` / 18 |
| `accuracycoin/$4F   SRE absolute` | pass / `0x01` / 13 | pass / `0x01` / 14 |
| `accuracycoin/$53   SRE indirect,Y` | pass / `0x01` / 13 | pass / `0x01` / 14 |
| `accuracycoin/$57   SRE zeropage,X` | pass / `0x01` / 13 | pass / `0x01` / 14 |
| `accuracycoin/$5B   SRE absolute,Y` | pass / `0x01` / 13 | pass / `0x01` / 14 |
| `accuracycoin/$5F   SRE absolute,X` | pass / `0x01` / 13 | pass / `0x01` / 14 |
| `accuracycoin/$6B   ARR immediate` | pass / `0x01` / 17 | pass / `0x01` / 18 |
| `accuracycoin/$8B   ANE immediate` | pass / `0x01` / 17 | pass / `0x01` / 18 |
| `accuracycoin/$AB   LXA immediate` | pass / `0x01` / 17 | pass / `0x01` / 18 |
| `accuracycoin/$CB   AXS immediate` | pass / `0x01` / 17 | pass / `0x01` / 18 |
| `accuracycoin/$EB   SBC immediate` | pass / `0x01` / 17 | pass / `0x01` / 18 |
| `accuracycoin/Address $2004 behavior` | fail / `0x0e` / 569 | fail / `0x0e` / 557 |
| `accuracycoin/Arbitrary Sprite zero` | pass / `0x01` / 521 | pass / `0x01` / 551 |
| `accuracycoin/CHR ROM is not writable` | pass / `0x01` / 159 | pass / `0x01` / 150 |
| `accuracycoin/DMA + $2002 Read` | pass / `0x05` / 533 | pass / `0x05` / 532 |
| `accuracycoin/DMA + $2007 Read` | pass / `0x01` / 533 | pass / `0x01` / 532 |
| `accuracycoin/DMA + $2007 Write` | pass / `0x01` / 533 | pass / `0x01` / 532 |
| `accuracycoin/DMA + $4015 Read` | fail / `0x0a` / 533 | fail / `0x0a` / 532 |
| `accuracycoin/DMA + $4016 Read` | pass / `0x09` / 533 | pass / `0x09` / 532 |
| `accuracycoin/DMA + Open Bus` | pass / `0x01` / 533 | pass / `0x01` / 532 |
| `accuracycoin/DMC DMA + OAM DMA` | fail / `0x0a` / 533 | fail / `0x0a` / 532 |
| `accuracycoin/DMC DMA Bus Conflicts` | fail / `0x0a` / 533 | fail / `0x0a` / 532 |
| `accuracycoin/Explicit DMA Abort` | fail / `0x0a` / 533 | fail / `0x0a` / 532 |
| `accuracycoin/Hybrid Addresses` | fail / `0x0a` / 212 | pass / `0x01` / 212 |
| `accuracycoin/INC $4014` | pass / `0x01` / 569 | pass / `0x01` / 557 |
| `accuracycoin/Implicit DMA Abort` | fail / `0x0a` / 533 | fail / `0x0a` / 532 |
| `accuracycoin/Interrupt flag latency` | fail / `0x0a` / 138 | fail / `0x0a` / 141 |
| `accuracycoin/Misaligned OAM DMA` | pass / `0x01` / 521 | pass / `0x01` / 551 |
| `accuracycoin/Misaligned OAM behavior` | fail / `0x16` / 521 | fail / `0x16` / 551 |
| `accuracycoin/NMI Control` | fail / `0x22` / 1463 | pass / `0x01` / 1437 |
| `accuracycoin/NMI Overlap BRK` | fail / `0x0a` / 138 | fail / `0x0a` / 141 |
| `accuracycoin/NMI Overlap IRQ` | fail / `0x06` / 138 | fail / `0x06` / 141 |
| `accuracycoin/NMI Suppression` | fail / `0x06` / 1463 | fail / `0x06` / 1437 |
| `accuracycoin/NMI Timing` | fail / `0x06` / 1463 | fail / `0x06` / 1437 |
| `accuracycoin/NMI at VBlank end` | fail / `0x06` / 1463 | fail / `0x06` / 1437 |
| `accuracycoin/NMI disabled at VBlank` | fail / `0x06` / 1463 | fail / `0x06` / 1437 |
| `accuracycoin/OAM Corruption` | fail / `0x0a` / 521 | fail / `0x0a` / 551 |
| `accuracycoin/PPU Read Buffer` | pass / `0x41` / 159 | pass / `0x41` / 150 |
| `accuracycoin/PPU Register Mirroring` | pass / `0x01` / 159 | pass / `0x01` / 150 |
| `accuracycoin/PPU Register Open Bus` | pass / `0x01` / 159 | pass / `0x01` / 150 |
| `accuracycoin/Palette RAM Quirks` | pass / `0x01` / 159 | pass / `0x01` / 150 |
| `accuracycoin/Rendering Flag Behavior` | pass / `0x01` / 569 | pass / `0x01` / 557 |
| `accuracycoin/Sprite 0 Hit behavior` | pass / `0x01` / 521 | pass / `0x01` / 551 |
| `accuracycoin/Sprite overflow behavior` | pass / `0x01` / 521 | pass / `0x01` / 551 |
| `accuracycoin/Suddenly Resize Sprite` | pass / `0x01` / 521 | pass / `0x01` / 551 |
| `accuracycoin/VBlank beginning` | pass / `0x01` / 1463 | pass / `0x01` / 1437 |
| `accuracycoin/VBlank end` | pass / `0x01` / 1463 | pass / `0x01` / 1437 |
| `accuracycoin/t Register Quirks` | pass / `0x01` / 569 | pass / `0x01` / 557 |

The timing-only changes above are the new measured frame counts under the same
cold-boot/input/cap policy; the final scoreboard retains each row's status and
raw byte. The two new passes are kept as measured outcomes. The `$2002` flag
timing raw change is directly tied to the corrected rule that RAM addresses
whose low bits resemble `$2002` do not run PPU status effects.

### Full hash inventory changes

The complete inventory was regenerated by `runner.write_hashes`; the
`runner.write_hashes.content` comparison passed after these pins were copied.
All 24 moved digests are listed below. The NMI-driven guest frame/audio rows
changed because captured edge timing changes the game's CPU update and APU
write schedule; rows retain their measured tick counts. State digests changed
because the serialized state now declares format 2 / behaviour revision 8 and
stores the live NMI history and retained request. State byte sizes did not
change: DABG 13087, M1SXROM 45869, M2 13088, M3 4896, M4TKROM 21303, M7 13088.
Every other frame and audio digest in the full inventory is unchanged.

| Hash key | Old ticks / SHA-256 | New ticks / SHA-256 | Cause |
|---|---|---|---|
| `nesrunner/play/audio/pcm` | — / `d21d8de676cd86d4a220a319d5f92593a51fe736e9b458c4cdec961700f4e854` | — / `509cb4c1625d855b45ac65e3678d9c047e19732f65cdd98a6b840ad3f6e7b7f5` | Guest NMI update cadence changes APU register writes and PCM. |
| `nesrunner/play/audio/transitions` | — / `470ec17a86fc8c04f29d835dcd8eb286e1211e359c4c5c9ad3473519db0f6873` | — / `8c5368976985549a8800301ac8c9a034b7c62d990ba8c207ee734c5b24c6413b` | Same guest/APU event schedule change. |
| `nesrunner/play/frame 180` | 128661024 / `1a9c0d42f6893cd1fb1d0b6ea91a2e0fa84e41cb7f6ba1eefe93464243788fd9` | 128661072 / `b1233a42c80a47d1f560ed28edd2fadc55efb273b2ff029dbca6a18352335595` | Guest NMI update cadence changes rendered game state. |
| `nesrunner/play/frame 300` | 214433712 / `8478ee93a2ec8bc5f110956eae466c81575ac927187cac565c845406b2083fc0` | 214434816 / `499ab10106bb67c663b295a81639c1505a39554fc1e5fadd9a6937711021b629` | Same guest NMI/input update schedule change. |
| `nesrunner/play/frame 450` | 321650496 / `d70d35decb6af04ecbd48a9d09cff7c0e7c6210679b3a32f60788b27b5694570` | 321652296 / `8570a4e5b35385c2d7b1dc7278b40cd4d7b6fe7e60ade1324377278c537cda51` | Same guest NMI/input update schedule change. |
| `nesrunner/play/frame 600` | 428866800 / `6bf066cfb147bf966dc88a59a4e85482c4b59c50fe70ef3d892b55e5ec1e7025` | 428868504 / `d7f312362cb9b57c2bf51de97a3242112d90052138ba2a89b2aa2b9a47aeb55c` | Same guest NMI/input update schedule change. |
| `nesteroids/boot/frame 30` | 21443256 / `f504aa7fa75f73dd733bb9454f9e1db91c9e903ece80c9ddda87e70052e35c25` | 21443376 / `e6afba6666d9d0ba1b9ae2f2f3c3f4e5f71f6bf1995f8b20e1e198a055bd8bf9` | NMI delivery shifts the guest's frame update schedule. |
| `nesteroids/boot/frame 60` | 42886752 / `d695d84c9721f7174f72f2c2357093170531d965f19d31dea17b0010cf49b938` | 42886872 / `73f4a7d3c377a519c8f990f9d9179730bf5cf86553e3fa1d79b59e8c17b99cdf` | Same guest NMI update schedule change. |
| `nesteroids/boot/frame 120` | 85773792 / `92cc9679c0a621d7b23f9f6414940956d807fcea46d3980e698d67239206e0f6` | 85773912 / `bbddb0e13f03ab3b31b4b68c168e8ebf7a9e28f76452cd5feb72ee3cabf4c1e9` | Same guest NMI update schedule change. |
| `nesteroids/boot/frame 180` | 128661144 / `6adc68fe11b7577c4fd375e47e0d958d36a2e143bfb663a35107d9464761f528` | 128661144 / `7cecae1314a25b2427bd22417bd6cdee2077492037dda45deef5a730a487aad3` | NMI delivery changes rendered game state even at the same tick count. |
| `state/dabg 0` | 0 / `70d7fdc6ebf5f47a2e8085a004c895916a2a2ce97d70e786303242defc3fe2b9` | 0 / `f105d0501e23a6bc10c626b99961258757fbb4e0cd2726f45cd249ed75def012` | Format / behaviour identity and live NMI history fields. |
| `state/dabg 60` | 42886680 / `57f7ccf1863779a217be4f0b9f81bdb389bcdeb46368b2051cabbbd6a8a2dbfd` | 42886680 / `ea063a567aaee6a8cd2cfad1527356914a8cfd1e9c2ab1f50c7f80c7b7b3afb6` | Format / behaviour identity and serialized NMI line/request history. |
| `state/m1sxrom 0` | 0 / `4b326291841235b635da1179009a95335dcb0bd5081fb1e4c7baf30681d200d8` | 0 / `9285b5e4fb97e6057bde66d7c1f4fbdd0641188d3f791e861d9b1b99aff2dd55` | Format / behaviour identity and live NMI history fields. |
| `state/m1sxrom 60` | 42885624 / `b2614c63c523ce859a2b19b0d0235b324b027266bd3ea78dda1a523a44c0a560` | 42885624 / `070dec6aa7185cec5590a692d3ff6f01c8685826a31a05be9d1ec45cf1b01859` | Format / behaviour identity and serialized NMI line/request history. |
| `state/m2 0` | 0 / `c3decbeb895f2462f0dc43272ba4c1d876a00784fd921b085c69f6636c8b4004` | 0 / `387ac411805ab63a1a8fe612f94cd0cf40cd215a11c9b50fbcfa62b1e611738e` | Format / behaviour identity and live NMI history fields. |
| `state/m2 60` | 42886032 / `f755cf07f870907d5df52f3a119f1a29fb26afcd238370f8869987cfceebee89` | 42886032 / `f8c54c7e0b968279c396daf6a5fe5d41b0b7616a635a249b8de2ad9f619f2e6c` | Format / behaviour identity and serialized NMI line/request history. |
| `state/m3 0` | 0 / `70fac23aa5042ca3984b805a8bbbaa4cfe1f3c2348b6c2ad09d5fa515f0c8e2f` | 0 / `67c5067aff2a4076744cb87af3cd35c0258772ae2c1d55a1e5cd54f55f9d80d3` | Format / behaviour identity and live NMI history fields. |
| `state/m3 60` | 42885984 / `ce3f32d8027f8b2336a4897638121c957c0f11bfbe1a46322f413df03825194a` | 42885984 / `208f9f9889dd74e22f652470dac059f210594bcea4b4e940e896a7073a0f08cc` | Format / behaviour identity and serialized NMI line/request history. |
| `state/m4tkrom 0` | 0 / `7b39fa5f1ff754b8f84392e130238d0e44e48501a897f26c66b9cacb765011cc` | 0 / `a43c893161026911671453f70d6d7f249ac0f98124743ced6b598a029dda0fa6` | Format / behaviour identity and live NMI history fields. |
| `state/m4tkrom 60` | 42885984 / `ef4017f5075eac3ebd3d8bcc7711b49e1e212b4b188d9aad9c9ff23c1dd88a61` | 42885984 / `e77766a8ce80f06b8035c50e3572bff7451b67162e8eeccf67aaf020ab8621b2` | Format / behaviour identity and serialized NMI line/request history. |
| `state/m7 0` | 0 / `37ff46b66837a464a568dff70c6bcdc0f1701b233a28cce49c23b8bf6b242ef4` | 0 / `0ce0219480c287d67494092430cd76bca5067bf1d08bdb96f6840ae3d5762102` | Format / behaviour identity and live NMI history fields. |
| `state/m7 60` | 42885960 / `f26c54a53b2196b134693226e7573107149fead9f86787b611bf47f7c3b056b4` | 42885960 / `bc6fd5040a6ae570822263c1b83b690fe89a55737cd9e769e9f7007c36c459ec` | Format / behaviour identity and serialized NMI line/request history. |

## Plan 20-03 final poll campaign

The final causal candidate is source revision `2c6ba6810823`, captured with
`source_dirty=1` before this plan's green commit. The full pinned campaign used
AccuracyCoin `6bee919529289c555803b612b8c9e4f7eaaeeabb`, the existing cold-boot
and controller sequence, 32-frame boot cap, five-frame navigation cap, and
1,800-frame page cap. It completed all 22 pages and 146 scored rows with 114
pass, 32 fail, zero timeout, and five DRAW rows. Report SHA-256 is
`29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac`; the
56-file build-input fingerprint is
`6b6bc9ecca830db8ca8114a9c70b7d488882c0bb9ac16b5956872249c600bd0a`; final
scoreboard SHA-256 is `38b08c8c93ca2e3b7b3a52f28284d089ef2240d10dcaa1a1bf902c02cf92f4bf`.
The run remains a local dirty-source measurement. The separate cold protected-
main observation and 108-key no-lost-pass gate passed; hosted Nightly CPU-vector
fetch evidence remains unresolved.

The page-18 and page-19 target rows all pass at default alignment:

| Full key | Result | Frames | Attempt |
|---|---:|---:|---|
| `accuracycoin/NMI Control` | pass `0x01` | 1418 | `page-18-cold-01` |
| `accuracycoin/NMI Timing` | pass `0x01` | 1418 | `page-18-cold-01` |
| `accuracycoin/NMI Suppression` | pass `0x01` | 1418 | `page-18-cold-01` |
| `accuracycoin/NMI at VBlank end` | pass `0x01` | 1418 | `page-18-cold-01` |
| `accuracycoin/NMI disabled at VBlank` | pass `0x01` | 1418 | `page-18-cold-01` |
| `accuracycoin/$2002 flag timing` | pass `0x01` | 537 | `page-19-cold-01` |

### Executed samples and guest markers

These arrays are the values stored by the pinned AccuracyCoin source sequences
in the full campaign. The NMI timing test accepts one whole ten-cell vector
selected by its first sample; the two recorded sequences are `[03,02,02,02,02,02,02,01,01,01]`
and `[02,02,02,02,02,02,01,01,01,01]`. NMI Suppression stored
`[00,02,02,02,00,01,01,03,03,03]` at `$50..$59` (`TEST_NMI_Suppression`,
store PC `$b454`); the source compares x=1,2,4,5,7,8,9 and deliberately skips
x=3 and x=6. Both NMI-at-VBlank-end sequences stored
`[01,01,01,00,00,00,00]`: sequence A at `$50..$56`, PC `$b49b`, and sequence B
at `$60..$66`, PC `$b4bf`. NMI-disabled-at-VBlank stored
`[00,00,00,00,01,01,01]` at `$50..$56`, PC `$b4fd`; the source skips x=3.
The `$2002` clear table is `[e0,80,80,00]`. Its set samples at `$53..$55`
were `$18,$58,$78` before the source `AND #$e0`, yielding `[00,40,60]`,
which satisfies the pinned bit-6 alternative at `$54` and expected `$60` at
`$55`.

The owned Control 8 fixture asserts the guest marker sequence: after `STA
$2000`, `LDX #$10` runs before NMI entry; the handler records entry X=`$10`,
returns to `$800b`, and the following `STX` records X=`$11`. Control 9's
`INC $2000` takes six cycles and retains both RMW writes; after the instruction
control and I/O bus are `$81`, the poll requests NMI before the following LDX,
and entry returns to `$8008` with handler-before-LDX. `core.nmi_poll` prints
these markers and the implied/read/store/RMW/stack/RTS/RTI and branch-class
poll traces as ordinary assertions. The race test has no expected-red path.

One global integer-timestamp model remains in use: an addressed bus effect is
applied before detector capture at the close; the PPU transition at that same
timestamp follows the capture. The CPU polls the captured request at phi2 open,
separately from line capture at close. No phase offset or per-table adjustment
was applied. Finite-margin cases support the sampled phase endpoints; the
exact physical winner at a tie remains unproven and is documented as a model
limit. The result does not move VBlank set or clear dots.

### Candidate row changes since Plan 20-02

Every changed key is listed below with status, raw error byte, and executed
frames before and after. Of 93 changed rows, 87 retained status and raw value
while frame counts shifted; five target failures became passes. `Hybrid
Addresses` changed from pass `0x01` to fail `0x0a` at 212 frames. It is not one
of the 108 protected starting PASS keys, and the no-lost-pass gate passed; the
adjacent row is recorded without broadening this plan's target scope.

| Full key | Previous status / raw / frames | Final status / raw / frames |
|---|---|---|
| `accuracycoin/$0B   ANC immediate` | pass / `0x01` / 18 | pass / `0x01` / 20 |
| `accuracycoin/$2002 flag timing` | fail / `0x0a` / 551 | pass / `0x01` / 537 |
| `accuracycoin/$2004 Stress Test` | fail / `0x0a` / 557 | fail / `0x0a` / 558 |
| `accuracycoin/$2007 Stress Test` | fail / `0x0a` / 557 | fail / `0x0a` / 558 |
| `accuracycoin/$2007 read w/ rendering` | pass / `0x01` / 557 | pass / `0x01` / 558 |
| `accuracycoin/$2B   ANC immediate` | pass / `0x01` / 18 | pass / `0x01` / 20 |
| `accuracycoin/$4B   ASR immediate` | pass / `0x01` / 18 | pass / `0x01` / 20 |
| `accuracycoin/$6B   ARR immediate` | pass / `0x01` / 18 | pass / `0x01` / 20 |
| `accuracycoin/$83   SAX indirect,X` | pass / `0x01` / 22 | pass / `0x01` / 19 |
| `accuracycoin/$87   SAX zeropage` | pass / `0x01` / 22 | pass / `0x01` / 19 |
| `accuracycoin/$8B   ANE immediate` | pass / `0x01` / 18 | pass / `0x01` / 20 |
| `accuracycoin/$8F   SAX absolute` | pass / `0x01` / 22 | pass / `0x01` / 19 |
| `accuracycoin/$97   SAX zeropage,Y` | pass / `0x01` / 22 | pass / `0x01` / 19 |
| `accuracycoin/$A3   LAX indirect,X` | pass / `0x01` / 22 | pass / `0x01` / 19 |
| `accuracycoin/$A7   LAX zeropage` | pass / `0x01` / 22 | pass / `0x01` / 19 |
| `accuracycoin/$AB   LXA immediate` | pass / `0x01` / 18 | pass / `0x01` / 20 |
| `accuracycoin/$AF   LAX absolute` | pass / `0x01` / 22 | pass / `0x01` / 19 |
| `accuracycoin/$B3   LAX indirect,Y` | pass / `0x01` / 22 | pass / `0x01` / 19 |
| `accuracycoin/$B7   LAX zeropage,Y` | pass / `0x01` / 22 | pass / `0x01` / 19 |
| `accuracycoin/$BF   LAX absolute,Y` | pass / `0x01` / 22 | pass / `0x01` / 19 |
| `accuracycoin/$C3   DCP indirect,X` | pass / `0x01` / 15 | pass / `0x01` / 13 |
| `accuracycoin/$C7   DCP zeropage` | pass / `0x01` / 15 | pass / `0x01` / 13 |
| `accuracycoin/$CB   AXS immediate` | pass / `0x01` / 18 | pass / `0x01` / 20 |
| `accuracycoin/$CF   DCP absolute` | pass / `0x01` / 15 | pass / `0x01` / 13 |
| `accuracycoin/$D3   DCP indirect,Y` | pass / `0x01` / 15 | pass / `0x01` / 13 |
| `accuracycoin/$D7   DCP zeropage,X` | pass / `0x01` / 15 | pass / `0x01` / 13 |
| `accuracycoin/$DB   DCP absolute,Y` | pass / `0x01` / 15 | pass / `0x01` / 13 |
| `accuracycoin/$DF   DCP absolute,X` | pass / `0x01` / 15 | pass / `0x01` / 13 |
| `accuracycoin/$E3   ISC indirect,X` | pass / `0x01` / 13 | pass / `0x01` / 14 |
| `accuracycoin/$E7   ISC zeropage` | pass / `0x01` / 13 | pass / `0x01` / 14 |
| `accuracycoin/$EB   SBC immediate` | pass / `0x01` / 18 | pass / `0x01` / 20 |
| `accuracycoin/$EF   ISC absolute` | pass / `0x01` / 13 | pass / `0x01` / 14 |
| `accuracycoin/$F3   ISC indirect,Y` | pass / `0x01` / 13 | pass / `0x01` / 14 |
| `accuracycoin/$F7   ISC zeropage,X` | pass / `0x01` / 13 | pass / `0x01` / 14 |
| `accuracycoin/$FB   ISC absolute,Y` | pass / `0x01` / 13 | pass / `0x01` / 14 |
| `accuracycoin/$FF   ISC absolute,X` | pass / `0x01` / 13 | pass / `0x01` / 14 |
| `accuracycoin/APU Register Activation` | fail / `0x12` / 120 | fail / `0x12` / 121 |
| `accuracycoin/Address $2004 behavior` | fail / `0x0e` / 557 | fail / `0x0e` / 558 |
| `accuracycoin/Arbitrary Sprite zero` | pass / `0x01` / 551 | pass / `0x01` / 537 |
| `accuracycoin/Branch Dummy Reads` | pass / `0x01` / 92 | pass / `0x01` / 94 |
| `accuracycoin/CHR ROM is not writable` | pass / `0x01` / 150 | pass / `0x01` / 159 |
| `accuracycoin/Controller Clocking` | pass / `0x09` / 120 | pass / `0x09` / 121 |
| `accuracycoin/Controller Strobing` | fail / `0x12` / 120 | fail / `0x12` / 121 |
| `accuracycoin/DMA + $2002 Read` | pass / `0x05` / 532 | pass / `0x05` / 533 |
| `accuracycoin/DMA + $2007 Read` | pass / `0x01` / 532 | pass / `0x01` / 533 |
| `accuracycoin/DMA + $2007 Write` | pass / `0x01` / 532 | pass / `0x01` / 533 |
| `accuracycoin/DMA + $4015 Read` | fail / `0x0a` / 532 | fail / `0x0a` / 533 |
| `accuracycoin/DMA + $4016 Read` | pass / `0x09` / 532 | pass / `0x09` / 533 |
| `accuracycoin/DMA + Open Bus` | pass / `0x01` / 532 | pass / `0x01` / 533 |
| `accuracycoin/DMA Landing on Write` | fail / `0x16` / 92 | fail / `0x16` / 94 |
| `accuracycoin/DMC DMA + OAM DMA` | fail / `0x0a` / 532 | fail / `0x0a` / 533 |
| `accuracycoin/DMC DMA Bus Conflicts` | fail / `0x0a` / 532 | fail / `0x0a` / 533 |
| `accuracycoin/DMC Reload Timing` | pass / `0x01` / 92 | pass / `0x01` / 94 |
| `accuracycoin/Delta Modulation Channel` | pass / `0x01` / 120 | pass / `0x01` / 121 |
| `accuracycoin/Explicit DMA Abort` | fail / `0x0a` / 532 | fail / `0x0a` / 533 |
| `accuracycoin/Frame Counter 4-step` | pass / `0x01` / 120 | pass / `0x01` / 121 |
| `accuracycoin/Frame Counter 5-step` | pass / `0x01` / 120 | pass / `0x01` / 121 |
| `accuracycoin/Frame Counter IRQ` | pass / `0x01` / 120 | pass / `0x01` / 121 |
| `accuracycoin/Frozen OAM2 Increment` | fail / `0x0a` / 151 | fail / `0x0a` / 122 |
| `accuracycoin/Hybrid Addresses` | pass / `0x01` / 212 | fail / `0x0a` / 212 |
| `accuracycoin/INC $4014` | pass / `0x01` / 557 | pass / `0x01` / 558 |
| `accuracycoin/Implicit DMA Abort` | fail / `0x0a` / 532 | fail / `0x0a` / 533 |
| `accuracycoin/Implied Dummy Reads` | fail / `0x0e` / 92 | fail / `0x0e` / 94 |
| `accuracycoin/Instruction Timing` | pass / `0x01` / 92 | pass / `0x01` / 94 |
| `accuracycoin/Internal Data Bus` | fail / `0x0a` / 92 | fail / `0x0a` / 94 |
| `accuracycoin/Interrupt flag latency` | fail / `0x0a` / 141 | fail / `0x0a` / 151 |
| `accuracycoin/JSR Edge Cases` | pass / `0x01` / 92 | pass / `0x01` / 94 |
| `accuracycoin/Length Counter` | pass / `0x01` / 120 | pass / `0x01` / 121 |
| `accuracycoin/Length Table` | pass / `0x01` / 120 | pass / `0x01` / 121 |
| `accuracycoin/Misaligned OAM DMA` | pass / `0x01` / 551 | pass / `0x01` / 537 |
| `accuracycoin/Misaligned OAM behavior` | fail / `0x16` / 551 | fail / `0x16` / 537 |
| `accuracycoin/Misaligned OAM2 Address` | fail / `0x0e` / 151 | fail / `0x0e` / 122 |
| `accuracycoin/NMI Control` | pass / `0x01` / 1437 | pass / `0x01` / 1418 |
| `accuracycoin/NMI Overlap BRK` | fail / `0x0a` / 141 | fail / `0x0a` / 151 |
| `accuracycoin/NMI Overlap IRQ` | fail / `0x06` / 141 | fail / `0x06` / 151 |
| `accuracycoin/NMI Suppression` | fail / `0x06` / 1437 | pass / `0x01` / 1418 |
| `accuracycoin/NMI Timing` | fail / `0x06` / 1437 | pass / `0x01` / 1418 |
| `accuracycoin/NMI at VBlank end` | fail / `0x06` / 1437 | pass / `0x01` / 1418 |
| `accuracycoin/NMI disabled at VBlank` | fail / `0x06` / 1437 | pass / `0x01` / 1418 |
| `accuracycoin/OAM Corruption` | fail / `0x0a` / 551 | fail / `0x0a` / 537 |
| `accuracycoin/PPU Read Buffer` | pass / `0x41` / 150 | pass / `0x41` / 159 |
| `accuracycoin/PPU Register Mirroring` | pass / `0x01` / 150 | pass / `0x01` / 159 |
| `accuracycoin/PPU Register Open Bus` | pass / `0x01` / 150 | pass / `0x01` / 159 |
| `accuracycoin/Palette RAM Quirks` | pass / `0x01` / 150 | pass / `0x01` / 159 |
| `accuracycoin/Rendering Flag Behavior` | pass / `0x01` / 557 | pass / `0x01` / 558 |
| `accuracycoin/Sprite 0 Hit behavior` | pass / `0x01` / 551 | pass / `0x01` / 537 |
| `accuracycoin/Sprite overflow behavior` | pass / `0x01` / 551 | pass / `0x01` / 537 |
| `accuracycoin/Sprites On Scanline 0` | fail / `0x0a` / 151 | fail / `0x0a` / 122 |
| `accuracycoin/Stale Sprite Shift Regs` | fail / `0x0e` / 151 | fail / `0x0e` / 122 |
| `accuracycoin/Suddenly Resize Sprite` | pass / `0x01` / 551 | pass / `0x01` / 537 |
| `accuracycoin/VBlank beginning` | pass / `0x01` / 1437 | pass / `0x01` / 1418 |
| `accuracycoin/VBlank end` | pass / `0x01` / 1437 | pass / `0x01` / 1418 |
| `accuracycoin/t Register Quirks` | pass / `0x01` / 557 | pass / `0x01` / 558 |

### Final native hash inventory changes

`runner.write_hashes` regenerated all 114 inventory lines through the owned
runner and movie writer. The content comparison passed after these pins were
copied. Against the Plan 20-02 pinned inventory, 83 measured rows changed: 54
digests and 29 tick/size-only values. State sizes increase by one byte because
format-2 serialization now writes the distinct `nmi_polled` history field.
Audio digests are listed independently from video because polling changes guest
interrupt timing and APU write/transition scheduling. Frame rows with equal
hashes retain identical rendered pixels while the tick count moves; the four
changed `nesrunner/play` frame digests record the changed guest state.

| Hash key | Previous ticks/size / SHA-256 | Final ticks/size / SHA-256 | Cause |
|---|---|---|---|
| `accuracycoin/$0B   ANC immediate` | pass / `0x01` / 18 | pass / `0x01` / 20 |
| `accuracycoin/$2002 flag timing` | fail / `0x0a` / 551 | pass / `0x01` / 537 |
| `accuracycoin/$2004 Stress Test` | fail / `0x0a` / 557 | fail / `0x0a` / 558 |
| `accuracycoin/$2007 Stress Test` | fail / `0x0a` / 557 | fail / `0x0a` / 558 |
| `accuracycoin/$2007 read w/ rendering` | pass / `0x01` / 557 | pass / `0x01` / 558 |
| `accuracycoin/$2B   ANC immediate` | pass / `0x01` / 18 | pass / `0x01` / 20 |
| `accuracycoin/$4B   ASR immediate` | pass / `0x01` / 18 | pass / `0x01` / 20 |
| `accuracycoin/$6B   ARR immediate` | pass / `0x01` / 18 | pass / `0x01` / 20 |
| `accuracycoin/$83   SAX indirect,X` | pass / `0x01` / 22 | pass / `0x01` / 19 |
| `accuracycoin/$87   SAX zeropage` | pass / `0x01` / 22 | pass / `0x01` / 19 |
| `accuracycoin/$8B   ANE immediate` | pass / `0x01` / 18 | pass / `0x01` / 20 |
| `accuracycoin/$8F   SAX absolute` | pass / `0x01` / 22 | pass / `0x01` / 19 |
| `accuracycoin/$97   SAX zeropage,Y` | pass / `0x01` / 22 | pass / `0x01` / 19 |
| `accuracycoin/$A3   LAX indirect,X` | pass / `0x01` / 22 | pass / `0x01` / 19 |
| `accuracycoin/$A7   LAX zeropage` | pass / `0x01` / 22 | pass / `0x01` / 19 |
| `accuracycoin/$AB   LXA immediate` | pass / `0x01` / 18 | pass / `0x01` / 20 |
| `accuracycoin/$AF   LAX absolute` | pass / `0x01` / 22 | pass / `0x01` / 19 |
| `accuracycoin/$B3   LAX indirect,Y` | pass / `0x01` / 22 | pass / `0x01` / 19 |
| `accuracycoin/$B7   LAX zeropage,Y` | pass / `0x01` / 22 | pass / `0x01` / 19 |
| `accuracycoin/$BF   LAX absolute,Y` | pass / `0x01` / 22 | pass / `0x01` / 19 |
| `accuracycoin/$C3   DCP indirect,X` | pass / `0x01` / 15 | pass / `0x01` / 13 |
| `accuracycoin/$C7   DCP zeropage` | pass / `0x01` / 15 | pass / `0x01` / 13 |
| `accuracycoin/$CB   AXS immediate` | pass / `0x01` / 18 | pass / `0x01` / 20 |
| `accuracycoin/$CF   DCP absolute` | pass / `0x01` / 15 | pass / `0x01` / 13 |
| `accuracycoin/$D3   DCP indirect,Y` | pass / `0x01` / 15 | pass / `0x01` / 13 |
| `accuracycoin/$D7   DCP zeropage,X` | pass / `0x01` / 15 | pass / `0x01` / 13 |
| `accuracycoin/$DB   DCP absolute,Y` | pass / `0x01` / 15 | pass / `0x01` / 13 |
| `accuracycoin/$DF   DCP absolute,X` | pass / `0x01` / 15 | pass / `0x01` / 13 |
| `accuracycoin/$E3   ISC indirect,X` | pass / `0x01` / 13 | pass / `0x01` / 14 |
| `accuracycoin/$E7   ISC zeropage` | pass / `0x01` / 13 | pass / `0x01` / 14 |
| `accuracycoin/$EB   SBC immediate` | pass / `0x01` / 18 | pass / `0x01` / 20 |
| `accuracycoin/$EF   ISC absolute` | pass / `0x01` / 13 | pass / `0x01` / 14 |
| `accuracycoin/$F3   ISC indirect,Y` | pass / `0x01` / 13 | pass / `0x01` / 14 |
| `accuracycoin/$F7   ISC zeropage,X` | pass / `0x01` / 13 | pass / `0x01` / 14 |
| `accuracycoin/$FB   ISC absolute,Y` | pass / `0x01` / 13 | pass / `0x01` / 14 |
| `accuracycoin/$FF   ISC absolute,X` | pass / `0x01` / 13 | pass / `0x01` / 14 |
| `accuracycoin/APU Register Activation` | fail / `0x12` / 120 | fail / `0x12` / 121 |
| `accuracycoin/Address $2004 behavior` | fail / `0x0e` / 557 | fail / `0x0e` / 558 |
| `accuracycoin/Arbitrary Sprite zero` | pass / `0x01` / 551 | pass / `0x01` / 537 |
| `accuracycoin/Branch Dummy Reads` | pass / `0x01` / 92 | pass / `0x01` / 94 |
| `accuracycoin/CHR ROM is not writable` | pass / `0x01` / 150 | pass / `0x01` / 159 |
| `accuracycoin/Controller Clocking` | pass / `0x09` / 120 | pass / `0x09` / 121 |
| `accuracycoin/Controller Strobing` | fail / `0x12` / 120 | fail / `0x12` / 121 |
| `accuracycoin/DMA + $2002 Read` | pass / `0x05` / 532 | pass / `0x05` / 533 |
| `accuracycoin/DMA + $2007 Read` | pass / `0x01` / 532 | pass / `0x01` / 533 |
| `accuracycoin/DMA + $2007 Write` | pass / `0x01` / 532 | pass / `0x01` / 533 |
| `accuracycoin/DMA + $4015 Read` | fail / `0x0a` / 532 | fail / `0x0a` / 533 |
| `accuracycoin/DMA + $4016 Read` | pass / `0x09` / 532 | pass / `0x09` / 533 |
| `accuracycoin/DMA + Open Bus` | pass / `0x01` / 532 | pass / `0x01` / 533 |
| `accuracycoin/DMA Landing on Write` | fail / `0x16` / 92 | fail / `0x16` / 94 |
| `accuracycoin/DMC DMA + OAM DMA` | fail / `0x0a` / 532 | fail / `0x0a` / 533 |
| `accuracycoin/DMC DMA Bus Conflicts` | fail / `0x0a` / 532 | fail / `0x0a` / 533 |
| `accuracycoin/DMC Reload Timing` | pass / `0x01` / 92 | pass / `0x01` / 94 |
| `accuracycoin/Delta Modulation Channel` | pass / `0x01` / 120 | pass / `0x01` / 121 |
| `accuracycoin/Explicit DMA Abort` | fail / `0x0a` / 532 | fail / `0x0a` / 533 |
| `accuracycoin/Frame Counter 4-step` | pass / `0x01` / 120 | pass / `0x01` / 121 |
| `accuracycoin/Frame Counter 5-step` | pass / `0x01` / 120 | pass / `0x01` / 121 |
| `accuracycoin/Frame Counter IRQ` | pass / `0x01` / 120 | pass / `0x01` / 121 |
| `accuracycoin/Frozen OAM2 Increment` | fail / `0x0a` / 151 | fail / `0x0a` / 122 |
| `accuracycoin/Hybrid Addresses` | pass / `0x01` / 212 | fail / `0x0a` / 212 |
| `accuracycoin/INC $4014` | pass / `0x01` / 557 | pass / `0x01` / 558 |
| `accuracycoin/Implicit DMA Abort` | fail / `0x0a` / 532 | fail / `0x0a` / 533 |
| `accuracycoin/Implied Dummy Reads` | fail / `0x0e` / 92 | fail / `0x0e` / 94 |
| `accuracycoin/Instruction Timing` | pass / `0x01` / 92 | pass / `0x01` / 94 |
| `accuracycoin/Internal Data Bus` | fail / `0x0a` / 92 | fail / `0x0a` / 94 |
| `accuracycoin/Interrupt flag latency` | fail / `0x0a` / 141 | fail / `0x0a` / 151 |
| `accuracycoin/JSR Edge Cases` | pass / `0x01` / 92 | pass / `0x01` / 94 |
| `accuracycoin/Length Counter` | pass / `0x01` / 120 | pass / `0x01` / 121 |
| `accuracycoin/Length Table` | pass / `0x01` / 120 | pass / `0x01` / 121 |
| `accuracycoin/Misaligned OAM DMA` | pass / `0x01` / 551 | pass / `0x01` / 537 |
| `accuracycoin/Misaligned OAM behavior` | fail / `0x16` / 551 | fail / `0x16` / 537 |
| `accuracycoin/Misaligned OAM2 Address` | fail / `0x0e` / 151 | fail / `0x0e` / 122 |
| `accuracycoin/NMI Control` | pass / `0x01` / 1437 | pass / `0x01` / 1418 |
| `accuracycoin/NMI Overlap BRK` | fail / `0x0a` / 141 | fail / `0x0a` / 151 |
| `accuracycoin/NMI Overlap IRQ` | fail / `0x06` / 141 | fail / `0x06` / 151 |
| `accuracycoin/NMI Suppression` | fail / `0x06` / 1437 | pass / `0x01` / 1418 |
| `accuracycoin/NMI Timing` | fail / `0x06` / 1437 | pass / `0x01` / 1418 |
| `accuracycoin/NMI at VBlank end` | fail / `0x06` / 1437 | pass / `0x01` / 1418 |
| `accuracycoin/NMI disabled at VBlank` | fail / `0x06` / 1437 | pass / `0x01` / 1418 |
| `accuracycoin/OAM Corruption` | fail / `0x0a` / 551 | fail / `0x0a` / 537 |
| `accuracycoin/PPU Read Buffer` | pass / `0x41` / 150 | pass / `0x41` / 159 |
| `accuracycoin/PPU Register Mirroring` | pass / `0x01` / 150 | pass / `0x01` / 159 |
| `accuracycoin/PPU Register Open Bus` | pass / `0x01` / 150 | pass / `0x01` / 159 |
| `accuracycoin/Palette RAM Quirks` | pass / `0x01` / 150 | pass / `0x01` / 159 |
| `accuracycoin/Rendering Flag Behavior` | pass / `0x01` / 557 | pass / `0x01` / 558 |
| `accuracycoin/Sprite 0 Hit behavior` | pass / `0x01` / 551 | pass / `0x01` / 537 |
| `accuracycoin/Sprite overflow behavior` | pass / `0x01` / 551 | pass / `0x01` / 537 |
| `accuracycoin/Sprites On Scanline 0` | fail / `0x0a` / 151 | fail / `0x0a` / 122 |
| `accuracycoin/Stale Sprite Shift Regs` | fail / `0x0e` / 151 | fail / `0x0e` / 122 |
| `accuracycoin/Suddenly Resize Sprite` | pass / `0x01` / 551 | pass / `0x01` / 537 |
| `accuracycoin/VBlank beginning` | pass / `0x01` / 1437 | pass / `0x01` / 1418 |
| `accuracycoin/VBlank end` | pass / `0x01` / 1437 | pass / `0x01` / 1418 |
| `accuracycoin/t Register Quirks` | pass / `0x01` / 557 | pass / `0x01` / 558 |
| `dabg/boot/frame 120` | 85773744 ticks / `3626e1456fcc49b48f63729b0e56e66fc063808f42a9a38cf109941192eff4be` | 85773792 ticks / `3626e1456fcc49b48f63729b0e56e66fc063808f42a9a38cf109941192eff4be` | The poll/interrupt instruction path shifts ticks at this selected frame; rendered hash is unchanged. |
| `dabg/boot/frame 180` | 128660136 ticks / `3626e1456fcc49b48f63729b0e56e66fc063808f42a9a38cf109941192eff4be` | 128660376 ticks / `3626e1456fcc49b48f63729b0e56e66fc063808f42a9a38cf109941192eff4be` | The poll/interrupt instruction path shifts ticks at this selected frame; rendered hash is unchanged. |
| `dabg/boot/frame 30` | 21443472 ticks / `3626e1456fcc49b48f63729b0e56e66fc063808f42a9a38cf109941192eff4be` | 21443520 ticks / `3626e1456fcc49b48f63729b0e56e66fc063808f42a9a38cf109941192eff4be` | The poll/interrupt instruction path shifts ticks at this selected frame; rendered hash is unchanged. |
| `dabg/boot/frame 60` | 42886680 ticks / `3626e1456fcc49b48f63729b0e56e66fc063808f42a9a38cf109941192eff4be` | 42886728 ticks / `3626e1456fcc49b48f63729b0e56e66fc063808f42a9a38cf109941192eff4be` | The poll/interrupt instruction path shifts ticks at this selected frame; rendered hash is unchanged. |
| `dabg/both/frame 120` | 85773072 ticks / `88262d7171fee2de7d02188cc13ec80585860eecb76d4fc2d2356bda733bde8d` | 85772760 ticks / `88262d7171fee2de7d02188cc13ec80585860eecb76d4fc2d2356bda733bde8d` | The poll/interrupt instruction path shifts ticks at this selected frame; rendered hash is unchanged. |
| `dabg/both/frame 180` | 128659512 ticks / `88262d7171fee2de7d02188cc13ec80585860eecb76d4fc2d2356bda733bde8d` | 128659272 ticks / `88262d7171fee2de7d02188cc13ec80585860eecb76d4fc2d2356bda733bde8d` | The poll/interrupt instruction path shifts ticks at this selected frame; rendered hash is unchanged. |
| `dabg/both/frame 30` | 21443400 ticks / `f5311d20271e6c0982005106a38f98806db1fdb22212746ead9659d4fb8f3d62` | 21443448 ticks / `f5311d20271e6c0982005106a38f98806db1fdb22212746ead9659d4fb8f3d62` | The poll/interrupt instruction path shifts ticks at this selected frame; rendered hash is unchanged. |
| `dabg/both/frame 60` | 42886440 ticks / `e2fd9e5e19f5465e70c2cf21047b8e5c8c003013316b35b05b244e81d792088d` | 42886392 ticks / `e2fd9e5e19f5465e70c2cf21047b8e5c8c003013316b35b05b244e81d792088d` | The poll/interrupt instruction path shifts ticks at this selected frame; rendered hash is unchanged. |
| `dabg/p0/frame 120` | 85774320 ticks / `d24f81ec1f58cbe5994e43f26eac9133d99316d7ba46702c76feb33af1291fbc` | 85774344 ticks / `d24f81ec1f58cbe5994e43f26eac9133d99316d7ba46702c76feb33af1291fbc` | The poll/interrupt instruction path shifts ticks at this selected frame; rendered hash is unchanged. |
| `dabg/p0/frame 180` | 128662176 ticks / `cbf5980d288e31fce3a07ab8798f287474636a042f63930049417e6aeb18eddf` | 128662224 ticks / `cbf5980d288e31fce3a07ab8798f287474636a042f63930049417e6aeb18eddf` | The poll/interrupt instruction path shifts ticks at this selected frame; rendered hash is unchanged. |
| `dabg/p0/frame 60` | 42886536 ticks / `5695548cb78c27711d1bc58b68ab2c75e45b22d8ee4485a1cc29d0e645041e59` | 42886728 ticks / `5695548cb78c27711d1bc58b68ab2c75e45b22d8ee4485a1cc29d0e645041e59` | The poll/interrupt instruction path shifts ticks at this selected frame; rendered hash is unchanged. |
| `dabg/p1/frame 120` | 85774320 ticks / `8de12125d32e21d317aab3bbe1c32ef6862109da07585d092815a3d8bc12e7e6` | 85774344 ticks / `8de12125d32e21d317aab3bbe1c32ef6862109da07585d092815a3d8bc12e7e6` | The poll/interrupt instruction path shifts ticks at this selected frame; rendered hash is unchanged. |
| `dabg/p1/frame 180` | 128662176 ticks / `b45972289fa17f53d6b8c66cf9b79d89d5995a6d45e4fd09db3c7a0785467d56` | 128662224 ticks / `b45972289fa17f53d6b8c66cf9b79d89d5995a6d45e4fd09db3c7a0785467d56` | The poll/interrupt instruction path shifts ticks at this selected frame; rendered hash is unchanged. |
| `dabg/p1/frame 60` | 42886536 ticks / `680473e4b6433630a943ab6f5470c4c3fcce4c1cffb5cf7934ce184bc602f7c4` | 42886728 ticks / `680473e4b6433630a943ab6f5470c4c3fcce4c1cffb5cf7934ce184bc602f7c4` | The poll/interrupt instruction path shifts ticks at this selected frame; rendered hash is unchanged. |
| `holymapperel/m1sgrom/audio/pcm` | — / `01e16205667412937ee88cb3ddd5fd953c08460f03e2bc80dc52761e10293d4e` | — / `2ed54534760c85dc60c303f080596a1e4b34042459fce0c782373cb558db430b` | NMI delivery changes guest/APU register and transition timing; PCM and transitions are measured separately from video. |
| `holymapperel/m1sgrom/audio/transitions` | — / `d958c5697b2dc8913bd57d947f35c62b3369d5ffb04b58674d9262fd9b391b32` | — / `d01cf80d6e9743977bee6f6e94b329981c94dc80eb43c440ca0bee6e367bb025` | NMI delivery changes guest/APU register and transition timing; PCM and transitions are measured separately from video. |
| `holymapperel/m1sjrom/audio/pcm` | — / `fadb97ca0b0577f3709a538048e4d3f3b15db8be1a6a16d6c0d4d72c730a7478` | — / `f26096b138754e93fde55ffead15dbbcec5730ea83ab90e20d8a02b3c59a1b57` | NMI delivery changes guest/APU register and transition timing; PCM and transitions are measured separately from video. |
| `holymapperel/m1sjrom/audio/transitions` | — / `57865633e31562b18aec8d27ffa5706d1f2d4e98a8070651bab351efe715dac7` | — / `f3e48c59b193effab5b22c7453d982e3d32fd213c17484d146f79c6195041a6b` | NMI delivery changes guest/APU register and transition timing; PCM and transitions are measured separately from video. |
| `holymapperel/m1skrom/audio/pcm` | — / `7291ebf80ad06d8f2e81b57e62847455801fba7dd66781523883c80737404e95` | — / `9ea052a84d988b5e57a1d26b7faec9976eee56155eb015b36e722a180771e77a` | NMI delivery changes guest/APU register and transition timing; PCM and transitions are measured separately from video. |
| `holymapperel/m1skrom/audio/transitions` | — / `7bafb5137d0367deff299c47b83896814ef60071766699036caccda1a9bf979b` | — / `f692d45186ae0cf15270070f37676e2d1e11627421def2411ab29650ecd0821f` | NMI delivery changes guest/APU register and transition timing; PCM and transitions are measured separately from video. |
| `holymapperel/m1surom/audio/pcm` | — / `3f8c33cd0ef176e239640a848cae7f39212e043a5539c0f1d0caffd482fb886a` | — / `496cb689153fa8f675ac96cf1b5f9e80ff55d13b8a23c775cc1718f65eadfef0` | NMI delivery changes guest/APU register and transition timing; PCM and transitions are measured separately from video. |
| `holymapperel/m1surom/audio/transitions` | — / `c5d94c0ef1e626a6dfd7df822d0807f9476db89590e000d2e165eff24f2c709c` | — / `f00fda9d612e86784877358369617cced2048b4f0de7933aecfe79af08e0e088` | NMI delivery changes guest/APU register and transition timing; PCM and transitions are measured separately from video. |
| `holymapperel/m1sxrom.saved/audio/pcm` | — / `54022eb78f4042177f522554f17818af73cd6a3be511ccf2700453356ee57ab3` | — / `538e3dd4420115791101d8e8ff804e673b72ec1c3dec6fb2ff221f65f6f8e43d` | NMI delivery changes guest/APU register and transition timing; PCM and transitions are measured separately from video. |
| `holymapperel/m1sxrom.saved/audio/transitions` | — / `741de9d78cf1e977e52379d25dcd7e04a2b4142efe68d08d7d3dc3c5310f075f` | — / `f87f9cbbb7023654ad42cd52974d380ac0378d0a2f95e62706b19af16aea688f` | NMI delivery changes guest/APU register and transition timing; PCM and transitions are measured separately from video. |
| `holymapperel/m1sxrom/audio/pcm` | — / `838a68708053ae433e4daf743bb190273f2f4905346e3cd3de0aa791783aed98` | — / `2152f6d27a4fdb0aead97d9e47e521287f4d756d8011575220d62cbbd05c01a2` | NMI delivery changes guest/APU register and transition timing; PCM and transitions are measured separately from video. |
| `holymapperel/m1sxrom/audio/transitions` | — / `ee71d3e5c45b2d7d7867ee78b4ea3a55d70276814e793ce22622dc72eb66958f` | — / `1e65869718125b727493958ca876c63b25edc5fcc757fc8e97d6ba5f80a6a256` | NMI delivery changes guest/APU register and transition timing; PCM and transitions are measured separately from video. |
| `holymapperel/m2/audio/pcm` | — / `4fae20228e4475ecbb0105f5d69da7bfbde703d0422c2eed7e7489d7c51c9355` | — / `15c5798f585b40b8356f622c0b02ab2a278df088f3270cccba8366c26f1e6ba6` | NMI delivery changes guest/APU register and transition timing; PCM and transitions are measured separately from video. |
| `holymapperel/m2/audio/transitions` | — / `157ad3cadde314c0c13ff347dd36d652990f7815a90831e6c1c45b6e6512cc7b` | — / `59f4b3d3bca41a1c43ba4574ae8ad633c065cefe4cc570837da2658afc88ff83` | NMI delivery changes guest/APU register and transition timing; PCM and transitions are measured separately from video. |
| `holymapperel/m3/audio/pcm` | — / `9d6624ca49c255d35d2cf23b3cc21bec6368e6433c5c2b342fe6bdf3018b8500` | — / `1aad24dfd56f6731417fb34ae43c980ccc45f0b4e3b419e7a03bf098e0191ab3` | NMI delivery changes guest/APU register and transition timing; PCM and transitions are measured separately from video. |
| `holymapperel/m3/audio/transitions` | — / `69e0b18fd668771eae33006dea3732531513e93594d9cd7d34d00f9ce5ee7605` | — / `c676257e63022a8f3fa1346708be58762f68306801076dd6714088667690d5b1` | NMI delivery changes guest/APU register and transition timing; PCM and transitions are measured separately from video. |
| `holymapperel/m4tkrom.saved/audio/pcm` | — / `2d48e8a568154cef4dbc802e7826bbba8f0e068b8f04694510d7b867da8b9776` | — / `6b197559751e03560774d7d8b7244ca1125ef162a8c53809209f3dfbcd385ca1` | NMI delivery changes guest/APU register and transition timing; PCM and transitions are measured separately from video. |
| `holymapperel/m4tkrom.saved/audio/transitions` | — / `cf3e93cdc0a85176157587afd65dbb92b8b62caf02fba778597ef4b46e67486c` | — / `beedbb31a350d584431aff6a3a73b785a183043b01f64367d993676142219eda` | NMI delivery changes guest/APU register and transition timing; PCM and transitions are measured separately from video. |
| `holymapperel/m4tkrom/audio/pcm` | — / `2f181d70d7cfffcbef205dc7215519a956830ec882c90474a684340ce48077c3` | — / `04d64ffb7779667e58bc03cb2f00ad18d7edbe9798a67aacd6b7f52d440d235c` | NMI delivery changes guest/APU register and transition timing; PCM and transitions are measured separately from video. |
| `holymapperel/m4tkrom/audio/transitions` | — / `da3aaa56fdf6b61641eeaf50f7d560df6004708437d55bbc02d36bd7b125c0f8` | — / `5b63501202b53aa34af161817fde4cc1f649aefc2379de03f0ead1bbbe1f15c3` | NMI delivery changes guest/APU register and transition timing; PCM and transitions are measured separately from video. |
| `holymapperel/m4tnrom/audio/pcm` | — / `ed97f98dffb986feefc934c7f587980ccb0f07d92c25152f62e72f94a1b6e649` | — / `7de6bb9d9360e917b2faced225934521ce31a54e4bfc9956c419abd61e75cd10` | NMI delivery changes guest/APU register and transition timing; PCM and transitions are measured separately from video. |
| `holymapperel/m4tnrom/audio/transitions` | — / `b2f60efe237274724ce7b6cafe6af522bcacfd2d6d8a3d8f5684628e6d813fef` | — / `3b67e4f14cecba52bc27755eff315a7e3f1fdc1e51a46c0106e65a9d19b5b874` | NMI delivery changes guest/APU register and transition timing; PCM and transitions are measured separately from video. |
| `holymapperel/m4txrom/audio/pcm` | — / `27fa5c6e3c38758b9e2b35e6e47e0dce4dd4260cb60e01ca922301812859e663` | — / `d046014d26cacefc679ee2e829ddc890db1b5f3ae5dc7575387ee91471d9a238` | NMI delivery changes guest/APU register and transition timing; PCM and transitions are measured separately from video. |
| `holymapperel/m4txrom/audio/transitions` | — / `e58f0d3c3f5cbea394de63e626729df6f6779c9d5b5609575f0272c737af2e44` | — / `29f98067b287073d3b0e2842c885f7bc4946cd43a055b3c28c507661a8d5bb48` | NMI delivery changes guest/APU register and transition timing; PCM and transitions are measured separately from video. |
| `holymapperel/m4w8k/audio/pcm` | — / `2f181d70d7cfffcbef205dc7215519a956830ec882c90474a684340ce48077c3` | — / `04d64ffb7779667e58bc03cb2f00ad18d7edbe9798a67aacd6b7f52d440d235c` | NMI delivery changes guest/APU register and transition timing; PCM and transitions are measured separately from video. |
| `holymapperel/m4w8k/audio/transitions` | — / `da3aaa56fdf6b61641eeaf50f7d560df6004708437d55bbc02d36bd7b125c0f8` | — / `5b63501202b53aa34af161817fde4cc1f649aefc2379de03f0ead1bbbe1f15c3` | NMI delivery changes guest/APU register and transition timing; PCM and transitions are measured separately from video. |
| `holymapperel/m7/audio/pcm` | — / `bfa6ccd0efdce20e68f85f6a5193f07ca7ce1e983b92ca6918e6e80574c7d6f1` | — / `c506c30dad28bcd83e362689b999e0c7582737f7ba60cd73c3d745b04ab1bb97` | NMI delivery changes guest/APU register and transition timing; PCM and transitions are measured separately from video. |
| `holymapperel/m7/audio/transitions` | — / `6aec4cb3d0f1985e53dfe6bac331fd9a20869b9129a556050d4e2bc354929164` | — / `95d028d5842e0becc700313cb49412bb6833226217950d315e2058d2e143c3c2` | NMI delivery changes guest/APU register and transition timing; PCM and transitions are measured separately from video. |
| `nesrunner/boot/audio/pcm` | — / `7418d67120f61e710d4e272706fe11c2a8fddb7eea66ddcbe56614bbe7ecf0e4` | — / `d0d182ca80131e5c882386fe07cbc4df43963c15392e1a2da8f9d4c8482f30a5` | NMI delivery changes guest/APU register and transition timing; PCM and transitions are measured separately from video. |
| `nesrunner/boot/audio/transitions` | — / `3c74022bb75349d43083d15859e8c37baef8c54132d181beb4913e108b84f98a` | — / `eead1b35c3a77b5013c6a12149f222fa7ce044f140f2663cfd184b98baa56f58` | NMI delivery changes guest/APU register and transition timing; PCM and transitions are measured separately from video. |
| `nesrunner/boot/frame 120` | 85774200 ticks / `65dbd82319b50546b14bdf6067d867ff7f4ad37f5cddd4c9e9470c1b9aa9d7d4` | 85773312 ticks / `65dbd82319b50546b14bdf6067d867ff7f4ad37f5cddd4c9e9470c1b9aa9d7d4` | The poll/interrupt instruction path shifts ticks at this selected frame; rendered hash is unchanged. |
| `nesrunner/boot/frame 180` | 128661072 ticks / `d407ddd6100a70ccd3c030842f4adb745587aa6eac88a2544034219250fea055` | 128659872 ticks / `d407ddd6100a70ccd3c030842f4adb745587aa6eac88a2544034219250fea055` | The poll/interrupt instruction path shifts ticks at this selected frame; rendered hash is unchanged. |
| `nesrunner/boot/frame 30` | 21443544 ticks / `ac8b18b70121fb8c3bc114d7ccd9fd3f5d12bda58ae8af23fd080a59c5990820` | 21443448 ticks / `ac8b18b70121fb8c3bc114d7ccd9fd3f5d12bda58ae8af23fd080a59c5990820` | The poll/interrupt instruction path shifts ticks at this selected frame; rendered hash is unchanged. |
| `nesrunner/boot/frame 60` | 42887112 ticks / `d407ddd6100a70ccd3c030842f4adb745587aa6eac88a2544034219250fea055` | 42886800 ticks / `d407ddd6100a70ccd3c030842f4adb745587aa6eac88a2544034219250fea055` | The poll/interrupt instruction path shifts ticks at this selected frame; rendered hash is unchanged. |
| `nesrunner/play/audio/pcm` | — / `509cb4c1625d855b45ac65e3678d9c047e19732f65cdd98a6b840ad3f6e7b7f5` | — / `255636674903d03dd9cb7ca632246c3375dff8c88300155b052ac5c273d4dcca` | NMI delivery changes guest/APU register and transition timing; PCM and transitions are measured separately from video. |
| `nesrunner/play/audio/transitions` | — / `8c5368976985549a8800301ac8c9a034b7c62d990ba8c207ee734c5b24c6413b` | — / `33e4bce709976dca002689e4cc164f694a677d9949fdfa325b7dd6cfa5cc4006` | NMI delivery changes guest/APU register and transition timing; PCM and transitions are measured separately from video. |
| `nesrunner/play/frame 120` | 85773936 ticks / `b53d3a8087edb1555ef0039ad438e8643dd8fbd72267a3d3566a1d5e504ccf3c` | 85773696 ticks / `b53d3a8087edb1555ef0039ad438e8643dd8fbd72267a3d3566a1d5e504ccf3c` | The poll/interrupt instruction path shifts ticks at this selected frame; rendered hash is unchanged. |
| `nesrunner/play/frame 180` | 128661072 ticks / `b1233a42c80a47d1f560ed28edd2fadc55efb273b2ff029dbca6a18352335595` | 128660520 ticks / `c2e72c42c608a8106d4fbd3382d669b670af2eee6e2cd3827004a1f064a31fea` | Changed NMI delivery changes the running guest state rendered at this frame. |
| `nesrunner/play/frame 30` | 21443544 ticks / `ac8b18b70121fb8c3bc114d7ccd9fd3f5d12bda58ae8af23fd080a59c5990820` | 21443448 ticks / `ac8b18b70121fb8c3bc114d7ccd9fd3f5d12bda58ae8af23fd080a59c5990820` | The poll/interrupt instruction path shifts ticks at this selected frame; rendered hash is unchanged. |
| `nesrunner/play/frame 300` | 214434816 ticks / `499ab10106bb67c663b295a81639c1505a39554fc1e5fadd9a6937711021b629` | 214434000 ticks / `de7969e7b644a23f2095ca1eb60f26b3743512a32148fcd3c32cba3735d933ae` | Changed NMI delivery changes the running guest state rendered at this frame. |
| `nesrunner/play/frame 450` | 321652296 ticks / `8570a4e5b35385c2d7b1dc7278b40cd4d7b6fe7e60ade1324377278c537cda51` | 321651120 ticks / `207054b1e158cd13ba94f3ba4875b381e5bec2d9d58ff22a4b9a9704823c929a` | Changed NMI delivery changes the running guest state rendered at this frame. |
| `nesrunner/play/frame 60` | 42887112 ticks / `d407ddd6100a70ccd3c030842f4adb745587aa6eac88a2544034219250fea055` | 42886800 ticks / `d407ddd6100a70ccd3c030842f4adb745587aa6eac88a2544034219250fea055` | The poll/interrupt instruction path shifts ticks at this selected frame; rendered hash is unchanged. |
| `nesrunner/play/frame 600` | 428868504 ticks / `d7f312362cb9b57c2bf51de97a3242112d90052138ba2a89b2aa2b9a47aeb55c` | 428867544 ticks / `14422e39cd4a56d1920a7c9cb02b0877c1d0ecf24f4d6e5b8ac4e3e3671865e0` | Changed NMI delivery changes the running guest state rendered at this frame. |
| `nesteroids/boot/frame 120` | 85773912 ticks / `bbddb0e13f03ab3b31b4b68c168e8ebf7a9e28f76452cd5feb72ee3cabf4c1e9` | 85773336 ticks / `bbddb0e13f03ab3b31b4b68c168e8ebf7a9e28f76452cd5feb72ee3cabf4c1e9` | The poll/interrupt instruction path shifts ticks at this selected frame; rendered hash is unchanged. |
| `nesteroids/boot/frame 180` | 128661144 ticks / `7cecae1314a25b2427bd22417bd6cdee2077492037dda45deef5a730a487aad3` | 128659800 ticks / `7cecae1314a25b2427bd22417bd6cdee2077492037dda45deef5a730a487aad3` | The poll/interrupt instruction path shifts ticks at this selected frame; rendered hash is unchanged. |
| `nesteroids/boot/frame 30` | 21443376 ticks / `e6afba6666d9d0ba1b9ae2f2f3c3f4e5f71f6bf1995f8b20e1e198a055bd8bf9` | 21443088 ticks / `e6afba6666d9d0ba1b9ae2f2f3c3f4e5f71f6bf1995f8b20e1e198a055bd8bf9` | The poll/interrupt instruction path shifts ticks at this selected frame; rendered hash is unchanged. |
| `nesteroids/boot/frame 60` | 42886872 ticks / `73f4a7d3c377a519c8f990f9d9179730bf5cf86553e3fa1d79b59e8c17b99cdf` | 42886440 ticks / `73f4a7d3c377a519c8f990f9d9179730bf5cf86553e3fa1d79b59e8c17b99cdf` | The poll/interrupt instruction path shifts ticks at this selected frame; rendered hash is unchanged. |
| `rhde/boot/frame 120` | 85772832 ticks / `cac57d02298fefb360f916c1ee67683db9beab13336d797b368e55a90801d2b3` | 85770816 ticks / `cac57d02298fefb360f916c1ee67683db9beab13336d797b368e55a90801d2b3` | The poll/interrupt instruction path shifts ticks at this selected frame; rendered hash is unchanged. |
| `rhde/boot/frame 180` | 128659272 ticks / `cac57d02298fefb360f916c1ee67683db9beab13336d797b368e55a90801d2b3` | 128655960 ticks / `cac57d02298fefb360f916c1ee67683db9beab13336d797b368e55a90801d2b3` | The poll/interrupt instruction path shifts ticks at this selected frame; rendered hash is unchanged. |
| `rhde/boot/frame 30` | 21443136 ticks / `cac57d02298fefb360f916c1ee67683db9beab13336d797b368e55a90801d2b3` | 21442920 ticks / `cac57d02298fefb360f916c1ee67683db9beab13336d797b368e55a90801d2b3` | The poll/interrupt instruction path shifts ticks at this selected frame; rendered hash is unchanged. |
| `rhde/boot/frame 60` | 42886392 ticks / `cac57d02298fefb360f916c1ee67683db9beab13336d797b368e55a90801d2b3` | 42885600 ticks / `cac57d02298fefb360f916c1ee67683db9beab13336d797b368e55a90801d2b3` | The poll/interrupt instruction path shifts ticks at this selected frame; rendered hash is unchanged. |
| `state/dabg 0` | 0 ticks / `f105d0501e23a6bc10c626b99961258757fbb4e0cd2726f45cd249ed75def012` | 0 ticks / `91f3b2a5c1654c84b94ba8aabbb20dd7fad7a856718f7b900c17473e0b277386` | Format-2 state now serializes distinct captured and polled NMI history; payload size grows by one byte. |
| `state/dabg 60` | 42886680 ticks / `ea063a567aaee6a8cd2cfad1527356914a8cfd1e9c2ab1f50c7f80c7b7b3afb6` | 42886728 ticks / `81c1d20e87ccaf2f2778bac95f715fe8fa1fd671aabacf98fcbb54aabf0c629d` | Format-2 state now serializes distinct captured and polled NMI history; payload size grows by one byte. |
| `state/dabg/size` | size 13087 / `—` | size 13088 / `—` | Format-2 state now serializes distinct captured and polled NMI history; payload size grows by one byte. |
| `state/m1sxrom 0` | 0 ticks / `9285b5e4fb97e6057bde66d7c1f4fbdd0641188d3f791e861d9b1b99aff2dd55` | 0 ticks / `d020d71d8eee4c4cc74b6de2a66c8745374f5d3f304367fbdb6090812a3af012` | Format-2 state now serializes distinct captured and polled NMI history; payload size grows by one byte. |
| `state/m1sxrom 60` | 42885624 ticks / `070dec6aa7185cec5590a692d3ff6f01c8685826a31a05be9d1ec45cf1b01859` | 42885624 ticks / `19ebec22c18e9d251f047f7b89d29c3ae8bfbb9bcf6c2e8bd7887b8e2df22d09` | Format-2 state now serializes distinct captured and polled NMI history; payload size grows by one byte. |
| `state/m1sxrom/size` | size 45869 / `—` | size 45870 / `—` | Format-2 state now serializes distinct captured and polled NMI history; payload size grows by one byte. |
| `state/m2 0` | 0 ticks / `387ac411805ab63a1a8fe612f94cd0cf40cd215a11c9b50fbcfa62b1e611738e` | 0 ticks / `1c974dd758e1a4554757d373db253d6562e4bdfc84b74c60488d352d555378e2` | Format-2 state now serializes distinct captured and polled NMI history; payload size grows by one byte. |
| `state/m2 60` | 42886032 ticks / `f8c54c7e0b968279c396daf6a5fe5d41b0b7616a635a249b8de2ad9f619f2e6c` | 42886032 ticks / `31c390cd7ccb6e47d3b319ec8d6c62b8bf58eccb5028717d2b0c67d40ad0e46c` | Format-2 state now serializes distinct captured and polled NMI history; payload size grows by one byte. |
| `state/m2/size` | size 13088 / `—` | size 13089 / `—` | Format-2 state now serializes distinct captured and polled NMI history; payload size grows by one byte. |
| `state/m3 0` | 0 ticks / `67c5067aff2a4076744cb87af3cd35c0258772ae2c1d55a1e5cd54f55f9d80d3` | 0 ticks / `469d139a5836ebea3dae6b87aa408b6697fb6f4f71aa48622ce7528992868a5b` | Format-2 state now serializes distinct captured and polled NMI history; payload size grows by one byte. |
| `state/m3 60` | 42885984 ticks / `208f9f9889dd74e22f652470dac059f210594bcea4b4e940e896a7073a0f08cc` | 42885984 ticks / `f1ba0b5355ecf335afdb4784aa653a5e6184b2112c85cce9e27bec7fade06a2f` | Format-2 state now serializes distinct captured and polled NMI history; payload size grows by one byte. |
| `state/m3/size` | size 4896 / `—` | size 4897 / `—` | Format-2 state now serializes distinct captured and polled NMI history; payload size grows by one byte. |
| `state/m4tkrom 0` | 0 ticks / `a43c893161026911671453f70d6d7f249ac0f98124743ced6b598a029dda0fa6` | 0 ticks / `193e5cd855b694b23f13539961650169a3ed689a5c30198b141b8311d04194c2` | Format-2 state now serializes distinct captured and polled NMI history; payload size grows by one byte. |
| `state/m4tkrom 60` | 42885984 ticks / `e77766a8ce80f06b8035c50e3572bff7451b67162e8eeccf67aaf020ab8621b2` | 42885984 ticks / `b7e3356328939081c3224e6bf79f9e48b0d8fa7eca74df2f366ced8d90fd800b` | Format-2 state now serializes distinct captured and polled NMI history; payload size grows by one byte. |
| `state/m4tkrom/size` | size 21303 / `—` | size 21304 / `—` | Format-2 state now serializes distinct captured and polled NMI history; payload size grows by one byte. |
| `state/m7 0` | 0 ticks / `0ce0219480c287d67494092430cd76bca5067bf1d08bdb96f6840ae3d5762102` | 0 ticks / `a7ce224a1348c0b4e9b9f74ee849522eb7c78f33395308c52e7dead83f60228d` | Format-2 state now serializes distinct captured and polled NMI history; payload size grows by one byte. |
| `state/m7 60` | 42885960 ticks / `bc6fd5040a6ae570822263c1b83b690fe89a55737cd9e769e9f7007c36c459ec` | 42885960 ticks / `51911b7a2b515ce1afa4a5d871026715ef7b958f9e55cb32e54ceea860a4ae72` | Format-2 state now serializes distinct captured and polled NMI history; payload size grows by one byte. |
| `state/m7/size` | size 13088 / `—` | size 13089 / `—` | Format-2 state now serializes distinct captured and polled NMI history; payload size grows by one byte. |

The metadata's `source_dirty=1`, campaign report, scoreboard hash, and input
fingerprint are retained as measured provenance. The source revision and
fingerprint describe the candidate used for this run; the local result does
not replace the cold protected-main observation or close the hosted vectors
fetch gap.

## Phase 20 Plan 04: serialized NMI cutpoints and bounded transition sweep

`core.nmi_state` runs complete `nesturbator__cpu_step` instructions from the
listed constructed origins. The origin is fixture state, not a claim of
cartridge-reset reachability; each named endpoint below is reached by actual
production instruction execution and is then passed through public save/load.
The disabled-NMI rows reproduce the earlier probe as a limited guard only.

| Probe | Constructed origin | Executed endpoint | Cycles / instructions | State at endpoint | Continuation evidence |
| --- | --- | --- | ---: | --- | --- |
| Disabled set guard | (261,0), NMI disabled | (241,1) | 87,069 / 29,023 JMPs | control=00, pending=0 | Matches 261,207 PPU dots; not an enabled-NMI acceptance case. |
| Disabled clear guard | (261,0), NMI disabled | (261,1) | 29,781 / 9,927 JMPs | control=00, pending=0 | Matches 89,343 PPU dots; not an enabled-NMI acceptance case. |
| Enabled VBlank set | (240,333), status=00, control=80 | (241,1) | 3 / 1 JMP | status=80, control=80, prev/pending/polled=0/0/0; audio phase=25,344, synth level=11,540 | Canonical save/load/save equal; uninterrupted/restored marker reaches 1 before the next frame and both instances then match frame ticks, video, 798 nonzero PCM samples, and full serialized state. |
| Enabled VBlank clear | (260,333), status=80, control=80, prev=1 | (261,1) | 3 / 1 JMP | status=00, control=80, prev/pending/polled=1/0/0; audio phase=25,344, synth level=11,540 | Canonical save/load/save equal; uninterrupted/restored marker reaches 1 before the next frame and both instances then match frame ticks, video, 798 nonzero PCM samples, and full serialized state. |
| Control 8 set edge before poll | (240,331), status=00, control=00; `STA $2000` with A=80 | (241,2) | 4 / 1 STA | PC=$8003; status=80, control=80, prev/captured/polled=1/1/0; audio phase=33,792, synth level=11,540 | Canonical load and re-save equal; the next LDX poll leads to the expected guest NMI marker 1. A CRC-correct state with `cpu.nmi_pending` omitted preserves the high live line but produces marker 0. The loaded instance first held pending=1, proving load clears stale latch history. |
| Control 8 clear edge before poll | (260,330), status=80, control=00; `STA $2000` with A=80 | (261,1) | 4 / 1 STA | PC=$8003; status=00, control=80, prev/captured/polled=1/1/0; audio phase=33,792, synth level=11,540 | Canonical load and re-save equal; the captured request survives VBlank clear, is polled by the next LDX, and reaches the expected guest NMI marker 1. Restored and uninterrupted frame output/state match. |

The bounded sweep holds CPU alignment at an existing instruction boundary and
tests every PPU-dot offset from -2 through +2 around each pinned transition.
Each run emits the full per-instruction PC/opcode, bus-cycle interval, PPU
coordinate, previous/captured/polled bits, guest count, and next action to
stdout; the table records every candidate's final executed boundary and marker
result. Origin dots are the base dot plus the listed offset: Control 8 starts
at 240/260:330..334; Control 9 starts at 240/260:309..313.

| Sequence | Edge | Offset | Final executed step | End PPU | Final step PC/opcode; bus cycles | prev/cap/poll | Guest count / handler-entry X |
| --- | --- | ---: | ---: | --- | --- | --- | ---: |
| Control 8 STA | set | -2 | 07 | 241:91 | $8005 / $8E; 30..34 | 1/0/0 | 1 / 16 |
| Control 8 STA | clear | -2 | 07 | 261:91 | $8005 / $8E; 30..34 | 0/0/0 | 1 / 16 |
| Control 9 INC | set | -2 | 11 | 241:121 | $8057 / $FF; 44..51 | 1/0/0 | 1 / 16 |
| Control 9 INC | clear | -2 | 07 | 261:76 | $8206 / $40; 30..36 | 0/0/0 | 1 / 0 |
| Control 8 STA | set | -1 | 07 | 241:92 | $8005 / $8E; 30..34 | 1/0/0 | 1 / 16 |
| Control 8 STA | clear | -1 | 11 | 261:101 | $8008 / $4C; 34..37 | 0/0/0 | 0 / 255 |
| Control 9 INC | set | -1 | 07 | 241:77 | $8206 / $40; 30..36 | 1/0/0 | 1 / 0 |
| Control 9 INC | clear | -1 | 11 | 261:101 | $800A / $4C; 41..44 | 0/0/0 | 0 / 255 |
| Control 8 STA | set | 0 | 07 | 241:93 | $8005 / $8E; 30..34 | 1/0/0 | 1 / 16 |
| Control 8 STA | clear | 0 | 11 | 261:102 | $8008 / $4C; 34..37 | 0/0/0 | 0 / 255 |
| Control 9 INC | set | 0 | 07 | 241:78 | $8206 / $40; 30..36 | 1/0/0 | 1 / 0 |
| Control 9 INC | clear | 0 | 11 | 261:102 | $800A / $4C; 41..44 | 0/0/0 | 0 / 255 |
| Control 8 STA | set | +1 | 07 | 241:94 | $8005 / $8E; 30..34 | 1/0/0 | 1 / 16 |
| Control 8 STA | clear | +1 | 11 | 261:103 | $8008 / $4C; 34..37 | 0/0/0 | 0 / 255 |
| Control 9 INC | set | +1 | 07 | 241:79 | $8206 / $40; 30..36 | 1/0/0 | 1 / 0 |
| Control 9 INC | clear | +1 | 11 | 261:103 | $800A / $4C; 41..44 | 0/0/0 | 0 / 255 |
| Control 8 STA | set | +2 | 07 | 241:95 | $8005 / $8E; 30..34 | 1/0/0 | 1 / 16 |
| Control 8 STA | clear | +2 | 11 | 261:104 | $8008 / $4C; 34..37 | 0/0/0 | 0 / 255 |
| Control 9 INC | set | +2 | 07 | 241:80 | $8206 / $40; 30..36 | 1/0/0 | 1 / 0 |
| Control 9 INC | clear | +2 | 11 | 261:104 | $800A / $4C; 41..44 | 0/0/0 | 0 / 255 |

Control 8 clear offset -2 is the captured-but-not-yet-polled boundary after
STA at (261,1): status is already clear while the captured request remains
set. Its restored continuation reaches NMI entry after LDX. Control 9's
six-cycle INC reaches NMI at its legal poll window; handler-entry X is 0 for
offsets -1 through +2 and 16 at offset -2, with all five set cases reaching
the expected marker. The set cases for both controls all reach the marker;
the clear cases only do so at offset -2. No candidate required a fabricated
mid-instruction state, target cap, or offset override.

The result establishes deterministic production save/load at complete CPU
instruction boundaries and at the modeled PPU coordinates. It does not decide
physical CPU/PPU A/C tie precedence beyond the source-backed global event order
already documented above; no public pause API, partial-instruction format, or
per-test timing adjustment was introduced.

The existing `core.state_reject` history matrix also passes across its ROM
fixtures: CRC-correct `pending=0, polled=1` is refused without instance
mutation; `pending=1, polled=0` and `pending=1, polled=1` remain canonically
loadable with the live NMI line low; the bounded-field sweep rejects
out-of-range history booleans; and format 1 retains its format-error
precedence. The new Control 8 clear replay separately reaches and restores a
captured request after the live PPU status has fallen.

## Phase 20 Plan 05: public host frame-boundary replay

This plan adds deterministic host replay evidence; it does not create an
AccuracyCoin campaign result or change the measured scoreboard. The owned NROM
fixture lives in test code and is written only into each runner or CTest scratch
directory. The AccuracyCoin metadata fingerprint was refreshed for the added
runner test registration while its ROM pin, report, scoreboard, source-dirty
provenance and local-vs-protected distinction were preserved.

The core fork and run-ahead tests check guest NMI count and one-based event
sequence at frame 0, the first NMI frame and successive boundaries. Speculative
depths 1–3 use changed inputs, two histories remain independent, and stale or
omitted restores are caught by the expected marker. The same test image changes
the visible backdrop and keeps pulse audio active; the checks cover native
state, displayed frame, frame number, ticks and non-silent PCM. The existing
`core.nmi_state` test remains the separate instruction-boundary and omitted
history oracle.

The runner process cases create frame-0, frame-1 and frame-4 states, verify the
saved guest sequence through the public state API, then load frame 1 in a fresh
process. Final native frame, ticks and serialized bytes match the uninterrupted
run. Transition and PCM hashes cover frames 2–4 only and match a direct
uninterrupted suffix reference. Idempotence is byte-for-byte; stale-checkpoint
and omitted-load controls finish normally and fail on the expected guest marker.

The libretro host serializes and restores at every displayed boundary for 12
ticks at each depth, in both real-future-input and repeated-current-input modes.
It checks the expected guest count and event sequence in each real and
speculative state, frame numbers, ticks, saved bytes, visible output and PCM.
Both omitted-load and stale-load controls must register a guest-marker
mismatch. Existing mapper, audio, serialization and JAM matrices remain in
place. These are public host frame-boundary results and do not replace Plan
20-04's exact modeled race cutpoints or the later hosted evidence required to
close KEEP-02/KEEP-03.

Validation on the local candidate: `core.state_fork.nmi`,
`core.state_runahead`, and `core.nmi_state` passed; all three
`runner.state.nmi_*` cases passed; and `libretro.host`, `libretro.serialize`,
and `libretro.runahead` passed. ASan/UBSan `libretro.host` and
`libretro.runahead` passed. The complete `cmake --workflow --preset ci` run
passed 683/683 tests and packaged the library, runner and libretro archives.
## Phase 20 Plan 06 final evidence refresh

The one final full-campaign invocation used the existing full-enabled CI wiring:

```sh
cmake --preset ci -DNESTURBATOR_ACCURACYCOIN_FULL=ON
cmake --build --preset ci
ctest --preset ci -R '^accuracycoin\.campaign$' --no-tests=error --output-on-failure
```

It passed all 22 pages and all 146 scored rows: 114 pass, 32 fail, 0 skip,
0 not-run, 0 timeout, and 5 DRAW. The six required target rows and complete
executed source arrays are listed above. The campaign report SHA-256 is
`29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac`; its
build-input fingerprint is `3465f9abcc455f5698f7c444417b496e617af9021a8c648ed5945346754ee3ed`
over 56 inputs; `source_revision=92db8e2a40a0`, `source_dirty=1`, and
`host_elapsed_guest_ns=7758848000`. Dirty provenance records the preserved
pre-existing `.planning/config.json` edit; protected-main authority remains
separate and was exercised by the cold observation/no-lost-pass checks.

The final generated hash inventory contains 114 sorted rows. Its SHA-256,
`b429a38b026750c6b0c9270bc3ce8b1a73d2bbb100ead0608ff5339dd51bc390`, matches
both `tests/runner/hashes.txt` and `build/ci/tests/hashes.txt`; the
`runner.write_hashes.content` comparison passed, so no native hash pin moved
in Plan 20-06. A six-platform scratch inventory with one duplicated row was
rejected at the expected 115-versus-114 row-count guard. The regular inventory
self-test also passed malformed-row, platform-digest mismatch, and missing or
renamed Holy Mapperel audio-key controls.

| KEEP-02 fallback backstop | Evidence and disposition |
|---|---|
| Adjacency: equal-tick frame/audio/state identities remain distinct | Inventory keys give frame, PCM, transition, and state lanes distinct names, and the full key inventory is unique. No explicit same-tick boundary fixture compares adjacent frame/audio/state keys, so this backstop remains unconfirmed. |
| Frame 0 and empty PCM retain canonical state and nonzero saved audio history | Plan 20-04 proves nonzero APU/synth history with no staged PCM at exact internal cutpoints and canonical save/load/save. Plan 20-05 separately covers frame-0 public replay. No fixture combines frame 0, empty PCM, and nonzero saved audio history, so this backstop remains unconfirmed. |
| Deterministic sorted inventory rejects equal keys | The generated complete 114-row inventory sorts identically to the checked-in file; the content check and six-platform inventory controls passed. The duplicate-row negative control was rejected. Confirmed. |

`NESTURBATOR_STATE_FORMAT_VERSION` remains 2 and behavior revision remains 8.
This refresh changes no runtime code and repins no frame, PCM, transition, or
state digest. Hosted exact-head six-platform equality/protected-PASS and
performance evidence remains pending.

The final local workflows also passed with the AccuracyCoin full option off:
`cmake --workflow --preset ci` completed 683/683 tests and generated the library,
runner, and libretro packages; `cmake --workflow --preset asan` completed
675/675 tests. The CI run includes the exact-six target gate, CPU-vector,
MMC3/Holy Mapperel, boot-to-play, state/JAM, host, and protected-pass checks.
These local workflow results do not supply hosted exact-head or cross-platform
evidence.

## Phase 20 Plan 07 local campaign refresh

The single full-enabled campaign for Plan 20-07 ran from committed source
revision `af268f348619` with `source_dirty=1`. The dirty checkout contained the
preserved `.planning/config.json` and `.planning/STATE.md` edits; the baseline
metadata and evidence updates below were made after measurement. The report
schema, ROM pin `6bee919529289c555803b612b8c9e4f7eaaeeabb`, ROM SHA-256
`8ac8904a3cf26fbdfa6be501e414d45886ff4478d30dee8e58fa436cf5fd5adc`, and
inventory SHA-256 `7d022800d01ad81159109bf7457af94ab6f80e4a682fce716ec476f7c454a4a5`
remain unchanged. The fingerprint is `152511afaec8cc129d7259eb49e5a72a11fb0e94adf9ed1757fc9526f0e77d84`
over 56 inputs; the measured report SHA-256 remains
`29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac`.

The run launched and completed all 22 pages and observed all 146 scored rows:
114 pass, 32 fail, 0 skip, 0 not-run, 0 timeout, and 5 DRAW. It measured
`host_elapsed_guest_ns=8625836000` with unchanged 32/5/1800 frame caps and
41,503 total frame budget. The exact six target rows all remain `pass`:
`accuracycoin/$2002 flag timing`, `accuracycoin/NMI Timing`,
`accuracycoin/NMI Suppression`, `accuracycoin/NMI Control`,
`accuracycoin/NMI at VBlank end`, and `accuracycoin/NMI disabled at VBlank`.
Fresh protected-main observation, protected verdict, and the no-lost-pass test
all passed; all 108 original PASS keys remain protected. The candidate hash
inventory content check passed without changing the checked-in hash inventory.

The focused CTest invocation initially reported the expected stale-fingerprint
activation failure after the campaign had recorded the new header-input hash;
campaign, exact-six target, protected-main, no-lost-pass, and hash-content tests
passed. Baseline metadata was then refreshed from the retained campaign output
and current fingerprint helper. The activation and target checks passed after
reconfiguring the full campaign off, without rerunning the measured campaign.
This local result does not close hosted exact-head, cross-platform, performance,
or Nightly `vectors-full` evidence.

## Phase 20 Plan 08: equal-tick identity and empty-output history witnesses

The production six-artifact inventory validator accepted a complete synthetic
fixture for `linux-arm64`, `linux-x64`, `macos-arm64`, `macos-x64`,
`windows-arm64`, and `windows-x64`. It retained these distinct full keys at
the same `42886728` ticks: `dabg/p0/frame 60` with digest `2222…2222`,
`dabg/p1/frame 60` with `3333…3333`, and `state/dabg 60` with `4444…4444`.
The fixture also retained distinct `dabg/boot/audio/pcm` (`5555…5555`) and
`dabg/boot/audio/transitions` (`6666…6666`) keys. These fixed synthetic digests
are fixture values, not emulator output. Every selected full key occurred once
in all six byte-identical artifacts.

The all-six duplicate control replaced the p1 frame key with the p0 row while
preserving sorted artifacts, row count, and byte equality across all six
platform files. The validator rejected it for not containing the expected
151-key inventory. Removing the p1 key from all six files was rejected at the
150-versus-151 inventory count; removing one key from only `windows-x64` was
rejected because that artifact differed. The existing malformed-row,
platform-difference, and Holy Mapperel missing/renamed controls remain active.
`hash.inventory.self_test` exercises these cases through the validator also
used by hosted hash-equality; local fixture equality is not an observed
six-platform hosted run.

`core.nmi_state` now records one combined internal frame-zero snapshot. From a
constructed `(240,333)` origin, one production JMP reaches `(241,1)` at frame
0 and tick 72. At that endpoint APU sample phase is 25,344, synth mixed level
is 11,540, impulse head is 0, integrator is 0, PCM count is 0, and
`sample_output` is null. Save over a destination poisoned by running a public
frame, then load and save again, produced canonical byte equality and direct
equality of the serialized APU/synth history. Empty drains on source and
restored instances returned 0, left the sentinel unchanged, kept the ring
empty, and left canonical state bytes unchanged.

Uninterrupted and restored continuation matched the guest NMI marker, native
state, frame/tick values, video, and nonempty 798-sample PCM with nonzero
content. The bounded lost-history control loaded the same valid state into a
third instance and reset only its synth history; the saved-state comparison
detected the loss and continuation diverged at frame 1, PCM sample 0, with the
guest count at 2. This is a test-owned perturbation, not a malformed-state
loader case. The origins and exact endpoint remain constructed/instruction-
executed evidence, not cartridge-reset reachability or a completed public
frame.

These results supersede the two earlier unconfirmed KEEP-02 fallback
dispositions locally. They do not establish hosted six-platform artifact
equality, hosted protected-PASS retention, or physical exact-tie behavior.
KEEP-02/KEEP-03, all four Phase 20 requirements, and the hosted and physical
debt remain open.

The single Plan 20-08 full-enabled campaign completed all 22 pages and 146
scored rows: 114 pass, 32 fail, 0 skip, 0 not-run, 0 timeout, and 5 DRAW. The
exact six target keys passed; the protected-main cold observation and all 108
starting PASS keys remain separately protected in the final CI checks. The
campaign report SHA-256 is
`29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac`; its
build-input fingerprint is
`1d324c83a1aac5a69939eecc149cf4e0e7b86b7175f1da26064969fa7cd26c0c` over 56
inputs; `source_revision=df10aafd2099`, `source_dirty=1`, and
`host_elapsed_guest_ns=8570616000`. The dirty provenance records these
uncommitted README/header documentation edits and the preserved pre-existing
`.planning/config.json` edit. Behavior revision 8, format 2, frame/PCM/state
pins, target set, and campaign caps are unchanged. This local measurement is
not a clean-source or hosted result.

## Phase 20 Plan 09: hosted evidence preparation

The one full-enabled local AccuracyCoin campaign ran after the collector CTest
registration and workflow edits. It completed all 22 pages and all 146 scored
rows: 114 pass, 32 fail, 0 skip, 0 not-run, 0 timeout, and 5 DRAW. The six
required NMI target keys were checked separately. The measured page-child total
is recorded as `host_elapsed_guest_ns=5958989000`; the report SHA-256 remains
`29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac`. The new
56-input AccuracyCoin build fingerprint is
`ebdaef6447e2ad737de5367bc85eec6fca365d8070953a3b58663c6d5262d864`, reflecting
the registered evidence control. Generated metadata names causal source
revision `f85e1b15f176` and records `source_dirty=1` because this local checkout
contains the README update and the preserved `.planning/config.json` edit.
This is local campaign provenance, not a clean or hosted result.

With the full option disabled after measurement, the focused local acceptance
group passed 9/9: the shared hosted-evidence controls, campaign activation,
exact six NMI targets, protected-main observation and verdict, all 108 original
PASS keys, and unchanged native hash inventory/content. The hosted-evidence
controls run offline through the same content validator as collection and
exercise named invalid source mappings, incomplete lanes, missing/expired
artifacts, dirty or stale campaign data, lost protected keys, hash omissions,
skipped/cancelled jobs, invalid performance evidence and missing host/Nightly
evidence. Synthetic passes remain fixtures; they do not supply hosted
provenance. No ROM bytes, native hash expectations, campaign caps, behavior
revision or state format changed.

`cmake --workflow --preset ci` then passed 684/684 tests with the full campaign
disabled and packaged the library, runner and libretro adapter. The full
workflow and local controls do not establish hosted exact-head, six-platform,
protected-main, performance, or Nightly acceptance. They leave physical
exact-tie precedence and the previously recorded local/Nightly fetch debt
unresolved.

## 2026-10-27 delivery formatting remeasurement

The normal pre-push check exposed unformatted unpublished source snapshots.
The formatting-only history map and provenance are retained in
`20-DELIVERY-COMMIT-MAP.tsv` and `20-DELIVERY.md` under the Phase 20 directory.
Historical source SHAs remain the identities actually measured; the mapping
does not turn them into measurements of rewritten source.

A fresh full campaign at source `28092e52803a` completed 22/22 pages and all
146 scored rows (114 pass, 32 fail, zero skip/not-run/timeout, five DRAW).
Its report SHA-256 stayed
`29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac`.
The actual refreshed 56-input fingerprint is
`b154c4a48e17134e1421a7852c52257fd6486c8ceb1a03beff0aa43fb606cab1`;
source_dirty is 1, including the preserved planning config edit and a temporary
Python import cache subsequently removed. Focused activation/target/protected/
hash/collector checks passed 10/10. Full campaign registration was disabled
after this one measurement. Native hash expectations and emulation behavior
were not changed. The effective source digest for delivery is `cf314a6d3c2494cefba4eb88bd6a779bb93e3d464e408103aa079b320bbb650f`
over 415 files; this supersedes 20-09's pre-format digest for new hosted
acceptance. Actual hosted results remain pending.

The formatted delivery source passed `cmake --workflow --preset ci`: 684/684
tests, followed by packaging of library, runner and libretro deliverables.

The first real PR runs exposed a GCC test narrowing warning, an MSVC-only
fixture-tool build definition omission and a missing generated host marker
cartridge. These are corrected in the existing test target/workflow seams.
Live API identities and full matrix names are now represented by the
collector's fixtures; bounded stale/head/merge/matrix controls remain strict.
After the target CMake edit, a newly measured 22-page campaign still reports
114 pass / 32 fail with the same report digest. Its actual 56-input fingerprint
is `7e9ebb45e5367df9b4b21f0fdbba86b19ca92a63b5bcbb592cfd9ef990a42e13`
and source revision is `d7ad9695d922`, dirty bit 1. The precise dirty-source
reason is preserved in baseline metadata. Focused acceptance passed 35/35.
The superseding full effective digest is
`f73937e970bedd54d6f9379d235f3f44887e014970a22718997c7929b6b2b3b5`
over 415 files. Source-bound real acceptance requires replacement hosted runs;
no old result is relabeled. Details are in `20-DELIVERY.md`.

The coherent hosted-runner/API repair passed final local CI 684/684 and
packaged all three deliverables; full campaign registration remains OFF.

## 2026-10-27 retained IRQ level restores the MMC3 guard

Actual Nightly 40501700252 and a local 13-test MMC3 run reproduced
`mmc3_test_2/4-scanline_timing` code 0x02 at frame 48. Protected main passed.
An owned NOP/INC witness exposes live IRQ 1 with the previous-close poll
required to remain 0 when A12 rises at a tied PPU transition. Capturing IRQ's
level at the existing close-edge detector seam restores this ordering; the
full MMC3 tier passes 13/13, with no oracle/ROM/offset change. The witness also
checks canonical load/continuation and retained-history restoration. Broader
IRQ I-flag/branch/hijack work remains Phase 21; no physical tie proof is claimed.

The final unpublished format-2 payload gains one bounded retained-IRQ byte.
`20-IRQ-STATE-HASH-MOVES.tsv` preserves all 12 measured old/new state hashes
and six +1-byte size rows. Other frame/PCM/transition hashes and ticks are
byte-identical. Size/CRC movement follows from that one serialized byte.
The state visitor and hostile-state table include the field; format-1 refusal
and behavior revision 8 remain in force.

A single fresh full-enabled campaign at source `2d904b7daef8`, dirty bit 1,
completed all 22 pages and 146 rows: 114 pass, 32 fail, zero skip/not-run/timeout,
five DRAW. The report digest remains
`29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac`;
the measured fingerprint is
`7a31d9bdc998cbb62593384dd7f809b9e4def2c8d259c31cb693d5a092431183`.
The exact-six/protected/hash/collector/state focused gate passed 15/15.
The effective source digest for fresh hosted acceptance is
`0cb4f7fec388edd64c7edf82760136689930e481c1e443fe3f7b13444219d436`
over 415 files. The full campaign option was disabled after measurement.

The final witness distinguishes retained IRQ 0 from live IRQ 1 at a final-cycle
A12 tie and restores it over poisoned history. Layout-dependent JAM controls
now target byte 92; the stress stimulus uses G=8 to retain OAM/DMC/poll coverage
within both full and short caps. All assertions remain active. Final local
verification passed CI 684/684, ASan 676/676 and MMC3 13/13; all three packages
were generated. The finalized effective source digest is
`30aabfeee892574cfe98ab69de1cacce4bfad45d9b7d72427e7abbd070e84f2a`
over 415 files, superseding the provisional fixture-stage digests. Campaign
fingerprint and frame/PCM/transition hashes remain as measured above.


## 2026-10-27 performance and isolated-session delivery repair

CI 40507506429 on b474e1d exposed missing runner builds in isolated session
jobs, a cold-interpreter cleanup setup race, and real per-ROM instruction
regressions of +37.54%, +36.43% and +38.34%. The companion Nightly MMC3 oracle
passed, confirming the preceding IRQ repair separately. These failed runs
remain historical and cannot certify the replacement source.

Both isolated workflows now build frame_index and nesturbator-run. The missing
runner failure reproduced in a fresh frame-index-only build and passed after
building the runner. Injecting 1.2 s per Python startup reproduced a lost-read
cleanup failure. Consolidating the fault and all witnesses into one interpreter
preserves the driver's five-second read slice, signal statuses and owned/foreign
process/lock assertions; the same delayed-startup matrix passes all eight cases.

The core avoids redundant PPU catch-ups where no early observation is required,
keeps scalar status sampling, inlines small private helpers and prioritizes PRG
reads. Sprite collision uses opacity and derived eligible-slot bits; dot-one
flag work and the overflow predicate avoid unnecessary repeated comparisons.
Mixer recomputation follows events that can change audible channel levels.
The two per-instance caches are not serialized: successful load rebuilds sprite
eligibility and invalidates mixer validity, while refusals remain atomic. Native
state bytes, behavior revision, state format and all existing hash pins stay
unchanged. Per-cycle all-channel/mode comparisons and loaded-history/eligibility
controls pass; removing either cache's load reconstruction produces the expected
wrong-level or wrong-sprite-hit failure.

A fresh 22-page campaign at actual source HEAD d5a2270989ef, dirty bit 1 for
these edits and the preserved config change, remains 114 PASS / 32 FAIL,
zero skip/not-run/timeout and five DRAW. Report digest remains
29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac.
Its measured build-input fingerprint is
3d427bc760a4be82aa6bba648c8b5f527fda5f6dae2b5b62f76bef3982f3338c
(56 inputs). Full campaign registration is OFF after that measurement.
The replacement effective source digest is
de6d4355222063e019c7a3a94e30a404b52041f3d994659d445f12b4a23318ab
(415 files). Earlier delivery digests are superseded for acceptance.

Local Linux x64 GCC 14.2 / Valgrind 3.22 measurements use the production
performance validator, exact 60/360-frame differencing, equal-length binary
paths, empty environments and two identical protected-main base measurements.
Against ab9c2e6, the first complete repaired measurement passes: nesteroids
+0.51%, runner -10.10%, MMC3 -5.85%. This is local dirty-source evidence, not
hosted acceptance. Full final CI/sanitizer results and new delivery/run
identities are recorded after verification and publication.

Final frozen-source verification passed CI 684/684 (all three packages),
ASan/UBSan 676/676 and MMC3 13/13. GCC 14.2 strict-warning syntax checks,
clang-format 18, workflow syntax, source hygiene and diff checks passed. A
fresh post-comment/format performance measurement produced the same passing
per-ROM results, with zero repeated-base spread. The existing config bytes
remain unchanged and unstaged. New hosted runs must certify the published
replacement; no phase/requirement closure or merge is claimed.

## 2026-10-27 Plan 20-11 collector repair

The full-enabled local AccuracyCoin campaign used causal source revision
`ac8456b60272`, with `source_dirty=1` because the pre-existing planning
configuration and execution-state edits remain in the checkout. It completed
all 22 pages and 146 scored rows: 114 pass, 32 fail, no skip/not-run/timeout,
and 5 DRAW. All six Phase 20 NMI target rows passed. The campaign report SHA-256
is `29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac`; its
measured 56-input fingerprint is
`b2cdcdd15785b590a415dac911187b8abfa0d025a1cc448d412650d1716f1240`, with
`host_elapsed_guest_ns=5017232000`. Native inventory and ROM pins are
unchanged. The effective hosted-evidence source tree is
`b07b42ac26cd92f698eec7a643bc77d5c3c4193ae9432b6f80032affdfabdede` over 416
files; planning files and this measurement ledger are excluded by the shared
digest contract.

The focused collector/availability/AccuracyCoin/hash/policy gate passed 12/12,
including the cold protected-main observation, all 108 protected PASS keys,
and no-lost-pass checks. This campaign and its controls are local preparation
only. The full campaign option was disabled after the single measurement, and
the effective source tree still requires fresh hosted exact-head evidence.
With the campaign disabled, `cmake --workflow --preset ci` passed all 686
tests and generated the library, runner, and libretro packages.

### Post-review correction: GitHub job logs are plain text

Before Plan 20-12 publication, review of the official GitHub Actions jobs API
and retained job `122224137324` from CI `40514060876/1` established that
`/actions/jobs/{job_id}/logs` returns a UTF-8-BOM timestamped plain-text job
log, not a ZIP. That old log was inspected only to validate response shape and
the CTest parser; it remains historical data and is not accepted as fresh
source-bound evidence. The collector now preserves the raw response byte for
byte, binds it to the selected successful workflow step's ID, run attempt and
API `started_at`/`completed_at` values, and re-derives the selected step log
from the preserved text before validating exact CTest result records. It
requires the unique expected group marker, excludes results outside the
metadata time window, and rejects missing, skipped, failed, duplicate and
contradictory results. The derived file normalizes a leading BOM, ANSI CSI
sequences and line endings; the raw SHA-256 and derived SHA-256 are retained
separately. Binary artifact ZIPs still use archive path, duplicate-member,
traversal and symlink checks. The raw GitHub CLI option is feature-detected
from installed `gh api --help`; raw job-log and artifact ZIP requests pass
`--allow-escape-sequences` through the same resolved executable.

The TDD correction's realistic adapter fixture first failed on the actual
plain-text response shape, then passed after the fix. Controls prove passing
CTests outside the selected step cannot count and prove skipped, failed,
duplicate and pass-plus-fail contradictory terminal results reject for the
expected reason. The old-CLI feature-detection control rejects before any raw
request. The production adapter also checks preserved BOM bytes and recomputes
the raw-to-selected-log digest mapping on validation. The captured historical
log replay passed the exact CMake terminal parser but remains data only.

Two full-enabled local campaigns completed after the correction. The first was
the planned measurement; a Task 2 focused CTest invoked the campaign once more
as a registered fixture while full mode was still enabled, so the latest
successful run is the recorded baseline below. Both runs completed 22/22 pages
and 146/146 scored rows (114 PASS, 32 FAIL, no skip/not-run/timeout, five
DRAW), kept all six NMI target keys passing, and produced the same report and
56-input fingerprint. The recorded run binds causal source revision
`f3b56e003dc8`, `source_dirty=1` for the preserved `.planning/config.json`
edit, `host_elapsed_guest_ns=4768711000`, fingerprint
`b2cdcdd15785b590a415dac911187b8abfa0d025a1cc448d412650d1716f1240`, and
report SHA-256
`29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac`. The
Task 2 focused gate passed 12/12 with the cold 108-key protected comparison,
all native hashes and policy controls. The Task 1 gate passed 28/28. With
full mode disabled, `cmake --workflow --preset ci` passed 686/686 tests in
389.05 seconds and generated the library, runner and libretro packages.

The repaired effective source digest is
`d1d4a456b40332a78678fbcf1ba1bf948a82641f20dfec5ecafbe0f2f678c69e` over 416
files, after refreshing the baseline metadata. It supersedes the prior
Plan 20-11 digest. The successful old CI/Nightly pair still belongs to
candidate `e6e83a31e413e12abe9dee142e3c0f7143d63eb4`, merge
`11533e456d41f740751c8353f947d65017aae692`, and digest
`de6d4355222063e019c7a3a94e30a404b52041f3d994659d445f12b4a23318ab`; the
failed collection and old plain-text job log remain diagnostic only. New
clean-source CI/Nightly results and later content acceptance are still
required. All four Phase 20 requirements remain pending.

### Phase 20 Plan 20-12 MSVC evidence-helper repair measurement

The fresh Windows ARM64 and x64 CI jobs rejected `phase20_fake_gh.c` because
MSVC's secure-CRT warnings were errors. The target now receives the same
`_CRT_SECURE_NO_WARNINGS` definition used by the other test executables. The
full-enabled local campaign was rebuilt and rerun at source revision
`c3a64f05a96e`, with `source_dirty=1` for the preserved `.planning/config.json`
edit. All 22 pages completed and all 146 scored rows were observed: 114 PASS,
32 FAIL, no skip/not-run/timeout, and five DRAW. `host_elapsed_guest_ns` was
`4710803000`; the report SHA-256 remains
`29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac`. The
fingerprint changed to
`4bebe4c94120d67c079d0c8f1d5332e1e5d7203ac82eb8ab4b599d52c6f3535c` over 56
inputs because `tests/CMakeLists.txt` is fingerprinted. The canonical report
rows and scoreboard remain unchanged. Hosted CI/Nightly evidence and content
acceptance are still pending. The resulting effective source digest is
`42b6369b20b3c5aae6b3bab23729f90e9d07f742ba5bbeeffcd2d7aec2f2cb0f` over 416
files. With full campaign mode disabled, the full CI workflow passed
686/686 in 385.86 seconds and generated the library, runner and libretro
packages. The collector/performance gate passed 28/28; the protected-key,
six-target, availability, hash and policy gate passed 37/37. All source-bound
hosted evidence remains pending.

## Phase 20 bounded vector transport repair provenance refresh

The added offline transport CTest changes the selected build-input fingerprint.
A fresh production 22-page campaign at source revision `f52a6108aec6`
(`source_dirty=1`) measured fingerprint `9b4d2199c8d6464c16d16f51766174aa64d633fb0a9d8b2968bac55013113993`
over 56 inputs and `host_elapsed_guest_ns=6286745000`.
All 22 pages completed: 114 PASS, 32 FAIL, no skip/not-run/timeout, and five DRAW rows.
The report SHA-256 remains `29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac`.
Actual raw output is retained under `build/phase20-fetch-campaign/`; baseline
metadata was refreshed from this run. Guest caps, ROM, scoreboard and acceptance
policy are unchanged. Hosted content acceptance remains pending Plan 20-10.


## Nightly Intel session helper repair: local provenance (2026-10-28)

The session driver's native clock/hash repair changes effective source. Required
public-header documentation also changes the selected 56-input AccuracyCoin
fingerprint to `424db30d3e712a95c6ebacd7785b710ca67beff078269dd9346c654d390ecc84`.
A fresh actual 22-page production campaign at `source_revision=6d2d1370d8fa`,
`source_dirty=1`, completed every page and all 146 scored rows: 114 PASS,
32 FAIL, zero skip/not-run/timeout, five DRAW. Guest caps remain 32/5/1800;
`host_elapsed_guest_ns=4678903000`, and report SHA-256 remains
`29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac`.
Raw local evidence is under `build/nightly-intel-session-campaign/`; current
baseline metadata comes from this observation. The historical hosted bundle
below retains its original source identity and confirms its own source only.
New-source hosted CI/Nightly validation, including Intel session controls,
remains required; this local repair does not renew hosted acceptance.

## Phase 20 Plan 10: clean hosted confirmation (2026-10-28)

Read-only production collection accepted CI `40700458574/1` and Nightly
`40700458462/1` with `verdict=hosted-pass`. Candidate
`fe70d619d6a3cf7b80ed1867ff9faf4c7c9053b1`, actual tested synthetic merge
`ef6d9e5ec2c94ddfd431c1d7162f620722a145df`, and protected base
`ab9c2e6eb822c06f0dfb0f08a0ca42ec541358be` remain distinct. The full effective
source digest is `a08da046cedeecbdd28227cc53d54e3d3e7dff2a93ee88aab9ead67da47bf61c`
over 417 files. Evidence ledger commit `e50e825` and this measurement edit
change only the named planning/evidence exclusions; they do not claim new hosted
runs. Production validation against the current checkout checks that mapping.

The clean Linux x64 campaign records `source_revision=ef6d9e5ec2c9`,
`source_dirty=0`, 22/22 pages and all 146 scored rows: 114 PASS / 32 FAIL,
zero skip/not-run/timeout and five DRAW. All six exact NMI target keys pass.
Report SHA-256 `29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac`
and 56-input fingerprint
`9b4d2199c8d6464c16d16f51766174aa64d633fb0a9d8b2968bac55013113993`
match the latest causal local observation; `host_elapsed_guest_ns=16457386307`.
The original dirty local observations, hashes, cutpoint qualifications and
constructed/executed origins remain historical facts above.

All six platform artifacts (Linux, macOS, Windows; x64 and arm64) contain the
same complete 114-row frame/PCM/transition/state inventory, SHA-256
`1a06cbabea1fee522e023a405af4474726ad24ef798d067b5e27575cf505bc62`.
Every lane's fresh protected observation/verdict/no-lost-pass checks retain
108/108 original protected-main PASS keys. Actual configured sample-vector,
Holy Mapperel and boot-to-play CTests pass in the selected per-job execution
logs; CI required and hash-equality jobs succeed.

The real performance artifact binds the protected base and tested merge above.
Production raw-measurement recomputation agrees with the summary; both base
controls have zero spread for all ROMs, and every row passes the unchanged +1%
limit without an allowance, n/a or self-only comparison. Tools are Valgrind
3.22.0 and GCC 14.2.0.

| ROM | Mapper | Base Ir/frame | Head Ir/frame | Delta | Base spread |
|---|---|---|---|---|---|
| nesteroids | 0 | 23791573 | 23914932 | +0.51% | 0 |
| runner | 1 | 24699006 | 22203492 | -10.10% | 0 |
| M4_P256K_C256K | 4 | 23367379 | 21998505 | -5.85% | 0 |

Same-source Nightly vectors-full inventory/JUnit establishes 258 successful
tests including cold fetch, all 256 opcodes and sample-match; MMC3 inventory/JUnit
establishes 13 successful tests including fetch and every configured oracle.
Neither has failed/skipped cases. Real RetroArch 1.22.2 progression reaches
16 boundaries and `VERDICT_OK`, including save/load, rewind and run-ahead;
drop/double-advance and wrong-origin controls reject as intended.

The immutable local bundle is `build/phase20-hosted-evidence/`. Its
`manifest.txt` SHA-256 is
`55857090681bef15f44671c8a0ef5a39fddda2c23b479b45d4802871e83a2386`,
`verdict.txt` SHA-256 is
`84867fe8bfc1292323e0f0cc88e9b906b1b66eb947dbd9290f934b1943c68f22`, and
114-file `provenance/evidence-files.sha256` digest is
`715e1b27b157ab117934bb3d4766ee9fd603e0b981eee442631e0a0a91130e54`.
The source/run/attempt/job/artifact and raw ZIP digest mapping is recorded in
`.planning/phases/20-nmi-line-and-vblank-flag/20-EVIDENCE.md`.

This resolves the specified clean-campaign and current-source hosted
platform/protected/performance/conformance/host evidence debt, including the
previously unconfirmed vectors-full/MMC3 fetch outcomes. Prior failed attempts,
unrelated debt, the exact physical paired-chip tie limit and the flagged
clean-room prohibition remain visible. All four requirements and Phase 20
completion stay pending independent re-verification at this task boundary.

## Phase 21 Plan 02: poll acceptance and state compatibility (2026-10-28)

The full local campaign used AccuracyCoin `6bee919529289c555803b612b8c9e4f7eaaeeabb`,
the unchanged `32/5/1800` frame caps, source revision `d6563967d90b`, and behavior
revision 9. It launched and attributed all 22 pages, observed all 146 scored
rows, and completed with 114 pass, 30 fail, 2 not-run, 0 skip, 0 timeout, and 5
inventory-only DRAW rows. The 56-input fingerprint is
`9c236d3f77cfa9c1edb4d1c774d9fec29142bb6a6f089f1aa6682e5740984ef2`; the
campaign report SHA-256 is
`c5c7fc0e1085da59c25a14e4ecb27ae12ab2f7bb8a89032ba4a3b60a761f51ef`; summed
guest execution time is 4.722662 seconds. The metadata and scoreboard checksums
are copied from this run. The planning config edit that predated this plan was
preserved and excluded from the input fingerprint.

The fresh protected-main comparison used revision
`a64c23fd3ca7caed7e3ad102ff1a9beaa29fa562`. All 114 protected PASS keys remain
PASS. Exactly three page-12 rows changed from the prior scoreboard:

| Key | Previous | Measured | Cause |
|---|---|---|---|
| `Interrupt flag latency` | fail / `0x0a` / 151 | fail / `0x03` / 3 | Guest JAM during test-E setup; accepted history is already present before the source's BCC C1 poll and C3 acknowledgement sequence. |
| `NMI Overlap BRK` | fail / `0x0a` / 151 | not-run / `0x00` / 3 | Unstarted after the earlier page-12 guest JAM. |
| `NMI Overlap IRQ` | fail / `0x06` / 151 | not-run / `0x00` / 3 | Unstarted after the earlier page-12 guest JAM. |

The source-defined IFlagLatency tests 1 through D reached completion; the guest
error counter was `$0E`, and the DMC/open-bus prerequisite checks had passed.
The pinned test E expects the first BCC poll at C1 to see IRQ, then requires
that acceptance to survive the later C3 `$4015` dummy read that acknowledges
the source. Its expected entry records return PC `$3FA5`, pushed `P=$20`, and
the `$5A` handler marker. In the candidate trace, accepted history is already
present immediately after the `JSR $4013` step and before the target instruction
executes: the CPU is at `$4013`, `X=$5A`, `P=$20`, and the logical IRQ line,
captured level, poll sample and accepted history are all asserted. This is an
earlier poll divergence than the source's BCC C1/C3 sequence. The subsequent
handler record shows return PC `$10A1`, followed by guest JAM at `$10A6`
(campaign `jam_frame=82`, active raw `$03`). The local measurement therefore
does not establish a pass for `Interrupt flag latency`; it records the first
test-E divergence with both prerequisites satisfied. The two overlap rows
remain unstarted and are still owned by Phase 21 Plan 04.

Runner frame hashes, PCM hashes and APU transition hashes did not move. State
hashes changed because the canonical state now serializes one bounded
`irq_accepted` byte and carries format 3 / behavior revision 9; every tested
state grew by exactly one byte. No CPU/APU timeline or audio output change is
attributed to these byte-only state changes.

| Mapper | State hash at 0 ticks, old → new | State hash at 60 ticks, old → new | Size, old → new |
|---|---|---|---|
| `dabg` | `e1095e7f0f1e25531866fb4758c9aa927fc5a4a44090d59fd15ebf0ef8525fc4` → `eb91bc38cb35e67c7abe11710ca3fef6bb075a1fe8ce0cc579e1a631e0c6b0dc` | `10567a2676d26cc32352591818f60a476b9593bf19c054ebb16033ae3725df8f` → `bbad277938241d9ac6abaffc842cb8651097e384067f1f6d9a6a4c1a7aafc4e2` | 13089 → 13090 |
| `m1sxrom` | `c684eacdf391d0385d275ec77707a9cb390553e683b652d3d29234479f26c506` → `31d7a7d74de95472192e207c391b724e781f15d165ecc95a8e23c9121f6f9518` | `0a18d9a7e1f8e26c9c933e614d48ca1cf3f155295b015d08eb98e34a99636fc1` → `443dc187ac53f0927876d581b220f5cd833457a446513120523e225a48be9092` | 45871 → 45872 |
| `m2` | `a3cb3ce2589ee0f1751111c891cd24ae4f28780925d79bac8ce09cc8714da35f` → `034f16fc881e009c1146fce3fb6d54981f7840739bea2c07379d515341ac946e` | `65085e74f0d494bd94eb829fb4cb74afc509eb0ce33b4f8fe11a917f7d031c14` → `747050bd4767712acec345b6de403f702083863166d083f09c72c2dd5c9cd2ed` | 13090 → 13091 |
| `m3` | `6e779e324a7bb71f3dfad40c5afcb3f53498a481788e9ec8d1e7db83278cfb52` → `734f814da74515c5d921b2c17a92de595d08e69bf35133a05c95979035312df9` | `62f9c8cb49ba30a49a533b34aa7c0e24c2d9baa4540c5b19e046c1d33ff6d028` → `690d12175d9fd6f656a3b6a2d9ed51f9ceed1b8432d738ef56e0ffaa883ab520` | 4898 → 4899 |
| `m4tkrom` | `be6d91d84268c095d71a6a2e236223083511c582c38e05a048a3073a4a7e95f1` → `a1596275787aba137ba75f866127d843afc0f856102ddf2311f4d44f54568d23` | `315136210957bed4db11cd0d3c06aaee7680ab1441ea8126fa0621326a58552a` → `30ac9a9c107a71f0f62d9ba4191115547475e3fc498fb552470c525a241399ea` | 21305 → 21306 |
| `m7` | `09963ebbcdfb8cf69a88d27ecaf38c82af9485a75a9adc87c902328bc17cc483` → `8da0f0413157f31a639c114c226e0a8dabe0e35333521b4a851d527cb5d82ab8` | `ca58348266621b2a416e8fecc3adec9357a24b15b4ce410c8b3fa2f7a70c5680` → `37b4105d6cbac871441b58ed9cd52ff2d386b0e86033014e0575a7b5d0f9fed4` | 13090 → 13091 |

The state corpus generator was run with `--write-seeds` and `--check-seeds`; all
17 manifest entries matched the generated corpus, so no seed bytes changed.
The ROM manifest and all ROM bytes were unchanged.

## Phase 21 Plan 04: shared entry and native continuation measurement (2026-10-28)

The full local campaign used AccuracyCoin
`6bee919529289c555803b612b8c9e4f7eaaeeabb`, the unchanged `32/5/1800` frame
caps, source revision `4a1b7cc970f9`, and behavior revision 9. It launched and
attributed all 22 pages, observed all 146 scored rows, and completed with 114
pass, 30 fail, 2 not-run, 0 skip, 0 timeout, and 5 inventory-only DRAW rows.
The 56-input fingerprint is
`4bf6ab823b3287d3e4a5934ed3564dc451f3eab2c2c4e908964ae2076fcf8ed0`; the
campaign report SHA-256 remains
`c5c7fc0e1085da59c25a14e4ecb27ae12ab2f7bb8a89032ba4a3b60a761f51ef`; summed
guest execution time was 4.966354 seconds. All 114 starting PASS keys remain
PASS.

The eleven named Plan 21 campaign rows did not all pass. Eight passed:
`$2002 flag timing`, `NMI Control`, `NMI Timing`, `NMI Suppression`, `NMI at
VBlank end`, `NMI disabled at VBlank`, `Sprite 0 Hit behavior`, and `Arbitrary
Sprite zero`. `Interrupt flag latency` remains fail / `0x03` / frame 3, so
`NMI Overlap BRK` and `NMI Overlap IRQ` remain not-run / `0x00` / frame 3. The
owned production-bus BRK32 and IRQ64 paired pushed-status sweeps now each match
their source tables exactly under one alignment. The first remaining campaign
divergence is inside the test's DMC/open-bus setup: a focused opcode trace at
`$4013` reads `$40` (`RTI`) where the ROM expects `$90` (`BCC`), before the
target BCC poll. The existing DMA/open-bus behavior is outside this plan's
authorized change scope, so no INT01 or overlap campaign pass is claimed. The
physical exact-tie winner remains unmeasured.

Every non-state row in the 114-row runner inventory is byte-for-byte unchanged,
including its selected frame tick totals, PCM hashes and APU-transition hashes.
All six native state sizes increased by exactly four bytes for the
origin/action/vector/partial-low continuation, and their hashes changed as
measured below. These selected runner milestones do not claim globally
unchanged timing or audio.

| Mapper | State hash at 0 ticks, old → new | State hash at 60 ticks, old → new | Size, old → new |
|---|---|---|---|
| `dabg` | `eb91bc38cb35e67c7abe11710ca3fef6bb075a1fe8ce0cc579e1a631e0c6b0dc` → `feacfb502a3952c6a33b04f45364747b65de2c51ed2eb12f3713197141717b12` | `bbad277938241d9ac6abaffc842cb8651097e384067f1f6d9a6a4c1a7aafc4e2` → `1bbb23ad86b2c7afae35342e70335d5e029e6fb1fe8a87c2f4ccb5ad720b28c1` | 13090 → 13094 |
| `m1sxrom` | `31d7a7d74de95472192e207c391b724e781f15d165ecc95a8e23c9121f6f9518` → `050f81968fa547aa52a4978d57534014ba35e87fe9fd4c86c8e73c5c42082ca3` | `443dc187ac53f0927876d581b220f5cd833457a446513120523e225a48be9092` → `489b694c2711e95c484cc329d4d520207159abdb8444f03590c53fb136aba788` | 45872 → 45876 |
| `m2` | `034f16fc881e009c1146fce3fb6d54981f7840739bea2c07379d515341ac946e` → `93f21f82f84cc1af60ef11d36008cfe751526c48ea496f1669b140374a461101` | `747050bd4767712acec345b6de403f702083863166d083f09c72c2dd5c9cd2ed` → `b0025aa74ed683cb7a57af5940fd55484bd976d6b1a96ce302284a62c3bec6c0` | 13091 → 13095 |
| `m3` | `734f814da74515c5d921b2c17a92de595d08e69bf35133a05c95979035312df9` → `764768824aaa4ffcd8204dc00bd6ac6c40ba24b459ebf2da3a64442f8fe6b7a7` | `690d12175d9fd6f656a3b6a2d9ed51f9ceed1b8432d738ef56e0ffaa883ab520` → `b581a90e0806652ac63d604cca0db50713c6e29c951f69f350a1ac757c7ccd2a` | 4899 → 4903 |
| `m4tkrom` | `a1596275787aba137ba75f866127d843afc0f856102ddf2311f4d44f54568d23` → `1b0a15c00a36846a5f54737aff04b0b72e68fd7689d04d8f70a87394cfabceae` | `30ac9a9c107a71f0f62d9ba4191115547475e3fc498fb552470c525a241399ea` → `0dcceac6740f6441441ef6a406dec4bb0b5bf6cb498db380d5ca4714cf5a4a73` | 21306 → 21310 |
| `m7` | `8da0f0413157f31a639c114c226e0a8dabe0e35333521b4a851d527cb5d82ab8` → `49340b2d168a7f5338f762c340bb2afa641b43d67e22382552ce36ab791f115b` | `37b4105d6cbac871441b58ed9cd52ff2d386b0e86033014e0575a7b5d0f9fed4` → `59a506a66c6db0ae9d7b5581203d5bdb4a110b21399f8919fee85a55fffffad6` | 13091 → 13095 |

The owned state corpus was regenerated and checked: all 17 seed bytes and
manifest rows are unchanged. These are local observations; the campaign target
failure means the shared-entry plan remains incomplete pending a scoped
resolution of the earlier DMC/open-bus divergence.

The first full CI workflow exposed a separate timing assumption in the shared
state-fork stress fixture: with its old delay `G=8`, the NROM image had no OAM
pending boundary in 91 frames (MMC3 still observed three). The narrow fixture
delay is now `G=9`; the requirement to observe OAM, DMC and IRQ-poll state at
frame boundaries is unchanged. The passing measured run saw NROM OAM/DMC/poll
at 1/4/27 boundaries (first OAM frame 58) and MMC3 at 1/3/20 (first OAM frame
54), with all 61 forks completed on each board. `core.state_runahead` and
`libretro.host` also passed with the adjusted fixture. This test-fixture
calibration does not alter the campaign ROM, runner hashes or protected PASS
authority.

## Phase 21 Plan 04: bounded DMC/open-bus prerequisite repair (2026-10-29)

The authorized prerequisite investigation found that a DMC halt blocked by a
CPU write waited for its original GET/PUT phase instead of retrying on the next
CPU cycle. Pinned AccuracyCoin test E's timed prefix includes such stack writes.
That incorrect retry shifted CPU progress relative to the free-running timer:
the reload halted JSR's final mapped high-operand read, and its resumed ROM read
correctly replaced DMC `$90` with `$40`. The following `$4013` fetch therefore
executed RTI and jammed. Retaining GET data over that resumed ROM read would
have been incorrect. The repair advances the pending halt-attempt phase after
an eligible write; the existing GET alignment and resumed read remain intact.

The corrected retry lets Implied Dummy Reads advance past its previous failure,
exposing an adjacent open-bus defect. The controller decoder forced D6 high,
so its source-required JSR `$4015` operands formed `$4061` instead of `$0021`.
The unintended RTI corrupted the guest stack and eventually reset page 15,
erasing protected results. Retaining D5-D7 fixes that one-line decoder defect.
The pinned assembly describes these operands in `TEST_ImpliedDummyRead`; the
DMA retry contract is in NES-HARDWARE-CPU-APU section 2.2 (HWC.01).

The failed pending-at-read-opening experiment was removed. No APU timer phase,
`$4010` write order, `$4015` latch behavior, sample-byte retention, ROM pin,
frame cap, answer table, state field, format or revision changed in this repair.
The owned JSR fixture now uses the equal bus/APU phase relationship reached by
cold execution; the branch fixture stalls the actual C1 opcode fetch and keeps
all C3 acknowledgement/C4-clear assertions. New owned tests cover both initial
halt phases after one, two and three writes, and both controller ports with all
eight upper-bit patterns and both serial data values.

Fresh Release/Ninja build `build/dmc-repair` used warnings as errors and
`NESTURBATOR_ACCURACYCOIN_FULL=ON`. The owned full campaign retained pin
`6bee919529289c555803b612b8c9e4f7eaaeeabb` and boot/navigation/execution caps
`32/5/1800`. Its result is 119 PASS / 27 FAIL, with all 146 scored rows terminal,
no not-run/skip/timeout, and five inventory-only DRAW rows. Every one of the
114 starting PASS keys remains PASS and all eleven named v5 target keys pass.
The five status improvements are:

| Key | Previous status / raw / execution frame | Repaired status / raw / execution frame |
|---|---|---|
| DMA + `$4015` Read | fail / `0x0a` / 533 | pass / `0x01` / 526 |
| Implied Dummy Reads | fail / `0x0e` / 94 | pass / `0x01` / 128 |
| Interrupt flag latency | fail / `0x03` / 3 | pass / `0x01` / 148 |
| NMI Overlap BRK | not-run / `0x00` / 3 | pass / `0x01` / 148 |
| NMI Overlap IRQ | not-run / `0x00` / 3 | pass / `0x01` / 148 |

There are 29 status/raw/frame row changes in total; the exact before/after list
is retained in `build/dmc-repair-evidence/final-campaign/row-changes.tsv`.
DMC Reload Timing remains PASS / `0x01` (page 15 execution frame 128), and Delta
Modulation Channel remains PASS / `0x01` (page 14 execution frame 122). Their
passing source windows require no timer-edge or timer-write-order change.

The final campaign report SHA-256 is
`56ebd01adb97b3916fde36f8739602284d951a1fd00d84285bb4ef441fb463a3`;
the 56-input fingerprint is
`7936186b09988b79b502e9e6382d2895343d6f35adc7f8a9782a99efa8079e91`.
Runner SHA-256 is
`57dc440efabcfe9291a911499f0eb32612b73dfc003bc13093865946cbec636d`;
`src/bus.c` SHA-256 is
`44013871a5a6cf140cc2545dfc3e3593188f28b9b77ce7f3d287ca1a02faf3f3`.
Exact source/binary/report identities and key gates are recorded under
`build/dmc-repair-evidence/final-campaign/`. Original, diagnostic and independent
revert runs each retain source copies, binary hash, commands and raw reports.

Removing only the retry fix restores the owned retry failures and canonical
latency `0x03`/3 with unstarted overlaps. Removing only controller retention
restores the owned upper-bit failures and the page-15 reset. Reapplying both
restores the campaign above. Fresh adjacent verification after reapplication
passed 42/42 in 173.81 seconds, including the full campaign, all selected core
and state-fork tests, native state seeds, CPU/bus and NMI-race controls. See
`build/dmc-repair-evidence/final-focused.log` and
`.planning/phases/21-interrupt-latency-and-hijack/21-DMC-PHASE-AUDIT.md`.

At that first repair run, this was prerequisite evidence rather than Plan 21-04
completion; the remaining campaign, pins, corpus, fresh protected-main and CI
closeout are recorded below. No hosted evidence or phase-completion claim is
made here.

### Fresh source-bound remeasurement after candidate-count refresh

The final campaign was rerun after the activation contract and candidate-derived
fixture counts were updated. It used source revision `a23394036f71`, behavior
revision 9, pin `6bee919529289c555803b612b8c9e4f7eaaeeabb`, and unchanged
`32/5/1800` caps. It completed all 22 pages and all 146 scored rows with 119
PASS, 27 FAIL, no skip/not-run/timeout and five inventory-only DRAW rows.
Guest execution time was 4.917281 seconds. Report SHA-256 remains
`56ebd01adb97b3916fde36f8739602284d951a1fd00d84285bb4ef441fb463a3`; the
current 56-input fingerprint is
`c1aeeddca9b0b5a2641d66ffa9309998dfb67bb620cb1cbb94d6c59534865283`. The
canonical scoreboard has 29 row deltas from the previous candidate; the fresh
`row-changes.tsv` retains the complete list. All 114 protected-main PASS keys
and all eleven v5 target rows pass, with 146/146 terminal rows.

The exact report, measured metadata and raw campaign rows are retained in
`build/dmc-repair/accuracycoin/full/`; `identity.json`, `gates.json` and
`row-changes.tsv` are in
`build/dmc-repair-evidence/final-campaign/`. Rechecking all twelve source,
runner and report digests in `identity.json` found zero mismatches. The earlier
`$4013 = $40` trace is a baseline-before-repair observation, not a final result;
withdrawn stale probe variants are not used as acceptance evidence.

Protected-main authority was freshly fetched from `origin/main` at
`a64c23fd3ca7caed7e3ad102ff1a9beaa29fa562`. Its canonical scoreboard projects
114 PASS keys. The candidate preserves all 114. The selected runner hash
inventory regenerated through `runner.write_hashes` and its content check
passed byte-for-byte; frame, PCM and APU-transition rows stayed unchanged.
The six native-state rows remain the measured four-byte format-3/revision-9
continuation increase documented above. The owned state-corpus generator and
`--check-seeds` reproduced all 17 entries; their bytes and ROM-manifest rows
were unchanged. Six new page-14 migration links now point to the final measured
frame 122 observations. No protected-main authority or answer table changed.

Final verification ran `cmake --workflow --preset ci` with the full campaign
enabled and the fetched protected-main scoreboard at
`a64c23fd3ca7caed7e3ad102ff1a9beaa29fa562`. The workflow passed 691/691 CTest
tests in 358.64 seconds and generated the library, runner and libretro ZIP
packages. Hosted cross-platform evidence remains assigned to the later Phase
21 acceptance work; this plan records local acceptance only.

## Phase 21 Plan 08: local source-bound final gates (2026-10-29)

This is a local macOS arm64 run for the Plan 08 source, not hosted or
cross-platform acceptance. A clean-equivalent checkout at candidate
`1d27a0f12542a235d8d79b9c97591168fbd77d8e` ran the full workflow with a fresh
protected-main baseline at `a64c23fd3ca7caed7e3ad102ff1a9beaa29fa562`. Its
`accuracycoin.campaign` and same-invocation `accuracycoin.protected_observation`
tests passed. The campaign used library `0.1.11`, behaviour revision 9, and
unchanged `32/5/1800` frame budgets; all 146 scored rows were terminal, with
119 PASS, 27 FAIL, zero skip/not-run/timeout, and five inventory-only DRAW
rows. All 114 fresh protected-main PASS keys and all eleven named v5 target
keys pass. The final local campaign's guest elapsed time was 4,982,296,000 ns.
Its report SHA-256 remains
`56ebd01adb97b3916fde36f8739602284d951a1fd00d84285bb4ef441fb463a3`, and the
scoreboard SHA-256 remains
`059d7c7558bd9f4785ab384c589f04e16f330ef3ba477393a8492b59e086dbbf`.

| Measure | Local result |
|---|---|
| Source revision / dirty state | `1d27a0f12542` / clean clone (`source_dirty=0`) |
| AccuracyCoin pin / ROM SHA-256 | `6bee919529289c555803b612b8c9e4f7eaaeeabb` / `8ac8904a3cf26fbdfa6be501e414d45886ff4478d30dee8e58fa436cf5fd5adc` |
| Build-input fingerprint / files | `566b32ce33b98b33277a516720f7073ec40f24cb11e67448436e7773e85d87b1` / 56 |
| Fresh protected-main PASS keys | 114; revision `a64c23fd3ca7caed7e3ad102ff1a9beaa29fa562`; authority SHA-256 `b814f7524d15e0f389e97cb1e8567efbbce601cc8c4b8902ef3471ab965991c4` |
| Full `ci` workflow | exit 0; 695/695 CTest tests; CTest total 427.78 seconds; one full campaign lane |
| `asan` workflow | 686/686 CTest tests; 230.04 seconds; configured short-run suite |
| `vectors-full` workflow | 258/258 tests; 8.26 seconds; pinned vectors obtained outside the repository |
| `mmc3-oracle` workflow | 13/13 tests; 1.59 seconds; ROM inputs obtained outside the repository |
| `nofp` on macOS arm64 | 4/5 ABI checks passed; `abi.undefined_symbols` rejected `stack_chk_fail` and `stack_chk_guard`; Linux nofp result remains external |
| Phase21 evidence controls | Passed in full CI; Phase20 controls and adapter passed with historical defaults intact |

The full CI workflow generated these three `0.1.11` macOS arm64 packages:

| Package | SHA-256 | Inventory |
|---|---|---|
| `nesturbator-0.1.11-library-macos-arm64.zip` | `f68cdd0c309ccff557e300d3e770add1fe363558045af25f919bd6caed961799` | Public header, static library, pkg-config and CMake package files |
| `nesturbator-0.1.11-runner-macos-arm64.zip` | `c82866961e4270ecbec58d7abf96dfcf969fad0f53ae8a7fea2c593eb82d7aa2` | `bin/nesturbator-run` |
| `nesturbator-0.1.11-libretro-macos-arm64.zip` | `9cf4734626f9c2a94636b2744209e26644582d91b72895bac869b100b7e8c3f8` | `cores/nesturbator_libretro.dylib` and its `.info` file |

`build/phase21-hosted-evidence/ci-final.log`, `asan-final.log`,
`vectors-full-final.log`, `mmc3-oracle-final.log` and `nofp-final.log` retain
the local command output. The independent
Git source identity manifest records 427 effective tracked inputs and
committed/index/worktree blob identities; its SHA-256 is
`daa659df9cd037d09c489f1795d7d1a8e564f82b6c2299d77428645f44e45c84`. It was
revalidated against both the clean source checkout and current checkout after
each final local workflow. The state validation comment now cites hardware
sections using integer wording so the unchanged no-float scanner accepts it;
runtime behavior did not change. The source comment and refreshed baseline
fingerprint are in candidate commit `1d27a0f`. The macOS `nofp` attempt's only
remaining failure is the Apple stack-protector symbol allowlist noted above;
the actual Linux nofp job remains pending. This manifest is run-specific Git
evidence; it does not resolve
the deferred collector mode-enforcement issue. The pre-existing
`.planning/config.json` edit is excluded from source identity and commits.

The source fingerprint changed only because the integer-only source citation
fixed the no-float scan; the campaign report and measured row table did not
change. The full workflow retained one campaign lane;
local synthetic controls test failure diagnostics only. No hosted CI/Nightly
attempt, tested merge, pull request identity, six-platform hash inventory,
hosted performance result or actual hosted marker-session artifact is supplied
here. Plan 21-09 remains pending those external facts; no local result is
substituted for them.

## Phase 21 source-review corroboration and inhibited NMI repair (2026-10-29)

The committed review at `7d5fbe1` was checked independently against the
Visual6502 State Machine and Interrupt Hijacking primary analyses. CR-01's
proposed independent IRQ queue is rejected: one NMI-priority entry consumes the
shared arbitration event. New simultaneous-poll controls prove an acknowledging
NMI handler suppresses later IRQ, while a held source is accepted again after
RTI restores I. The exact proposed preservation mutant instead enters IRQ
before the first NMI-handler opcode despite I being set, and fails the controls.
The ordinary branch-accepted IRQ/source-acknowledgement contract is unchanged.

CR-02 is reproduced: sampled post-choice pulses released before handler
recognition were stored as sticky recognized requests. Before-fix cases for
all three entry origins at C5/C6/C7 and the first-handler branch enter an extra
NMI. The repair uses the existing pending byte to distinguish a recognized
request from an inhibited candidate. Release cancels only the candidate; a held
candidate is qualified before the first handler's legal poll. Already recognized
fresh edges remain sticky and consumed held inputs do not duplicate service.
C5/C6 cases supply later entry-cycle margin; a C7 close followed by release at
the next call boundary records deterministic model behavior, without claiming a
physical exact-tie winner.

State format 3, behavior revision 9, ABI 1 and state byte counts remain unchanged.
The visitor now bounds the existing pending byte to 0 through 2 and validates
candidate history against entry stage, sampled input and poll/JAM markers before
APPLY. New CRC-correct active and idle-after-C7 rejection controls preserve the
full instance and generation; a sampled-high/current-input-low candidate remains
legal. Native held/released-candidate replay, the actual adapter's existing
entry replay matrix, ordinary Phase20 capture/poll and DMC/open-bus controls pass.
The detailed disposition, source citations, fixture correction and exact
negative-control identities are in
`.planning/phases/21-interrupt-latency-and-hijack/21-REVIEW-DISPOSITION.md`.

The actual adapter additionally loads four candidate-2 histories through
`retro_unserialize`: active C6 and idle after C7, each held and released. Its
initial serialization must echo the entire saved byte string, then both scripts
and depths one through three compare committed/speculative video, stereo PCM,
state, ticks, frame numbers and handler markers with native continuation.
Absolute first-frame markers require IRQ=1 and NMI=1 for held/0 for released.
The extended `libretro.runahead` passed in 26.63 seconds, reporting four histories
and 24 history/script/depth cases. The existing 20 base cutpoints, 60 depth cases,
120 script cases and all four adapter controls remain intact. This test-only
extension did not change runtime, module, runner or campaign-input identities;
its host source/binary and output are retained in the final identity file and
`build/entry-review-evidence/adapter-candidate2.log`.

Fresh Release/Ninja build `build/entry-review-repair`, with warnings as errors
and full AccuracyCoin enabled, passed the final selected 19 tests in 68.11
seconds. This includes native and actual libretro runahead, entry/state history,
CPU/bus, NMI/IRQ, native seeds and a fresh full campaign. The unchanged
pin6bee919/cap1800 campaign remains **119 PASS / 27 FAIL**, all 119 prior PASS
keys retained, all eleven target keys PASS, all 146 scored rows terminal and five
DRAW rows outside the score. Every status/raw/frame row is unchanged.

The report SHA-256 remains
`56ebd01adb97b3916fde36f8739602284d951a1fd00d84285bb4ef441fb463a3`.
The measured 56-input fingerprint changes from
`566b32ce33b98b33277a516720f7073ec40f24cb11e67448436e7773e85d87b1`
to `d96efe4a88bdaa0680398334a3c84a0e04faef405016172e06ef2bb9c4999e57`.
The current runner SHA-256 is
`7f10856c0f4612b07430f53427edbfbae283b7376b36d3bb54a5a97818cc48e9`;
the repaired `src/bus.c` SHA-256 is
`446079ddc3c479d5a9cb3e5899b0d95f3142deee07ef9e5d0f0ea88d686922a6`.
The owned tracked-input checker still records 427 effective paths, with dirty
working-source manifest SHA-256
`5b472e1930388bfe56fb2c795ecbe00766f95ba2d5d05dcf1ab436ad1a989305`.
The final identity/key checks and logs are in `build/entry-review-evidence/`;
the integer-only scan reports 23 files with no floating point.

This working repair invalidates candidate `1d27a0f` and its previous Plan 08
manifest. Previous full-CI/ASAN/vector/MMC3/package evidence remains historical;
executor closeout must refresh fingerprints, hashes/corpus, full validation and
the new source candidate coherently before a causal commit and re-review. The
original review report remains unchanged. Plan 09 delivery, hosted acceptance,
requirements and phase completion remain pending; this measurement does not
substitute focused checks for those gates.

## Phase 21 post-review source qualification refresh (2026-10-29)

The causal source commit is `43d260a914ba76a130809524fba53ad58aca1298`.
Final CI, ASAN, vector, MMC3 and campaign tests compiled the preceding dirty
working tree at `7d5fbe18f367`; source, test, and public documentation bytes from
that tested tree were committed unchanged. CI is therefore documented as a
dirty-tree run, not a clean-clone build. After commit, a 427-path tracked
Git-mode/blob manifest verified the candidate and an absolute clean clone at
that candidate. The candidate manifest SHA-256 is
`20460339ae9f742d3906ebe7ef6d5c4a2fff53c5c4b54cf59bcbb7b99cb9a1b2`.

Fresh protected main was fetched from `origin/main` at
`a64c23fd3ca7caed7e3ad102ff1a9beaa29fa562`. Its 114 PASS-key authority SHA-256
is `b814f7524d15e0f389e97cb1e8567efbbce601cc8b4c8902ef3471ab965991c4`.
After preparing that authority, a selected CTest invocation passed its campaign
fixture, same-invocation fresh protected observation/verdict, no-lost-pass and
all-eleven target gate (5/5 total). It recorded 119 PASS / 27 FAIL, 146/146
terminal scored rows, zero skip/not-run/timeout and five inventory-only DRAWs.
Guest execution was 5,517,533,000 ns. All 114 protected PASS keys and all eleven
target rows passed. Campaign report SHA-256 remains
`56ebd01adb97b3916fde36f8739602284d951a1fd00d84285bb4ef441fb463a3`; scoreboard
SHA-256 remains `059d7c7558bd9f4785ab384c589f04e16f330ef3ba477393a8492b59e086dbbf`.
The 56-input fingerprint is
`d96efe4a88bdaa0680398334a3c84a0e04faef405016172e06ef2bb9c4999e57`.

The final `cmake --workflow --preset ci` run passed 695/695 in 421.60 seconds
and packaged all three deliverables. `cmake --workflow --preset asan` passed
686/686 in 192.01 seconds, `vectors-full` passed 258/258 in 17.05 seconds, and
`mmc3-oracle` passed 13/13 in 2.47 seconds. The local CI workflow was first run
without the GitHub `RUNNER_TEMP`/`GITHUB_ENV` handoff; its fresh authority was
then fetched explicitly and supplied to the selected current-source fixture
run above. No second full CI lane was added for that authority refresh.

The regenerated 114-row frame, PCM, APU-transition and state-hash inventory
matches `tests/runner/hashes.txt` byte-for-byte; both writer and content checks
passed. Inventory SHA-256 is
`76f999668c5bf584074a37f050a19cb4c31f9c63b6671b06b85388e9950be850`. The six
0-tick and six 60-tick state hashes, six state sizes and format pin did not
move. State format 3, behavior revision 9, ABI 1 and payload sizes are
unchanged; the inhibited request uses value 2 in the existing byte. The state
corpus remains 17 files / 23 bytes (path/content manifest SHA-256
`3b6b71143ce28de6c88b02a4ac65bb73b322edeab950564db705abf2b568bc09`). The
ROM manifest remains 97 rows, SHA-256
`e56a6b2cde320a3c4b5315904b9d4dd99cefc7edf80292ed3f81fc5c008b850d`.

All three packages are retained under
`build/phase21-hosted-evidence/packages/post-review-source/`. Their SHA-256
values are library `5e91fefcdbda743bd9e0c26dec3417e1cf5908f9f585340c6b0f548096995112`,
libretro `c095033d872d2f3ea89ac87468359ec83a3f6c36ab1e6132231017ded4f23a7c`,
and runner `a975a24b213895d5d86774785d2c4486bc20c992a7d18659f8459a7e845588c1`.

The current-source macOS arm64 no-FP workflow passed the 23-file float scan,
scan self-test, global-symbol check and float fixture. Its undefined-symbol
test still rejects Apple `stack_chk_fail` and `stack_chk_guard`; no allowlist
waiver was made. Linux no-FP hosted results remain pending. The initial CI
attempt had five strict metadata-parser failures because a prose value
contained an equals sign. After removing that character, the affected five
controls and the full CI workflow passed. No source gate was weakened.

Plan 21-09 remains pending real delivered CI/Nightly run and attempt IDs, the
actual PR and tested merge, and immutable hosted artifacts. Local package and
fixture controls do not satisfy those external facts or complete INT-01,
INT-02, KEEP-01, or the phase.

## Phase 21 publication formatting refresh, 2026-10-29

The first publication push was rejected by the repository pre-push formatter, which checks outgoing historical source blobs as well as the final tree. An isolated publication checkout preserves the original execution branch locally and formats the 19 affected owned C/header/test paths with clang-format 18. No hook or acceptance threshold is bypassed. The publication uses a clean commit on protected main; earlier causal commits and their qualification remain recorded as local execution history.

The refreshed publication campaign retains 119 PASS / 27 FAIL, all 146 scored rows terminal, and five inventory-only DRAW rows. Its report SHA-256 remains `56ebd01adb97b3916fde36f8739602284d951a1fd00d84285bb4ef441fb463a3`; its 56-input fingerprint is `cb1f1ce5de0bfe13b87e6754a8631a3a8d07365c7df1a62677ab4ef60257906c`. Metadata truthfully records the staged source checkout before the publication commit. Fresh publication CI/hygiene and hosted CI/Nightly results must be recorded separately from the earlier unformatted candidate.

Publication preflight: hygiene 13/13 (including clang-format 18) and focused interrupt/state/actual-adapter suite 77/77 passed on the formatted source. The complete original execution history is preserved locally at `refs/phase21-history/execution-6c42607`. Hosted qualification remains pending the publication runs.

## Phase 21 hosted-regression repair, 2026-10-29

Actual CI `40919233064/1` rejected deterministic instruction increases of
2.88%, 3.10% and 3.19%, and its Phase 21 evidence controls timed out at the
unchanged 120-second cap on macOS Intel and both Windows architectures.
Those failed results remain historical; Nightly `40919233254/1` passed its
seven work jobs on that earlier source.

The enlarged bus cycle prevented GCC 14 from specializing a dynamic
PPUSTATUS path, retaining status-only register and stack work on ordinary
reads. Constant cycle arguments restore specialization; combining the
unobserved 3 + 12 tick advance removes another intermediate store. NMI edge
handling branches on transitions and the rare inhibited candidate first.
IRQ exceptions commit at their own legal polls, preserving old I, branch OR
history and JSR timing without generic per-opcode exception flags.

Fresh local Linux amd64 GCC `14.2.0-4ubuntu2~24.04.1` / Valgrind 3.22 raw
60/360-frame measurements compare the repaired source against protected base
`a64c23fd3ca7caed7e3ad102ff1a9beaa29fa562`:

| ROM | Base Ir/frame | Repaired Ir/frame | Delta |
|---|---:|---:|---:|
| nesteroids | 23914932.2267 | 24103968.3533 | +0.79045% |
| runner | 22203492.7433 | 22392202.7133 | +0.84991% |
| M4_P256K_C256K | 21998505.2900 | 22215976.1400 | +0.98857% |

The base counts independently match the retained hosted repeat exactly. The
original `perf_compare.cmake` gate passes with the existing allowances and
threshold. A scratch source-point revert of constant status dispatch raises
M4 to 22482530.8 Ir/frame (+2.200265%); reapplication restores exactly
22215976.14. Only scratch objects/archive/binaries changed during that causal
experiment; the canonical 428-input manifest verifies unchanged before/after.

The identity helper previously launched Git 1285 times for one 427-path
inventory and ran three full inventories in its controls. Batching committed
tree and raw worktree queries reduces one inventory to seven launches and
preserves its byte-for-byte manifest on the same checkout. A measured
12-process budget rejects the old helper. Independent synthetic-Git controls
exercise committed/index/worktree divergence, an index-mode change,
staged-only input, spaces and raw CRLF, symlink target bytes where supported,
missing input and unresolved stages. The new control script makes the tracked
effective inventory 428 inputs. Full Phase 21 controls pass in 19.69 seconds
with every inherited content/provenance negative and the original cap intact.

The fresh campaign on dirty source `51ad35e39ef3` retains 119 PASS / 27 FAIL,
146 terminal scored rows and five DRAW rows. Its report remains
`56ebd01adb97b3916fde36f8739602284d951a1fd00d84285bb4ef441fb463a3`;
the current 56-input fingerprint is
`bf09fe6ebb319fbb668abb65f70e5142bc1477a0dc70f391a305d7f3e97c136a`.
State format 3, behavior revision 9, ABI 1 and pinned hash rows remain unchanged.

Final local CI passes 695/695 in 380.80 seconds and generates all three
packages. That invocation uses freshly fetched protected-main authority with
all 114 starting PASS keys; its campaign, protected observation/verdict,
no-lost-pass and eleven-target checks pass. Full vectors pass 258/258 in
10.03 seconds, MMC3 passes 13/13 in 1.86 seconds, and hygiene passes 13/13.
ASAN passes 686/686 in 156.32 seconds. Independent read-only source review
reports no findings. Exact physical tie precedence and collector
CR-01/WR-03/WR-04 remain deferred. Actual new-source hosted qualification is
recorded separately; these local facts do not complete Plan 21-09 or phase
requirements.
