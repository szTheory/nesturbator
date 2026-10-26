---
phase: 20-nmi-line-and-vblank-flag
plan: "02"
status: blocked
checkpoint_type: decision
gate: blocking-human
completed_tasks: 0
updated: 2026-10-26
---

# Plan 20-02: unresolved detector order

Plan 20-01 is complete at `2878bab`; its two test commits are `58607db`
and `88dead0`. The orchestrator's subsequent `cmake --workflow --preset ci`
passed all 677 tests and produced the library, runner and libretro packages.
Plan 20-02 has no completed task, production commit or SUMMARY. Plans
20-03 through 20-06 remain pending behind it. No phase completion or
requirement acceptance is claimed.

## Stop condition and decision

Task 1 explicitly requires: "If no independently grounded observable
distinguishes the remaining candidates, report that bounded ambiguity and
leave the causal capture claim incomplete."

Exactly three candidate builds were tried at the unchanged default alignment.
The live pinned tests distinguish B from A/C, but do not establish A versus C.
An owned short-pulse program distinguishes their predictions; it supplies no
independent hardware oracle. Therefore no candidate was selected. The executor
retracted its provisional A selection and restored all experimental edits.
State format remains 1 and behaviour revision remains 7.

Resume requires either independent hardware/source evidence settling A versus C,
or an explicit revision of this plan's acceptance rule. A possible revision is
to permit A as a documented provisional model, complete polling, then reassess
all six target tables; that is an unapproved alternative, not hardware proof.
Do not silently repeat the exhausted three-candidate experiment or advance to
dependent plans. Do not create a completion SUMMARY for this checkpoint.

## Live results retained

The executor observed AccuracyCoin pages 18 and 19 with the existing owned
runner, pinned ROM `6bee919529289c555803b612b8c9e4f7eaaeeabb`, ROM SHA-256
`8ac8904a3cf26fbdfa6be501e414d45886ff4478d30dee8e58fa436cf5fd5adc`, and
inventory SHA-256
`7d022800d01ad81159109bf7457af94ab6f80e4a682fce716ec476f7c454a4a5`.
All candidate metadata records `source_revision=2878bab664a0`,
`source_dirty=1`, and behaviour revision 7. These are experimental local
observations, not full-campaign results or updated accepted baselines.

| Candidate | Event order | NMI Control | Timing | Suppression | VBlank-end NMI | Disabled at VBlank | $2002 flag timing |
|---|---|---|---|---|---|---|---|
| A / 1 | M2-fall capture before register side effect | pass 01 | fail 06 | fail 06 | fail 06 | fail 06 | fail 0a |
| B / 2 | Register side effect before capture | fail 22 | fail 06 | fail 06 | fail 06 | pass 01 | fail 0a |
| C / 3 | Defer edge at M2 fall until next sample | pass 01 | fail 06 | fail 06 | fail 06 | fail 06 | fail 0a |

Raw bytes above are hexadecimal. Page-18 rows record frame 1483; page-19
rows record frame 528. Whole observations ended at 1592 and 642 frames.
All six metadata files report `verdict=complete` and
`stop_cause=all-page-results-terminal`.

Copies of all six `raw.tsv`, `report.tsv` and `metadata.txt` files are in
[probes/20-02-order-experiment](probes/20-02-order-experiment/), with
[SHA256SUMS.tsv](probes/20-02-order-experiment/SHA256SUMS.tsv).
The original diagnostics remain in ignored build scratch under
`build/ci/accuracycoin/order-candidate-{1,2,3}/page-{18,19}/`.
The parent inspected the retained reports and metadata before recording this
checkpoint. A/C reports are byte-identical on both pages.

## Commands and reproduction limit

For each candidate number, the executor configured with
`cmake --preset ci -DCMAKE_C_FLAGS='-DNESTURBATOR_NMI_EXPERIMENT_ORDER=n -DNESTURBATOR_NMI_EXPERIMENT_LOG'`
and built `cmake --build build/ci --target nesturbator-run`.
Here `n` denotes the numeric candidate substituted in the actual invocation.
The observation command pattern was:

```sh
build/ci/runner/nesturbator-run --accuracycoin-observe 18 \
  --rom tests/roms/accuracycoin.nes --frames 1924 \
  --accuracycoin-boot-cap 32 --accuracycoin-navigation-cap 5 \
  --accuracycoin-execution-cap 1800 \
  --accuracycoin-output build/ci/accuracycoin/order-candidate-n/page-18
```

Page 19 used `--accuracycoin-observe 19`, `--frames 1929` and its page-19
output directory. The experimental macro implementation, conditional owned
probe, candidate patches and candidate binaries were not retained. The
commands therefore document provenance but cannot reproduce these candidates
from the committed tree. Only the raw guest observations are durable evidence.
No new production experiment was run while packaging the checkpoint.

The executor reported that the temporary owned probe at start dot `240:330`
captured and reached the handler for A, while B/C missed; at `240:332`, A/C
captured while B missed. The pre-render probe returned `$95` (early bit 7 set,
late sprite flags clear). Its output survives only in the execution transcript;
these statements are executor-reported diagnostics, not independently retained
hardware evidence or a reproducible committed probe.

## Source limits

The phase's [hardware reference](../../preparation/NES-HARDWARE-CPU-APU.md)
HWC.02 describes NMI edge sampling during phi2 each cycle; HWC.03 describes
M2/phi2. Pinned AccuracyCoin `TEST_2002FlagTiming` (HWC.19) describes bit 7 at
M2 rise and sprite flags at M2 fall. These references do not settle precedence
for a simultaneous PPU line transition and CPU detector sample. The plan's
independent-evidence rule prevents choosing A just because its schedule is
plausible and A/C share the same current test failures.

## Restored-tree verification

After the executor restored the provisional edits, the parent ran
`cmake --preset ci`, `cmake --build --preset ci`, and
`ctest --preset ci -R '^(ppu\.nmi_races|core\.nmi_poll|core\.nmi_edge|ppu\.registers)$' --no-tests=error --output-on-failure`.
All four tests passed. The cache has empty `CMAKE_C_FLAGS` and full campaign
disabled. Production source and test files match `2878bab`; only checkpoint
documentation/evidence and STATE are added by closeout. The user-owned
configuration edit remains unstaged. No full Plan 20-02 campaign, hash refresh,
state-format migration or phase-goal verification was performed.
