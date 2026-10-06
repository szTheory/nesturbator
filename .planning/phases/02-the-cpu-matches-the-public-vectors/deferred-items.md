## Deferred Items

- `cmake --workflow --preset ci` failed only at the unrelated `retroarch.testframe`: RetroArch exited with `Subprocess aborted` after 5.15 seconds. The 326 other tests passed, including all 256 sample opcode vectors. The policy changes do not touch RetroArch; diagnose this host-specific abort separately.
  status: open
