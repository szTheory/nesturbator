---
title: "GSD hand-off: kickoff, per-phase steps and owner tasks"
summary: "How to start and run this project with opengsd one step at a time: the kickoff text, the settings and why, the per-phase steps, and the tasks only the owner can do."
read_when: "Before starting the GSD project, before each phase, and when a GSD step behaves unexpectedly."
updated: 2026-10-06
---

# GSD hand-off

## Why this file exists
The project is built with opengsd (gsd-core). Preparation wrote the files GSD normally creates by asking questions, so the first command resumes at roadmap creation. This file holds that command, the settings that make GSD stop after every step, and the tasks only the owner can do.

## 1. Before the kickoff
- **GSD version.** `cat ~/.claude/gsd-core/VERSION` should print 1.15.0, the version this package was checked against [GH.01]. Version 1.10.0 has no resume path: it stops with "project already initialized" when `PROJECT.md` exists [GH.02]. To install 1.15.0 for Claude Code: `npx -y @opengsd/gsd-core@1.15.0 --claude --global`, then open a new session. Check the version again before each phase step; on 2026-10-02 a reinstall from a stale npx cache put 1.10.0 back.
- **Runtime.** Run this project's GSD steps in Claude Code. There GSD writes its generated guide to `.claude/CLAUDE.md`; under Codex it writes to `AGENTS.md`, which holds this project's rules [GH.01].
- **Read** `.planning/PROJECT.md`, `.planning/REQUIREMENTS.md` and `AGENTS.md`, about ten minutes. Change them now if anything is off: the roadmap is built from them.

## 2. Kickoff
In a new Claude Code session at the repository root, paste this as one message:

```
/gsd-new-project Resume the pre-seeded bootstrap. PROJECT.md, REQUIREMENTS.md, config.json and research/SUMMARY.md were written during preparation and are final for initialization: keep them as they are and continue from roadmap creation. Do not run questioning, workflow preferences, project research or requirements scoping. Structure mode is Horizontal Layers (standard template, no Mode lines). Copy each suggested phase's "Canonical refs" line from research/SUMMARY.md into ROADMAP.md and add "UI hint: no" to every phase. Stop after the roadmap is approved and committed.
```

GSD finds a project without a roadmap and resumes instead of starting over [GH.01]. Expect two prompts:

1. **Structure.** Choose "Horizontal Layers". It is GSD's name for its standard roadmap template; the other choice adds a user-story format check to every phase.
2. **Roadmap.** Approve if the checklist below holds; otherwise choose "Adjust phases" and say what to change.

GSD then commits `ROADMAP.md`, `STATE.md`, the updated `REQUIREMENTS.md` and `.claude/CLAUDE.md` on `main` and stops.

**Roadmap checklist**
- Four phases, each ending in an automated release: a test frame in RetroArch, the CPU against the public vectors, a real game in RetroArch, sound.
- Phase 1 contains shipped C code, the `ci` preset, CI and the first release.
- No phase whose goal is a decision, a study or an experiment.
- Every phase has a "Canonical refs" line and "UI hint: no"; none has a `**Mode:**` line.
- All 19 requirements are mapped.

**If GSD asks its full set of questions instead**

| Prompt | Answer |
|---|---|
| What do you want to build? | "Use .planning/PROJECT.md as written", then "Create PROJECT.md" |
| Saved defaults found | "Configure fresh". "Use as-is" would import the yolo mode and the web checks |
| Mode, granularity, execution, git tracking | Interactive, Coarse, Sequential, Yes |
| Research before each phase, plan check, verifier | Yes to all three |
| Compact content, AI models, PR body sections | No, Inherit, none |
| Research the domain first? | Skip research |
| Feature scoping | "Use the existing REQUIREMENTS.md unchanged"; add nothing |
| Structure | Horizontal Layers |

GSD's config writer keeps an existing `config.json`, so the settings answers matter only if that file is missing [GH.01].

**After the kickoff**, each of these prints the value shown:

```
node ~/.claude/gsd-core/bin/gsd-tools.cjs query config-get mode --raw                        # interactive
node ~/.claude/gsd-core/bin/gsd-tools.cjs query config-get workflow.auto_advance --raw       # false
node ~/.claude/gsd-core/bin/gsd-tools.cjs query config-get workflow.test_command --raw       # cmake --workflow --preset ci
```

## 3. Each phase
One session per numbered step, so the model can be changed between steps.

1. `git switch main && git pull && git switch -c phase/NN-short-name` (before the repository has a remote, leave out `git pull`).
2. `/gsd-discuss-phase N`
3. `/gsd-plan-phase N`
4. `/gsd-execute-phase N`. Code review, the test command and the verifier run inside this step.
5. Read the phase's `VERIFICATION.md`.
6. `/gsd-ship N`
7. In a terminal, give the pull request a Conventional Commit title and let it merge when CI passes: `gh pr edit --title "feat: <what the phase delivers>"`, then `gh pr merge --auto --squash`. Keep these two out of a GSD command's text; GSD would read the merge option as its own auto flag. If the title check ran before the new title, start it again with `gh run rerun --failed`.
8. The release publishes itself: release-please opens its pull request, which merges when CI passes. Install it with the README's one line and try it in RetroArch.

### Verification before owner UAT

Before conversational UAT, run every command-verifiable criterion locally or
in CI and record the command, exit status, commit or run URL, and evidence in
`VERIFICATION.md` or the phase UAT record. Add a missing regression gate when
a test-tier prohibition has no enforcing check. After an authorized merge,
collect release and hosted-run outcomes with the phase's read-only evidence
command; keep not-yet-triggered external events pending. Existing CI failure
and issue reporting remain the automatic backstop. Prompt the owner only for
identity-bound consent not already granted or an irreducible subjective
judgment, and explain why no useful command can answer it. Fix a reproduced
defect with a known fix under the existing authority. Clean-room provenance
remains a judgment item because repository scans cannot establish which
sources a person or agent opened. Preserve one GSD step per command and the
owner's merge boundary. Do not invent a no-human-verify setting: installed GSD
supports end-of-phase and mid-flight verification only.

What keeps GSD from running on to the next step [GH.01]:
- Give GSD commands only a phase number. Its auto flag and its chain flag (two dashes followed by `auto` or `chain`) make one step start the next.
- `/gsd-next`, `/gsd-progress` with its next option, `/gsd-autonomous` and `/gsd-manager` each start the following step by themselves.
- `scripts/hygiene.sh` stops a commit if `.planning/config.json` leaves interactive mode or switches a chain setting on.

GSD creates no branch with these settings and its executor does not commit to `main`, which is why step 1 comes first [GH.01]. `/gsd-ship` titles the pull request "Phase N: name" and does not merge it.

## 4. Settings and why
`.planning/config.json` was written by GSD's own config writer [GH.01].

| Setting | Value | Why |
|---|---|---|
| `mode` | interactive | With yolo, the end of one phase starts the next |
| `workflow.auto_advance`, `workflow._auto_chain_active` | false | Stops discuss, plan and execute from chaining |
| `granularity` | coarse | Milestone 1 is four phases |
| `parallelization`, `workflow.use_worktrees` | false | A small, coupled codebase; no worktree paths in tracked files |
| `model_profile`, `resolve_model_ids` | inherit, omit | Every GSD subagent uses the model of the session that started it |
| `workflow.ui_phase`, `ui_safety_gate`, `ui_review`, `ai_integration_phase`, `api_coverage_gate`, schema checks | false | Written for web projects; they react to words such as "page", "screen" and "vector" |
| `workflow.security_enforcement` | false | A web threat-model checklist; sanitizer builds and fuzz targets are requirements here instead |
| `workflow.nyquist_validation`, `pattern_mapper` | false | GSD's own choice at coarse granularity; no code to map yet |
| `workflow.test_command` | `cmake --workflow --preset ci` | Without it the test step runs `true`. It fails until phase 1 creates the preset |
| `git.branching_strategy`, `git.create_tag` | none, false | The owner branches before discuss, so planning and code share one pull request; release-please owns tags |
| `hooks.community` | true | Turns on GSD's Conventional Commit check |

The `preferences` key comes from the owner's global GSD defaults; gsd-tools prints a warning that it ignores it.

## 5. Owner tasks
Things an assistant must not decide or cannot do.

| When | Task |
|---|---|
| Before executing phase 1 | Say yes to creating the public GitHub repository. The push hook scans the whole history first. |
| Before executing phase 1 | Create the credential release-please uses to open its pull request: a GitHub App (recommended; short-lived tokens) or a fine-grained personal access token. It is stored as an Actions secret. It is the project's only secret; local automation reads `.env.local`. |
| Before executing phase 1 | Install RetroArch from `RetroArch_Metal.dmg` or with `brew install --cask retroarch-metal`, so the phase's RetroArch test can run on your Mac. The `retroarch` cask is Intel-only, and the App Store build is sandboxed with no known way to add an outside core [GH.03]. |
| Phase 1 | Signing. Recommendation: none beyond the linker's ad-hoc signature, installed by the README's one-line script. A Developer ID certificate would put a legal name in every artifact. |
| Before the repository is public | Read `PROVENANCE.md`, `ASSET_POLICY.md` and the README's "How the code is written" paragraph. They state that AI assistants write this code; reword them if you prefer. |
| Any time | Decide which process Playstead launches for NES games before a shared host exists: RetroArch with this core is the available one. The runner has no window. |
| Optional | Point local test runs at your own ROM folder through `.env.local`. Results stay on your machine. |
| Later | Whether the nightly run fetches GPL-licensed test ROMs; whether and when to submit a score to the AccuracyCoin leaderboard. |
| Never in this repository | `/gsd-profile-user`. It writes a developer profile into `.claude/CLAUDE.md`, which is tracked and public. |

## 6. Asked for during preparation, placed later

| Asked for | Where it is |
|---|---|
| Property tests, long fuzz runs, a speed check that blocks merges, tail frame times | SEED-004 (requirements PERF-01 to PERF-03). Milestone 1 has sanitizer builds, the loader's fuzz corpus and cross-platform hash equality |
| Recurring CI-speed and quality passes | SEED-006, at the start of every milestone |
| A rolling near, mid and long-term roadmap | `ROADMAP.md` is the near term; SEED-001 to SEED-004 the next milestones; SEED-005 the long term |
| More mappers, save states, PAL, expansion audio, FDS | SEED-001, SEED-002, SEED-003, SEED-005 |
| Tracing without run-time cost | A compile-time option; release builds contain no trace code (ARCHITECTURE 7) |
| What users want from a frontend (shaders, menus, input mapping) | NES-ECOSYSTEM 5, tagged "host", for the shared host project |
| Signing and notarization | Not needed for the script install; LIBRETRO-AND-RUNNER 3 has the facts if that changes |

## 7. What was checked
- `scripts/hygiene.sh` was run in scratch repositories in its staged, tree and history modes against planted home paths, e-mail addresses, ROM files by name and by content, and a config with chaining on; each was reported. It passes on this tree and on a copy holding a generated `ROADMAP.md`, `STATE.md` and `.claude/CLAUDE.md`.
- `config.json` was read back with `config-get`: interactive mode, both chain settings false, the test command set.
- Under gsd-core 1.15.0, `query init.new-project` reports `project_exists` true, `init_incomplete` true and `is_brownfield` false, which is the state the resume path needs [GH.01].
- The 1.15.0 roadmapper was run twice on copies of these inputs. The second run gave four phases with all 19 requirements mapped, a "Canonical refs" line and `**UI hint**: no` on each, and no `**Mode:**` line; GSD's `roadmap.analyze` and `validate consistency` accepted the result. Its remaining notes were then applied to the inputs.
- A fresh reviewer re-fetched 25 claims from their primary sources: licences, test-suite facts, libretro and RetroArch facts, GitHub runner and release-please facts, the timing arithmetic, and the GSD behaviours this file relies on. Twenty-two held. Two were wrong (the Windows runner images, and two different pins for `libretro.h`) and one could not be confirmed (a MesenCE option); all three are corrected or marked in the files.
- A second reviewer read only the files GSD reads and proposed deletions. Its findings are applied: requirements worded as commands, the first phase split in two, prohibitions and process limits removed.
- `list-seeds` reads all six seeds; every frontmatter block parses as strict YAML; every citation ID and every relative link resolves.

## Open questions
- The resume path was exercised through GSD's init query and roadmapper runs on copies, not through a full `/gsd-new-project` session. The table in section 2 covers the case where it asks its questions anyway.
- Frame and audio hashes are recorded from this core's own output. What makes a first recording right is AccuracyCoin, the RetroArch screenshot test and the owner's look at the release.

## Sources
| ID | Tier | Source | Pin or access date | Supports |
|---|---|---|---|---|
| GH.01 | T1 | `@opengsd/gsd-core` npm package: `gsd-core/workflows/new-project.md`, `transition.md`, `discuss-phase.md`, `plan-phase.md`, `execute-phase.md`, `ship.md`; `gsd-core/bin/lib/init.cjs`, `config.cjs`; `agents/gsd-executor.md`, `gsd-roadmapper.md` | 1.15.0 | Resume path, chaining, branches, config |
| GH.02 | T1 | Same package: `gsd-core/workflows/new-project.md`; `VERSION` and install manifest of the Claude Code install | 1.10.0; 2026-10-02 | No resume path in 1.10.0; the reinstall |
| GH.03 | T2 | [LIBRETRO-AND-RUNNER.md](LIBRETRO-AND-RUNNER.md) section 3 and its Sources | 2026-10-02 | RetroArch builds on macOS |
