---
phase: 16-host-evidence-and-roadmap
plan: "21"
subsystem: testing
tags: [posix, retroarch, cleanup, regression]
requires:
  - phase: 16-15
    provides: Correct actual-driver fixture identity
provides:
  - Failed cleanup request writes preserve owned teardown and original status
  - Causal actual-helper RED and eight-case integrated GREEN regression
  - Current committed-source full workflow and three checked local packages
  - Independent clean six-path source review with digest-bound disposition
affects: [16-07, 16-08]
tech-stack:
  added: []
  patterns: [Guarded best-effort cleanup, source-bound case evidence]
key-files:
  created:
    - .planning/phases/16-host-evidence-and-roadmap/16-21-SOURCE-ACCEPTANCE.md
    - .planning/phases/16-host-evidence-and-roadmap/16-21-REVIEW.md
    - .planning/phases/16-host-evidence-and-roadmap/16-21-REVIEW-DISPOSITION.md
  modified:
    - tests/retroarch/state_session.sh
    - tests/retroarch/test_state_session.sh
    - README.md
    - include/nesturbator.h
key-decisions:
  - Guard cleanup request materialization locally without disabling errexit globally
  - Reuse the existing complete QUIT negative and preserve the separate complete positive
  - Preserve every invalid development run and require one coherent same-harness proof
requirements-completed: []
requirements-covered: [EVID-01, EVID-02, EVID-04]
coverage:
  - id: cleanup-write-teardown
    description: Failed cleanup writes preserve original reason/status and reap owned resources
    requirement: EVID-02
    verification:
      - kind: integration
        ref: retroarch.session.controls and round5 causal RED/GREEN
        status: pass
    human_judgment: false
  - id: current-source-acceptance
    description: Current committed candidate passes full CI and local archive checks
    verification:
      - kind: integration
        ref: cmake --workflow --preset ci and check_archives
        status: pass
    human_judgment: false
  - id: current-source-review
    description: Independent six-path source review has no actionable findings
    verification:
      - kind: other
        ref: 16-21-REVIEW.md and 16-21-REVIEW-DISPOSITION.md
        status: pass
    human_judgment: false
completed: 2026-10-16
status: complete
parent-gates: passed
---

# Phase16 Plan21: Owned cleanup after request-write failure

**A failed one-shot cleanup QUIT write now records zero sends and reaches owned host/child reaping and identity-checked endpoint release while preserving the first failure and exit status.**

## Accomplishments

- The actual reviewed helper first failed causally; the same frozen harness then passed all8repaired cases and7fresh endpoint reacquisitions.
- Normal CI runs the same regression inside the existing649-test registration. The expensive complete QUIT negative is reused, with the complete positive retained.
- Current committed-source configure/build/test/package passed649/649/no skips/jobs4 and three fresh actual local ZIP checks. Independent standard-depth six-path review reported0findings.

## Task Commits

| Task | Commit | Result |
|---|---|---|
|1: paired source/test/README/header repair|38e14fbc0104038a7eb9ad17d16499c7924c2b9b|Causal RED and all8GREEN pass|
|1: accepted source proof|2ede710|Current ten-path source/candidate identity|
|2: complete normal workflow and packages|5f3704a|649tests,4stages,3fresh checkedZIPs|
|3: independent source review/disposition|2544b55|Clean/open0 and complete source/raw/digest binding|

## Verification

Selected root: build/phase16-acceptance21-development/round-5. RED actual wait1/3166ms established READINESS_REPLY after the consumed request became a directory, with owned host/lock surviving before emergency cleanup. GREEN actual wait0/105367ms passed EXIT,INT,TERM,HUP,writable cleanup,already-QUIT,foreign endpoint and foreign-owner replacement. Its reused complete primary took58461ms and all other matrix time46906ms. The local batch took696455ms below3600000ms. Outer wait and stream hashes were finalized after exit using consistent clock accounting.

The one formal900sworkflow completed in293.375720seconds, CTest291.56seconds:649unique outcomes,611ordinary/38registered-regex forms including2WILL_FAIL, jobs4, zero skips and configure/build/test/package. All three freshly generated local library/runner/libretro ZIPs passed inventory,CRC,digest,freshness and existing archive checks. The new matrix also passed inside normalCI in107999ms.

The independent typed reviewer inspected the exact six-source snapshotb071e63ee0e11c8b944e7f0d8b426867f227dda3, source-equivalent to candidate38e14fb. Report SHA2562eb2369ab458597ee7aea6875121a27145463faa528861c61e44ca0e270c935d binds an open0 disposition. The exact Task3 source/full/review/raw/invocation/case-binding/config verification passed after review.

## Parent verification

The separately accounted parent build passed in 0.125235 seconds. Its mandatory full-workflow repeat passed in 298.625100 seconds: 649/649 unique tests, zero skips, jobs4 and configure/build/test/package, followed by the existing three-archive checker. Source-before/after matches all ten candidate blobs, the protected config is exact, and the review digest is unchanged. Actual argv, process IDs, consumed/completed ledgers, timings, streams, CTest report and registration validation are retained in build/phase16-acceptance21-development/round-5/parent-gates. The observed parent snapshot is 11fc2bf8cec6ba864240e76d3bd7be85378ef760.

## Deviations and preserved failures

The initial5s/60stest envelopes were infeasible for the required complete-path control. A checked amendment reused the existing complete negative and assigned distinct finite test budgets without changing production/native criteria or900s registration. The historical53.2s measurement was a reference, not a universal minimum.

Two initial fixture runs and four development rounds remain retained invalid or unaccepted for their exact setup/finalization defects. No unrelated failure counted as causal proof. A checked fifth changed-source round produced coherent proof; no unchanged retry or renewed formal/native slot was used. Corrections included request-path identity, foreign signal observation, fail-closed ledgers, consistent monotonic clocks and actual post-wait stream finalization. A final read-only consumer amendment recognizes only the exact frozen-harness executable input links while retaining strict raw-output alias checks.

The complete QUIT control's actual journal spans the existing30soperation deadline: the baseline exports STATE_SESSION_QUIT_GRACE_MS while the helper reads STATE_SESSION_TEST_QUIT_GRACE_MS. This existing behavior was preserved, not represented as an effective300msgrace.

## Remaining gates and requirement scope

All three tasks, mandatory parent build/full-workflow repeat and enabled source-review gate passed. Post-wave tracking is reconciled at9/11 live plans complete;07 is the next dependency-ready plan and08 still depends on07. The global requirement status is unchanged, so requirements-completed is empty: this plan contributes source evidence rather than claiming EVID-04 publication or full native acceptance.

Plan07 still owns partialPR50reconciliation, fresh native exact-final-head requiredCI/perf/pinned-host evidence, affirmative exact-head owner consent and normal release publication. Plan08 owns actual18released archives/checksums/attestations, six metadata copies and exact-release CI. No07/08execution, native observation, consent request, publication, phase completion or another lifecycle command occurred here. Historical native timing causes remain UNKNOWN. Plans10–14/16–20 remain superseded incomplete without SUMMARY/credit; their evidence is preserved. Protected config remains exact and excluded.

## Self-Check: PASSED

All three local task gates, actual source/README/header pairing, current candidate/full/package identity and fresh independent review binding passed. Key artifacts exist and all10source/config identities match. Mandatory local parent gates also passed; native and release gates remain pending.
