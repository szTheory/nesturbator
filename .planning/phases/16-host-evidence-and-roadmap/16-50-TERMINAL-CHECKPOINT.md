# Plan 50 — terminal parent receive-wrapper syntax failure

Recorded: 2026-10-22T17:24:46Z. Invocation: `gsd-execute-phase 16 --gaps-only`.
Input HEAD: `783b56d6f96609769476ac79afeb85e902e1f9c6`.
Branch: `gsd/phase-16-host-evidence-and-roadmap`.

Plan50 stops incomplete with **0/2 tasks accepted**, before any bootstrap,
helper write, executor dispatch or runtime action. The parent constructed an
in-memory retained-prefix data reader and independent receiver, then invoked:

```javascript
// @exec: {"max_output_tokens": 18000}
await eval(load("p50-data-next"));
```

The stored evaluated source contained top-level `await` without an async-IIFE.
Direct eval in this context rejected the source during parsing:

```text
SyntaxError: await is only valid in async functions and the top level bodies of modules
```

The parent introduced this wrapper defect. The nested Node READ request never
ran, so there is no actual READ result, packet reception, receiver commit or
ACK. This is an outer JavaScript syntax error, not a producer logical refusal,
OS exit, failed nonempty B, transport-bound measurement or emulator defect.
No prospective A/B source or operation was dispatched. The terminal prewrite
rule in16-51-AMENDMENT forbids repairing/retrying this attempt, a suffix/root
fallback or dispatch based on a DAG-ready label. Only administrative records
follow the stop. A subsequent planning amendment must resolve the wrapper and
retain every existing proof and acceptance duty; this record grants none.

## Actual observations before the failure

The exact50budget setup and second probe ran with explicit outer/nested18000.
The submitted second invocation matched its retained descriptor. The actual
exec probe returned exit0 and exact ten bytes123456789+LF, with no session.
Five synthetic budget cases passed; hidden effective provider budget remains
unavailable. These observations grant no snapshot or first-write authority.

A separate readonly persistent-Node capability request returned its expected
small frame, including exact bigint-to-string conversion. Its full actual
object/request was retained before validation. Node exposes no token parameter.
The historical literal mode oracle passed4positive/6negative cases using old
envelopes as DATA, with fresh_acceptance=false. Historical source strings and
helper CLIs were never executed. The new codec's empty/abc SHA256 construction
checks were in-memory synthetic checks only, not phase transport acceptance.

The data-reader, receiver and codec drafts were retained in caller memory.
No complete prospective producer/A-B/decoder/graph source closure had been
finalized or accepted. No actual retained38prefix was transported to the
independent receiver. Administrative direct readback later confirmed its
original13136bytes/SHA256
`5d9dc44bca2c69a9d4e5ac248ae1d9942c4cafd9ee4e208902b6b87695d9b195`;
that readback is DATA/preservation evidence, never replacement reception.

## Retained evidence and provenance limits

[The administrative archive](16-50-TERMINAL-PROVENANCE.json) is46008bytes,
SHA256`5f27ed535425188a737fca065a2ed15f86545c1e7716458ddd7f1a738cfa4c5d`.
It retains the exact failed evaluated wrapper, intended nested request,
unexecuted data-reader/receiver/codec drafts, actual budget/capability/baseline
request/results, prior descriptor, terminal observations and administrative
preservation result. Original returned objects and their immutable exact-field
serializations are retained where exposed. The outer functions.exec error was
visible only in the transcript; no programmatic original outer-result object
was exposed to the caller. Its observed text is explicitly labeled, not
manufactured into a nested actual receipt. The setup's visible original tool
input remains transcript provenance; the archive retains its probe source and
next descriptor, not a fabricated complete setup-call original.

The administrative Python result's original stdout retains exact decimal
nanosecond/inode fields. Its convenience JSON-parsed JavaScript metadata cache
rounds nanoseconds beyond Number precision; that cache is not exact integer
evidence. Administrative verification reparses the original stdout in Python
and compares those exact integers to lstat. No canonical snapshot was produced.

There is no actual nested READ result to reconstruct. No fresh complete
snapshot, READ/COMPARE stream, finite metadata/prompt/dispatch/header/archive/
seal graph, bootstrap, helper import/repair, CLI RED/matrix/export/freeze,
PREPARATION/SUMMARY, task credit, source edit, formal CI, runtime/native/release
action or actual parent runtime gate occurred.

## Administrative preservation and disposition

After the stop and before administrative writes, a fresh readonly check found:

- All six original dirty-file hashes equal the retained baseline, all unstaged.
- Input HEAD/branch unchanged, raw index SHA256
  `c3a4972269ead5d20c9a9b441afa26c59c43de0ac94cf2ba65e8383ad40df640`, index empty.
- All15execution roots for50–54 absent, all10PREPARATION/SUMMARY artifacts absent.
- Four mode000 files statted as metadata only, without content reads or chmod.
- The retained38journal retains its exact original length/SHA.

This is not a complete historical before/after inventory comparison. Plan26
active terminal=false/pending diagnosis, all historical UNKNOWNs, four flagged
assumptions and metadata-only limitations remain. Historical plan bodies and
earlier terminal records are unchanged.

Only this checkpoint/archive and parent STATE/ROADMAP updates are administrative
outputs of the invoked execute step. No success-only SUMMARY is written. Counts
remain54physical/17live/10complete;50terminal0/2,51unexecuted0/2 and52/53/54
unexecuted0/2,0/3,0/2. Verification remains gaps_found. All later separate
preparation/runtime/parent/native/exact-head consent/normal publication/actual
released-byte gates remain blocking. Next: `$gsd-plan-phase 16 --gaps`.
Stop after this execute step; no next lifecycle command is invoked automatically.
