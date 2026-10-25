---
schema_version: 1
open_count: 0
waived_count: 4
fixed_count: 14
total_count: 18
last_updated: 2026-10-23T18:44:55Z
---

# Broken Windows Ledger

> Cross-phase defect register. With `workflow.windows_enforce` enabled, `/gsd-ship` blocks while `open_count > 0`.
> Waive with `gsd-tools windows waive <id> "<reason>"` (reason required).
> Mark fixed with `gsd-tools windows fixed <id>`.

| id | phase | kind | file | line | description | status | reason | recorded_at | resolved_at |
|----|-------|------|------|------|-------------|--------|--------|-------------|-------------|
| 1 | 02 | deviation | tests/cmake/vector_source_policy.cmake |  | Parser policy self-test now calls the live classifier and covers parser dependency variants. | fixed |  | 2026-10-06T14:53:03.690Z | 2026-10-13T16:52:31.428Z |
| 2 | 02 | deviation | .github/workflows/release.yml |  | Actionlint flagged an unused retry counter; converted bounded release metadata retries to an explicit counter. | fixed |  | 2026-10-06T17:11:14.497Z | 2026-10-13T16:52:44.138Z |
| 3 | 02 | deviation | scripts/phase2_outcomes.sh |  | A metadata mismatch retry could bypass the counter increment, and pending outcomes shared failure's exit code; both were corrected. | fixed |  | 2026-10-06T17:13:00.823Z | 2026-10-13T16:52:44.221Z |
| 4 | 03 | deviation | .planning/phases/03-a-real-game-in-retroarch/03-02-SUMMARY.md |  | Task carried tdd=true but lacked a test-first RED commit; documented for phase TDD review. | waived | 2026-10-13: Accepted historical process deviation; 03-02-SUMMARY.md records implementation preceding tests (b2af55f). A missing test-first RED commit cannot be supplied retroactively. | 2026-10-08T17:15:25.758Z | 2026-10-13T16:52:44.618Z |
| 5 | 03 | unrun-verify | .github/workflows/nightly.yml |  | The hosted Linux nightly libFuzzer run is not verified because this branch is local-only and no Actions run exists. | fixed |  | 2026-10-08T17:24:58.129Z | 2026-10-13T16:52:44.316Z |
| 6 | 03 | deviation | src/frame.c |  | Framebuffer is caller-owned and written per visible dot to preserve the existing 64 KiB instance allocator contract. | waived | 2026-10-13: Intentional allocator-contract choice documented in 03-03-SUMMARY.md (2e7c020) and include/nesturbator.h: caller-owned per-visible-dot framebuffer preserves the 64 KiB instance contract. | 2026-10-08T17:35:40.416Z | 2026-10-13T16:52:44.695Z |
| 7 | 03 | deviation | tests/cmake/vector_api_policy.cmake |  | Refreshed the Phase 3 public declaration digest after the planned input API changed the header. | waived | 2026-10-13: Planned input API addition deliberately refreshed the temporary declaration digest; 03-05-SUMMARY.md and commit 2337f9f document the update path authorized by 02-09-SUMMARY.md. | 2026-10-08T18:02:58.176Z | 2026-10-13T16:52:44.769Z |
| 8 | 03 | unrun-verify | .github/workflows/ci.yml |  | The six hosted build lanes were not exercised from the local plan run; observe the GitHub Actions matrix result. | fixed |  | 2026-10-08T19:49:17.206Z | 2026-10-13T16:52:44.395Z |
| 9 | 03 | skipped-test | tests/retroarch/run_retroarch.cmake |  | Local retroarch.testframe skipped because RetroArch aborted before startup; hosted retroarch-e2e is the existing evidence path. | fixed |  | 2026-10-09T14:02:27.608Z | 2026-10-13T16:52:44.470Z |
| 10 | 03 | skipped-test | tests/retroarch/run_retroarch.cmake |  | Local retroarch.game skipped because RetroArch aborted before startup; hosted retroarch-e2e is the existing evidence path. | fixed |  | 2026-10-09T14:02:27.707Z | 2026-10-13T16:52:44.545Z |
| 11 | 16 | deviation | tests/retroarch/frame_index.c |  | Moved the generated marker to scanline 224 after exhaustive qualification exposed a split cell at frame 1467; final frames 4..10000 pass. | fixed |  | 2026-10-13T19:48:51.579Z | 2026-10-13T19:48:59.476Z |
| 12 | 16 | deviation | .github/workflows/ci.yml |  | Wired the required hosted marker invocation and retention during Task 1 so its tracer gate could run before Task 2 expansion. | fixed |  | 2026-10-13T19:48:59.550Z | 2026-10-13T19:49:05.056Z |
| 13 | 16 | deviation | README.md |  | Replaced the unsupported 61-fps wall-time bound with the enforced absolute marker window 4..10000; out-of-window evidence fails. | fixed |  | 2026-10-13T19:49:05.121Z | 2026-10-13T19:49:09.830Z |
| 14 | 16 | deviation | tests/retroarch/CMakeLists.txt | 59 | Removed output-only CTest pass regexes so process exit status governs progression test success | fixed |  | 2026-10-13T22:23:37.344Z | 2026-10-13T22:23:50.741Z |
| 15 | 16 | deviation | .planning/phases/16-host-evidence-and-roadmap/16-04-SUMMARY.md | 105 | Task 2 RED used a reduced root-equality and branch-protection guard instead of the verbatim supplied pin block. | waived | The pinned root and protected-branch conditions were verified; this was a process-only difference in diagnostics, and later commits used the verbatim guard. | 2026-10-15T00:12:18.425Z | 2026-10-15T00:12:58.556Z |
| 16 | 16 | deviation | include/nesturbator.h |  | Plan 16-07 Task 1 corrected a no-float false positive in a public-header comment; executable behavior and scan rules were unchanged, and native CI passed. | fixed |  | 2026-10-23T17:54:21.348Z | 2026-10-23T17:54:44.538Z |
| 17 | 16 | deviation | tests/retroarch/test_state_session.sh |  | Plan 16-07 Task 1 repaired host timing test measurements and scheduling after native CI findings without loosening bounds; local CI/ASan and final native CI passed. | fixed |  | 2026-10-23T17:54:46.994Z | 2026-10-23T17:54:49.121Z |
| 18 | 16 | coverage | tests/retroarch/CMakeLists.txt | 71 | Source-root preflight was callable but absent from normal CTest coverage (WR-07). | fixed | Registered a serial bounded test; full CI 650/650 with packages and focused ASan 1/1 passed; independent follow-up review clean. | 2026-10-23T18:44:55Z | 2026-10-23T18:44:55Z |

````json
[
  {
    "id": 1,
    "kind": "deviation",
    "phase": "02",
    "file": "tests/cmake/vector_source_policy.cmake",
    "line": null,
    "description": "Parser policy self-test now calls the live classifier and covers parser dependency variants.",
    "status": "fixed",
    "reason": "",
    "recorded_at": "2026-10-06T14:53:03.690Z",
    "resolved_at": "2026-10-13T16:52:31.428Z",
    "milestone": null
  },
  {
    "id": 2,
    "kind": "deviation",
    "phase": "02",
    "file": ".github/workflows/release.yml",
    "line": null,
    "description": "Actionlint flagged an unused retry counter; converted bounded release metadata retries to an explicit counter.",
    "status": "fixed",
    "reason": "",
    "recorded_at": "2026-10-06T17:11:14.497Z",
    "resolved_at": "2026-10-13T16:52:44.138Z",
    "milestone": null
  },
  {
    "id": 3,
    "kind": "deviation",
    "phase": "02",
    "file": "scripts/phase2_outcomes.sh",
    "line": null,
    "description": "A metadata mismatch retry could bypass the counter increment, and pending outcomes shared failure's exit code; both were corrected.",
    "status": "fixed",
    "reason": "",
    "recorded_at": "2026-10-06T17:13:00.823Z",
    "resolved_at": "2026-10-13T16:52:44.221Z",
    "milestone": null
  },
  {
    "id": 4,
    "kind": "deviation",
    "phase": "03",
    "file": ".planning/phases/03-a-real-game-in-retroarch/03-02-SUMMARY.md",
    "line": null,
    "description": "Task carried tdd=true but lacked a test-first RED commit; documented for phase TDD review.",
    "status": "waived",
    "reason": "2026-10-13: Accepted historical process deviation; 03-02-SUMMARY.md records implementation preceding tests (b2af55f). A missing test-first RED commit cannot be supplied retroactively.",
    "recorded_at": "2026-10-08T17:15:25.758Z",
    "resolved_at": "2026-10-13T16:52:44.618Z",
    "milestone": null
  },
  {
    "id": 5,
    "kind": "unrun-verify",
    "phase": "03",
    "file": ".github/workflows/nightly.yml",
    "line": null,
    "description": "The hosted Linux nightly libFuzzer run is not verified because this branch is local-only and no Actions run exists.",
    "status": "fixed",
    "reason": "",
    "recorded_at": "2026-10-08T17:24:58.129Z",
    "resolved_at": "2026-10-13T16:52:44.316Z",
    "milestone": null
  },
  {
    "id": 6,
    "kind": "deviation",
    "phase": "03",
    "file": "src/frame.c",
    "line": null,
    "description": "Framebuffer is caller-owned and written per visible dot to preserve the existing 64 KiB instance allocator contract.",
    "status": "waived",
    "reason": "2026-10-13: Intentional allocator-contract choice documented in 03-03-SUMMARY.md (2e7c020) and include/nesturbator.h: caller-owned per-visible-dot framebuffer preserves the 64 KiB instance contract.",
    "recorded_at": "2026-10-08T17:35:40.416Z",
    "resolved_at": "2026-10-13T16:52:44.695Z",
    "milestone": null
  },
  {
    "id": 7,
    "kind": "deviation",
    "phase": "03",
    "file": "tests/cmake/vector_api_policy.cmake",
    "line": null,
    "description": "Refreshed the Phase 3 public declaration digest after the planned input API changed the header.",
    "status": "waived",
    "reason": "2026-10-13: Planned input API addition deliberately refreshed the temporary declaration digest; 03-05-SUMMARY.md and commit 2337f9f document the update path authorized by 02-09-SUMMARY.md.",
    "recorded_at": "2026-10-08T18:02:58.176Z",
    "resolved_at": "2026-10-13T16:52:44.769Z",
    "milestone": null
  },
  {
    "id": 8,
    "kind": "unrun-verify",
    "phase": "03",
    "file": ".github/workflows/ci.yml",
    "line": null,
    "description": "The six hosted build lanes were not exercised from the local plan run; observe the GitHub Actions matrix result.",
    "status": "fixed",
    "reason": "",
    "recorded_at": "2026-10-08T19:49:17.206Z",
    "resolved_at": "2026-10-13T16:52:44.395Z",
    "milestone": null
  },
  {
    "id": 9,
    "kind": "skipped-test",
    "phase": "03",
    "file": "tests/retroarch/run_retroarch.cmake",
    "line": null,
    "description": "Local retroarch.testframe skipped because RetroArch aborted before startup; hosted retroarch-e2e is the existing evidence path.",
    "status": "fixed",
    "reason": "",
    "recorded_at": "2026-10-09T14:02:27.608Z",
    "resolved_at": "2026-10-13T16:52:44.470Z",
    "milestone": null
  },
  {
    "id": 10,
    "kind": "skipped-test",
    "phase": "03",
    "file": "tests/retroarch/run_retroarch.cmake",
    "line": null,
    "description": "Local retroarch.game skipped because RetroArch aborted before startup; hosted retroarch-e2e is the existing evidence path.",
    "status": "fixed",
    "reason": "",
    "recorded_at": "2026-10-09T14:02:27.707Z",
    "resolved_at": "2026-10-13T16:52:44.545Z",
    "milestone": null
  },
  {
    "id": 11,
    "kind": "deviation",
    "phase": "16",
    "file": "tests/retroarch/frame_index.c",
    "line": null,
    "description": "Moved the generated marker to scanline 224 after exhaustive qualification exposed a split cell at frame 1467; final frames 4..10000 pass.",
    "status": "fixed",
    "reason": "",
    "recorded_at": "2026-10-13T19:48:51.579Z",
    "resolved_at": "2026-10-13T19:48:59.476Z",
    "milestone": null
  },
  {
    "id": 12,
    "kind": "deviation",
    "phase": "16",
    "file": ".github/workflows/ci.yml",
    "line": null,
    "description": "Wired the required hosted marker invocation and retention during Task 1 so its tracer gate could run before Task 2 expansion.",
    "status": "fixed",
    "reason": "",
    "recorded_at": "2026-10-13T19:48:59.550Z",
    "resolved_at": "2026-10-13T19:49:05.056Z",
    "milestone": null
  },
  {
    "id": 13,
    "kind": "deviation",
    "phase": "16",
    "file": "README.md",
    "line": null,
    "description": "Replaced the unsupported 61-fps wall-time bound with the enforced absolute marker window 4..10000; out-of-window evidence fails.",
    "status": "fixed",
    "reason": "",
    "recorded_at": "2026-10-13T19:49:05.121Z",
    "resolved_at": "2026-10-13T19:49:09.830Z",
    "milestone": null
  },
  {
    "id": 14,
    "kind": "deviation",
    "phase": "16",
    "file": "tests/retroarch/CMakeLists.txt",
    "line": 59,
    "description": "Removed output-only CTest pass regexes so process exit status governs progression test success",
    "status": "fixed",
    "reason": "",
    "recorded_at": "2026-10-13T22:23:37.344Z",
    "resolved_at": "2026-10-13T22:23:50.741Z",
    "milestone": null
  },
  {
    "id": 15,
    "kind": "deviation",
    "phase": "16",
    "file": ".planning/phases/16-host-evidence-and-roadmap/16-04-SUMMARY.md",
    "line": 105,
    "description": "Task 2 RED used a reduced root-equality and branch-protection guard instead of the verbatim supplied pin block.",
    "status": "waived",
    "reason": "The pinned root and protected-branch conditions were verified; this was a process-only difference in diagnostics, and later commits used the verbatim guard.",
    "recorded_at": "2026-10-15T00:12:18.425Z",
    "resolved_at": "2026-10-15T00:12:58.556Z",
    "milestone": null
  },
  {
    "id": 16,
    "kind": "deviation",
    "phase": "16",
    "file": "include/nesturbator.h",
    "line": null,
    "description": "Plan 16-07 Task 1 corrected a no-float false positive in a public-header comment; executable behavior and scan rules were unchanged, and native CI passed.",
    "status": "fixed",
    "reason": "",
    "recorded_at": "2026-10-23T17:54:21.348Z",
    "resolved_at": "2026-10-23T17:54:44.538Z",
    "milestone": null
  },
  {
    "id": 17,
    "kind": "deviation",
    "phase": "16",
    "file": "tests/retroarch/test_state_session.sh",
    "line": null,
    "description": "Plan 16-07 Task 1 repaired host timing test measurements and scheduling after native CI findings without loosening bounds; local CI/ASan and final native CI passed.",
    "status": "fixed",
    "reason": "",
    "recorded_at": "2026-10-23T17:54:46.994Z",
    "resolved_at": "2026-10-23T17:54:49.121Z",
    "milestone": null
  }
]
````
## Closure evidence — 2026-10-13

The original descriptions, timestamps and historical file references above are
retained. Fixed records use this section because the ledger writer accepts a
reason for waivers only. Archived summaries are under `milestones/`.

| ID | Evidence and disposition |
|---|---|
| 1 | Fixed: [02-09-SUMMARY.md](milestones/v1-phases/02-the-cpu-matches-the-public-vectors/02-09-SUMMARY.md), commit `08c1df4`: mutation self-test invokes the live parser classifier. |
| 2 | Fixed: [02-11-SUMMARY.md](milestones/v1-phases/02-the-cpu-matches-the-public-vectors/02-11-SUMMARY.md), commit `fe21b93`: explicit bounded release metadata retry counter. |
| 3 | Fixed: 02-11-SUMMARY.md, commit `c1f1c3c`: every failed metadata attempt advances the counter; pending exits 2 and failure exits 1. |
| 4 | Waived: [03-02-SUMMARY.md](milestones/v1-phases/03-a-real-game-in-retroarch/03-02-SUMMARY.md), commit `b2af55f`, preserves the missing RED commit as an accepted historical process deviation; history cannot be made test-first retroactively. |
| 5 | Fixed: [03-VERIFICATION.md](milestones/v1-phases/03-a-real-game-in-retroarch/03-VERIFICATION.md) and [Nightly run 37962202123](https://github.com/szTheory/nesturbator/actions/runs/37962202123): `rom-loader-fuzz` completed successfully. |
| 6 | Waived: [03-03-SUMMARY.md](milestones/v1-phases/03-a-real-game-in-retroarch/03-03-SUMMARY.md), commit `2e7c020`, and public frame-buffer comments document the intentional caller-owned per-dot output preserving the 64 KiB allocator contract. |
| 7 | Waived: [03-05-SUMMARY.md](milestones/v1-phases/03-a-real-game-in-retroarch/03-05-SUMMARY.md), commit `2337f9f`: the planned input API deliberately refreshed the temporary declaration digest, as allowed by 02-09-SUMMARY.md. |
| 8 | Fixed: 03-VERIFICATION.md and [CI run 37994563525](https://github.com/szTheory/nesturbator/actions/runs/37994563525): all six hosted builds and `CI required` passed on exact remediation SHA `20331371c8036cd5fccec6491d25262e3a2c0364`. |
| 9, 10 | Fixed by supersession: [05-03-SUMMARY.md](milestones/v2-phases/05-tune-up-and-v1-debt/05-03-SUMMARY.md) records removal of local GUI test registrations and `policy.no-skip`; [05-CI-RECORD.md](milestones/v2-phases/05-tune-up-and-v1-debt/05-CI-RECORD.md) and [CI run 38061698752](https://github.com/szTheory/nesturbator/actions/runs/38061698752) show hosted `retroarch-e2e` passed on `e853abe7d9c7121f68a8e065b1328ddd8fbd9cc9`. The local GUI failures were superseded, not repaired locally. |

Read-only GitHub queries reconfirmed the named hosted conclusions and exact SHAs
on 2026-10-13. All named commits exist in repository history. Unrelated milestone
audit debt, including Phase 13 WR-01 through WR-03, remains unchanged.
