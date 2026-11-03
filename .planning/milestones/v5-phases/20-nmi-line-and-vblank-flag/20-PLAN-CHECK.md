# Phase 20 Plan Check

**Status:** VERIFICATION PASSED
**Plans checked:** 20-01 through 20-06
**Reviewed:** 2026-10-26

## Goal and coverage

The Phase 20 goal is to make `$2002` reads and `$2000` NMI changes around
VBlank produce the console's flag and NMI behavior, with race history that
survives save/load, rewind, and run-ahead. Roadmap requirements NMI-01,
NMI-02, KEEP-02, and KEEP-03 are each claimed in executable plan frontmatter
and have concrete tasks.

| Requirement | Plans | Coverage |
|-------------|-------|----------|
| NMI-01 | 20-01, 20-02, 20-03, 20-04, 20-06 | Split status samples, complete tables, six target rows, and race-state proof |
| NMI-02 | 20-01, 20-02, 20-03, 20-04, 20-06 | Enable/disable paths, STA/INC marker order, end-of-VBlank cases, and exact target gate |
| KEEP-02 | 20-02, 20-03, 20-05, 20-06 | Causal hash pins and reasons, six-lane evidence obligations, and protected PASS checks |
| KEEP-03 | 20-02 through 20-06 | Format 2/revision 8, rejection atomicity, exact race cutpoints, and core/runner/libretro replay |

Plan 20-01 remains complete. Plan 20-02 remains 0/3 with no SUMMARY; plans
20-02 through 20-06 remain pending in the original six-wave order.

## D-10 amendment and accuracy obligations

The amended D-10 is implemented consistently. The plans use the reproduced
six-case finite-margin circuit results as constraints, distinguish internal
phi2 from widened M2, and separate projected phase-close behavior from
real-bus and eventual guest-handler evidence. The unresolved equal-timestamp
case must use one documented global model policy with its hardware limit; no
plan claims an A/C hardware winner or requires the exhausted candidate sweep.
The old checkpoint is marked `superseded`, and the research addendum states
which earlier stop language is historical.

The amendment does not weaken the acceptance contract: all six complete
source sequences must be executed (not merely comparator-self-checked),
default alignment is measured first, all six final target rows must pass,
all 108 initial PASS keys remain protected, and format 2/revision 8,
state rejection atomicity, causal hash explanations, full CI, and required
hosted proof remain mandatory. Plan 04 explicitly leaves KEEP-03 incomplete
if the required edge-before-poll state cannot be represented. Plan 06 keeps
hosted proof pending and prohibits a full phase-completion claim until it is
obtained. The missing candidate source is explicitly called out; no A/C
classification is inferred from aggregate pages.

The research responsibility map matches the planned PPU/bus, CPU, state,
runner/campaign, and host-test tiers. Deferred Phase 21 IRQ/hijack, DMA,
regional, and public stepping/debug work is excluded. The retained die-input
driver/trace is identified as research evidence; Node, network access, and
external die data are not CI or implementation dependencies.

## Structure and gates

`verify.plan-structure` returned valid for all six plans. Every task has
files, action, verify, and done fields. Plans contain 2–3 tasks each; task
file lists are at most five files. Dependencies form the declared sequential
chain 20-01 → 20-02 → 20-03 → 20-04 → 20-05 → 20-06, with matching waves and
no same-wave plan pairs. Plan 20-02's parser warnings about one-way tasks are
covered by locked D-14 in CONTEXT.md, which explicitly approves the version
change without another checkpoint.

The failure-direction probe reports 24/24 automated commands `ok`, with no
blocker or warning. The path probe reports all 24 CMake commands as
`not_applicable`; that is not positive path-resolution evidence, so this
check does not claim those commands were path-verified. No plan `<automated>`
commands or application tests were run.

The project has recorded execution actuals for Phases 17–19, and the completed
20-01 summary records 10,624 tokens and 32 minutes. The estimate checker still
reports low confidence, `sample_count: 0`, and no applied calibration; this
means these plan estimates are not calibrated, not that the project lacks
actuals. They are all below its 100,000-token budget:

| Plan | Tasks | Files | Estimated tokens | Budget share | Estimate confidence |
|------|-------|-------|------------------|--------------|---------------------|
| 20-01 | 2 | 5 | 44,000 | 44% | low |
| 20-02 | 3 | 15 | 78,000 | 78% | low |
| 20-03 | 3 | 15 | 62,000 | 62% | low |
| 20-04 | 3 | 5 | 38,000 | 38% | low |
| 20-05 | 3 | 11 | 56,000 | 56% | low |
| 20-06 | 3 | 12 | 32,000 | 32% | low |

### Accepted scope advisories

Plans 20-02 and 20-03 each retain their truthful 15-file manifests. CONTEXT.md
explicitly treats the generic file-count threshold as advisory for these
atomic D-20/D-23 causal changes: production behavior, state/public contract,
tests, documentation, measured pins, campaign metadata, and compatibility
fixtures must agree before one green change. Each is divided into three
sequential tasks; no required path or acceptance check was removed. CONTEXT.md
also keeps the 11-file and 12-file counts for Plans 20-05/20-06 visible as
planning advisories. These accepted scope risks do not conceal a technical
gap or exempt any runnable verification failure.

## Resumption status

`.planning/STATE.md` now says `planning`, records Plan 20-02 at 0/3 with
20-01 complete, and directs the next action to `$gsd-execute-phase 20` after
this check. It no longer requires the superseded exact-tie evidence or blocks
dependent plans. The checkpoint remains superseded; no Plan 20-02 SUMMARY or
implementation completion is implied.

## Recommendation

The plans pass the revision gate and are safe to resume. Run
`$gsd-execute-phase 20`; execution starts with Plan 20-02 Task 1. The accepted
scope advisories do not waive the six target rows, complete source sequences,
108 protected PASS keys, format 2/revision 8, state replay, causal hash
reasons, full CI, or hosted proof. No plan `<automated>` command, application
test, or hosted check was run during this review.
