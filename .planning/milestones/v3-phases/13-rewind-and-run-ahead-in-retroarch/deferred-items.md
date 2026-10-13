
## Plan 13-07: macOS Intel build exceeds its job timeout
`build (macos-15-intel, ci, macos, x64)` is cancelled at the job's `timeout-minutes: 7` (ci.yml line 69) while the
`libretro.host` and `core.state_fork` tests are running. Seen on run 38289751391 (head 719f678, before plan 07) and
on run 38291063413 (head 2d4f991, twice). `CI required` fails because of it. Not caused by plan 07; needs a decision
on the timeout (the comment says 198 s was the slowest observed) or on the test cost added in phase 13.

- Plan 13-07 macOS Intel build timeout: original observation retained above.
  status: resolved
  Resolution (2026-10-13): [13-08-SUMMARY.md](13-08-SUMMARY.md) records Intel
  at 640 seconds in run 38295163207 and the resulting 22-minute build budget.
  [Final CI run 38297707729](https://github.com/szTheory/nesturbator/actions/runs/38297707729)
  passed every job on `137d855cfbf9a8bf9875aaf911ccb38a4afe4d4e`.
  The current `ci.yml` build budget is 23 minutes following later measurements;
  this does not change the historical 22-minute fix. Phase 13 WR-01 through
  WR-03 remain substantive review debt in the milestone audit.
