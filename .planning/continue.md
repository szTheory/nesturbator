# Resume Phase 23 after interrupted regression gate

Read AGENTS.md, STATE.md, this file, Phase 22 VERIFICATION/REFRESH-EVIDENCE/
REVIEW-DISPOSITION, and Phase 23 REVIEW, REVIEW-DISPOSITION, VERIFICATION and
EXECUTION-EVIDENCE. All seven v6 plans are implemented. Phase 22 verification
passed; its three fixes changed README.md, docs/progress.md and
tests/cmake/reproduction_contract.cmake, making Phase 23's earlier verification
stale.

Phase 23's standard-depth review and disposition were refreshed and committed.
The review covered those three changed files and found no new issues. During the
regression gate, the configured command resolved to `cmake --workflow --preset
ci`, although the preserved boundary requires excluding the AccuracyCoin
campaign. The workflow started; CTest output reached roughly 608/706, with
`accuracycoin.campaign` scheduled. CTest was interrupted; it left two identified
child processes, one of which had already exited when signaled, while the other
exited after SIGINT. A follow-up process check found none from this run. The run
left no usable final CTest log, so whether the campaign test completed is
unknown. The verifier did not run; Phase 23's
fingerprint remains stale. Do not claim the regression gate or verification
passed.

Stop here for the owner's decision about whether to continue after reviewing
that uncertain campaign activity. The recommended path is a bounded CTest-only
refresh using the already documented selections in
`phases/22-install-and-protect-progress/22-REFRESH-EVIDENCE.md`, with
`-FA accuracycoin_full_report`; do not rerun the full workflow. If approved, the
next command is `$gsd-execute-phase 23`, with `workflow.test_command` set to the
bounded selection for that run. After a passing refresh, milestone audit is a
separate command and must not be started implicitly. Context may be cleared
before resuming.

The local CI cache enables the full campaign. No collector/provenance refresh,
host/device, private-content, daily-save, publication, push or PR is authorized.
No timeout was raised or machine restart performed. PowerShell and other-platform
execution remain untried; commercial scenes remain unclassified. Preserve all
collector/platform/host-recovery debt and historic runtime windows. The owner's
unstaged AGENTS.md and .planning/config.json edits remain untouched.
