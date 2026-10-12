
## Plan 13-07: macOS Intel build exceeds its job timeout
`build (macos-15-intel, ci, macos, x64)` is cancelled at the job's `timeout-minutes: 7` (ci.yml line 69) while the
`libretro.host` and `core.state_fork` tests are running. Seen on run 38289751391 (head 719f678, before plan 07) and
on run 38291063413 (head 2d4f991, twice). `CI required` fails because of it. Not caused by plan 07; needs a decision
on the timeout (the comment says 198 s was the slowest observed) or on the test cost added in phase 13.
