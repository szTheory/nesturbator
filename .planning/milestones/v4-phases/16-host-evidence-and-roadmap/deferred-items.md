# Deferred items

- Local macOS `cmake --workflow --preset nofp` reported `abi.undefined_symbols`
  for `stack_chk_fail;stack_chk_guard` from the Apple toolchain. The Linux
  native no-float legs are the relevant acceptance environment; investigate
  Apple-toolchain symbol policy in a separate scoped change if it remains
  necessary.
  status: acknowledged
