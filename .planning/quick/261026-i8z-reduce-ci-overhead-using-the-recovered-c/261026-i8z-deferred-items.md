# Deferred items

- **Separate title workflow and remove CI's edited trigger.** The current required
  check is only `CI required`. Moving `title` out of its dependency graph without
  requiring the independent check would let a later invalid title bypass the gate.
  Keep the existing job and trigger until the workflow, tracked ruleset and live
  ruleset can migrate together. No repository setting changed in this quick task.
- **Cold nightly vectors download failures.** The pause handoff records three
  upstream fetch stalls on 2026-10-26. This task preserves the cold-fetch policy;
  it does not add a cache, change the pin, or claim a new successful hosted fetch.
- **Hosted duration measurements.** The existing successful-build timeout envelope
  is retained for the isolated session jobs. Re-measure their actual hosted times
  before tightening limits or claiming an overall PR wall-time improvement.
- **Testing proposals.** The saved property helpers, deterministic signal checks
  and later mutation-testing ideas still need the separate capture step. They are
  proposals, not Phase 20 requirements.
