# Phase 20 hosted delivery

The owner authorized the delivery needed to unblock Phase 20 on 2026-10-27: push the prepared phase branch and open its evidence PR so PR CI and Nightly can run. This authorizes delivery and collection readiness; phase completion and merge still require their existing evidence gates.

## Formatting repair and original evidence

The first normal push was rejected by the existing pre-push formatter because unpublished C/header snapshots were unformatted. Clang-format 18 was applied to those snapshots without changing their intended behavior. Of 51 outgoing commits, 45 acquired new object identities; 15 distinct source/test files needed formatting. Commit subjects, original author/committer metadata, plan ordering and the published base were preserved. No force push is required. Normal hygiene and formatting hooks remain enabled.

`20-DELIVERY-COMMIT-MAP.tsv` maps every original outgoing commit to its formatted identity. Historical measurement records continue to name their actual original source SHA and fingerprint; the map does not relabel them as runs against formatted source. Original commit objects remain retained by a local backup ref. The source digest from 20-09 is superseded for delivery because formatting and the measured baseline metadata changed effective inputs.

## Remeasured delivery inputs

- Formatting-only source head before metadata refresh: `28092e52803ab73ae9b3537383ca4423b8632a78`.
- One full-enabled campaign: 22 pages, 146 scored rows, 114 PASS / 32 FAIL, zero skip/not-run/timeout, five DRAW.
- Campaign report SHA-256: `29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac` (unchanged).
- Measured input fingerprint: `b154c4a48e17134e1421a7852c52257fd6486c8ceb1a03beff0aa43fb606cab1` over 56 inputs.
- Campaign source revision `28092e52803a`, dirty bit 1; the preserved config edit and a temporary Python import cache contributed to the local dirty observation. That generated cache was removed after measurement. This is local evidence, not clean hosted evidence.
- Focused activation, exact six targets, cold protected observation, all 108 original PASS protections, baseline parser, native hashes and hosted-control checks: 10/10 passed.
- Effective delivery source digest: `cf314a6d3c2494cefba4eb88bd6a779bb93e3d464e408103aa079b320bbb650f` over 415 files. Exclusions remain only `.planning/**` and `tests/accuracy/measurements.md`.
- Full local `cmake --workflow --preset ci`: 684/684 passed in 377.45 seconds; library, runner and libretro packages generated.

## Hosted acceptance

The PR must run CI including actual base/head per-ROM performance, and Nightly on the same tested source. Record the candidate head, synthetic merge/base mapping, run IDs and attempts in `build/phase20-hosted-evidence/delivery.json` once identified. Only completed available results can pass 20-10's read-only precondition. Missing, failed or incompatible results remain pending; original dirty-source and Nightly fetch debt are preserved until actual results resolve their scope. No result is waived by opening the PR.

## First hosted run and GCC correction

Draft PR #59 was opened at candidate `0be8f4bb27cb0483ab903f28417d687b2ad122fa`.
CI run `40499487090` and Nightly run `40499486950` started automatically.
The first GCC jobs rejected two implicitly narrowed tick-offset expressions
in `test_circuit_phase_projection` under `-Werror=conversion`. The exact
GCC 14 compile flags reproduced both diagnostics against the original test.
Explicit `uint16_t` casts for the fixed 2428/2452 tick fixture resolve them;
the corrected translation unit passes GCC 14's same strict syntax check.
No emulation behavior or assertion expectation changed. Focused local
NMI/state/activation/hash/collector tests passed 7/7 after correction.
The campaign build-input fingerprint remains unchanged; the full effective
source digest changes to
`0515b19f452f9280234e4611b3ac613c7e21e1a4a56c2d3e3b2a68456fe01e66`
over 415 files. New hosted runs are required for this corrected source.
The earlier 684/684 local workflow result belongs to the pre-correction
formatted source, while the correction has its own focused verification.

## Hosted integration corrections

The second candidate's GCC no-floating-point jobs passed, confirming the
explicit tick-offset correction. Windows then reported C4996 for the owned
runner fixture's standard `fopen` call. Its target now uses the same
MSVC-only `_CRT_SECURE_NO_WARNINGS` definition as neighboring test tools.
The RetroArch job built its tools but failed its preflight because the
marker cartridge is produced by `retroarch.marker.generate`, not by the build.
The existing hosted job now runs exactly that fixture CTest before using it.
A fresh separate build demonstrated that the build alone had no marker ROM
and the targeted CTest created it successfully. Roll-up policy checks pass.

Live GitHub run metadata reports candidate head `d7ad9695d9226b447b98a117015946da184ed65b`,
while the tested merge was `51d193d0132c3af453f17d67a56e66f64a4aa645`.
The previous offline fixture incorrectly put the merge SHA in `head_sha` and
omitted matrix fields in job names. Updating the fixture first reproduced
the collector failure. The collector now binds the run to the candidate,
retains a matching PR snapshot, resolves its `merge_commit_sha`, then verifies
merge parents, artifact source and full source trees independently. Required
job identities include the full session/nofp matrices and actual build matrix
fields. Five added controls reject wrong run head, stale PR snapshot, wrong
PR merge, missing matrix jobs and truncated build job names at their intended
validation stages. The corrected resolver also passed against the actual
read-only PR/run metadata. All original fixture/refusal controls remain active.

The one new campaign after the CMake target change retained 114 PASS / 32 FAIL
and the original report hash, with measured fingerprint
`7e9ebb45e5367df9b4b21f0fdbba86b19ca92a63b5bcbb592cfd9ef990a42e13`.
It records source revision `d7ad9695d922` and dirty bit 1 for the in-flight
CI/collector/documentation edits and preserved config edit. Full campaign
registration is OFF after measurement. Focused acceptance passed 35/35,
including all performance controls, complete hash keys, target/protected
observations, baseline parser and collector controls.

The superseding effective input digest is
`f73937e970bedd54d6f9379d235f3f44887e014970a22718997c7929b6b2b3b5`
over 415 files. Earlier hosted runs cannot establish acceptance of these new
inputs. Full local workflow result and final delivery IDs are recorded once
the corresponding processes finish; no hosted acceptance is claimed here.

Final repaired-source `cmake --workflow --preset ci` passed 684/684 in
376.67 seconds and packaged all three deliverables. Full campaign registration
is OFF. Hygiene, roll-up policy, clean marker generation, API resolver and
35-test focused checks passed before the delivery commit.

## Delivered candidate and active runs

Draft PR: https://github.com/szTheory/nesturbator/pull/59
Candidate: `2d904b7daef8f4882e3d50389869f21571727b62`
PR merge snapshot: `67e8301a4a0703fb828ee39ee92277e26752dbfc`
Protected base: `ab9c2e6eb822c06f0dfb0f08a0ca42ec541358be`

- CI: run `40501700246`, attempt `1`, observed status `in_progress`.
- Nightly: run `40501700252`, attempt `1`, observed status `in_progress`.

Both read-only API records bind the run to PR #59 and the delivered candidate.
`build/phase20-hosted-evidence/delivery.json` contains these actual identities.
This records delivery and work in progress, not successful hosted acceptance.
20-10 must still require completed jobs/artifacts and validate the bundle.
Use the superseding source digest `f73937e970bedd54d6f9379d235f3f44887e014970a22718997c7929b6b2b3b5`
when reconciling the historical 20-09 summary with the delivered source.
Planning-only handoff commits made after this candidate remain local so they
do not cancel and restart its PR CI. No merge or phase completion occurred.

## MMC3 regression found by actual Nightly

Nightly 40501700252 reached its ROM tests and failed only
`mmc3_test_2/4-scanline_timing`: code 0x02 at frame 48 (12/13 CTests passed).
The exact case reproduced locally. A scratch build of protected main
`ab9c2e6` passed it; scratch builds at the Phase 20 line-capture and poll commits
both failed, narrowing the regression to 20-02. CLI-inhibit and poll-position
experiments did not restore the case and were discarded outside the working
tree. No ROM constants, expected verdicts or alignment offsets were changed.

The owned trace found a missing piece of the selected global bus order. NMI
used the detector sample before a tied PPU transition, but IRQ's following
instruction poll read the already changed live line. In the first failing
case A12 rises at pre-render dot 261, tied to the NOP opcode-fetch close.
Protected main reached the next instruction with live IRQ 1 and poll 0; the
Phase 20 model reached it with live IRQ 1 and poll 1 and entered IRQ before
the guest INC marker. Retaining the IRQ level at the same close-edge sample
as NMI restores the preceding-close input to the instruction poll. This is
a shared bus-order regression repair, not implementation of Phase 21's full
I-flag, branch/IRQ or vector-hijack requirements. Physical exact-tie precedence
remains a model-policy qualification.

`ppu.mmc3::test_irq_capture_before_tied_a12` constructs the finite instance
state at (261,258), then executes production NOP/INC/IRQ entry. Before the fix
it failed the expected poll-0 and guest marker assertions. After the fix it
checks live IRQ 1 versus poll 0, the completed guest INC, IRQ entry and
canonical save/load continuation over a destination with different retained
history. The retained level is a bounded boolean in the same production state
visitor; hostile-state sweeps include it. Unpublished format 2 is finalized
with this byte, retaining format-1 refusal and behavior revision 8.

The full MMC3 tier now passes 13/13 locally, and NMI/state/rejection/policy
checks pass. The newly measured native inventory changes only 12 state hashes
and six state sizes (+1 byte); frame, PCM, transition and tick rows are
identical. `20-IRQ-STATE-HASH-MOVES.tsv` retains every old/new row. The added
serialized byte changes payload length, header total size and CRC, accounting
for the state hash movement. This repin does not mask an output regression.

One remeasured AccuracyCoin campaign still completed 22 pages with 114 PASS /
32 FAIL, no skip/not-run/timeout, the original six target passes and unchanged
report digest. Its actual input fingerprint is
`7a31d9bdc998cbb62593384dd7f809b9e4def2c8d259c31cb693d5a092431183`,
source revision `2d904b7daef8`, dirty bit 1 for this coherent repair and the
preserved config edit. Focused final acceptance passed 15/15. Full campaign
registration is OFF. The superseding effective source digest is
`0cb4f7fec388edd64c7edf82760136689930e481c1e443fe3f7b13444219d436`
over 415 files. Final local CI and replacement hosted identities follow; no
previous run can certify this repaired source.

### Final fixture and verification closeout

The first full run after IRQ retention exposed two layout-dependent JAM
controls still targeting byte 91 and a stress stimulus that no longer reached
pending OAM on both boards. The JAM controls now target the actual byte 92
(64-byte header + 14 core bytes + 14 CPU bytes before jammed). Both the normal
JAM check and deliberate-divergence check passed. The owned stress program's
DMC-to-OAM gap is calibrated to G=8, which reaches pending OAM on NROM frame
60 and MMC3 frame 56 within the existing short-run cap; pending DMC and poll
coverage remains required. No coverage assertion, frame cap, oracle verdict
or hardware timing offset was relaxed. The normal stress replay passed.

The A12 witness also covers a tie at the NOP's final close: live IRQ is 1
while the retained level is 0. Save/load must restore that low history over a
destination poisoned high, rather than reconstructing it from the live pin.
Both retained-level cases continue through the guest INC and IRQ entry.

The final source passed `cmake --workflow --preset ci` 684/684 with library,
runner and libretro packages, `cmake --workflow --preset asan` 676/676, and
the independent MMC3 tier 13/13. Formatting and hygiene checks passed.
The final effective source digest is
`30aabfeee892574cfe98ab69de1cacce4bfad45d9b7d72427e7abbd070e84f2a`
over 415 files; it supersedes the earlier repair digests for actual delivery.
The fixture-only refinements do not change the measured campaign's narrower
build-input fingerprint, and the full manifest still covers all their bytes.
Fresh hosted results must test this finalized source.

## Final repaired delivery

Candidate `b474e1d081c12d44bd03e224d6248a5cac08b503` is published on draft PR #59. The selected PR merge
snapshot is `6b2b1e4b199f577b7dd8dc9993fdac06c5ef93bb`, with protected base
`ab9c2e6eb822c06f0dfb0f08a0ca42ec541358be`.

- CI `40507506429`, attempt `1`: observed `in_progress`.
- Nightly `40507506358`, attempt `1`: observed `in_progress`.
- Effective source digest: `30aabfeee892574cfe98ab69de1cacce4bfad45d9b7d72427e7abbd070e84f2a`
  over 415 files.

`build/phase20-hosted-evidence/delivery.json` now selects only these final run
identities. All earlier candidates/runs in this record remain historical and
are superseded for acceptance. Full local CI 684/684, ASan 676/676 and MMC3
13/13 passed before this push. Normal push hooks passed. Actual final hosted
acceptance still requires completed jobs/artifacts and 20-10's content gates.
Only this planning handoff is committed locally afterward, so it does not
restart the source's CI. No merge or phase/requirement completion is claimed.


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


## Replacement publication after session and performance repairs

Candidate `e6e83a31e413e12abe9dee142e3c0f7143d63eb4` is published on draft PR #59. Normal
push hooks passed, including all 34 outgoing committed C/header blobs.
Protected base remains `ab9c2e6eb822c06f0dfb0f08a0ca42ec541358be` and the actual PR merge
snapshot is `11533e456d41f740751c8353f947d65017aae692`.

- CI `40514060876`, attempt `1`: observed `pending`.
- Nightly `40513950000`, attempt `1`: observed `in_progress`.
- Effective source: `de6d4355222063e019c7a3a94e30a404b52041f3d994659d445f12b4a23318ab`
  over 415 files.

Updating the PR description triggered its configured edited-event CI; select
CI 40514060876 rather than the superseded synchronize run 40513949984.
Nightly's original same-source synchronize run remains selected. The delivery
JSON now contains exactly these replacement IDs. The local CI 684/684,
ASan/UBSan 676/676, MMC3 13/13 and real per-ROM local performance results above
belong to this effective source. The hosted jobs must still complete and their
actual artifacts must pass 20-10. No merge, requirement closure, phase closure,
20-10 task commit or summary is claimed. This planning-only handoff remains
local to avoid another unnecessary CI restart.
