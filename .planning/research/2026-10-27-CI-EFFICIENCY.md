---
title: "CI/CD efficiency review"
summary: "Measured low-risk reductions in duplicate work, with test, security, provenance and maintenance tradeoffs."
updated: 2026-10-27
scope: "Investigation and isolated proposal; live workflows and the Phase 20 candidate remain unchanged."
---

# Recommendation

Start with reusing the CTest-verified hash file, canceling superseded PR Nightly runs, and aligning required evidence retention. These use existing CMake/GitHub facilities and add no dependency. Narrow specialized build targets in a second measured batch. Treat metadata-trigger routing and release-run reuse as separate changes because they touch merge protection and artifact trust.

The review covers developer feedback, product determinism, C/C++ build behavior, host-timing tests, failure diagnosis, release provenance, runner capacity, security and maintenance. UI/rendering-design advice is irrelevant to this pipeline; the emulator's rendering correctness is a test guarantee to preserve.

A concrete four-change proposal is in [CI-EFFICIENCY-LOW-RISK.patch](CI-EFFICIENCY-LOW-RISK.patch). It applies cleanly and passed isolated static/policy checks. It is **not applied or pushed**. Collect Phase 20's already-running source-bound evidence before changing its workflow inputs.

# Evidence and limits

Read-only GitHub API collection sampled the latest **40 CI, 40 Nightly and 20 Release runs**, plus one targeted same-commit release pair. Scratch inputs and pilot logs are under `/tmp/nesturbator-ci-efficiency/`. Histories include failures and cancellations; current snapshots are partial where jobs were still running. Older runs had a different topology, with session controls embedded in build lanes, so their durations are not pooled into a forecast for the new topology.

Times below are `completed_at - started_at` from job/step metadata or reported CTest timings. Aggregate job-seconds are occupied elapsed work across parallel runners, **not wall-clock savings, CPU profiling or billed minutes**. Skipped/placeholder timestamps are excluded from comparisons. Compiler graph counts describe work removed, not proportional runtime savings.

| Observation | Exact evidence | Meaning |
|---|---|---|
| Duplicate hash generation | CI [40507506429](https://github.com/szTheory/nesturbator/actions/runs/40507506429): six second-generation steps took 30, 32, 49, 28, 31 and 48 s | **218 elapsed runner-seconds** repeated per six-platform run; up to the affected critical lane's 48–49 s can disappear, rather than 218 s of wall time |
| Metadata restart | Same-source CI [40513949984](https://github.com/szTheory/nesturbator/actions/runs/40513949984) was replaced after the PR body edit; its jobs consumed **1,107 s / 18.45 job-minutes** | Every `edited` event currently restarts the expensive verification group |
| Required proof expiry | Perf artifact **12738188820**, 313,713 bytes, expires 2026-11-03; perf retention 7 d versus marker evidence 60 d | Complete proof currently has a practical seven-day retention ceiling |
| Full vectors | Nightly [40513950000](https://github.com/szTheory/nesturbator/actions/runs/40513950000), job **122222842595**: CTest 76.29 s, fetch 72.13 s, sample-match 0.76 s, all 256 full-op runs about 3.4 s | More CTest parallelism offers only a few seconds here |
| Tool setup | Successful perf samples: median about 12 s, observed range 9–27 s; current setup 11 s, build 15 s, measurement 131 s | Tool caching/custom images are not the first lever; canceled long installs are not counted as successful timing tails |
| Artifact upload | Recent archive/evidence upload steps mostly 1–2 s | Recompressing ZIPs is avoidable but not a headline bottleneck |
| Release duplication | Release [39993108038](https://github.com/szTheory/nesturbator/actions/runs/39993108038) and main CI [39993108056](https://github.com/szTheory/nesturbator/actions/runs/39993108056) both tested `e47feb3afc34baf1d611ef44153af199424f41a2`; ordinary CI consumed **5,238 s / 87.3 job-minutes** | Large per-release redundant work, with a larger provenance migration required to remove it |
| Serial host tests | Frozen-source local CI: session label 277.51 of 347.33 test seconds; main controls 252.26 s | A future profiling target, not evidence that timeout-sensitive tests can safely run concurrently |

The replacement Phase 20 CI [40514060876](https://github.com/szTheory/nesturbator/actions/runs/40514060876) completed successfully during this review, including the performance gate. Nightly [40513950000](https://github.com/szTheory/nesturbator/actions/runs/40513950000) subsequently completed successfully too, on the same candidate and attempt 1. This review does not close plan 20-10 or any requirement.

Standard hosted runners for public repositories are currently free, so the established benefits here are feedback time, available concurrency and reduced maintenance—not a claimed dollar saving. Larger or self-hosted runners have different economics. [GitHub billing](https://docs.github.com/en/actions/concepts/billing-and-usage).

# First batch: small, concrete changes

## 1. Reuse the current-run hash inventory

**Choose:** replace CI's second `write_hashes.cmake` execution at `.github/workflows/ci.yml:159` with:

```yaml
- run: cmake -E copy "build/${{ matrix.preset }}/tests/hashes.txt" hashes.txt
```

`runner.write_hashes` already creates that file and `runner.write_hashes.content` compares it to the pins (`tests/CMakeLists.txt:678–691`). The later workflow command runs the same generator with a different output path. Reuse preserves the artifact path, six-platform key set, equality check and existing source identity.

**Tradeoff:** this relies on those fixtures remaining in the selected CTest inventory and a fresh build tree. The current pipeline does not restore a build tree. Never fall back to committed pins or an older artifact when the file is missing. Revisit freshness explicitly if build-output caching is introduced.

**Proof before adoption:** from an empty build directory, run the normal workflow and upload the copied output; compare complete bytes/keys against the original artifact. Omit the generator in a negative experiment and require copy/upload failure. A corrupt generated inventory must still fail the CTest content check. The isolated local byte-copy, missing-file and corruption controls passed.

**Why first:** it removes measured emulator work without reducing a correctness check, adding infrastructure or changing platform coverage.

## 2. Cancel superseded PR Nightly runs

**Choose:** coalesce first-attempt PR runs by workflow event and PR number; reruns and non-PR groups use their individual run IDs. Enable cancellation only for first-attempt `pull_request` runs. The proposal uses:

```yaml
concurrency:
  group: nightly-${{ github.event_name }}-${{ github.run_attempt == '1' && github.event.pull_request.number || github.run_id }}
  cancel-in-progress: ${{ github.event_name == 'pull_request' && github.run_attempt == '1' }}
```

Nightly currently has no concurrency group. A newer PR source makes an old PR's full-vector/fuzz/session proof unusable for current-source acceptance. Scheduled, protected-main and explicitly dispatched runs keep independent groups. The rolling issue reporter already excludes PR events. [GitHub concurrency semantics](https://docs.github.com/en/actions/how-tos/write-workflows/choose-when-workflows-run/control-workflow-concurrency).

**Tradeoff:** superseded partial diagnostics can be lost. Preserve deliberate manual diagnostics through `workflow_dispatch` or isolated rerun groups. GitHub reruns retain their original event/ref/SHA: the attempt guard keeps an older PR rerun from canceling a current-source first attempt. Run-ID groups still serialize repeated attempts of the same run. [GitHub rerun semantics](https://docs.github.com/en/actions/how-tos/manage-workflow-runs/re-run-workflows-and-jobs). Cancellation does not imply immediate zero resource use; the observed CI restart incurred cleanup/queue delay.

**Proof before adoption:** on a test PR, synchronize twice and observe only the older PR run being canceled; show separate PRs, non-PR events and an older-PR rerun versus current-source verification remain independent. Test the run-attempt guard with actual rerun metadata. Preserve artifact/run/attempt selection and reject canceled evidence. Static workflow validation passed; actual hosted cancellation remains an implementation acceptance check.

## 3. Retain the complete proof for a coherent horizon

**Choose:** raise performance retention from 7 to **60 days**, matching required marker evidence. Keep every required artifact available for at least that interval. The hosted collector needs perf, six hash/protected bundles and the other configured evidence together (`tests/cmake/phase20_hosted_evidence.cmake:440–445`).

**Tradeoff:** more artifact storage. The observed perf artifact is about 314 kB; no storage-cost forecast is made. This is a reliability/avoided-rerun improvement, not faster execution of the current run. Changing retention does not rescue already expired artifacts.

**Proof before adoption:** inspect actual new artifact `expires_at`, collect a complete source-bound bundle, and retain missing/expired evidence refusal. Keep current immutable artifact identities and the repository's allowed retention settings. [Pinned upload action retention input](https://raw.githubusercontent.com/actions/upload-artifact/cf430e030ddbb5b0abf93d22962f4752f3646cd9/action.yml).

## 4. Avoid compressing already compressed packages again

**Choose:** `compression-level: 0` only for the upload of `packages/*.zip`. Leave text logs, reports and raw Cachegrind output compressed. The pinned action supports this directly. [Pinned action schema](https://raw.githubusercontent.com/actions/upload-artifact/cf430e030ddbb5b0abf93d22962f4752f3646cd9/action.yml).

**Tradeoff:** possible larger outer archive, with a small observed timing upside because current uploads take only 1–2 s. Preserve archive names, grouping, extracted ZIP bytes and the exact 18-package release set. Do not substitute `archive: false`: this pinned version restricts that mode to one file and ignores the configured artifact name.

# Second batch: prune build breadth before adding caches

Use native `buildPresets.targets` for label-selected special lanes. The existing CMake 3.25 minimum supports this; keep `cmake --workflow --preset ...` as the entry point and leave selected test labels/inventories unchanged. [CMake 3.25 build presets](https://cmake.org/cmake/help/v3.25/manual/cmake-presets.7.html#build-preset).

| Lane | Proposed target closure | Local compile commands / total Ninja commands | Acceptance |
|---|---|---|---|
| Current all-target build | All | 144 / 218 | Reference graph |
| `nofp` | `nesturbator` | 19 / 20 | All five ABI tests; negative float fixture still builds itself |
| `mmc3-oracle` | `mmc3-oracle` | 20 / 22 | All 13 tests, authenticated cold fetch, complete JUnit/inventory |
| `vectors-full` | `cpu.vectors`, `vecconv` | 6 / 8 | All 258 tests / 2,560,000 vectors, cold fetch and sample-match |
| Real-host job | `nesturbator_libretro`, `nesturbator-run`, `compare_frame`, `frame_index`, `holymapperel-derived` | 37 / 46 | Full real-host path; keep explicit `retroarch.marker.generate` |

These counts are from fresh local configured graphs, not a promise of matching percentage speedups. Current vector time is predominantly network fetch; the current MMC3 workflow is already short. Measure absolute build/queue/test time on required hosted platforms after pruning.

**Pilot evidence:** minimal vector targets built and a representative committed opcode test passed. The minimal nofp configuration ran all five ABI tests, with four passing and a symbol check failing on local AppleClang 21 / CMake 4.4.3. An all-target control in the same Release configuration produced the **same failure and identical library SHA-256**. Missing build closure is ruled out; a separate local helper-symbol/toolchain investigation remains. No full nofp success is claimed. Required Linux GCC 14 minimal-target pilots, full cold-vector/MMC3 runs and the complete real-host path remain necessary before adoption.

The recent missing-runner failure is the relevant footgun: executable closure and CTest-generated fixtures are different dependencies. Retain fresh-tree positive checks rather than trusting a warm workstation build.

# Larger decisions: valuable, separate migrations

## Separate metadata validation from source computation

The description update in the preceding delivery step caused the measured restart. **Operationally, finish title/body before pushing source and keep subsequent run updates in local records until verification finishes.** That works immediately, but pipeline routing should eventually handle metadata edits efficiently.

**Choose later:** a small event/title entry workflow with a reusable computation boundary. Independently require a uniquely named live-title check using existing owned shell/`gh` logic, read-only permission and no checkout. Run computation for source updates, opening/reopening and base-retarget edits. Body/title-only edits must neither cancel computation nor emit a successful/skipped check named `CI required`.

**Why not a one-line `edited` removal:** later invalid titles would escape the current required rollup, and base retargets arrive as edited events too. The repository uses loose required checks. GitHub treats skipped jobs as successful for required-check purposes, so simply skipping expensive jobs while emitting the same rollup can make failed source appear acceptable. Distinct check contexts and coordinated ruleset/collector/policy changes are essential. [Required checks](https://docs.github.com/en/repositories/configuring-branches-and-merges-in-your-repository/managing-protected-branches/about-protected-branches#require-status-checks-before-merging), [edited payload schema](https://raw.githubusercontent.com/octokit/webhooks/main/payload-schemas/api.github.com/pull_request/edited.schema.json).

**Alternative:** shared job predicates plus distinct metadata rollup and concurrency groups keep existing code-job names but spread subtle conditions across the DAG. Prefer the reusable boundary when scheduling this change; do not add a workflow framework/dependency.

**Adversarial acceptance:** body edit during green/in-progress/red computation; invalid title then correction; base retarget with unchanged head; concurrent source/title events; stale PR snapshots; metadata-only success refused by the hosted collector; unchanged release `workflow_call` behavior. Nightly currently uses default PR activity types and therefore does not refresh on a base-retarget edit. The redesign must refresh applicable **CI and Nightly** evidence against the new base/merge snapshot and reject the earlier Nightly bundle, without enabling wasteful body edits. Job-name/ruleset migration and source/protected-main consumers must move together.

## Consolidate exact-main release verification

**Choose later:** one authoritative full run for the exact protected-main release commit, with publishing consuming that immutable run's verified artifacts. Do not reuse PR results: PR merge and squash/main commits have different identities.

Current ordinary main CI and release's nested CI can both run on the same SHA. Removing that duplication has a much larger per-release work upside than tuning uploads, but the current release gate is structurally strong: publish depends on successful exact-commit CI.

**Required replacement:** validate repository, workflow identity, push/main event, exact commit/tag SHA, run attempt, required jobs and exact artifact set; bind publishing to the chosen run ID, preserve 18-archive inventory, attestations and tamper refusal, and audit the protected-main AccuracyCoin baseline consumer. Never accept “latest successful,” same-named fork workflows, arbitrary PR artifacts or a mixture of two runs. Keep App credentials limited to protected-main release work. [GitHub secure-use guidance](https://docs.github.com/en/actions/reference/security/secure-use).

**Tradeoff:** queue/polling and API failure modes can erase some latency gains and expand the trusted handoff. Retain the existing release dependency until the replacement passes adversarial provenance checks. This is not in the small proposal.

# Deliberately hold or reject

| Option | Decision / reason | Evidence that would reopen it |
|---|---|---|
| ccache or another compiler cache | Hold. Existing policy measured 4–22% compile share and requires at least 40%; first remove unneeded targets. No new cache action/dependency now. | Fresh comparable cold/warm cohorts show compile dominating and trustworthy cache keys across compiler/preset/runner/source inputs |
| Cached raw full vectors | Reject for this batch. Cold fetching is an explicit tested obligation; 72 s fetch time does not authorize removing it. | A separate approved evidence design preserving a cold canary; converted-data caching already has a roughly 10-minute threshold, currently unmet |
| More vector parallelism | Low payoff: about 3.4 s of full-op execution versus 72.13 s fetch | Execution becomes a material portion of the lane |
| Remove `RUN_SERIAL` or replace it with resource locks | Reject as an easy win. Deadline-sensitive controls need isolation from unrelated CPU load; a resource lock excludes only other holders. Dedicated hosted session jobs already exist. | A measured isolation-preserving redesign with all timing/ownership/signal assertions intact; [CTest RUN_SERIAL](https://cmake.org/cmake/help/v3.25/prop_test/RUN_SERIAL.html), [RESOURCE_LOCK](https://cmake.org/cmake/help/v3.25/prop_test/RESOURCE_LOCK.html) |
| Reduce six-platform coverage or enable matrix fail-fast | Keep current coverage and fail-fast=false. Cross-platform identity is a product claim; early cancellation loses useful GCC/MSVC/architecture diagnostics and can cause more repair cycles. | Explicit product scope change or evidence that queueing/diagnostic economics justify a different gate |
| Broad new preflight barrier | Hold. It delays every green matrix; hygiene/title do not predict all compiler failures. Preserve the existing current-run vector-inventory authority barrier. | Failure-rate and critical-path data show a narrowly defined early check wins overall |
| Custom tool image, self-hosted public-PR runners or third-party APT cache | Hold/reject here. Current successful perf setup median is about 12 s; added patching, trust and persistence are poor trades. | Large repeatable installation cost and an owned, isolated image lifecycle; [secure runner guidance](https://docs.github.com/en/actions/reference/security/secure-use) |
| APT `--no-install-recommends` | Small later experiment only; inspect actual package closure and preserve Valgrind's real dependencies. Canceled long setup outliers are not the normal baseline. | Package simulation and cold job measurements show worthwhile savings without changing perf controls |
| Remove install/package or negative checks | Reject. Local installed-consumer check was about 0.15 s; these protect the three deliverables and fail-closed evidence | Remove only a demonstrated duplicate whose guarantee remains exercised |
| Refactor the serial clock/transport harness | Profile first. Interpreter startup is plausible, but intentional waits and ownership lineage are correctness contracts. Reuse owned/platform facilities if justified. | Journals/counters separate process startup from deliberate waits and preserve monotonic deadlines, first failure, cleanup and signal behavior |
| Add a CI monitoring service/orchestrator dependency | No demonstrated need. Existing APIs, CMake and small owned analysis scripts answered this review. | Existing tools cannot answer a specific recurring operational need simply |

# Proposal verification and rollout

The four-change patch passed `git apply --check`, actionlint's workflow/expression validation, the production `ci_required_rollup.cmake` controls, Nightly workflow policy, action-pin checks (33 keys), timeout-comment checks (19 entries), and local hash copy/missing/corruption controls. A final adversarial pass added an attempt guard for older PR reruns and explicit Nightly base-retarget acceptance to the later metadata redesign. These are **local proposal checks**, not a hosted six-platform run of the patch. Existing informational ShellCheck issues elsewhere are not claimed fixed; syntax validation used `-shellcheck=''`.

Roll out after Phase 20 collection: apply the small proposal on a follow-up branch, run the normal required CI/Nightly and preserve their identities, compare per-step output/timing against the recorded baseline, and test supersession behavior on that PR. Revert individual independent changes if identity, inventory, cancellation or evidence-lifetime checks regress. Then pilot the specialized-target batch. Stop adding machinery once measured savings become seconds with disproportionate trust or maintenance cost.

Live workflow files, required checks, thresholds, pins, the phase candidate and the user's existing config edit were unchanged during this investigation. Research artifacts stay local so recording this review does not restart CI.
