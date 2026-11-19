---
phase: 30-mmc2-mainline-compatibility-and-delivery
reviewed: 2026-11-19T22:39:19Z
depth: standard
diff_base: 8923b66cca16739c5b402643798515cf5537ae22
reviewed_head: 48f6f054ed363ea50b5da0bfe96f960230f673b7
files_reviewed: 27
files_reviewed_list:
  - CMakeLists.txt
  - README.md
  - docs/provider-ntsc.json
  - include/nesturbator.h
  - runner/main.c
  - src/cartridge.c
  - src/mapper.h
  - src/mapper_mmc2.c
  - src/ppu.c
  - tests/CMakeLists.txt
  - tests/accuracy/input-binding-v1.json
  - tests/cmake/accuracycoin_activation.cmake
  - tests/cmake/accuracycoin_input_fingerprint.cmake
  - tests/cmake/accuracycoin_input_fingerprint_controls.cmake
  - tests/cmake/info_claims.cmake
  - tests/cmake/phase21_hosted_evidence_controls.cmake
  - tests/cmake/provider_profile.cmake
  - tests/cmake/vector_api_policy.cmake
  - tests/core/test_api.c
  - tests/core/test_cartridge.c
  - tests/core/test_mapper_mmc2.c
  - tests/core/test_state_reject.c
  - tests/header/header_c.c
  - tests/header/header_cxx.cpp
  - tests/libretro/libretro_host.c
  - tests/mmc2_fixture.h
  - tests/runner/hashes.txt
findings:
  critical: 0
  warning: 0
  info: 0
  total: 0
  necessity: 0
status: clean
---

# Phase 30: Code Review Report

## Summary

Reviewed the full 27-file product scope declared by the union of Plans 30-01, 30-02 and amended 30-03 against the pinned mainline base, with particular attention to the final compatibility-test edits. No actionable BLOCKER or WARNING was established. The earlier escaped-member and restored-PCM-bound fixes remain present. This bounded source review does not establish silicon accuracy or PR, mainline, package publication or consumer acceptance.

The reviewed tree was clean at the recorded head before writing this report. No project agent skills were configured. Necessity guidance was read; no necessity finding was established. No tests, campaigns, builds or workflows were run by this reviewer. Read-only source comparisons and hash calculations below are independent of the successful third workflow reported by the orchestrator. Planning checkpoint prose still describes the earlier pre-run state and is not used as acceptance evidence.

## Narrative Findings (AI reviewer)

No current findings.

## Resolved findings

The initial escaped-duplicate-member warning is resolved. `tests/cmake/accuracycoin_input_fingerprint.cmake:168` now rejects escaped member names before JSON decoding. The standalone controls include escaped duplicate top-level and current-input path members at `tests/cmake/accuracycoin_input_fingerprint_controls.cmake:212`. The fixed ASCII schema makes this rejection compatible with every legitimate member name. The initial CMake probe demonstrated why the former literal-key counting guard was insufficient; no acceptance run was used for that probe.

The initial restored-libretro audio-bound warning is resolved. The sample loop at `tests/libretro/libretro_host.c:4130` now requires both equality with the known direct sample count and a count within `AUDIO_CAPTURE_CAPACITY`. Direct-library restored execution also exits through cleanup on an oversized output count at line 4160 before sample comparison or PCM hashing. Cold direct execution already has the corresponding capacity check before its hash calls. The callbacks' counts remain assertions, so an invalid count still fails the test even when sample traversal is skipped.

## Follow-up scope and evidence

The final compatibility changes preserve their intended checks:

- `tests/cmake/phase21_hosted_evidence_controls.cmake:14-19` still requires the protected minimum and target-key count, and now requires revision 10. Its wrapper reads the revision from the current public header (`tests/cmake/phase21_hosted_evidence.cmake:24-30`); historical campaign revision/provenance is not rewritten.
- `tests/cmake/vector_api_policy.cmake:6-23` retains the declaration-stream digest check and its added-declaration negative at lines 35-37. An independent read-only normalization reproduced the new digest. Comparing the baseline header after changing only revision 9 to 10 and state format 3 to 4 produced exactly the current declaration stream: the digest refresh hides no interface change.
- `tests/cmake/info_claims.cmake:154-165` selects the canonical MMC3 clause explicitly, allowing the separate MMC2 clause while continuing to reject repeated canonical MMC3 clauses. Mapper, submapper, MMC6 and four-screen checks remain at lines 195-223. The unsupported-board control at lines 493-495 now expects the missing-MMC3 diagnostic. Duplicate/contradiction negatives for metadata, README and header remain at lines 514-525.
- Both public-header smoke sources pin format 4 at `tests/header/header_c.c:28` and `tests/header/header_cxx.cpp:28`.

The earlier runtime and evidence conclusions remain supported by the actual source:

- `src/cartridge.c:103-107,224-228` restricts MMC2 to the selected NES 2.0 submapper-0 ROM geometry with no trainer/RAM. `src/mapper_mmc2.c:34-62,101-127` keeps the selected PRG/mirroring and read-before-latch model. The hook is called only for watched pattern reads at `src/ppu.c:473-480`; renderer and buffered PPUDATA reads reach that boundary. Existing boards retain their unwatched path.
- `src/mapper_mmc2.c:129-137` visits all eight bounded bytes. `src/state.c:543-574` refuses incompatible versions, identities, lengths, checksums and invalid body values before APPLY; map reconstruction at lines 605-610 restores the new watch bit and derived banks without resetting the latches. The direct tests cover joint latch states, poisoned derived state and transactional corrupt-byte refusal at `tests/core/test_mapper_mmc2.c:855-1083`.
- `tests/libretro/libretro_host.c:3963-4244` exercises actual video and nonzero PCM, cold and restored execution, serialized state equality and the real runner/module. The restored callback loop at lines 4130-4139 is bounded by both known direct count and capture capacity; restored direct output exits at lines 4159-4161 before oversized traversal or hashing. The owned marker tests provide an oracle beyond shared-core parity.
- Recomputed all seven admitted product hashes, both external checker hashes and the registration hash in `tests/accuracy/input-binding-v1.json`; all match the current source, using only the existing release-version normalization for the header. Every historical binding field outside the explicitly current-source fields is unchanged against the base. The pinned historical manifest is authenticated before its rows are consumed (`tests/cmake/accuracycoin_input_fingerprint.cmake:338-355`); exact closure/current-byte checks and metadata/report freshness remain at lines 358-466. The escaped-member precheck and duplicate/mutation/freshness negatives remain intact.
- README, public-header and runner changes retain ABI 1, revision 10, format 4 and the bounded mapper-9 profile. Only serialized-state hash rows change in `tests/runner/hashes.txt`; existing frame/audio rows and state sizes remain unchanged. No title, silicon, broad mapper-9, MMC4, receipt or installed-consumer qualification is asserted by the new support clauses.

The unrelated coordination document visible in the complete Git delta is outside the explicit 27-file plan scope and is not counted as reviewed. Existing stale internal hook comments are inherited documentation debt; they do not alter runtime dispatch or the public support boundary.

## Compiler and formatting follow-up

At 2026-11-19T22:59:13Z, reviewed only the uncommitted three-file follow-on diff against head `146e75c50ddaf825b0ca83e8c8ba95e336619340`: `src/cartridge.c`, `tests/core/test_mapper_mmc2.c` and `tests/mmc2_fixture.h`. The orchestrator supplied the exact remote diagnostics: MSVC C4310 for the complemented masks, GCC stringop-overflow for the potentially underflowing trainer length, a conversion warning for the mirror assignment, and clang-format 18 initializer alignment rejection. The reviewer did not independently retrieve those remote logs or execute compiler, formatting or test checks. No BLOCKER or WARNING was established in these edits.

- `src/cartridge.c:376` explicitly converts a ternary whose only possible values are 0 and 1 to `uint8_t`. The cast is lossless, removes the reported narrowing diagnostic and preserves header-derived mirroring for every input byte.
- `tests/core/test_mapper_mmc2.c:145-148` replaces `(uint8_t)~0x0cu` with `0xf3u`. Both preserve exactly the same eight-bit mask. This removes the out-of-range constant conversion MSVC rejects without changing the invalid iNES/archaic fixtures or their refusal assertions.
- `tests/core/test_mapper_mmc2.c:139-141` rejects lengths below the 16-byte header and lengths whose required trainer/trailing capacity calculation would overflow before evaluating that addition. The trainer `memmove` length can therefore no longer underflow, and its destination fits the checked capacity. The actual caller supplies the same fixed valid fixture and sufficient capacity; every existing negative case still proceeds to its original assertion.
- `tests/mmc2_fixture.h:25-62` limits clang-format exclusion to the instruction/operand/comment table, restores formatting immediately afterward and changes no initializer bytes. C17 supports these line comments. The exclusion preserves useful instruction grouping; it does not remove the file from repository format checks or alter a behavior assertion.

Independently verified the subsequent binding edit changes only `current_input_hashes.src/cartridge.c`, its SHA-256 matches the current source, and all historical binding fields remain unchanged.

These are source-level remedies for the supplied diagnostics, not proof that hosted jobs now pass. The production cast changes measured source bytes even though its behavior is identical; the orchestrator must refresh the exact current-source admission and candidate identity through the existing evidence process before relying on new acceptance. The prior accepted candidate's evidence remains bound to its original source.

---

Reviewed read-only; only this report was created. Product and evidence source files were not modified by the reviewer.
