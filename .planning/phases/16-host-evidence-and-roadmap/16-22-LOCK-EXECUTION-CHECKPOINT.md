# Plan 22 lock execution checkpoint

Status: incomplete. The three named development rounds are consumed. No active task is complete or committed, no success SUMMARY exists, and no new formal acceptance, independent source review or parent gate ran. Plans07/08 remain blocked; Phase16 remains9/12 live plans complete.

## Observed results

The typed sequential executor was gsd-executor, gpt-6-luna/xhigh, on branch gsd/phase-16-host-evidence-and-roadmap from a72f97f444c2cb3058723a190eb260e727c393ad. It returned a terminal blocking-human decision checkpoint after task1 failed. Parent inspection confirms the five source/doc edits remain uncommitted; this record grants no acceptance to them.

| Round | Actual observation | Disposition |
|---|---|---|
| 1 | Actual mirrored old-driver RED recorded143 / oracle1 / FOREIGN_OBJECT_CHANGED. | Preserved diagnostic. Frozen harness lacked an actual post-close reacquiring driver; no current acceptance. |
| 2 | RED marker/raw143/oracle1 and GREEN marker/raw143/oracle0 were recorded. GREEN's saved outer status is0 and includes actual exact-port reacquisition. | Preserved diagnostic. RED's outer wrapper used zsh's readonly status variable, so no saved outer status exists. Later source/harness corrections also invalidate this pair as current-candidate acceptance. |
| 3 | Saved RED outer0, driver143, oracle1/FOREIGN_OBJECT_CHANGED. Saved GREEN outer1; driver output LOCK_OWNER_UNAVAILABLE. | Failed before foreign-object injection. No GREEN safety verdict or captured driver wait/status exists. No preservation claim. |

Round3's GREEN raw root is build/phase16-lock22/round-3/tracer-green. The missing private ra/lock-reservation.tsv caused its harness assertion to return before injection or terminal wait collection. Gate evidence has one release-boundary event and no release-completed event. No host/transport launch marker exists. A parent read-only kill-0 check found the recorded driver PID no longer live; this is not a substitute for the missing wait status or a process-tree proof.

The diagnosed code defect is in record_test_lock_reservation: the shell passes its PID to a separate Perl helper, but the helper checks that value against its own $$ and records its own $$. The helper therefore refuses the valid driver identity. The next correction should validate the passed driver PID against the preserved bootstrap identity and emit that driver PID, while retaining actual kernel-descriptor checks. This is a proposed correction, not an executed or verified fix.

All roots round-1 through round-3 stay untouched. The executor reported1540s of3600s aggregate elapsed at its stop; that figure is reported telemetry, not a reconstructed complete launch/wait ledger. Zero named rounds remain regardless of time remaining. Preserve missing capture/ledger facts as missing; do not manufacture terminal results or reuse a root. The current harness also needs failure-path owned-child cleanup and terminal-result capture before another accepted tracer run.

## Source and raw identity

Current working-file SHA256 values, independently read by the parent after the terminal handoff:

| Path | SHA256 |
|---|---|
| tests/retroarch/state_session.sh | 4900b533576eceab1b8596f6f47ca2de219c0eb96a8e6134b012b8f89be8c87c |
| tests/retroarch/test_state_session.sh | 0b47312a814e446d974ab3d4add466ed169c83427204f4dc3ffa7baf430db8b9 |
| tests/retroarch/interruption_trace.sh | a899d87ed398cbfff75b965b01a5575912cb9b0696b048ffdd55732d278ee165 |
| README.md | 0e15834f9a456d06058e35eef9aa0a70fea1520c400216e23b236cce754ad3ac |
| include/nesturbator.h | baa12158e284f161d3f7daa7d254d5d17545c43f48b427e8fd071ba390f40f93 |

Round3 launch.manifest and frozen-inputs-before-red.sha256 agree with these driver/harness/trace values. The raw diagnosis.txt has an incorrectly transcribed candidate driver hash; its original bytes are preserved. The authoritative observed driver hash is4900b533576eceab1b8596f6f47ca2de219c0eb96a8e6134b012b8f89be8c87c as recorded above and in the launch manifest. The frozen common oracle digest is1963fbb71f538a7b14ff6c10a679ee09f992528151204b25d219c97c6ecf0c7b. The old mirrored driver digest is0829053b2054025711aa768caf20cd52baf30ce50f3371d2ab04be267fbe8757.

Protected config remains byte-exact SHA2560517388f5d422ae97afdc5dc1ae11bc43ebd481a757424f152ee8af61559ff5e, unstaged and excluded. Original22-NATIVE-REPAIR,22-REVIEW and22-REVIEW-DISPOSITION remain unchanged; review CR-01 is still open. Completed21 and all old/native results retain their existing dispositions.

## Required continuation

Next GSD step: `$gsd-plan-phase 16 --gaps`.

Prefer a scoped substantive amendment of22: reconcile these uncommitted five-file edits and three consumed rounds; correct the concrete PID evidence-writer defect and failure-path terminal collection; audit all active requirements before freezing another harness; then define a fresh bounded root/allowance for a source-bound old/new pair and remaining lifecycle expansion. Do not reset an old slot, rerun unchanged inputs or replay completed21/original22 tasks.

The existing new formal slots remain unused, not accepted: hygiene, registered CI pair, current ASan configure/build and registered pair, full normal649 unique/no skips/jobs4/four stages, three fresh ZIP checks, and one independent exact-committed standard review/open0. Every configured parent build/post-wave/execute:post/code-review/full-workflow repeat remains separately mandatory before07. Native exact-final-head CI, affirmative exact-head consent, publication and08 actual released-byte evidence remain pending. No timeout/registration/architecture/requirement change is authorized by this checkpoint.
