cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED ROOT)
  message(FATAL_ERROR "release_policy: pass -DROOT=<repository root>")
endif()

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

  string(FIND "${workflow}" "\n  publish:\n" publish_marker)
  if(publish_marker LESS 0)
    string(APPEND errors "publish-job: missing release publication job\n")
  else()
    string(SUBSTRING "${workflow}" ${publish_marker} -1 publish_job)
    string(FIND "${publish_job}" "needs: [release-please, ci]" publish_needs)
    string(FIND "${publish_job}"
      "if: needs.release-please.outputs.release_created == 'true'" release_guard)
    if(publish_needs LESS 0 OR release_guard LESS 0)
      string(APPEND errors "publish-gate: publication must require a created release and successful CI\n")
    endif()
  endif()

  set(${out_errors} "${errors}" PARENT_SCOPE)
endfunction()

function(expect_rejected label config workflow)
  check_policy("${config}" "${workflow}" mutation_errors)
  if(mutation_errors STREQUAL "")
    message(FATAL_ERROR "release_policy self-test: accepted ${label}")
  endif()
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

  string(REPLACE "if: needs.release-please.outputs.release_created == 'true'"
    "if: always()" publish_mutation "${workflow}")
  expect_rejected("an unconditional publish job" "${config}" "${publish_mutation}")

  message(STATUS "release_policy: docs/chore stay non-releasing; behavior and breaking commits release")
elseif(NOT errors STREQUAL "")
  message(FATAL_ERROR "release_policy:${errors}")
else()
  message(STATUS "release_policy: pinned release defaults and publish gate are intact")
endif()
