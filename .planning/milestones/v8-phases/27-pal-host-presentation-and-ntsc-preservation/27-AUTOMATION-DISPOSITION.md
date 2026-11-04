# Phase 27 automation and checkpoint disposition

Recorded 2026-11-03 under the project [verification and CI/CD policy](../../VERIFICATION-POLICY.md).
This disposition replaces three administrative owner prompts with a bounded review of current
phase evidence. It does not resolve the underlying prohibitions, accept hardware claims, or restart
the stopped historical campaign.

| Former prompt | Current disposition | Evidence and limit |
|---|---|---|
| Acknowledge that software AV reports and callbacks do not prove physical host pacing, device behavior, or PAL silicon correctness. | No human UAT required; preserve the prohibition as unresolved. | The phase exposes exact software rates and adapter callbacks. The [acceptance report](27-ACCEPTANCE.md), README, public header, API guide, and testing guide explicitly exclude physical host and hardware claims. The goal does not require physical-device access. |
| Acknowledge that the named 2C07 model is not universal calibration or direct hardware color proof. | No human UAT required; preserve the prohibition as unresolved. | The full PAL table is checked against an independent model digest and samples, and documentation labels it as a model. Current CI passed the palette digest, regeneration, conversion, and adapter checks. No hardware capture or display-preference decision is part of this goal. |
| Acknowledge the no-repin rule and that the older interrupted campaign remains stopped and unknown. | No human UAT required; preserve the prohibition and existing stop. | Fixed NTSC references were unchanged, the current candidate's configured campaign and activation passed, and no old run was resumed. This disposition grants no authority to restart the stopped campaign or reinterpret its result. |

These prompts ask for acknowledgment of limits already locked by the plans and project state. The
plan explicitly says to keep transparency prohibitions flagged and unverified. The owner policy
therefore treats them as preserved boundaries, not as acceptance decisions or manual tests. The
current [verification report](27-VERIFICATION.md) records 16/16 technical truths verified and keeps
all three prohibitions flagged.

The [incremental code review](27-REVIEW.md) also records WR-01 as open: the checked-in baseline
metadata retains the earlier capture identity and path. The fresh current-workflow metadata and
report were verified separately; this disposition does not change the baseline metadata or close
that warning. Keep it visible for the v8 milestone audit.

Reopen a checkpoint only if the required action, evidence, claim, user preference, or authority
changes. The [older interrupted campaign](27-ACCEPTANCE.md) remains stopped and unknown.
