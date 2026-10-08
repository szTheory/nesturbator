cmake_minimum_required(VERSION 3.25)

foreach(required_env IN ITEMS RUNNER_TEMP GITHUB_ENV)
  if(NOT DEFINED ENV{${required_env}} OR "$ENV{${required_env}}" STREQUAL "")
    message(FATAL_ERROR "${required_env} must be set to prepare the protected-main scoreboard baseline")
  endif()
endforeach()

set(runner_temp "$ENV{RUNNER_TEMP}")
set(github_env "$ENV{GITHUB_ENV}")
if(runner_temp MATCHES "[\r\n]" OR github_env MATCHES "[\r\n]")
  message(FATAL_ERROR "RUNNER_TEMP and GITHUB_ENV must not contain newlines")
endif()
if(NOT IS_DIRECTORY "${runner_temp}")
  message(FATAL_ERROR "RUNNER_TEMP is not an existing directory: ${runner_temp}")
endif()
if(NOT EXISTS "${github_env}" OR IS_DIRECTORY "${github_env}")
  message(FATAL_ERROR "GITHUB_ENV is not an existing file: ${github_env}")
endif()

execute_process(
  COMMAND git fetch --no-tags origin refs/heads/main
  RESULT_VARIABLE fetch_result
  OUTPUT_VARIABLE fetch_stdout
  ERROR_VARIABLE fetch_stderr)
if(NOT "${fetch_result}" STREQUAL "0")
  message(FATAL_ERROR "Could not fetch protected refs/heads/main (${fetch_result}): ${fetch_stderr}")
endif()

execute_process(
  COMMAND git ls-tree FETCH_HEAD -- tests/accuracy/scoreboard.txt
  RESULT_VARIABLE tree_result
  OUTPUT_VARIABLE tree_entry
  ERROR_VARIABLE tree_stderr)
if(NOT "${tree_result}" STREQUAL "0")
  message(FATAL_ERROR "Could not inspect protected-main scoreboard path (${tree_result}): ${tree_stderr}")
endif()

set(baseline_file "${runner_temp}/nesturbator-scoreboard-main.txt")
if(tree_entry STREQUAL "")
  file(WRITE "${baseline_file}" "")
else()
  if(NOT tree_entry MATCHES "^[0-7]+ blob [0-9a-f]+\ttests/accuracy/scoreboard[.]txt\n$")
    message(FATAL_ERROR "Protected-main scoreboard path lookup returned an unexpected tree entry: ${tree_entry}")
  endif()
  execute_process(
    COMMAND git show FETCH_HEAD:tests/accuracy/scoreboard.txt
    RESULT_VARIABLE show_result
    OUTPUT_FILE "${baseline_file}"
    ERROR_VARIABLE show_stderr)
  if(NOT "${show_result}" STREQUAL "0")
    message(FATAL_ERROR "Could not retrieve protected-main scoreboard bytes (${show_result}): ${show_stderr}")
  endif()
endif()

if(NOT EXISTS "${baseline_file}" OR IS_DIRECTORY "${baseline_file}")
  message(FATAL_ERROR "Protected-main scoreboard baseline was not created: ${baseline_file}")
endif()
file(APPEND "${github_env}" "NESTURBATOR_SCOREBOARD_BASELINE=${baseline_file}\n")
