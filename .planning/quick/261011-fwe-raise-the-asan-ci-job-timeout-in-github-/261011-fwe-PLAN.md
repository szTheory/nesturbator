---
phase: quick-261011-fwe
plan: 01
type: execute
wave: 1
depends_on: []
files_modified:
  - .github/workflows/ci.yml
autonomous: true
requirements:
  - QUICK-261011-fwe

estimate:
  tokens: 15000
  raw_tokens: 15000
  tasks: 1
  confidence: low

must_haves:
  truths:
    - "The asan job in .github/workflows/ci.yml has timeout-minutes: 20"
    - "Its trailing comment says 20 is provisional until a complete measured run, and cites cancelled run 38150104151 (477/480, zero failures)"
    - "No other job's timeout and no product code, README or header changes"
    - "The workflow file still parses as YAML and scripts/hygiene.sh passes"
  artifacts:
    - path: .github/workflows/ci.yml
      provides: "asan job timeout raised from 10 to 20 minutes with a provisional note"
  key_links:
    - from: ".github/workflows/ci.yml asan job"
      to: "ENGINEERING §5 timeout rule (twice a measured cold run)"
      via: "one-line comment marking 20 as provisional"
---

<objective>
Raise the asan CI job timeout from 10 to 20 minutes so Phase 10's boot-to-play and libretro parity tests can finish under ASan on PR #37.

Purpose: run 38150104151 was cancelled by the 10-minute cap at 477/480 tests with zero failures. The new Phase 10 tests cost about 235 s for runner.boot_to_play and about 55 s for each libretro.parity.* mapper 4 test under ASan. ENGINEERING §5 sets a timeout at twice a measured cold run, and there is no complete measured run yet, so 20 is provisional.
Output: one edited line in .github/workflows/ci.yml, committed on the current branch gsd/phase-10-close-out-and-boot-to-play.
</objective>

<execution_context>
@~/.claude/gsd-core/workflows/execute-plan.md
@~/.claude/gsd-core/templates/summary.md
</execution_context>

<context>
@.planning/STATE.md
@CLAUDE.md
@.github/workflows/ci.yml
</context>

<tasks>

<task type="tracer">
  <name>Task 1: Raise asan timeout-minutes to 20 with a provisional comment</name>
  <files>.github/workflows/ci.yml</files>
  <read_first>.github/workflows/ci.yml (the asan job, line 126, and the other timeout comments at lines 33, 43, 69, 140 for comment style)</read_first>
  <action>
In the asan job of .github/workflows/ci.yml (line 126, directly under `runs-on: ubuntu-24.04`), change `timeout-minutes: 10` to `timeout-minutes: 20` and replace the trailing comment with one terse line in the same style as the existing ones. The comment must: say the value is provisional until a complete measured run (ENGINEERING §5 asks for twice a measured cold run); cite run 38150104151, cancelled by the 10-minute cap at 477/480 with zero failures after Phase 10 added runner.boot_to_play (about 235 s) and the libretro.parity mapper 4 runs (about 55 s each) under ASan. It may keep a short reference to run 38097794967 (4m14s) as the earlier measured baseline. Suggested wording: `timeout-minutes: 20 # provisional until a complete measured run (ENGINEERING §5); run 38150104151 hit the 10-minute cap at 477/480 with no failures after Phase 10 added runner.boot_to_play (about 235 s) and libretro.parity mapper 4 runs (about 55 s each) under ASan`.
Do not touch any other job's timeout, any product code, the README or any header. Keep the edit on the current branch gsd/phase-10-close-out-and-boot-to-play (it feeds PR #37). Commit with the Conventional Commit message `ci: raise asan timeout to 20 minutes`, using the repository's configured noreply identity, with the attribution trailer from the session.
  </action>
  <verify>
    <automated>python3 -c 'import yaml; d=yaml.safe_load(open(".github/workflows/ci.yml")); assert d["jobs"]["asan"]["timeout-minutes"]==20' && grep -n 'timeout-minutes: 20 #.*provisional.*38150104151' .github/workflows/ci.yml && scripts/hygiene.sh</automated>
  </verify>
  <done>The asan job parses with timeout-minutes 20; the comment marks it provisional and cites run 38150104151; only .github/workflows/ci.yml changed outside .planning; scripts/hygiene.sh passes; the commit `ci: raise asan timeout to 20 minutes` is on gsd/phase-10-close-out-and-boot-to-play.</done>
</task>

</tasks>

<threat_model>
## Trust Boundaries

| Boundary | Description |
|----------|-------------|
| repo -> GitHub Actions runner | Workflow YAML controls how long a hosted runner may run |

## STRIDE Threat Register

| Threat ID | Category | Component | Severity | Disposition | Mitigation Plan |
|-----------|----------|-----------|----------|-------------|-----------------|
| T-261011-01 | Denial of Service | asan job timeout | low | accept | A hung asan job can now hold a runner for 20 rather than 10 minutes; bounded, and the comment flags the value for re-measurement |
| T-261011-SC | Tampering | npm/pip/cargo installs | high | mitigate | No package installs in this plan; nothing to gate |
</threat_model>

<verification>
- `python3 -c 'import yaml; ...'` confirms jobs.asan.timeout-minutes is 20 and the file parses.
- `grep -n 'timeout-minutes: 20' .github/workflows/ci.yml` hits the asan job line.
- `scripts/hygiene.sh` passes.
</verification>

<success_criteria>
The asan job allows 20 minutes, the comment states it is provisional and cites run 38150104151, and the change is committed on gsd/phase-10-close-out-and-boot-to-play as `ci: raise asan timeout to 20 minutes`.
</success_criteria>

<output>
Create `.planning/quick/261011-fwe-raise-the-asan-ci-job-timeout-in-github-/261011-fwe-SUMMARY.md` when done
</output>
