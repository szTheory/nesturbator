---
quick_id: 261029-sdj
status: complete
implementation_commit: dc3451b
verification_scope: implementation and full local CI; hosted publication checks reported separately
date: 2026-10-29
---

# Remove repeated CI inventory and hash work

The no-skip checker extracts each test object once before inspecting properties.
Its rejection rules remain intact; new self-test cases place each forbidden
property after an ordinary property on a later test. A deliberately wrong
first-test-only extractor is rejected by the new cases.

Hosted build lanes remove any prior `build/<preset>/tests/hashes.txt` before
running the workflow, then upload the file produced by `runner.write_hashes`
and pin-checked by `runner.write_hashes.content`. The duplicate post-test
generation step is removed. Artifact names, the root `hashes.txt` member,
missing-output errors and all six-platform inventory/equality checks remain.
The existing uploader handles the single-file path directly; no new helper or
dependency is needed.

README and public-header comments describe the same contract. The measured
AccuracyCoin baseline is refreshed for the changed public-header fingerprint.

## Verification

| Check | Result |
|---|---|
| Real 695-test inventory, original helper | 8.756 s; all tests accepted |
| Same inventory, optimized helper | 1.951 s; same result; about 77.7% less local wall time |
| policy.no-skip.self_test | Pass, including all three later-test/property cases |
| First-test-only extractor mutation | Rejected by the later SKIP_RETURN_CODE case |
| Fresh native hash producer/content and existing inventory self-test | 3/3 pass in 11.33 s after clearing the old file |
| Actual produced file copied to six local artifact-layout fixtures | Complete 114-key consumer passes; missing and changed artifact negatives reject; restoration passes |
| Actionlint workflow syntax/expressions, ShellCheck disabled | Pass |
| clang-format 18 public-header check, tree hygiene and whitespace | Pass |
| Metadata correction recheck | 5/5 pass in 53.95 s |
| Final `cmake --workflow --preset ci` | 695/695 pass in 418.82 s; library, runner and libretro ZIPs generated |
| Final archive validation and retained hash file | All three archives pass; native hash output still byte-identical to the pin after packaging/archive checks |

The isolated timing pair is retained at
`build/ci-efficiency-quick/policy-timings.json`. An earlier timing pair overlapped
a build/self-test and is not used as performance evidence. The artifact-layout
check uses six copies of the local native output; it is not six-platform proof.

The final-source actual campaign, after explicit configure/build, retains
119 PASS / 27 FAIL, all 146 scored rows terminal, five inventory-only DRAW rows
and no skip/not-run/timeout. Its report hash remains
`56ebd01adb97b3916fde36f8739602284d951a1fd00d84285bb4ef441fb463a3`.
The 56-input fingerprint is
`ab0bdf5142426482fa18fecae5c2c7272c4957b808f6c7aa8feaaf3b767cec80`;
compiled revision `d4802809ff41`, dirty 1, host guest-execution cost 5091122000 ns.
Artifacts: `build/ci-efficiency-quick/campaign-final/`.

## Observation history and publication

The first full CI attempt passed the policy/hash/session tests but failed five
evidence checks because the new explanatory source_dirty_reason value contained
an extra equals sign. The strict readers correctly rejected the malformed note.
That note is corrected; all five failed checks pass on recheck. This failed run
is not acceptance. A fresh final `cmake --workflow --preset ci` passes all 695
tests and generates all packages; raw output is `build/ci-efficiency-quick-ci-final.log`.

Implementation commit: `dc3451b`. Code and the planning records publish together
through existing PR #61, starting one fresh hosted verification wave. Hosted
policy and six-platform hash-equality results are supplied by that publication
head's checks; observations are retained in
`build/ci-efficiency-quick/hosted-results.json` when collected. No hosted wall-time
improvement is inferred from the local benchmark. No merge or further lifecycle step.

## Limits and deferred work

The local helper measurement is not a claim about hosted PR wall time. Prior
hosted acceptance remains tied to its original source. All original checks,
timeouts, platform lanes and required-rollup behavior remain.

A planning-only fast path remains proposed followup scope: it needs a separate
effective-source/required-check design. Compiler caching remains lower priority
given the measured small compile share. Existing collector debt and physical
timing/provenance limits remain unchanged.

The pre-existing config edit remains unstaged and byte-identical (SHA-256
`62a22f6862efb7c74ee31baf2f22bae32d453e79aaec6726dbcf3b7213d47167`).

## References

- GitHub workflow run/shell syntax: https://docs.github.com/en/actions/reference/workflows-and-actions/workflow-syntax
- Existing pinned upload action single-file/path behavior: https://github.com/actions/upload-artifact
- Hosted baseline CI: https://github.com/szTheory/nesturbator/actions/runs/40960199568
