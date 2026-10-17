# Plan 22 lock amendment: verification passed

Current independent verdict: zero open blockers or warnings after one focused revision. The initial finding below is retained as history; the final revision section records its resolution and exact plan hashes. No runtime acceptance is claimed.

## First-round result — ISSUES FOUND (historical)

**Phase:** 16 — host evidence and roadmap
**Plans checked:** 12 live contracts, including completed plans and the 22 → 07 → 08 gap chain
**Issues:** 1 blocker, 0 warnings, 0 info

### Coverage and compatibility checked

| Requirement | Live coverage | Status |
|---|---|---|
| EVID-01 | 01, 02, 03, 22; retained native gate in 07 | Covered; no completed work replayed |
| EVID-02 | 03, 09, 15, 21, 22 | Covered; new concurrency repair is scoped to 22 |
| EVID-03 | 04 | Retained completed evidence |
| EVID-04 | 05, 07, 08, 09, 15, 21, 22 | Covered; actual release evidence remains 08's gate |
| DIR-01, DIR-02 | 06 | Retained completed evidence |

The amendment keeps the accepted work in 01–06, 09, 15, and 21 historical, and makes only 22 → 07 → 08 active (waves 9 → 10 → 11). Plan 07 still requires fresh final-head native evidence and exact-head owner consent; 08 still requires the actual 18 release assets, six metadata checks, and exact-release CI. Plan 22 does not claim publication credit or authorize a lifecycle step. The six requirement IDs and all 17 decisions remain in scope. Of the eight supplied fallback items, four retain explicit accepted criteria (EVID-02 idempotency/concurrency and EVID-03 empty/encoding); the other four (EVID-01, EVID-04, DIR-01, DIR-02) remain flagged unresolved and unclassified.

The selected non-listening TCP reservation is constrained to cooperating drivers in one network namespace. The amendment distinguishes TCP 55355 from the host's UDP endpoint, states same-UID and SIGKILL limits, forbids shared-path lock deletion, uses test-only port injection, and requires descriptor provenance, child-FD closure, real same-port contention, release, and reacquisition evidence. It reuses the already required Perl runtime, fails closed on missing Socket/Fcntl, and leaves CMake registration, platform scope, public APIs, and 649-test/jobs-4 suite bounds unchanged. The actual driver, trace, harness, README, header, registered consumers, and 07/08 dependencies are addressed. The prior review remains byte-identical at SHA-256 `acfb6e8f45dc9a8ccd885336bc85ba9ca3770a6b89550c51ed19a1feb662959c`; the protected config remains at SHA-256 `0517388f5d422ae97afdc5dc1ae11bc43ebd481a757424f152ee8af61559ff5e`.

The supplied path probe is consumed verbatim: overall `status: not_applicable`, 132 commands, 0 blockers, 0 warnings, `readError: null` (it provides no runtime proof). The supplied failing-direction probe is consumed verbatim: `status: ok`, 132 commands, 0 blockers, 0 warnings, `readError: null`. No tests or runtime experiments were run for this review.

### Blocker

**1. [task_completeness] The tracer verification must establish both actual-source outcomes with one frozen oracle.**

- **Plan:** 16-22
- **Task:** 1
- **Required property:** The declared tracer verification proves the preserved old driver causes the in-window foreign-object damage and the corrected driver preserves the same object's inode, type, mode, bytes, and link count, using the same frozen harness/oracle; its successful gate result requires both outcomes.
- **Evidence:** `16-22-PLAN.md` lines 156–157 declare one automated command with a single `$P16_LOCK_DRIVER` argument. Its `fails_when` requires both the expected old-source RED and corrected-source GREEN, then tells the executor to set that same variable to the old source for RED and the tracked source for GREEN. The amendment at `16-22-LOCK-AMENDMENT.md` lines 41–45 confirms these are two distinct source inputs and requires the same oracle for both, but neither the command nor task verify defines two launches or an internal dual-source mode. Since the old RED intentionally exits nonzero, pointing the sole command at the old source cannot itself be a passing check; pointing it at the corrected source does not prove the causal old failure. The central CR-01 acceptance therefore has no unambiguous successful verification path.
- **Example fix (non-binding):** Declare a dual-source harness invocation that runs and classifies both actual drivers under the frozen oracle, or specify separate old-RED and corrected-GREEN verification invocations and how their raw results jointly satisfy the task gate.

### Structured issues

```yaml
issues:
  - plan: "16-22-PLAN.md"
    dimension: "task_completeness"
    severity: "blocker"
    required_property: "The declared tracer verification proves the preserved old driver causes the in-window foreign-object damage and the corrected driver preserves the same object's inode, type, mode, bytes, and link count, using the same frozen harness/oracle; its successful gate result requires both outcomes."
    description: "Task 1 declares one automated command with one P16_LOCK_DRIVER argument, while its fails_when requires both the old-source RED and corrected-source GREEN and instructs that same variable to name each source for its respective run. The amendment requires two distinct source inputs under one oracle but does not define two invocations or a dual-source runner. An old-source run intentionally exits nonzero, and a corrected-source run alone cannot prove the causal old failure, so the plan has no unambiguous successful verification path for CR-01."
    task: 1
    fix_hint: "Define one dual-source command that classifies both expected outcomes, or declare separate RED and GREEN commands and require both source-bound results for acceptance."
```

### Recommendation

1 blocker requires revision before execution. The remaining reviewed plan coverage and boundaries are coherent.

## Revision recheck — VERIFICATION PASSED

**Current verdict:** 0 blockers, 0 warnings, 0 info. The first-round finding above is preserved as history and is **resolved by this revision**.

Task 1 now declares two ordered classifier commands. The old command explicitly selects `P16_LOCK_OLD_DRIVER` and only returns zero after verifying raw driver status 143, raw common-oracle status 1 with `FOREIGN_OBJECT_CHANGED`, proven after-check/before-delete replacement and damage, matching frozen harness/oracle digests, and no unrelated setup failure. The green command explicitly selects `P16_LOCK_NEW_DRIVER`; it requires the same-round accepted red baseline, status 143, raw oracle status 0 with `FOREIGN_OBJECT_PRESERVED`, unchanged object identity/content, matching digests, and same-port exclusion/reacquisition. Both outer commands must return zero. The amendment defines the mirrored old source and tracked new source, explicitly forbids old-source resolution/re-exec into the new driver, defines both commands and marker meanings, and keeps classifier inputs out of the shared safety oracle. This supplies an unambiguous passing path for the causal RED and corrected GREEN without changing the accepted safety assertion.

No additional blocker or warning was found in the revised Task 1 or its amendment. Existing scope, requirement coverage, completed-work preservation, and 22 → 07 → 08 handoff remain intact.

Current source bindings:

| Artifact | SHA-256 |
|---|---|
| `16-22-PLAN.md` | `d7ec34222bea71a12f1a246974395941d1ded6851c9ce49ba28f755ee223410b` |
| `16-22-LOCK-AMENDMENT.md` | `7b7f5b00efb845463c05eedc7e989d91c5ed6470144fd0c34c83ad1b8af85b0d` |
| unchanged `16-07-PLAN.md` | `2e37394cf61921ee045253ed45d22397f3850111d35a3fafef54ab75599236df` |

The supplied probes are consumed verbatim: path resolution `status: not_applicable`, 133 commands, 0 blockers, 0 warnings, `readError: null`; failing directions `status: ok`, 133 commands, 0 blockers, 0 warnings, `readError: null`. The path result is not runtime proof. No tests or runtime experiments were run.

**Final verdict:** `## VERIFICATION PASSED` — the revision resolves the only reported blocker; the prior finding remains recorded above for audit history.
