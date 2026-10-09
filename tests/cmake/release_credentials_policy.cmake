cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED ROOT)
  message(FATAL_ERROR "release_credentials_policy: pass -DROOT=<repository root>")
endif()

function(check_policy workflow out_errors)
  set(errors "")
  string(REPLACE "\r\n" "\n" workflow "${workflow}")

  # This credential-bearing workflow must only run for pushes to main. In
  # particular, workflow_dispatch can select a non-default ref whose workflow
  # file is branch-controlled:
  # https://docs.github.com/en/actions/reference/workflows-and-actions/events-that-trigger-workflows#workflow_dispatch
  string(FIND "${workflow}" "\non:\n" on_marker)
  string(FIND "${workflow}" "\npermissions:" permissions_marker)
  if(on_marker LESS 0 OR permissions_marker LESS_EQUAL on_marker)
    string(APPEND errors "workflow-events: cannot isolate top-level on block\n")
  else()
    math(EXPR on_start "${on_marker} + 1")
    math(EXPR on_length "${permissions_marker} - ${on_start}")
    string(SUBSTRING "${workflow}" ${on_start} ${on_length} event_block)
    string(REGEX REPLACE "\n[ \t]+" "\n" event_block "${event_block}")
    string(STRIP "${event_block}" event_block)
    if(NOT event_block STREQUAL "on:\npush:\nbranches: [main]")
      string(APPEND errors "workflow-events: expected push to main only; got:\n${event_block}\n")
    endif()
  endif()

  string(FIND "${workflow}" "\npermissions: {}\n" root_permissions)
  if(root_permissions LESS 0)
    string(APPEND errors "root-permissions: workflow-level GITHUB_TOKEN permissions must be empty\n")
  endif()

  string(FIND "${workflow}" "\njobs:\n" jobs_marker)
  string(FIND "${workflow}" "\n  ci:\n" ci_marker)
  if(jobs_marker LESS 0 OR ci_marker LESS_EQUAL jobs_marker)
    string(APPEND errors "release-job: cannot isolate release-please job\n")
    set(release_job "")
  else()
    math(EXPR jobs_start "${jobs_marker} + 1")
    math(EXPR jobs_length "${ci_marker} - ${jobs_start}")
    string(SUBSTRING "${workflow}" ${jobs_start} ${jobs_length} release_job)
  endif()

  string(REGEX MATCHALL "NESTURBATOR_APP_PRIVATE_KEY" private_key_refs "${workflow}")
  list(LENGTH private_key_refs private_key_count)
  if(NOT private_key_count EQUAL 1 OR
     NOT release_job MATCHES "NESTURBATOR_APP_PRIVATE_KEY")
    string(APPEND errors "private-key-scope: private key must occur once, inside release-please job\n")
  endif()

  string(REGEX MATCHALL "NESTURBATOR_APP_ID" app_id_refs "${workflow}")
  list(LENGTH app_id_refs app_id_count)
  if(NOT app_id_count EQUAL 1 OR NOT release_job MATCHES "NESTURBATOR_APP_ID")
    string(APPEND errors "app-id-scope: App ID must occur once, inside release-please job\n")
  endif()

  string(REGEX MATCHALL "actions/create-github-app-token@" token_actions "${workflow}")
  list(LENGTH token_actions token_action_count)
  if(NOT token_action_count EQUAL 1 OR
     NOT release_job MATCHES "actions/create-github-app-token@")
    string(APPEND errors "token-mint: App token must be minted once, inside release-please job\n")
  endif()

  string(REGEX MATCHALL "steps[.]app-token[.]outputs[.]token" app_token_refs "${workflow}")
  list(LENGTH app_token_refs app_token_count)
  if(NOT app_token_count EQUAL 2)
    string(APPEND errors "app-token-scope: minted token must have exactly two consumers\n")
  endif()

  set(release_token_input [=[token: ${{ steps.app-token.outputs.token }}]=])
  set(merge_token_env [=[GH_TOKEN: ${{ steps.app-token.outputs.token }}]=])
  set(private_key_input [=[private-key: ${{ secrets.NESTURBATOR_APP_PRIVATE_KEY }}]=])
  if(NOT release_job MATCHES "googleapis/release-please-action@" OR
     NOT release_job MATCHES "Auto-merge the release pull request")
    string(APPEND errors "app-token-consumers: expected release-please and auto-merge only\n")
  endif()
  string(FIND "${release_job}" "${release_token_input}" release_token_position)
  string(FIND "${release_job}" "${merge_token_env}" merge_token_position)
  string(FIND "${release_job}" "${private_key_input}" private_key_position)
  if(release_token_position LESS 0 OR merge_token_position LESS 0 OR
     private_key_position LESS 0)
    string(APPEND errors "app-token-wiring: token/key inputs are not scoped to the two approved steps\n")
  endif()

  set(${out_errors} "${errors}" PARENT_SCOPE)
endfunction()

function(expect_rejected label workflow)
  check_policy("${workflow}" mutation_errors)
  if(mutation_errors STREQUAL "")
    message(FATAL_ERROR "release_credentials_policy self-test: accepted ${label}")
  endif()
endfunction()

file(READ "${ROOT}/.github/workflows/release.yml" workflow)
check_policy("${workflow}" errors)

if(SELFTEST)
  if(NOT errors STREQUAL "")
    message(FATAL_ERROR "release_credentials_policy self-test: current workflow rejected:\n${errors}")
  endif()

  string(REPLACE "\npermissions: {}\n" "\n  workflow_dispatch:\n\npermissions: {}\n"
    dispatch_mutation "${workflow}")
  expect_rejected("manually selected ref" "${dispatch_mutation}")

  set(private_key_ref [=[${{ secrets.NESTURBATOR_APP_PRIVATE_KEY }}]=])
  string(REPLACE "\n  ci:\n" "\n    accidental-secret-use: ${private_key_ref}\n  ci:\n"
    leak_mutation "${workflow}")
  expect_rejected("an extra private-key consumer" "${leak_mutation}")

  message(STATUS "release_credentials_policy: rejects dispatch refs and extra key consumers")
elseif(NOT errors STREQUAL "")
  message(FATAL_ERROR "release_credentials_policy:${errors}")
else()
  message(STATUS "release_credentials_policy: credentials are isolated to protected-main release steps")
endif()
