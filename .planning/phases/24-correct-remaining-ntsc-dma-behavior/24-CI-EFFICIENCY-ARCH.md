# CI efficiency and approval friction review

## Recommendation

The current pipeline already removes several obvious sources of waste: the six build lanes share this run's vector registration inventory, the long AccuracyCoin campaign runs on Linux x64 only, and the generated Release Please main push skips direct CI while the Release caller keeps exact-commit validation. The most promising next implementation is to narrow builds in specialized jobs. Separating PR metadata checks from source computation has greater potential for repeated-run savings, but it needs a deliberate required-check migration. Do not add compiler caches, broad parallelism, or fewer acceptance lanes based on the evidence here.

No CI workflow currently requires a human approval gate. The main ruleset requires zero approving reviews, and Release auto-merges after its required check. Its GitHub App token is minted only on protected-main pushes and passed only to release creation and auto-merge. Keep that least-privilege arrangement; broadening the token or adding a bypass actor would not remove a demonstrated prompt.

## Prioritized recommendations

### 1. Pilot target-limited builds in specialized lanes — safe candidate, hosted evidence needed

**Repository evidence.** `CMakePresets.json` has `nofp`, `vectors-full`, and `mmc3-oracle` build presets without target restrictions. The workflows run their full workflows (`.github/workflows/ci.yml:283-306` and `.github/workflows/nightly.yml:3-11`). The RetroArch job separately configures and builds every target before it runs one marker-generation test and the host checks (`.github/workflows/ci.yml:347-351`). The normal six-platform `build` matrix packages all three deliverables and should stay full (`.github/workflows/ci.yml:90-190`).

The prior local build-graph investigation in [CI efficiency research](../../research/2026-10-27-CI-EFFICIENCY.md#second-batch-prune-build-breadth-before-adding-caches) counted 144 compile commands and 218 total Ninja commands for an all-target build. Its proposed target closures counted 19 compile commands for `nofp`, 6 for `vectors-full`, 20 for `mmc3-oracle`, and 37 for the real-host job. These are graph-work counts from that earlier checkout, not hosted time savings or a current-source guarantee.

**Smallest implementation.** Use native CMake `buildPresets.targets` and point each specialized workflow build step at that preset while retaining `cmake --workflow --preset ...`. CMake 3.25 supports target lists in build presets; the repository minimum is already 3.25. The RetroArch target closure previously identified is `nesturbator_libretro`, `nesturbator-run`, `compare_frame`, `frame_index`, and `holymapperel-derived`, with `retroarch.marker.generate` retained explicitly. This adds no dependency or custom wrapper.

**Expected benefit and uncertainty.** It removes unnecessary compile/link work from special lanes; measured reduction in wall time is unknown. The RetroArch job is a clear duplicate full build, but moving its outputs to another job could add an artifact dependency and put the build matrix on the host-test critical path. Prefer a target-limited local build in the existing job unless a run graph proves cross-job reuse improves total completion time.

**Risks and invariants.** A fresh build must still produce every selected CTest executable and generated fixture. Preserve every selected label, cold vector/oracle fetch and digest check, complete inventory/JUnit evidence, the full six-platform matrix, and the real RetroArch progression checks. An earlier local `nofp` pilot failed the Apple toolchain symbol check identically with minimal and all-target builds; it did not establish Linux hosted success. The current-source Linux GCC 14 target pilot and complete lane evidence are still required before adoption.

### 2. Split PR metadata validation from source computation — high payoff, structural change

**Repository evidence.** CI subscribes to `pull_request.edited` (`.github/workflows/ci.yml:9-12`) and cancels the active run for any new PR event (`.github/workflows/ci.yml:28-35`). Its title check queries the live title (`.github/workflows/ci.yml:58-88`), while the same run fans out to six platform builds, sanitizer/ABI lanes, session checks, and RetroArch. The prior read-only sample recorded 1,107 runner-seconds spent by a same-source run that was replaced after a PR body edit ([CI efficiency research](../../research/2026-10-27-CI-EFFICIENCY.md#evidence-and-limits)).

**Smallest safe design.** Keep the live-title query and run it for title edits, but route body/title-only events away from source computation. Keep source computation for open, reopen, synchronize, and base-retarget edits. Give the title check and source computation distinct required-check identities and migrate `.github/rulesets/main.json`, `ci-required`, policy tests, and any evidence collectors together. A small owned `gh`/shell check and reusable workflow are sufficient; no workflow framework is needed.

**Expected benefit and uncertainty.** It can eliminate a full matrix restart for metadata-only edits. Realized savings depend on how often metadata changes occur during or after CI; the single measured replacement demonstrates the mechanism, not a recurring rate.

**Risks and invariants.** Do not simply remove `edited` or let the current `CI required` rollup go green when compute jobs are skipped. GitHub reports conditionally skipped jobs as successful even when their contexts are required ([GitHub status-check documentation](https://docs.github.com/en/pull-requests/reference/status-checks)); this repository's rollup currently rejects every skipped job except non-PR performance checks (`.github/workflows/ci.yml:608-632`). Base retargets change the merge candidate and must refresh both applicable CI and Nightly evidence. Test invalid-title correction, body edits during green/running/failed CI, base retargets, concurrency, stale artifacts, and the Release reusable caller before changing the ruleset. GitHub documents that reusable workflows inherit the caller's `github` context ([reusable workflow documentation](https://docs.github.com/en/actions/reference/workflows-and-actions/reusing-workflow-configurations)).

Until that migration is ready, batch title/body edits before pushing source changes. No approval-rule change is needed.

### 3. Align performance-evidence retention with the evidence window — low runtime impact

**Repository evidence.** `phase20-perf-*` is retained for 7 days (`.github/workflows/ci.yml:600-606`), while the hosted RetroArch marker artifact is retained for 60 days (`.github/workflows/ci.yml:379-385`). The prior investigation found the perf artifact was about 314 KB and that a Phase 20 evidence collector needed multiple source-bound artifacts together.

**Smallest implementation.** If current evidence consumers still need to collect the same run bundle beyond one week, change only the perf artifact's `retention-days` to 60, matching the marker evidence. Keep run ID, attempt, SHA, and artifact naming unchanged.

**Expected benefit and uncertainty.** This does not shorten a workflow run. It reduces the chance that an otherwise valid source-bound result must be reconstructed after its performance artifact expires. The storage increase is likely small for the sampled artifact size, but no current repository storage-cost data was inspected.

**Risks and invariants.** Verify the active collector's actual retention requirement before changing it. Do not accept artifacts from another run or attempt, or weaken missing/expired-artifact refusal. Retention cannot recover artifacts that have already expired.

### 4. Keep the generated-release skip and observe its natural production event — delivered, one observation outstanding

**Repository evidence.** Quick task [261031-fq6](../../quick/261031-fq6-skip-the-redundant-main-push-ci-run-for-/261031-fq6-SUMMARY.md) added the caller-aware guard to each direct CI job. The live guard skips only a `push` to `main`, in direct workflow `CI`, whose commit message starts `chore(main): release `; the `Release` reusable caller still runs on that exact commit (`.github/workflows/ci.yml:38-47`, `.github/workflows/release.yml:58-70`). Its policy self-test protects that distinction.

The task's hosted PR and ordinary main-push runs passed, but the exact generated Release Please main-push skip event was not observed. The correct next step is passive verification at the next natural release: confirm direct CI skips, Release CI validates and uploads the exact commit's artifacts, and publication remains gated on it. Do not add a second release-run consolidation mechanism before this behavior is observed. The prior duplicate-run sample is historical and does not establish current savings after the delivered guard.

## Guardrails for follow-on work

- Keep the six platform builds and `fail-fast: false`; cross-platform deterministic hashes and compiler coverage are product claims.
- Keep the fresh current-run vector inventory barrier and same-run artifact provenance. The build jobs intentionally download the inventory created from this checkout (`.github/workflows/ci.yml:135-145`, `193-211`).
- Keep full cold fetches for pinned external test data. Do not cache the full-vector or MMC3 sources, or treat a warm cache as fetch evidence.
- Preserve PR cancellation for genuinely superseded source. If metadata routing changes concurrency, keep scheduled, manual, release, and current-source rerun groups distinct. GitHub allows conditional cancellation by event/expression, but concurrency groups can replace pending runs as well as cancel in-progress ones ([concurrency documentation](https://docs.github.com/en/actions/how-tos/write-workflows/choose-when-workflows-run/control-workflow-concurrency)).
- Do not put a new broad preflight job in front of the matrix: most checks currently fan out, with only the source-bound vector registration as a build prerequisite. A new serial barrier can increase the green-path critical time.
- Do not add a compiler cache yet. The prior measured compile share was 4–22%, below the project's 40% threshold; narrow targets first and remeasure.

## Evidence limits

This is a read-only architecture review plus this document. I did not run CMake, CTest, a workflow, a campaign, or a ROM. Historical timing and graph evidence is explicitly labeled as such; no hosted timing forecast is claimed. Links and paths are repository-relative and contain no personal machine details.
