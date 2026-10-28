---
created: 2026-10-26T17:32:52.434Z
title: Evaluate scoped nightly mutation testing
area: testing
severity: minor
scope: proposed
files:
  - .github/workflows/nightly.yml:1
  - tests/CMakeLists.txt:1
  - .planning/preparation/ENGINEERING.md:53
---

## Problem

The 2026-10-26 findings proposed mutation testing as a later way to identify
assertions that pass despite wrong behavior. Existing owned negative controls
and synthetic mutations provide targeted evidence; a broader automated pilot
could identify additional weak spots. No general mutation-testing tool or new
nightly lane has been selected or implemented.

## Solution

Later proposed scope for future prioritization. Evaluate Mull as a candidate
released CI-only binary, following the existing approach for external analysis
tools. Check its current C/compiler compatibility and operating cost when this
proposal is promoted. Keep emulation and test implementations owned, and keep
the core's dependency boundary unchanged.

Pilot a bounded, non-blocking nightly run on a small changed-code scope, with a
defined comparison base and selected CTests. Retain a reproducible report of
mutants killed, surviving, equivalent or unexecuted, plus tool identity and
timings. Use surviving mutants to propose specific assertions. Measure useful
findings and runtime before expanding the scope or proposing a merge gate.

Source: `ci-efficiency-findings-2026-10-26` project memory. Severity minor confirmed
in the capture session. This proposal does not add requirements to Phase 20 or
the committed v5 scope.
