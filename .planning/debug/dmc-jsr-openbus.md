---
status: resolved
trigger: "Approved bounded Plan 21-04 DMC/open-bus prerequisite repair"
created: 2026-10-29
updated: 2026-10-29T11:57:26Z
---

## Current Focus

hypothesis: Write-delayed DMC halts must retry on the next CPU cycle; this exposes a separate forced controller D6 defect in newly reached implied-read open-bus execution.
test: Independent one-hunk reverts and final reapplication, complete fixed-pin campaign and adjacent core/state tests.
expecting: Reverting retry restores raw03 JAM; reverting controller retention restores guest reset; applying both retains all114 protected PASS and all11 target PASS.
next_action: none; Plan21-04 completed the coherent pins, corpus, protected-main comparison, and full CI closeout.
reasoning_checkpoint:
  hypothesis: A DMC halt delayed by a write wrongly waits for its original GET/PUT phase instead of retrying the next CPU cycle, shifting CPU progress relative to the free-running DMC before JSR4013.
  confirming_evidence:
    - Original-source canonical E has eligible pending reload writes at cycles2381138,2382866,2391938,2400146,2401010,2402738, all PUT0, before the final C6 divergence.
    - New owned GET/PUT and one/two/three-write regression fails because the first following read completes without DMA; expected DMA sample5A is absent.
  falsification_test: Correct retry behavior fails to move canonical E onto its expected opcode-fetch halt, or loses protected PASS keys.
  fix_rationale: Retarget the already-persistent pending attempt phase to the next CPU cycle only when a write blocked an eligible attempt; preserve initial load/reload scheduling and actual GET alignment.
  blind_spots: Exact physical paired-chip tie precedence remains unmeasured; the final campaign, protected-key comparison, and full local CI gates pass.
  candidate_causes: [code/write-delayed retry, code/output edge, data/unreachable fixture phase, environment/stale runner]
  and_gate: yes; wrong retry can alter later parity while a separate request-edge defect remains.
adjacent_reasoning_checkpoint:
  hypothesis: Forced controller D6 changes JSR4015 operands from source-required0021 to4061; the unintended RTI corrupts the implied-read test stack and subsequently resets the guest.
  confirming_evidence:
    - Fresh retry trace records JSR4015 then opcodeRTI at4061 instead of source-described0021; PLP iteration1B returns with corrupted stack and PLA iteration1C enters unrelated APU register test.
    - New 32-case two-port upper-bit regression fails exactly when retainedD6 is0; decoder unconditionally forcesD6=1.
  falsification_test: Correcting only retainedD6 leaves the canonical page15 reset or breaks a protected PASS key.
  fix_rationale: All three upper controller bits are undriven in the existing standard-controller model; preserve their previous values without changing serial clocking or adding devices.
  blind_spots: Exact physical paired-chip tie precedence remains unmeasured; final canonical output and acceptance gates are terminal.
  candidate_causes: [code/forced controllerD6, code/4015 external-latch update, data/test stack choreography]
  and_gate: yes; write-retry correction exposes this separate open-bus defect that the old earlier failure hid.
bug_class: Bohrbug
candidate_causes: [code/request-to-halt phase, code/timer-write ordering, data/fixture phase history, environment/stale runner]
and_gate: yes; a timing repair can expose an independent timer-write boundary, and an unreachable fixture can hide canonical failure.

## Symptoms

expected: Pinned test E completes JSR then DMC stalls the target opcode read; $4013 returns BCC $90. All 114 protected PASS keys remain PASS, all eleven v5 rows pass and all 146 scored rows are terminal.
actual: The pre-repair candidate returned RTI $40 at $4013 and jammed. The final source-bound campaign completes with 119 pass / 27 fail; all eleven target rows, including both overlap rows and latency, pass.
errors: None in the final campaign or 691-test local CI workflow. The earlier latency raw03/frame3 and unstarted overlap rows are historical pre-repair evidence.
reproduction: Cold pin6bee919/cap1800 AccuracyCoin page12; protected page14 and page15 controls.
started: Existing prerequisite discovered during Plan21-04 shared-entry measurement.

## Eliminated

- hypothesis: Retain DMC GET data over resumed mapped CPU read.
  evidence: Independent mapped read must replace GET $5A with resumed ROM $EA; original $40 follows that ordinary bus rule.
- hypothesis: Pending-at-read-opening gate is the causal minimum repair.
  evidence: It loses two protected PASS keys and does not repair canonical E. Removed while retaining shared-entry sampling changes.
- hypothesis: Change global APU phase or $4010 write order to repair the protected timer-window test.
  evidence: Original passing-key trace shows iteration3 write before output reload and iteration4 after; no such changes are needed by the actual retry repair.

## Evidence

- checked: Source audit 21-DMC-PHASE-AUDIT.md.
  found: Cold bus/APU phases start equal and toggle together; the owned JSR fixture explicitly makes them opposite. Current source has post-clock halt qualification and a pending-at-opening guard.
  implication: Owned green cannot prove canonical timing; fresh source-bound observations required.
- checked: Fresh build/dmc-repair baseline, observations in build/dmc-repair-evidence/baseline.
  found: Removing only pending-at-opening guard restores Delta Modulation Channel and DMC Reload Timing PASS; E remains raw03/frame3.
  implication: The experimental guard loses protected behavior and is not retained.
- checked: Fresh diagnostic runner and canonical page12, all-write-attempt-trace identity/sources/logs retained.
  found: Six eligible pending PUT writes occur in the timed prefix; current implementation does not track failed attempts. C6 request occurs at post-toggle PUT and GET is parked atC544.
  implication: Test documented failed-halt retry before altering global phase or timer reload order.
- checked: Pinned protected timer-window trace, original-trace/page-15.
  found: Iteration3 write precedes the next output edge in this model, while iteration4 output uses the old27 before decode. Both agree with the protected answer key.
  implication: Earlier same-internal-cycle $4010 hazard is not causal in the passing baseline; do not change ordering without additional evidence.
- checked: Owned one/two/three-write retries on both initial GET/PUT phases.
  found: Red before write-retry fix; green after. The first read must halt even when its phase is opposite the initial attempt; data alignment and resumed mapped read remain checked.
  implication: Existing dma_halt_phase can represent the next attempt; no additional persistent state or format change required.
- checked: Canonical page15 with retry-only repair.
  found: The instruction timing row finishes; implied-read test advances beyond its original0E failure. Source-required JSR4015 target0021 instead becomes4061 because controllerD6 is forcedhigh; unintendedRTI corrupts the stack and guest later resets.
  implication: Adjacent one-line controller upper-bit retention is required to preserve protected downstream rows; owner approved this exact bounded correction.
- checked: Independent retry-only revert with controller fix retained.
  found: core.apu fails; canonical page12 returns latency03/frame3 and both overlap rows unstarted. See retry-only-revert identity and report.
  implication: The retry correction, rather than controller behavior or a moved timer, causes the page12 repair.
- checked: Independent controller-only revert with retry correction retained.
  found: core.controller fails; canonical page15 resets and erases results, leaving all seven rows unstarted at cap1800. See controller-only-revert identity and report.
  implication: Upper-bit retention is independently necessary for protected downstream results.
- checked: Final reapplied complete campaign and exact-key comparison.
  found: 119PASS/27FAIL,146terminal/146,5DRAW,all114protectedPASS retained,all11targetPASS. New passes are DMA+$4015Read, ImpliedDummyReads, latency and both overlaps.
  implication: Bounded behavioral acceptance is green; executor still owns coherent pins/corpus/fullCI and causal commit.

## Resolution

root_cause: A blocked DMC halt waited for its original phase instead of retrying the next CPU cycle, shifting JSR4013 relative to the output timer. Correct retry exposed forced-high controllerD6, which corrupted newly reached JSR4015 open-bus operands and reset the page15 guest.
fix: Advance the pending attempt phase after an eligible CPU write; retain controllerD5-D7. Restore original read qualification; correct private fixtures to reachable equal phases and real C1 halt. Preserve timer/register order, shared entry, state layout and mapped-read overwrite semantics.
oracle_type: specified (pinned hardware-test assembly and documented DMA retry/GET cadence)
verification:
  target_test: { result: pass, suite: core.apu and core.controller and canonical pages12/15 }
  mutation_check: { result: skipped, reason_if_skipped: No Stryker configured for owned C; exact independent fix-hunk reverts were exercised instead }
  no_op_deletion: { result: pass, deletion_justified_by_rca: true, detail: Removed failed experimental pending-opening gate; added hardware retry and retained floating bit; no assertions removed }
  adjacent_tests: { result: pass, suites_run: 42-test core/state/cpu/bus/fuzz-state and campaign selection, elapsed_seconds: 173.81, log: build/dmc-repair-evidence/final-focused.log }
  revert_and_reconfirm: { result: pass, bug_returned_on_revert: true, fixed_on_reapply: true }
  guardrail_verdict: accepted
files_changed: [src/bus.c, src/internal.h, tests/core/test_apu.c, tests/core/test_controller.c, tests/core/test_irq_poll.c, tests/core/test_interrupt_entry.c, README.md, include/nesturbator.h]

## Final closeout verification

- Full campaign: 119 PASS / 27 FAIL, 146/146 terminal, five DRAW, all 114 protected starting PASS keys retained, and all eleven v5 target rows pass.
- Fresh protected-main authority: `a64c23fd3ca7caed7e3ad102ff1a9beaa29fa562`, 114 projected PASS keys.
- Full workflow: `cmake --workflow --preset ci` passed 691/691 in 358.64 seconds and produced library, runner, and libretro packages.
- Corpus and runner pins: all 17 generated state entries and ROM manifest rows match; `runner.write_hashes` and its content comparison pass.
- Final report and identity evidence: `build/dmc-repair/accuracycoin/full/`, `build/dmc-repair-evidence/final-campaign/`, and `build/dmc-repair-evidence/final-focused.log`.
- Plan21-04 retains its one coherent causal source commit boundary. Hosted cross-platform acceptance remains pending later Phase21 plans.
