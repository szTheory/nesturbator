---
phase: 18
review: 18-REVIEW.md
source_head: 78bb8d84bd6b579b21c8283684cfa8593055c724
review_findings: [WR-01, WR-02, WR-03, WR-04, WR-05]
status: fixed
verification:
  local_ci: "78bb8d8: 669/669 tests and three packages"
  linux_campaign_run: 40148240864
  linux_campaign_artifact: 12580614764
  matched_replay: "build/execution-measure-review-fix-78bb"
requirements_completed: []
---

# Phase 18 Review Fixes

This supplement records the corrections and evidence for WR-01 through WR-05 from
[`18-REVIEW.md`](18-REVIEW.md). The review report remains unchanged. All synthetic crash inputs
below are explicitly test controls; the bounded real-corpus campaign reported no findings.
Phase requirements ROB-01 through ROB-04 remain pending independent phase verification.

## Source and Verification Identity

The review fixes are on Phase 18 branch head `78bb8d84bd6b579b21c8283684cfa8593055c724`.
The principal fixes are in `91ca8364598cb883a4aac3b0f4582f5fb25f99ed` (`WR-01`, `WR-02`, `WR-04`,
`WR-05`). `25ee82a725eece237111cea0d8ef581a6b38c492` guards the replay-only acceptance helper.
`9b0402488737e2f7eb86de1cf069dab85a118254` adds the shared collector control and isolates the
Nightly self-test output. Commit `78bb8d84bd6b579b21c8283684cfa8593055c724` replaces deprecated
`getenv` calls with the MSVC-safe `_dupenv_s` path in the test-only fuzz control. The
exact Linux campaign and its controls ran in Nightly run
[40148240864](https://github.com/szTheory/nesturbator/actions/runs/40148240864), whose PR head is
`78bb8d8` and whose merge revision recorded in the artifact is
`c0518a464e4ce8c02513d0bc93f63385ef53d26f`.

The campaign artifact is
[`execution-fuzz-evidence-40148240864`](https://github.com/szTheory/nesturbator/actions/runs/40148240864/artifacts/12580614764),
ID `12580614764`, 354,132 bytes, SHA-256
`d7ff6f7a93c3b33143c62c85315cde76cdf2ddf309c190d63896abd85debaf7b`. Its controls are under
`controls/fixtures/minimize/`; campaign run records and copied corpora are at the artifact root.

The corrected matched replay ran on the same source head. Its complete instrumented input overlay
SHA-256 is `3df856b8ca4c89dc5a81047026f3557493464bda1eef867a1d6e2dfd7549f05a`. Full per-target
metrics, corpus hashes and exported coverage hashes are in
[`execution-measurements.md`](../../../tests/fuzz/execution-measurements.md), under “Review-fix
replay and WR-05 tick correction.”

Local `cmake --workflow --preset ci` completed with 669/669 tests and produced the library, runner
and libretro ZIP packages. This included the `fuzz.state.jam_oracle`,
`fuzz.state.jam_oracle_divergence` and `fuzz.execution.campaign_controls` tests. The long local
`retroarch.session.controls` test passed in 301.12 seconds.

## Fixed Issues

### WR-01: Loader-target minimization validates inputs as execution recipes

**RED — review reproduction:** The original review found that `validate_minimized` sent loader
inputs through the NEXE execution measurement path, so accepted ROM-loader and state-loader
inputs could not be validated as their own targets. See the original finding at
[`18-REVIEW.md`](18-REVIEW.md#wr-01-loader-target-minimization-validates-inputs-as-execution-recipes).

**GREEN — exact Linux controls:** Acceptance validation now dispatches by target and runs each
original and reduction through that target's `--accept-only` path. In the exact-source Nightly
controls, all three intended synthetic failures were accepted by their corresponding target and
replayed through the matching instrumented libFuzzer binary. The result records show:

| Target | Acceptance | Observed class and operation | Original → reduced bytes |
| --- | --- | --- | ---: |
| ROM loader | accepted | AddressSanitizer at `nesturbator_load_cartridge` | 131088 → 131088 |
| State loader | accepted | AddressSanitizer at `nesturbator_load_state` | 3 → 2 |
| Execution | accepted | AddressSanitizer at `nesturbator_load_cartridge` | 23 → 17 |

ROM-loader minimization found no smaller input; the original and reduction have the same SHA-256,
`327effc4c95863847b86119c23314f30b87db431b586b3ec46a2849efb69c3cb`. The state-loader and
execution controls retained the injected failure while reducing their synthetic input sizes.

### WR-02: Execution minimization cannot verify a reproduced failure

**RED — review reproduction:** The original validator compared caller-supplied class strings and
a sequential operation count, then required successful execution measurement. It did not derive
the failure from the instrumented binary or identify which named API call immediately preceded
an abort. This was recorded in the original review finding at
[`18-REVIEW.md`](18-REVIEW.md#wr-02-execution-minimization-cannot-verify-a-reproduced-failure).

**GREEN — exact Linux controls:** The fixed path performs target acceptance as a separate check,
then runs original and reduced inputs in disposable processes using the target's same instrumented
fuzz binary. It derives sanitizer/timeout/resource/oracle class from process output and records
named public API markers written before each call. The positive controls above observed the same
`address-sanitizer` class and named operation on both inputs. The same hosted control run rejected
each invalid claim: a wrong target produced no failure, the wrong `undefined-behavior` class was
rejected, `nesturbator_destroy` did not match the observed `nesturbator_load_cartridge` operation,
and an unaccepted execution input was rejected before failure acceptance. The fixture and replay
logs are retained in the artifact.

### WR-03: Evidence self-test writes directly to the expected artifact paths

**RED — exact hosted failure:** Nightly run
[40147344115](https://github.com/szTheory/nesturbator/actions/runs/40147344115) completed its
campaign, then failed the workflow's post-campaign self-test. The old workflow asserted
`fixtures/crash-expected` and `fixtures/timeout-expected`, while the self-test reused the campaign
evidence root. The exact job log ended with “Process completed with exit code
1”; its artifact is ID `12581023804`, SHA-256
`e6440be36c4e05d1aa4ab14112749afd5f9f9c0114721d583a4d3f0ac3672d98`.

**GREEN — exact collector path:** The self-test now runs under a separate `self-test/` output
directory. Its synthetic crash and timeout children write into the real campaign `artifact_dir`,
then the shared `collect_reproducers` function copies them to the normal reproducer directory. On
the 78bb run, the two source artifacts and two uploaded reproducer files all had the exact
triggering-input SHA-256 `b7141fe73c546dd006b4969657d0fb0cce4c79b0ca759230f5e3697df7b693b5`.
The control also rejected a missing artifact directory as incomplete evidence. The Nightly job
verified both separate self-test output paths and passed.

### WR-04: State-loader oracle hashes frame outputs after JAM

**RED — destructive control:** The old oracle hashed frame and audio outputs after `STOP_JAM`,
although those fields are unspecified and the buffers are shared static storage. In the local
control, making the oracle inspect those buffers after JAM caused the positive JAM control to
fail; omitting canonical state equality caused the deliberate divergence control to fail its
expected rejection.

**GREEN — actual continuation oracle:** The repaired `nt_run` hashes the stop status and exits
without reading frame metadata or video/audio buffers on JAM. The continuation path saves and
compares the canonical post-run states. `fuzz.state.jam_oracle` passed with equivalent JAM states
and different poisoned output buffers. `fuzz.state.jam_oracle_divergence` also passed its negative
control: it gives both runs identical output-buffer poison, deliberately resets one instance after
JAM, and confirms the actual canonical-state comparison rejects the divergent continuation. Both
CTest results are part of the 669/669 local CI run.

### WR-05: State-loader guest-tick metric sums absolute timestamps

**RED — retained historical observation:** The earlier matched replay recorded
`guest_ticks=25,733,664` by summing cumulative timestamps, including the restored origin more than
once. That value is preserved in the measurement history but is superseded and is not valid guest
work.

**GREEN — corrected matched replay:** The metric now reads the accepted canonical saved state's
little-endian tick origin at offset 40, initializes the per-call baseline from it, and adds only
`frame.ticks - previous_ticks` for successful frames. JAM contributes no output ticks. The
78bb replay measured 17,155,728 guest ticks over 24 successful state-loader frames; 12 JAM stops
contributed zero. Execution replay remained at 20,728,248 guest ticks and retained its 24-function
increment in `src/`. The old value and new source-bound result are both retained in
[`execution-measurements.md`](../../../tests/fuzz/execution-measurements.md).

## Exact Linux Campaign

The 78bb-head campaign used the existing measured budgets without changes. Its source corpus
manifest was `f524ddcbf03dea6faa115938df004cbfe08eae8aaac9921c3b4b39ec9bb95ac1` for 60 ROM, 17
state and 8 execution descriptors. All three before/after corpus integrity checks passed.

| Target | Configured limits: runs / time / sampled RSS / outer | Actual runs | Stop cause | Elapsed | Peak RSS | Findings |
| --- | --- | ---: | --- | ---: | ---: | ---: |
| ROM loader | 100,000 / 10 s / 768 MiB / 30 s | 100,270 | run-count cap | 2.11 s | 524,369,920 B | 0 |
| State loader | 1,000 / 15 s / 512 MiB / 30 s | 755 | max-total-time | 16.10 s | 380,121,088 B | 0 |
| Execution | 2,000 / 15 s / 256 MiB / 30 s | 2,000 | run-count cap | 10.30 s | 128,348,160 B | 0 |

All three completed with process result 0 and `failure_class=none`. The run reports sampled
libFuzzer RSS limits; they are not instantaneous allocation quotas. All failure-class and
reproducer controls used labeled synthetic inputs, and no synthetic result is reported as a real
campaign finding.

## Exact-Source Hosted Verification

Pull-request CI run [40148240830](https://github.com/szTheory/nesturbator/actions/runs/40148240830)
tested PR head `78bb8d84bd6b579b21c8283684cfa8593055c724`; GitHub executed the generated merge
revision `c0518a464e4ce8c02513d0bc93f63385ef53d26f` against base `e47feb3afc34baf1d611ef44153af199424f41a2`.
All 15 CI jobs passed: ASan, Linux x64/ARM64, macOS x64/ARM64, Windows x64/ARM64, both no-FP
lanes, performance, hygiene, Conventional Commit title, hosted RetroArch E2E and the six-platform
hash-equality gate.

The six platform hash manifests were byte-identical with SHA-256
`718946884e27fd589ba243a6d22296d1ac8d8e3c416d14541dcd34ebb5eb0fa1`. The Linux x64 AccuracyCoin
artifact [12582281034](https://github.com/szTheory/nesturbator/actions/runs/40148240830/artifacts/12582281034)
was generated by the actual full campaign on the exact PR source. It completed all 22 pages and
146 scored rows: 101 pass, 42 fail, 2 not-run, 1 timeout, plus 5 inventory-only draw rows. The
campaign report SHA-256 remained the canonical `6d0a811960f6964e7fc66802c6e4ade0c4ffb8772c40d88d50cf79e1f8463652`;
the protected scoreboard SHA-256 stayed `9e96015ca205f0ed2a351c21d7b6b0a718b26e8e6da87b84f2542adadd04279a`.
The campaign reported complete infrastructure and trustworthy observation, not a clean conformance
result; its existing failures and timeout remain visible. `tests/accuracy/baseline-metadata.txt`
now records this actual campaign's source revision (`c0518a464e4c`), clean-source bit and elapsed
time, while preserving the canonical report digest, scoreboard digest and outcome counts.

The exact-source Cachegrind comparison passed all three ROMs. `nesteroids` measured 23,743,914
base to 23,743,918 head Ir/frame, `runner` 24,651,894 to 24,651,898, and `M4_P256K_C256K`
23,319,733 to 23,319,737. Each increase was 4 Ir/frame (+0.00%); the job used Valgrind 3.22.0
and GCC 14.2.0. The evidence is in the `perf` job of the linked CI run.

Hosted RetroArch E2E in the same run passed pinned qualification, exact progression, and three
production-verdict negative controls. The marker evidence is artifact
[12581357116](https://github.com/szTheory/nesturbator/actions/runs/40148240830/artifacts/12581357116),
SHA-256 `0cf97ecea9d2225bb367b04e97a3b4ef052935d96ae8b69e6e78073e3967cabf`. The host's frame 60
matched the runner, both mapper-1 and MMC3 battery-save round trips matched at frames 400 and 600,
and deterministic state lanes matched at frames 120, 121, 150, 179 and 180 after state save/load.
The E2E frame and state evidence is artifact
[12581966495](https://github.com/szTheory/nesturbator/actions/runs/40148240830/artifacts/12581966495).

Nightly run [40148240864](https://github.com/szTheory/nesturbator/actions/runs/40148240864) also
passed the bounded Linux campaign, target-specific minimization controls, collector controls,
state-fuzz, ROM-loader-fuzz, MMC3 oracle and full vector lane. The three-pass randomized suite
also completed successfully, and the workflow run concluded with `success`. Its evidence and
precise limits are recorded above.

## Remaining Verification and Scope

The fixes close the five code-review warnings only. They do not establish universal reliability,
do not change campaign budgets, and do not complete requirements ROB-01 through ROB-04. Any
phase-level verification must use the recorded evidence and make its own requirement decisions.
