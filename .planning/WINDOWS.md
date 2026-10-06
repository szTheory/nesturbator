---
schema_version: 1
open_count: 3
waived_count: 0
fixed_count: 0
total_count: 3
last_updated: 2026-10-06T17:13:00.823Z
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
  }
]
````
