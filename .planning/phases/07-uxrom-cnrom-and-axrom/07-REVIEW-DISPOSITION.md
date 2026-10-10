---
phase: 07-uxrom-cnrom-and-axrom
source: 07-REVIEW.md
open: 5
---

# Phase 07 code review disposition

One row per finding in 07-REVIEW.md. Every row starts as `open`. Set a row to `fixed`, `skipped` or `deferred` by hand, and write the reason in Source.

| ID | Severity | Disposition | Source |
|----|----------|-------------|--------|
| WR-01 | warning | open | 07-REVIEW.md: AxROM reset vector from bank 0 holds only at power-on |
| WR-02 | warning | open | 07-REVIEW.md: hash_inventory compares only the last platform's keys |
| IN-01 | info | open | 07-REVIEW.md: README status paragraph stale and garbled |
| IN-02 | info | open | 07-REVIEW.md: bus-conflict AND computed for $4020-$7FFF writes |
| IN-03 | info | open | 07-REVIEW.md: magic board-size limits in board_profile_ok |
