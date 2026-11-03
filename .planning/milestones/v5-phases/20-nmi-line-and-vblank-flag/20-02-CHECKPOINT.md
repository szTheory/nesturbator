---
phase: 20-nmi-line-and-vblank-flag
plan: "02"
status: superseded
checkpoint_type: decision
resolution: superseded-by-amended-plan
completed_tasks: 0
total_tasks: 3
updated: 2026-10-26
---

# Plan 20-02: detector-order checkpoint superseded by amended acceptance

## Operative resolution — 2026-10-26

The owner's request to unblock execution after independent-oracle research
authorizes the dated D-10 amendment in [20-CONTEXT.md](20-CONTEXT.md) and the
revised [20-02-PLAN.md](20-02-PLAN.md). **This checkpoint is superseded by
the amended plan; its old ambiguity stop is historical and is not an active
execution gate.** Resume Plan 20-02 at Task 1 under that plan after the planning
revision is checked. Do not repeat the exhausted three-candidate sweep as a
prerequisite or infer a provisional A winner from aggregate page results.

The independently reproduced 6/6 circuit oracle constrains finite-margin,
phase-qualified sampling and retained requests; it distinguishes internal
phi2 from widened M2. Owned C tests must preserve all six waveform meanings
with an explicit phase map and helper/bus representation limits, plus real
decoded status/enable/disable guest-marker paths and wrong-model controls.
One source-consistent globally applied deterministic convention handles
unresolved equal timestamps, explicitly as model policy with hardware limits.
No physical A/C winner is claimed. Unknown exact physical precedence alone
does not block development or downstream first-divergence diagnosis; actual
contradictory finite-margin evidence, lost protected PASS or a final target
failure still blocks the relevant acceptance. All six target rows, 108 PASS
protection, whole-table/default-alignment discipline, format 2/revision 8,
state atomicity, causal hash evidence and full CI remain mandatory.

This resolution completes no implementation task: Plan 20-02 remains **0/3**,
has no SUMMARY, and production remains format 1/revision 7 until its causal
change is actually implemented and verified. Plan 20-01 stays complete;
the remaining five plans stay pending in the original six-plan/wave order.

## Historical execution checkpoint (superseded stop, retained evidence)

Plan 20-01 is complete at `2878bab`; its two test commits are `58607db`
and `88dead0`. The orchestrator's subsequent `cmake --workflow --preset ci`
passed all 677 tests and produced the library, runner and libretro packages.
Plan 20-02 has no completed task, production commit or SUMMARY. Plans
20-03 through 20-06 remain pending behind it. No phase completion or
requirement acceptance is claimed.

## Original stop condition and decision — historical

At the original stop, Task 1 explicitly required: "If no independently grounded observable
distinguishes the remaining candidates, report that bounded ambiguity and
leave the causal capture claim incomplete."

Exactly three candidate builds were tried at the unchanged default alignment.
The live pinned tests distinguish B from A/C, but do not establish A versus C.
An owned short-pulse program distinguishes their predictions; it supplies no
independent hardware oracle. Therefore no candidate was selected. The executor
retracted its provisional A selection and restored all experimental edits.
State format remains 1 and behaviour revision remains 7.

The owner then selected independent-oracle research on 2026-10-26. The resulting
[research and executed die-network probe](20-NMI-ORACLE-RESEARCH.md) establishes
six nonzero-margin CPU sampling/retention cases, with an owned reproduction
driver and exact trace retained. This is new independent circuit evidence,
but does not select A/C at the old zero-margin paired-chip boundary. The
block remained at the end of that research-only step; no acceptance change
or further candidate sweep was then authorized or performed. The missing
physical evidence was a source-backed PPU-output-to-CPU-input schedule, or a
paired-chip measurement establishing that relation. The later owner request
and D-10 amendment above supersede this development stop; the scientific gap
remains honestly recorded.

At that checkpoint, resumption required independent hardware/source evidence
settling A versus C or explicit acceptance revision. A provisional A selection
was only an unapproved alternative; it was not hardware proof and is not the
amendment now adopted. The adopted rule instead constrains capture by the
independent circuit evidence, labels the global tie policy and retains every
final acceptance obligation. The old instruction not to advance dependent
plans applied while this checkpoint was active; after checked revision,
normal plan dependencies govern. Do not create a completion SUMMARY for this
checkpoint.

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
for a simultaneous PPU line transition and CPU detector sample. The
independent-evidence rule continues to prevent claiming A as a hardware
winner just because its schedule is plausible and A/C share the same current
test failures. Amended D-10 permits documented global model policy without
such a claim; this source limitation no longer bars capture implementation.

## Restored-tree verification

After the executor restored the provisional edits, the parent ran
`cmake --preset ci`, `cmake --build --preset ci`, and
`ctest --preset ci -R '^(ppu\.nmi_races|core\.nmi_poll|core\.nmi_edge|ppu\.registers)$' --no-tests=error --output-on-failure`.
All four tests passed. The cache has empty `CMAKE_C_FLAGS` and full campaign
disabled. Production source and test files match `2878bab`; only checkpoint
documentation/evidence and STATE are added by closeout. The user-owned
configuration edit remains unstaged. No full Plan 20-02 campaign, hash refresh,
state-format migration or phase-goal verification was performed.
