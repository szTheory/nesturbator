## Deferred Items

- The first sandboxed `cmake --workflow --preset ci` run failed at `retroarch.testframe`. RetroArch also exited 134 for `--version` inside the shell sandbox, but the installed v1.22.2 binary returned its version outside the sandbox; the targeted test and full 327-test CI workflow then passed outside it. The test driver now captures stdout and stderr separately for future launch failures.
  status: resolved
