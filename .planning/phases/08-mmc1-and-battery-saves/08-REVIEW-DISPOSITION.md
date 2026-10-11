---
phase: 08-mmc1-and-battery-saves
source: 08-REVIEW.md
open: 5
---

# Phase 08 code review disposition

One row per finding in 08-REVIEW.md. Every row starts as `open`. Set a row to `fixed`, `skipped` or `deferred` by hand, and write the reason in Source.

| ID | Severity | Disposition | Source |
|----|----------|-------------|--------|
| WR-01 | warning | open | 08-REVIEW.md: A failed interval flush is never retried, so the exit flush can silently skip the save |
| WR-02 | warning | open | 08-REVIEW.md: NES 2.0 MMC1 submapper 5 (fixed 32 KiB PRG) is accepted but emulated as ordinary MMC1 |
| WR-03 | warning | open | 08-REVIEW.md: A directory at the .sav location reports a size mismatch |
| IN-01 | info | open | 08-REVIEW.md: Interval test depends on a wall-clock timeout |
| IN-02 | info | open | 08-REVIEW.md: Failed flush leaves a stale `.tmp` file with no cleanup |
