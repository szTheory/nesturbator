# Deferred Items

- `.planning/WINDOWS.md` could not accept the prior plan's deviation entry: its
  frontmatter counts 18 entries, while the authoritative fenced JSON and table
  contain 17 records (table row 18 has no JSON entry). `gsd_run windows append`
  refused the update. The pre-existing ledger consistency issue remains
  deferred to its owning planning-health work.

## Phase 21 Plan 08 platform-limited nofp probe

## macOS arm64 `nofp` undefined-symbol allowlist

- **Reproduced at:** final candidate `1d27a0f12542a235d8d79b9c97591168fbd77d8e`.
- **Command:** `cmake --workflow --preset nofp` on macOS arm64.
- **Result:** `abi.float_scan`, `abi.float_scan.selftest`, `abi.global_symbols`,
  and `abi.float_fixture` passed. `abi.undefined_symbols` rejected the Apple
  stack-protector references `stack_chk_fail` and `stack_chk_guard`.
- **Scope:** the configured hosted `nofp` matrix uses Linux x64 and Linux
  arm64. This local macOS failure does not establish either hosted result. Keep
  the allowlist gate intact and require the actual Linux CI results before
  treating the `nofp` requirement as verified.

The initial local `nofp` run also reported the decimal `2.3` in the state
validation comment. That source citation was changed to integer-only section
wording in `1d27a0f`; the final source scan passes. The scanner and its hard
gate were not weakened.
