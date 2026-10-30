---
phase: 21-interrupt-latency-and-hijack
plan: "04"
status: resolved
checkpoint_type: decision
gate: blocking-human
created: 2026-10-29T04:19:19Z
decision_at: 2026-10-29T09:45:34Z
decision: bounded DMC/open-bus prerequisite investigation and repair approved
completed_plans: ["21-01", "21-02", "21-03"]
---

# Plan 21-04 execution checkpoint — resolved

## Approved continuation

On 2026-10-29 the owner answered yes to the bounded investigation and repair
proposed below. Resume Task 3 with an owned reproducer and the minimum
source-supported prerequisite fix, preserving all acceptance gates and the
existing uncommitted implementation. This approval supersedes the prior
waiting state; it does not authorize a general DMA cluster or remote delivery.
The original checkpoint evidence follows unchanged.

Plans 21-01 through 21-03 are committed. Plan 21-04 is incomplete. Its prepared
source/test/docs/pin changes remain uncommitted on the Phase 21 branch; there is
no 21-04-SUMMARY.md and no phase-completion claim. Preserve this working tree and
the preexisting, unrelated `.planning/config.json` edit.

## Prepared and verified work

- Shared BRK/IRQ/NMI begin/action/drain mechanism with instance-owned entry
  continuation and a coherent vector pair selected between C4 and C5.
- Source-qualified lifetime/edge/priority/wrap tests; owned BRK32 and IRQ64
  paired guest tables match exactly.
- Visitor serialization and candidate-byte cross-field validation, rejection
  before APPLY, and public save/load continuation cases.
- README/public-header updates, format3/revision9 compatibility, measured
  native state hash/size movement from four continuation bytes, regenerated
  seed checks, and stable frame/PCM/APU-transition hashes.
- JSR C5/C6 poll-placement reproducer and correction. The final trace has no
  accepted IRQ immediately after `JSR $4013`, before the target fetch.
- State-fork fixture calibration G=8 to G=9, measured to retain OAM, DMC and
  poll straddles on both boards with the original coverage assertions.

The executor's final `cmake --workflow --preset ci` passed **691/691** and
packaged the library, runner and libretro adapter. The earlier OAM stress
coverage failure is resolved in that final run. This is local automation
evidence, not campaign acceptance or hosted evidence.

## Historical campaign blocker (superseded by final campaign evidence)

The full enabled pin6bee919/cap1800 campaign is **114 PASS / 30 FAIL /
2 not-run**. All 114 protected starting PASS keys remain PASS. `Interrupt flag
latency` is `fail 0x03` at frame 3; the two overlap rows are unstarted.

The exact first diagnostic fetch after the corrected JSR poll is at `$4013`:
the bus returns `$40` (RTI), where the pinned sequence expects DMC/open-bus `$90`
(BCC). The guest subsequently jams. This places the observed divergence before
the target BCC polling and shared entry. It identifies the prerequisite boundary;
it does not establish a complete DMA root cause or authorize a general DMA fix.
Detailed source/measurement attribution is retained in
`tests/accuracy/measurements.md` under the Plan 21-04 checkpoint.

## Historical stop rationale

Plan 21-04 Task 3 requires all eleven v5 rows to pass and all 146 scored rows
to remain terminal. Its action says to report a blocking prerequisite
"without cap/table/DMA scope changes"; its change-atomicity section excludes
a DMA cluster and withholds the causal commit until all tasks are green.
The local CI result cannot waive those conditions.

Tasks 1–2 have prepared passing evidence. Task 3's acceptance is unmet. Plans
21-05 through 21-09 depend on this incomplete plan and have not started. Phase
review, goal verification, completion, PR publication and hosted runs have not
run. INT-01, INT-02 and KEEP-01 remain pending.

## Historical decision record (resolved)

Preferred: authorize a bounded investigation and repair of the traced
DMC/open-bus prerequisite, with an owned reproducer, primary hardware sources,
unchanged pin/cap/answer tables, protected PASS checks and fresh final CI and
campaign evidence. Retain the existing shared-entry work and its one green
causal commit boundary; do not silently expand to a general DMA cluster.

Alternative: keep this checkpoint blocked for separate scope planning. Do not
mark Plan 21-04 complete or commit its causal change with red acceptance.

The owner approved the bounded investigation and repair on 2026-10-29. Task 3
then completed with fresh source-bound evidence; no general DMA cluster or
remote delivery was included.

## Final closeout

The final campaign at pin `6bee919529289c555803b612b8c9e4f7eaaeeabb` completed
22/22 pages and all 146 scored rows: 119 PASS, 27 FAIL, zero not-run/skip/
timeout, and five inventory-only DRAW rows. All 114 protected starting PASS
keys and all eleven v5 target rows pass. Fresh protected-main authority is
`a64c23fd3ca7caed7e3ad102ff1a9beaa29fa562`. The measured report SHA-256 is
`56ebd01adb97b3916fde36f8739602284d951a1fd00d84285bb4ef441fb463a3`; its
56-input fingerprint is
`c1aeeddca9b0b5a2641d66ffa9309998dfb67bb620cb1cbb94d6c59534865283`.

Final `cmake --workflow --preset ci` passed 691/691 tests and generated the
library, runner and libretro packages. The coherent causal source change is
ready for commit; plan summary and metadata closeout follow. INT-01, INT-02
and KEEP-01 remain pending later hosted/all-lane Phase21 acceptance.
