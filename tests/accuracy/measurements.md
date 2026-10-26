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

The global integer-timestamp policy is PPU transition, CPU sample, then bus
side effect. Finite-margin cases support the phase endpoints, while exact-tie
precedence remains a deterministic model rule. The PPU owns flag, enable and
suppression; the bus samples the derived asserted line at the cycle close and
retains a rising-edge request independently. A falling line does not revoke a
request; CPU service consumes it and reset clears both sampled history and
pending request. State format 2 serializes those fields and refuses format-1
states; behaviour revision is 8.

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
