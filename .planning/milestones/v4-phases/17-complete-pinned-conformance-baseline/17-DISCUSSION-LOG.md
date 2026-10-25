# Phase 17: Complete pinned conformance baseline - Discussion Log

> Audit trail only. Not input to planning, research or execution agents. Decisions are in `17-CONTEXT.md`.

**Date:** 2026-10-23
**Areas discussed:** Pin/inventory migration; bounded execution; reports/regression gates; CI cost/evidence.

The user requested broad relevant perspectives, primary-source research, parallel analysis and an adversarial synthesis, with recommendations followed automatically. All areas were selected under that delegation; no questionnaire or additional confirmation was necessary. Three `gsd-advisor-researcher` agents used the configured `gpt-6-luna`/`xhigh` routing and cross-reviewed the combined policy. No emulator source was used; upstream AccuracyCoin MIT test source/docs were inspected. No implementation or new-ROM campaign ran.

## Pin, inventory and protected-key migration

| Option | Benefit | Cost/risk | Selected |
|---|---|---|---|
| Bump ROM/hash alone | Small diff | Misses compressed names, changed test meaning and protection migration | |
| Atomic pin/decoder/inventory/migration | Reproducible complete baseline with readable keys | Needs bounded decoding and reviewed semantic comparison | Yes |
| New opaque/page-ordinal ID registry | Can separate labels from identity | Adds schema/migration burden; ordinals drift too | |

**Delegated selection:** Atomic migration with pin-scoped decoded-name keys, retaining navigation coordinates as metadata. All 17 labels occur once at both pins; that proves label continuity only. APU routine changes need a semantic disposition. Source tables contain 149 old entries (144 scored/five DRAW) and 151 new entries (146/five) over 22 pages. F0–F4 compression affects both row names and a suite title. The new digest was streamed, not stored as an extra ROM; it must be revalidated during implementation. D-01–D-04 capture the result.

## Bounded execution and classification

| Option | Benefit | Cost/risk | Selected |
|---|---|---|---|
| One run-all attempt | Least navigation/boot cost | First hang hides later pages | |
| Independent cold page attempts | Broad bounded reach after a page hangs | A hung row can leave that page's tail not-run | Yes |
| Cold individual attempts everywhere | Maximum isolation | Repeated boot cost; prerequisites need qualification | Conditional fallback |

**Delegated selection:** All-page attempts first; individual attempts only when useful and qualified. Source confirms header/row A controls, plus boot-time pretests. Never patch the guest or fabricate observed execution. Guest-frame timeouts and outer host kills are different evidence. `$FF` must precede running-bit decoding; valid upper pass variants remain passing. D-05–D-08 capture the result.

## Reports and regression gates

| Option | Benefit | Cost/risk | Selected |
|---|---|---|---|
| Reuse strict all-pass page verdict for the full suite | Existing path | Conflicts with approved honest-baseline scope | |
| Pin-bound text observations plus separate protection gate | Clear data and failure semantics; small owned surface | Needs explicit metadata/migration and status validation | Yes |
| Dashboard/framework and candidate-only reset | More presentation or less migration effort | Unneeded dependency surface or silent loss of protection | |

**Delegated selection:** Five status counts sum to 146; DRAW remains separate. Retain raw diagnostics and compact committed comparison data. Complete valid observation can succeed despite known nonpasses; malformed/incomplete infrastructure evidence and lost protected passes fail. Reuse protected-main authority and ordinary reviewed transition data. The local fallback has six rows versus 17 in the candidate scoreboard; refresh it. An advisor's initial duplicate-row suspicion came from concatenated files, was checked against each file and withdrawn; no duplicate defect is asserted. D-09–D-12 capture the result.

## CI cost and evidence

| Option | Benefit | Cost/risk | Selected |
|---|---|---|---|
| Full run in every platform lane | Broad full-suite comparison | Multiplied unmeasured cost | Only if justified |
| One full required PR run plus existing portable gates | Early detection with limited duplication | Full outcomes initially measured on one lane | Preferred if measured fit |
| Initial full campaign plus recurring nightly observation | Avoids costly PR critical path | Delayed detection for newly measured keys | If PR cost rules it out |

**Delegated selection:** Measure before placement; normal CI always checks integrity and actual protected passes. An actual initial complete bounded report is required whichever recurring tier is chosen. Retain sanitizer/parity/perf coverage and focused semantic negative controls; add no generic validation framework. D-13–D-15 capture the result.

## Adversarial synthesis

- Same names can conceal changed upstream assertions: require source-backed dispositions, never cross-pin total comparison.
- A clean report file does not prove guest progress: verify target/start and preserve raw observations, active timeout and untouched tail separately.
- All-pass reporting conflates product accuracy with measurement correctness: separate observations, infrastructure verdict and protected regression gate.
- Candidate data can erase prior passes: retain protected-main provenance and explicit transition policy; test missing-baseline and lost-key cases.
- Exact raw equality can reject a documented hardware variant: keep raw differences visible while applying the pinned protocol.
- Count equality alone misses substitutions or DRAW scoring: validate the complete inventory and status/denominator invariants.
- Unlimited fallback/diagnostic machinery can consume the phase: use finite measured attempts, existing tools and ordinary PR review.

## Agent discretion and remaining measurements

Exact CLI/report fields, helper splits, run caps, evidence retention and CI tier are bounded implementation choices. New-ROM project decoding, controller qualification, semantic migration and campaign timing/results remain untested here. Three independent reports and one cross-review round converged; further generic ecosystem research had diminishing returns.

Primary evidence links are retained in `17-CONTEXT.md`: exact old/new AccuracyCoin READMEs/assembly/licence and comparison. Local evidence was `runner/main.c`, both scoreboard files and parser, ROM manifest, CTest registrations and CI baseline-fetch code. Existing prior contexts and preparation references were carried forward.

## Deferred Ideas

Named hardware fixes follow the baseline; execution/lifecycle fuzzing remains Phase 18. Regional/new-board work, leaderboard submission and unrelated vectors-full debt remain outside this discussion. No future requirement is marked complete.
