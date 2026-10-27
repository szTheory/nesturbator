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
