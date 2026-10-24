# AccuracyCoin bounded campaign measurements

## Final campaign identity

The final registered run used the canonical 6bee ROM and production collector:

- Pin: `6bee919529289c555803b612b8c9e4f7eaaeeabb`
- ROM SHA-256: `8ac8904a3cf26fbdfa6be501e414d45886ff4478d30dee8e58fa436cf5fd5adc`
- Inventory SHA-256: `7d022800d01ad81159109bf7457af94ab6f80e4a682fce716ec476f7c454a4a5`
- Retained comparison pin: `673ef550db296136d52229961e7d39366116882a`, ROM SHA-256 `4fe8c2bc9abc6f4d418da47b73f62cba89fcacd950fae763097a1681d650e839`, inventory SHA-256 `3271803cdc63c4b4a83cf65453b817dc66ec65ed97863ad19261b56f36821c84`
- Old inventory: 149 entries (144 scored, 5 DRAW); new inventory: 151 entries (146 scored, 5 DRAW)
- Report/inventory schema: 1
- Compiled source revision: `cc48e64b4426` (`source_dirty=1`). The capture includes the paired README and public-header test-contract documentation edits; the only preexisting unrelated edit was `.planning/config.json`.
- Deterministic build-input fingerprint: `b61aafe19b707527006f1945011c90ab0818d02f291b3c88494aa2829d15556f` over 56 source/build input files. Generated reports are excluded. Campaign report SHA-256: `6d0a811960f6964e7fc66802c6e4ade0c4ffb8772c40d88d50cf79e1f8463652` (unchanged).
- Input policy: cold boot; one page-header A input; one-frame A press; one-frame release; execute the selected page independently.
- Accepted bounds: boot 32, navigation 5 per page switch, start 2 frames, execution 1800; page child 5 seconds, collector 10 seconds, CTest 150 seconds.
- Actual CTest campaign duration: 5.77 seconds. Sum of per-page host measurements: 5.681307 seconds; slowest child was page 19 at 1.551326 seconds. A separate collector-only run completed below `/usr/bin/time -p`'s 0.01-second display resolution.
- Retained output: `build/ci/accuracycoin/full/` contains all 22 page directories, child stdout/stderr, raw snapshots, diagnostics, page reports/metadata, `outer-outcomes.tsv`, and the production `inventory.tsv`, `attempts.tsv`, `observations.tsv`, `report.tsv`, `summary.txt`, and campaign `metadata.txt`.

`outer-outcomes.tsv` records 22 `EXIT_ZERO` children, no outer timeout and no child failure. Campaign metadata says `trustworthy_observation=1` and `infrastructure_verdict=complete`.

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

| Page | Protected keys | Old `673ef` measurement | New `6bee` measurement |
|---:|---|---|---|
| 2 | Absolute Indexed; Zero Page Indexed; Indirect; Indirect, X; Indirect, Y; Relative | All six pass, raw `$01`, 11 frames each | All six pass, raw `$01`, 12 frames each |
| 14 | Length Counter; Length Table; Delta Modulation Channel; Frame Counter IRQ*; Frame Counter 4-step*; Frame Counter 5-step* | All six pass, raw `$01`, 120 frames each | All six pass, raw `$01`, 120 frames each |
| 17 | CHR ROM is not writable; PPU Register Mirroring; PPU Register Open Bus*; PPU Read Buffer; Palette RAM Quirks | Four pass raw `$01` and one pass raw `$41`, all 151 frames | Four pass raw `$01` and one pass raw `$41`, all 159 frames |

`*` indicates a source-reviewed changed meaning: the three frame-counter tests changed their cycle synchronization sequence, and PPU Register Open Bus changed its initial setup. The assertions and test symbols were reviewed per pinned source in `migration.tsv`. PPU Read Buffer `$41` remains a source-defined accepted pass for that key only. `tests/accuracy/old-673ef-scoreboard.txt` preserves the old strict expectations; `migration.tsv` carries the exact old/new raw bytes, frame counts, attempts, source symbols and fresh observation references.

## Per-page measurements

| Page | Boot | Navigation | Start | Execution | Total frames | Verdict | Host ms |
|---:|---:|---:|---:|---:|---:|---|---:|
| 1 | 26 | 0 | 2 | 51 | 79 | complete | 70.250 |
| 2 | 26 | 5 | 2 | 12 | 45 | complete | 35.277 |
| 3 | 26 | 10 | 2 | 13 | 51 | complete | 42.105 |
| 4 | 26 | 15 | 2 | 13 | 56 | complete | 43.961 |
| 5 | 26 | 20 | 2 | 13 | 61 | complete | 49.168 |
| 6 | 26 | 25 | 2 | 13 | 66 | complete | 50.904 |
| 7 | 26 | 30 | 2 | 22 | 80 | complete | 65.839 |
| 8 | 26 | 35 | 2 | 15 | 78 | complete | 63.509 |
| 9 | 26 | 40 | 2 | 13 | 81 | complete | 66.004 |
| 10 | 26 | 45 | 2 | 14 | 87 | complete | 71.268 |
| 11 | 26 | 50 | 2 | 17 | 95 | complete | 76.300 |
| 12 | 26 | 55 | 2 | 138 | 221 | complete | 156.450 |
| 13 | 26 | 60 | 2 | 533 | 621 | complete | 624.818 |
| 14 | 26 | 65 | 2 | 120 | 213 | complete | 193.236 |
| 15 | 26 | 70 | 2 | 92 | 190 | complete | 178.089 |
| 16 | 26 | 75 | 2 | 15 | 118 | complete (DRAW-only) | 96.690 |
| 17 | 26 | 80 | 2 | 159 | 267 | complete | 234.472 |
| 18 | 26 | 85 | 2 | 1463 | 1576 | complete | 1060.198 |
| 19 | 26 | 90 | 2 | 1800 | 1918 | guest-timeout | 1551.326 |
| 20 | 26 | 95 | 2 | 503 | 626 | complete | 469.987 |
| 21 | 26 | 100 | 2 | 212 | 340 | complete | 264.718 |
| 22 | 26 | 105 | 2 | 151 | 284 | complete | 216.738 |

At page 16, the ROM's DRAW-only sequence reports no scored result transition. The start is attributed to the observed row-cursor transition and automation flag; completion is the later cursor return to the page header with automation cleared. All five DRAW rows remain inventory-only.

At page 19 under the final five-frame navigation policy, the final diagnostic sample has menu cursor `$05` (`menuCursorYPos`, CPU RAM `$0016`) and `AutomateTestSuite` `$01` (CPU RAM `$0034`). This qualifies inventory row 6, `Arbitrary Sprite zero`, as the active `$03` test at the execution cap. Row 7, `Misaligned OAM behavior`, also retains raw `$03`, but is not the current cursor row and is classified `not-run` with cause `noncurrent-running-marker`; row 8 is observed `$00`/`unstarted`. The observer and collector retain the bytes and cross-check the cursor/automation diagnostics before assigning the timeout.

## Separate navigation-cap-6 exploratory sample

This is one page-19 exploratory attempt, not part of the 22-page final campaign and not combined into its counts. With `boot=32`, `navigation=6`, and `execution=1800`, it used 108 navigation frames, reached the cap after 1936 total frames, and took 1.91 seconds externally. The final diagnostic cursor was `$06` with automation `$01`; the active key was row 7, `Misaligned OAM behavior`, raw `$03`, so row 7 timed out while row 6 was terminal fail `$06`. The per-page raw and diagnostic output is retained separately under `build/ci/accuracycoin/navigation-cap-6-page-19/`.

The difference demonstrates that this fixed frame sequence's navigation cap also changes guest timing: an extra neutral frame per page shift changes which test is active when page 19 reaches its execution cap. The final campaign therefore uses the measured five-frame input unit throughout; statuses from the six-frame probe are not interchangeable with its results. The adversarial report fixture separately proves that when two `$03` samples appear in one snapshot set, only the `$0016` cursor-qualified row can be timeout and the other raw marker remains visible as `not-run`.

## Cap rationale and CI placement input

- Boot: all 22 attempts reached menu-ready by frame 26. Cap 32 leaves 6 frames (23%) for startup variation without increasing the longer per-page deadline.
- Navigation: five frames per page change is the measured fixed sequence: one Right edge followed by four neutral frames. The six-frame probe changes guest timing, so it is a distinct policy rather than a free safety margin.
- Start/release: two frames, with A released on the second frame, were observed in all 22 attempts. Controller-sensitive page 14 also completed through the released-input path.
- Execution: cap 1800 leaves 337 frames above the longest completed execution (page 18 at 1463); page 19 remains honestly timed out. Increasing this cap to erase that hardware observation is not justified.
- Wall bounds: the slowest final page was 1.756 seconds, about 2.8 times below the 5-second page deadline. The 22 sequential page deadlines plus 10-second collection cap total 120 seconds; the 150-second CTest timeout leaves 30 seconds for orchestration and final output checks.
- Placement selected for Plan 17-04: one full campaign in the required Linux x64 build lane on each PR, not all six platform matrix legs. CI turns on `NESTURBATOR_ACCURACYCOIN_FULL` only in that lane; the other five still run decoder, classifier, activation, migration, protected-pass and parity checks. The full campaign CTest took 6.46 seconds locally. Each build lane uploads its `build/<preset>/accuracycoin/` evidence with an `always()` upload step and fails if the required directory is absent. The existing Linux build timeout remains 39 minutes: this repository sets it from sampled hosted runs, so the selected lane's actual hosted execution must be recorded before revisiting it. The same applies to the current timeout comment; local duration does not establish hosted performance.

No row-A fallback was run. The cold campaign observed raw bytes for every scored row, and the only nonterminal results retain a current active timeout, a source-disqualified running marker, and an unstarted `$00` row. A separately selected-row attempt would change the guest context, and no additional raw byte is missing from this baseline. The two honest `not-run` findings remain visible for later plans.
