---
schema_version: 1
open_count: 8
waived_count: 0
fixed_count: 0
total_count: 8
last_updated: 2026-10-08T19:49:17.206Z
---

# Broken Windows Ledger

> Cross-phase defect register. With `workflow.windows_enforce` enabled, `/gsd-ship` blocks while `open_count > 0`.
> Waive with `gsd-tools windows waive <id> "<reason>"` (reason required).
> Mark fixed with `gsd-tools windows fixed <id>`.

| id | phase | kind | file | line | description | status | reason | recorded_at | resolved_at |
|----|-------|------|------|------|-------------|--------|--------|-------------|-------------|
| 1 | 02 | deviation | tests/cmake/vector_source_policy.cmake |  | Parser policy self-test now calls the live classifier and covers parser dependency variants. | open |  | 2026-10-06T14:53:03.690Z |  |
| 2 | 02 | deviation | .github/workflows/release.yml |  | Actionlint flagged an unused retry counter; converted bounded release metadata retries to an explicit counter. | open |  | 2026-10-06T17:11:14.497Z |  |
| 3 | 02 | deviation | scripts/phase2_outcomes.sh |  | A metadata mismatch retry could bypass the counter increment, and pending outcomes shared failure's exit code; both were corrected. | open |  | 2026-10-06T17:13:00.823Z |  |
| 4 | 03 | deviation | .planning/phases/03-a-real-game-in-retroarch/03-02-SUMMARY.md |  | Task carried tdd=true but lacked a test-first RED commit; documented for phase TDD review. | open |  | 2026-10-08T17:15:25.758Z |  |
| 5 | 03 | unrun-verify | .github/workflows/nightly.yml |  | The hosted Linux nightly libFuzzer run is not verified because this branch is local-only and no Actions run exists. | open |  | 2026-10-08T17:24:58.129Z |  |
| 6 | 03 | deviation | src/frame.c |  | Framebuffer is caller-owned and written per visible dot to preserve the existing 64 KiB instance allocator contract. | open |  | 2026-10-08T17:35:40.416Z |  |
| 7 | 03 | deviation | tests/cmake/vector_api_policy.cmake |  | Refreshed the Phase 3 public declaration digest after the planned input API changed the header. | open |  | 2026-10-08T18:02:58.176Z |  |
| 8 | 03 | unrun-verify | .github/workflows/ci.yml |  | The six hosted build lanes were not exercised from the local plan run; observe the GitHub Actions matrix result. | open |  | 2026-10-08T19:49:17.206Z |  |

````json
[
  {
    "id": 1,
    "kind": "deviation",
    "phase": "02",
    "file": "tests/cmake/vector_source_policy.cmake",
    "line": null,
    "description": "Parser policy self-test now calls the live classifier and covers parser dependency variants.",
    "status": "open",
    "reason": "",
    "recorded_at": "2026-10-06T14:53:03.690Z",
    "resolved_at": null,
    "milestone": null
  },
  {
    "id": 2,
    "kind": "deviation",
    "phase": "02",
    "file": ".github/workflows/release.yml",
    "line": null,
    "description": "Actionlint flagged an unused retry counter; converted bounded release metadata retries to an explicit counter.",
    "status": "open",
    "reason": "",
    "recorded_at": "2026-10-06T17:11:14.497Z",
    "resolved_at": null,
    "milestone": null
  },
  {
    "id": 3,
    "kind": "deviation",
    "phase": "02",
    "file": "scripts/phase2_outcomes.sh",
    "line": null,
    "description": "A metadata mismatch retry could bypass the counter increment, and pending outcomes shared failure's exit code; both were corrected.",
    "status": "open",
    "reason": "",
    "recorded_at": "2026-10-06T17:13:00.823Z",
    "resolved_at": null,
    "milestone": null
  },
  {
    "id": 4,
    "kind": "deviation",
    "phase": "03",
    "file": ".planning/phases/03-a-real-game-in-retroarch/03-02-SUMMARY.md",
    "line": null,
    "description": "Task carried tdd=true but lacked a test-first RED commit; documented for phase TDD review.",
    "status": "open",
    "reason": "",
    "recorded_at": "2026-10-08T17:15:25.758Z",
    "resolved_at": null,
    "milestone": null
  },
  {
    "id": 5,
    "kind": "unrun-verify",
    "phase": "03",
    "file": ".github/workflows/nightly.yml",
    "line": null,
    "description": "The hosted Linux nightly libFuzzer run is not verified because this branch is local-only and no Actions run exists.",
    "status": "open",
    "reason": "",
    "recorded_at": "2026-10-08T17:24:58.129Z",
    "resolved_at": null,
    "milestone": null
  },
  {
    "id": 6,
    "kind": "deviation",
    "phase": "03",
    "file": "src/frame.c",
    "line": null,
    "description": "Framebuffer is caller-owned and written per visible dot to preserve the existing 64 KiB instance allocator contract.",
    "status": "open",
    "reason": "",
    "recorded_at": "2026-10-08T17:35:40.416Z",
    "resolved_at": null,
    "milestone": null
  },
  {
    "id": 7,
    "kind": "deviation",
    "phase": "03",
    "file": "tests/cmake/vector_api_policy.cmake",
    "line": null,
    "description": "Refreshed the Phase 3 public declaration digest after the planned input API changed the header.",
    "status": "open",
    "reason": "",
    "recorded_at": "2026-10-08T18:02:58.176Z",
    "resolved_at": null,
    "milestone": null
  },
  {
    "id": 8,
    "kind": "unrun-verify",
    "phase": "03",
    "file": ".github/workflows/ci.yml",
    "line": null,
    "description": "The six hosted build lanes were not exercised from the local plan run; observe the GitHub Actions matrix result.",
    "status": "open",
    "reason": "",
    "recorded_at": "2026-10-08T19:49:17.206Z",
    "resolved_at": null,
    "milestone": null
  }
]
````
