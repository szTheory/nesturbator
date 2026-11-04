# Phase 24 automation and checkpoint disposition

Recorded 2026-11-01 in response to the owner's direction to automate useful
verification, reserve human UAT for necessary owner input, and retain the result
so the same administrative checkpoint does not recur. The standing project
[verification policy](../../VERIFICATION-POLICY.md) applies going forward.

## Superseding decision

The owner instruction supersedes Plan 24-05's blanket human-review method for
the three inherited Plan 24-04 prohibition labels. It changes the verification
method, not DMA requirements, evidence thresholds, compatibility policy, runtime
limits or action authority. Original plans and the historical unresolved table
remain history; this disposition governs current and equivalent future reviews.
The owner is not recorded as passing a test, attesting to prior approvals or
accepting unknown risk.

| Former human prompt | Disposition | Evidence and limit |
|---|---|---|
| Confirm stopped-campaign authority | Preserve the operational guard automatically; no owner acknowledgment required | Only bounded local checks and record updates occur in this verification. No hosted scheduling, campaign restart, push, merge or release. Historical run identity is distinct from permission: API success and a request's `remote_action_authorized=false` are not approval records. Undocumented historical authority is not retroactively certified. A necessary new action without authority still stops. |
| Confirm oracle/target claims | Completed bounded agent review | README, header and DMA ledger distinguish pinned compatibility, inferred early-G ordering and hardware evidence. Matching hashes establish determinism, and implicit `0x09` is a pinned accepted variant. No universal silicon claim is made. Semantic source/document review is identified as agent review; string presence is not hardware proof. |
| Confirm native compatibility claims | Existing native checks plus source/document review | Fresh `core.state_reject --short` checks atomic old/future-format refusal on owned NROM/MMC1/MMC3 fixtures. Fresh `core.state_fork --dma` checks reachable silent/pending/collision/latch/abort continuation and negative controls. `state.format_pin`, `core.state_fork.entry` and `docs.reproduction` pass. Header/README say no converter and recorded-only behavior revision. Loader source checks exact format and does not use behavior revision as a compatibility gate. No wider host-container or cross-build qualification is inferred. |

No necessary human question remains. UAT records an agent/evidence review,
not a human pass. The earlier checkpoint is explicitly superseded rather than
silently approved. Reopen for changed implementation, claims, evidence or a
necessary new owner decision; a stale historical label alone is insufficient.

## Checks and CI placement

CTest selection was inspected as JSON; selected checks have no campaign setup
fixture. Existing limits were retained. Successful commands:

```sh
ctest --test-dir build/ci -R '^(docs\.reproduction|state\.format_pin|core\.state_fork\.entry)$' --no-tests=error --output-on-failure --output-log build/phase24-automation-checks.log
build/ci/tests/core.state_reject --short tests/roms/dabg.nes tests/roms/runner.nes tests/roms/hm/M4_P128K_CR8K.nes
build/ci/tests/core.state_fork --dma
```

The initial CTest selection passed in 4.40 seconds; the two native executables
returned exit 0 together in under one second. The final check after documentation
pointers passed in 4.48 seconds and is retained in `build/phase24-automation-final.log`. No emulation, format, output
baseline, existing test selection or workflow changed.

Reusable behavior checks already run in CI. The authority rule is operational,
and claim wording needs bounded semantic review. Another permanent syntax-only
test would duplicate weak evidence. No new CI suite, dependency, cache,
concurrency setting or hosted run is justified. Existing hosted acceptance stays
bound to candidate d54e218; documentation follow-through is not a new hosted run.

## Read-only CI/CD efficiency review

Reviewed retained exact-candidate CI 41336227793 and Nightly 41336227847 job/step
metadata and workflow dependencies. These are elapsed durations, not billing
estimates; runner rates and overlap differ.

| Measured work | Elapsed | Assessment |
|---|---|---|
| CI macOS Intel build | 591 s; workflow step 556 s | Longest CI job duration; build/tests dominate rather than the small metadata gates. |
| CI Windows x64 build | 569 s; workflow step 512 s | Another major matrix duration; required platform coverage remains. |
| CI macOS session-controls | 458 s; CTest 443 s | Distinct host-bound coverage; core tests cannot replace it. |
| Nightly suite-flake | 1,376 s; repetitions 1,350 s | Dominant Nightly job duration. Three random-order repetitions intentionally detect ordering/flakiness failures. Removing them reduces signal. |
| Nightly session-controls | 659 s; CTest 629 s | Separate runtime observations; matching labels alone do not establish duplicate coverage. |
| CI hash equality | 8 s | Required downstream gate; negligible next to builds. |

The CI graph waits for matrix outputs before hash equality and aggregate status.
Longest job duration indicates a bottleneck; exact queue/critical-path attribution
requires the timestamp graph rather than summing durations. No billed-cost
saving is claimed.

The concrete saving here is replacing manual signoff and a potential full replay
with existing bounded checks and unchanged-source receipt evidence. Existing
generated-release main-push deduplication already preserves reusable Release CI
and publication gates; its event-observation limitation remains in quick task
261031-fq6. No additional safe workflow edit is supported by this run. Future
speed work should profile slow matrix test steps and repeated Nightly setup,
preserving all unique repetition/platform signal. No optimization phase or new
run was started.
