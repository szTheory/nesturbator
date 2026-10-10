cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED ROOT)
  message(FATAL_ERROR "release_policy: pass -DROOT=<repository root>")
endif()

# Canonical gate lines. Edit together with .github/workflows/release.yml: the
# check is exact, so any extra condition or reformatting fails on purpose.
set(RELEASE_PUBLISH_NEEDS "    needs: [release-please, ci]")
set(RELEASE_PUBLISH_IF "    if: needs.release-please.outputs.release_created == 'true'")
set(RELEASE_CI_NEEDS "    needs: release-please")
set(RELEASE_CI_IF "    if: needs.release-please.outputs.release_created == 'true'")

# Keep ';', '[' and ']' from splitting or merging list elements (a shell line
# such as `for c in a b; do` must stay one line).
function(encode_text text out_var)
  string(REPLACE ";" "<SEMI>" text "${text}")
  string(REPLACE "[" "<LB>" text "${text}")
  string(REPLACE "]" "<RB>" text "${text}")
  set(${out_var} "${text}" PARENT_SCOPE)
endfunction()

# Comment-stripped, trailing-whitespace-trimmed, encoded line list. A YAML
# comment starts at a '#' in column 0 or after a space or tab, so `${TAG#v}`
# survives.
function(workflow_lines text out_var)
  string(REPLACE "\r\n" "\n" text "${text}")
  encode_text("${text}" text)
  string(REPLACE "\n" ";" raw "${text}")
  set(result "")
  foreach(line IN LISTS raw)
    string(REGEX REPLACE "(^|[ \t])#.*$" "\\1" line "${line}")
    string(REGEX REPLACE "[ \t]+$" "" line "${line}")
    if(NOT line STREQUAL "")
      list(APPEND result "${line}")
    endif()
  endforeach()
  set(${out_var} "${result}" PARENT_SCOPE)
endfunction()

# Lines of one job: from its two-space key up to the next two-space key.
function(job_lines lines job out_var)
  set(result "")
  set(in_job FALSE)
  foreach(line IN LISTS lines)
    if(line STREQUAL "  ${job}:")
      set(in_job TRUE)
      continue()
    endif()
    if(in_job AND line MATCHES "^  [A-Za-z0-9_-]+:$")
      break()
    endif()
    if(in_job)
      list(APPEND result "${line}")
    endif()
  endforeach()
  set(${out_var} "${result}" PARENT_SCOPE)
endfunction()

# The job's needs: and if: lines each appear once and equal the canonical text.
function(check_gate job_body label needs if_line out_errors)
  encode_text("${needs}" needs)
  encode_text("${if_line}" if_line)
  set(errors "")
  set(needs_count 0)
  set(if_count 0)
  foreach(line IN LISTS job_body)
    if(line MATCHES "^    needs:")
      math(EXPR needs_count "${needs_count} + 1")
      if(NOT line STREQUAL needs)
        string(APPEND errors "${label}-gate: needs line must equal the canonical text\n")
      endif()
    elseif(line MATCHES "^    if:")
      math(EXPR if_count "${if_count} + 1")
      if(NOT line STREQUAL if_line)
        string(APPEND errors "${label}-gate: if line must equal the canonical text\n")
      endif()
    endif()
  endforeach()
  if(NOT needs_count EQUAL 1 OR NOT if_count EQUAL 1)
    string(APPEND errors "${label}-gate: needs and if must each appear exactly once\n")
  endif()
  set(${out_errors} "${errors}" PARENT_SCOPE)
endfunction()

function(commit_triggers_release type breaking out)
  set(releasing_types feat fix perf revert)
  if(breaking OR type IN_LIST releasing_types)
    set(${out} TRUE PARENT_SCOPE)
  else()
    set(${out} FALSE PARENT_SCOPE)
  endif()
endfunction()

function(check_policy config workflow out_errors)
  set(errors "")

  string(JSON release_type ERROR_VARIABLE release_type_error
    GET "${config}" packages . release-type)
  if(release_type_error OR NOT release_type STREQUAL "simple")
    string(APPEND errors "release-type: expected the pinned simple strategy\n")
  endif()

  # The pinned release-please 17.6.0 filter hides docs/chore (and the other
  # non-release types):
  # https://github.com/googleapis/release-please/blob/v17.6.0/src/util/filter-commits.ts
  # A changelog-sections override can accidentally make docs/chore commits
  # release-triggering, so force review if added.
  string(FIND "${config}" "\"changelog-sections\"" changelog_override)
  if(NOT changelog_override LESS 0)
    string(APPEND errors "changelog-sections: custom sections require a fresh release-policy review\n")
  endif()

  string(FIND "${workflow}"
    "googleapis/release-please-action@45996ed1f6d02564a971a2fa1b5860e934307cf7"
    action_pin)
  if(action_pin LESS 0)
    string(APPEND errors "release-action-pin: expected release-please-action v5.0.0 (bundled 17.6.0)\n")
  endif()

  workflow_lines("${workflow}" lines)
  job_lines("${lines}" publish publish_lines)
  job_lines("${lines}" ci ci_lines)

  if("${publish_lines}" STREQUAL "")
    string(APPEND errors "publish-job: missing release publication job\n")
  else()
    check_gate("${publish_lines}" publish "${RELEASE_PUBLISH_NEEDS}"
      "${RELEASE_PUBLISH_IF}" gate_errors)
    string(APPEND errors "${gate_errors}")
  endif()
  if("${ci_lines}" STREQUAL "")
    string(APPEND errors "ci-job: missing release CI job\n")
  else()
    check_gate("${ci_lines}" ci "${RELEASE_CI_NEEDS}" "${RELEASE_CI_IF}" gate_errors)
    string(APPEND errors "${gate_errors}")
  endif()

  # Nothing inside publish may weaken, skip or hide the gate.
  encode_text("${RELEASE_PUBLISH_IF}" canonical_publish_if)
  foreach(line IN LISTS publish_lines)
    if(line MATCHES "continue-on-error")
      string(APPEND errors "publish-continue-on-error: publication must not tolerate failures\n")
    endif()
    if(line MATCHES "^ +(- )?if:" AND NOT line STREQUAL canonical_publish_if)
      string(APPEND errors "publish-step-if: publish steps must not carry their own condition\n")
    endif()
    if(line MATCHES "(^|[ ])(-|[A-Za-z0-9_-]+:)[ ]+[&*][A-Za-z0-9_-]" OR line MATCHES "<<:")
      string(APPEND errors "publish-anchor: YAML anchors, aliases and merge keys are not allowed in publish\n")
    endif()
  endforeach()

  # Exactly one job makes the release public, and it is publish.
  set(edit_count 0)
  set(edit_outside FALSE)
  foreach(line IN LISTS lines)
    if(line MATCHES "release edit" AND line MATCHES "--draft=false")
      math(EXPR edit_count "${edit_count} + 1")
      list(FIND publish_lines "${line}" edit_index)
      if(edit_index LESS 0)
        set(edit_outside TRUE)
      endif()
    endif()
  endforeach()
  if(NOT edit_count EQUAL 1 OR edit_outside)
    string(APPEND errors "publish-unique: only the publish job may run the --draft=false release edit, once\n")
  endif()

  set(${out_errors} "${errors}" PARENT_SCOPE)
endfunction()

function(expect_rejected label config workflow)
  check_policy("${config}" "${workflow}" mutation_errors)
  if(mutation_errors STREQUAL "")
    message(FATAL_ERROR "release_policy self-test: accepted ${label}")
  endif()
endfunction()

# A mutation that changes nothing would pass the self-test silently.
function(mutate out label find replace)
  string(REPLACE "${find}" "${replace}" result "${workflow}")
  if(result STREQUAL workflow)
    message(FATAL_ERROR "release_policy self-test: mutation '${label}' changed nothing")
  endif()
  set(${out} "${result}" PARENT_SCOPE)
endfunction()

file(READ "${ROOT}/release-please-config.json" config)
file(READ "${ROOT}/.github/workflows/release.yml" workflow)
check_policy("${config}" "${workflow}" errors)

if(SELFTEST)
  if(NOT errors STREQUAL "")
    message(FATAL_ERROR "release_policy self-test: current configuration rejected:\n${errors}")
  endif()

  foreach(type IN ITEMS docs chore ci test build refactor style)
    commit_triggers_release("${type}" FALSE triggers_release)
    if(triggers_release)
      message(FATAL_ERROR "release_policy self-test: ${type} commit unexpectedly triggers release")
    endif()
  endforeach()
  foreach(type IN ITEMS feat fix perf revert)
    commit_triggers_release("${type}" FALSE triggers_release)
    if(NOT triggers_release)
      message(FATAL_ERROR "release_policy self-test: ${type} commit failed to trigger release")
    endif()
  endforeach()
  foreach(type IN ITEMS docs chore)
    commit_triggers_release("${type}" TRUE triggers_release)
    if(NOT triggers_release)
      message(FATAL_ERROR "release_policy self-test: breaking ${type} commit was incorrectly suppressed")
    endif()
  endforeach()

  string(REPLACE "\"release-type\": \"simple\","
    "\"release-type\": \"simple\",\n      \"changelog-sections\": [{\"type\": \"docs\", \"section\": \"Docs\"}],"
    override_mutation "${config}")
  expect_rejected("a changelog override" "${override_mutation}" "${workflow}")

  set(gate_condition "needs.release-please.outputs.release_created == 'true'")
  set(publish_needs "    needs: [release-please, ci]\n")
  set(publish_gate "${publish_needs}    if: ${gate_condition}\n")

  mutate(m "appended || always()" "${publish_gate}"
    "${publish_needs}    if: ${gate_condition} || always()\n")
  expect_rejected("a publish gate with || always()" "${config}" "${m}")

  mutate(m "unconditional publish" "${publish_gate}"
    "${publish_needs}    if: always()\n")
  expect_rejected("an unconditional publish job" "${config}" "${m}")

  mutate(m "&& !cancelled()" "${publish_gate}"
    "${publish_needs}    if: ${gate_condition} && !cancelled()\n")
  expect_rejected("a publish gate with && !cancelled()" "${config}" "${m}")

  mutate(m "|| failure()" "${publish_gate}"
    "${publish_needs}    if: ${gate_condition} || failure()\n")
  expect_rejected("a publish gate with || failure()" "${config}" "${m}")

  mutate(m "wrapped if" "${publish_gate}"
    "${publish_needs}    if: \${{ ${gate_condition} }}\n")
  expect_rejected("a publish gate wrapped in an expression" "${config}" "${m}")

  mutate(m "duplicate if" "${publish_gate}" "${publish_gate}    if: ${gate_condition}\n")
  expect_rejected("a duplicated publish if" "${config}" "${m}")

  mutate(m "dropped needs" "${publish_gate}"
    "    needs: [release-please]\n    if: ${gate_condition}\n")
  expect_rejected("a publish job that dropped ci" "${config}" "${m}")

  mutate(m "reordered needs" "${publish_gate}"
    "    needs: [ci, release-please]\n    if: ${gate_condition}\n")
  expect_rejected("reordered publish needs" "${config}" "${m}")

  mutate(m "ci widened" "    needs: release-please\n    if: ${gate_condition}\n"
    "    needs: release-please\n    if: ${gate_condition} || always()\n")
  expect_rejected("a widened ci job condition" "${config}" "${m}")

  mutate(m "step if" "      - uses: actions/download-artifact@3e5f45b2cfb9172054b4087a40e8e0b5a5461e7c # v8.0.1\n"
    "      - uses: actions/download-artifact@3e5f45b2cfb9172054b4087a40e8e0b5a5461e7c # v8.0.1\n        if: always()\n")
  expect_rejected("a step-level if in publish" "${config}" "${m}")

  mutate(m "continue-on-error" "${publish_gate}" "${publish_gate}    continue-on-error: true\n")
  expect_rejected("continue-on-error in publish" "${config}" "${m}")

  mutate(m "second publisher"
    "did not expose the exact 18 archives and SHA256SUMS\"\n          exit 1\n"
    "did not expose the exact 18 archives and SHA256SUMS\"\n          exit 1\n  publish-again:\n${publish_gate}    runs-on: ubuntu-24.04\n    steps:\n      - run: gh release edit \"$TAG\" --draft=false\n")
  expect_rejected("a second job that publishes the release" "${config}" "${m}")

  mutate(m "gate in comments" "${publish_gate}"
    "    # needs: [release-please, ci]\n    # if: ${gate_condition}\n")
  expect_rejected("a gate present only in comments" "${config}" "${m}")

  mutate(m "semicolon" "${publish_gate}"
    "${publish_needs}    if: ${gate_condition} || contains('a;b', 'a')\n")
  expect_rejected("a semicolon in the gate line" "${config}" "${m}")

  mutate(m "anchor" "    env:\n      TAG:" "    env: &publish_env\n      TAG:")
  expect_rejected("a YAML anchor in publish" "${config}" "${m}")

  mutate(m "alias" "    env:\n      TAG:" "    env: *publish_env\n      TAG:")
  expect_rejected("a YAML alias in publish" "${config}" "${m}")

  message(STATUS "release_policy: docs/chore stay non-releasing; behavior and breaking commits release")
elseif(NOT errors STREQUAL "")
  message(FATAL_ERROR "release_policy:${errors}")
else()
  message(STATUS "release_policy: pinned release defaults and publish gate are intact")
endif()
