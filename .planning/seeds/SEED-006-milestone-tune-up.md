---
id: SEED-006
status: dormant
planted: 2026-10-02
planted_during: preparation, before milestone 1
trigger_when: every milestone after the first
scope: small
audit_acknowledged:
  milestone: v1
  at: 2026-10-09
  status: dormant
---

# SEED-006: Each milestone starts with a faster CI run, no flaky test, and docs that match the release

## Why This Matters

CI time, test reliability and documentation drift a little with every phase. One short phase at the start of each milestone removes what built up, so the next phases run on a fast, trustworthy pipeline and a newcomer reading the README gets the current release.

## When to Surface

**Trigger:** every milestone after the first. Keep this seed after it is used; it applies again next time.

## Scope Estimate

**Small** — one short phase whose results are merged changes: the slowest CI job is faster; a test that failed without a code change is fixed or removed; action pins, runner images and the AccuracyCoin pin are current; the README and the public header's comments describe the code as released; `.planning/seeds/` reflects what is next.

## Breadcrumbs

- `.planning/preparation/ENGINEERING.md` section 5: CI jobs, timeouts, caches and pins.
- `.planning/preparation/ENGINEERING.md` section 8: where the sibling repositories disagree on CI practice.
- `.planning/preparation/CONFORMANCE.md`: how often AccuracyCoin changes, and why a pin move is its own change.
- `.planning/preparation/DECISIONS.md`: the open questions at its end.

## Notes

The seeds are this project's rolling roadmap: near term is the current milestone in `.planning/ROADMAP.md`, the next milestones are SEED-001 to SEED-004, and SEED-005 is the long term.
