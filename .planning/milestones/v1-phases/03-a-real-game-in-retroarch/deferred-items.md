# Deferred Items

- The existing local `retroarch.testframe` GUI harness aborts with `Subprocess aborted` and empty stdout/stderr in this macOS session. The 341 other CI workflow tests pass; investigate the local RetroArch session requirement separately.
  status: resolved
  Historical acknowledgement (2026-10-09 milestone close): acknowledged.
  Resolution (2026-10-13): Phase 05 removed the local GUI test registrations,
  rather than repairing this local session. [05-03-SUMMARY.md](../../v2-phases/05-tune-up-and-v1-debt/05-03-SUMMARY.md)
  records the removal and `policy.no-skip` enforcement. Hosted `retroarch-e2e`
  passed in [CI run 38061698752](https://github.com/szTheory/nesturbator/actions/runs/38061698752)
  on `e853abe7d9c7121f68a8e065b1328ddd8fbd9cc9`, as recorded in
  [05-CI-RECORD.md](../../v2-phases/05-tune-up-and-v1-debt/05-CI-RECORD.md).
