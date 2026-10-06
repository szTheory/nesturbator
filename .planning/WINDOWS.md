---
schema_version: 1
open_count: 1
waived_count: 0
fixed_count: 0
total_count: 1
last_updated: 2026-10-06T14:53:03.690Z
---

# Broken Windows Ledger

> Cross-phase defect register. With `workflow.windows_enforce` enabled, `/gsd-ship` blocks while `open_count > 0`.
> Waive with `gsd-tools windows waive <id> "<reason>"` (reason required).
> Mark fixed with `gsd-tools windows fixed <id>`.

| id | phase | kind | file | line | description | status | reason | recorded_at | resolved_at |
|----|-------|------|------|------|-------------|--------|--------|-------------|-------------|
| 1 | 02 | deviation | tests/cmake/vector_source_policy.cmake |  | Parser policy self-test now calls the live classifier and covers parser dependency variants. | open |  | 2026-10-06T14:53:03.690Z |  |

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
  }
]
````
