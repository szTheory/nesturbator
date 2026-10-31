# Phase24 iteration and CI efficiency

## Decision and delivered changes

Continue required local engineering under task approval; ask only for a
materially new consequence or an explicit boundary. Standing preferences
now preserve that authority across sessions. The earlier repeated prompts
were a workflow error, not evidence that long-running tests need permission
by default. Specific stops, limits and no-retry instructions still apply.
[Authority lesson](24-APPROVAL-LESSONS.md).

Add `cmake --workflow --preset dev-check` for immediate feedback while
editing. It configures Debug, builds18 exact executable targets and runs20
exact owned/source checks with the existing four-worker policy. It excludes
campaigns, host sessions, network suites and the full state-fork matrix;
existing CI presets and all acceptance gates remain unchanged. No dependency
or cache was added. [Independent review](24-CI-EFFICIENCY-REVIEW.md).

Fix the sprite fixture to advance its bus/APU/PPU clocks together with a real
read. Its old timestamps selected an inconsistent DMA phase. Expected DMA
durations and copied-byte assertions are unchanged. This removes a stale
fixture failure without weakening the oracle.

## Measurements

| Observation | Measured time / result | Meaning |
|---|---|---|
| One authorized local full workflow | 444.60s; exit8 | Complete diagnostic evidence, not passing acceptance |
| CTest within that workflow | 441.98s | Dominates this warm local run |
| Session controls | 253.71s | Largest single test, deliberately serial |
| Session readiness + source-root | 32.26s | Further serial host checks |
| Complete22-page DMA campaign | 4.97s | Campaign cost is small on this machine/run |
| Full workflow outside CTest | about2.62s | Warm configure/build; no demonstrated compiler-cache win |
| First dev-check worker CTest | 2.57s | Focused tier only |
| Independent fresh-tree pilot | configure0.854s, build0.834s, CTest2.720s | All20 selected checks pass; macOS local observation |
| Independent warm CTest pilot | 0.09s | Warm measurement, not a clean-build forecast |

The fresh-tree pilot used a new ignored build directory, the preset's actual
target list and regex, and four workers. This rules out stale selected
binaries in the initial inventory. Keep target and test-name lists aligned
when updating the preset. Details/logs are under
`build/phase24-owned/dev-check-clean.{json,log}`.

## Next optimizations and tradeoffs

1. Measure configure, build and tests separately on representative hosted
   runs and normal incremental edits. Prioritize actual critical-path or
   duplicate-work reductions. Local session cost does not establish hosted
   matrix cost; hosted CI already separates sessions from build lanes.
2. Consider narrower build targets in specialized lanes, after inventorying
   every consumer and fixture. Missing target/test alignment can silently
   run a stale executable; a cold build is a useful pilot.
3. Investigate PR metadata-only rebuilds only with required-check and
   concurrency semantics preserved. Retargeting a PR changes its merge
   candidate. Skipped required checks and stale artifacts are real hazards.
4. Keep session serialization and timing isolation. More CTest workers can
   add contention without shortening the serial path. Existing four-worker
   owned checks already complete quickly. Do not increase jobs by default.
5. Defer compiler/external-data caching until measurement shows a worthwhile
   gain and freshness/provenance remain covered. Keep cold external-data
   checks. Smaller, flatter owned tooling is preferable to new dependencies.

The previously delivered redundant release-push skip already exists; do not
reimplement it. No hosted workflow, required-check policy, performance limit,
full-CI selection or external campaign baseline changed in this work.
Architecture/DevOps, test/reproducibility and adversarial authority reviews
converged on these recommendations; further unrelated role fan-out would not
resolve the measured bottlenecks.

## Supporting reviews

- [Architecture and CI tradeoffs](24-CI-EFFICIENCY-ARCH.md)
- [Test selection and reproducibility](24-CI-EFFICIENCY-TESTS.md)
- [Adversarial preset review](24-CI-EFFICIENCY-REVIEW.md)
- [Actual full-workflow result](24-CI-RUN.md)

The reviews link CMake and GitHub primary documentation for fixture closure,
preset semantics, required checks and concurrency. Their evidence supports
these decisions; no generic ecosystem recommendation replaces local timing.
