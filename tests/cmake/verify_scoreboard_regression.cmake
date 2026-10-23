cmake_minimum_required(VERSION 3.25)

foreach(required_var IN ITEMS SCOREBOARD_TEST SOURCE_DIR WORK EVIDENCE_ROOT MODE)
  if(NOT DEFINED ${required_var} OR "${${required_var}}" STREQUAL "")
    message(FATAL_ERROR "protected scoreboard control requires ${required_var}")
  endif()
endforeach()

set(baseline "${SOURCE_DIR}/tests/accuracy/scoreboard-main.txt")
set(metadata "${SOURCE_DIR}/tests/accuracy/scoreboard-main-metadata.txt")
set(revision "9f9386c661f6fb8f6063bf756655713aedadae77")
set(scoreboard "${SOURCE_DIR}/tests/accuracy/scoreboard.txt")
set(rom "${SOURCE_DIR}/tests/roms/accuracycoin.nes")
set(manifest "${SOURCE_DIR}/tests/roms/manifest.txt")
set(migration "${SOURCE_DIR}/tests/accuracy/migration.tsv")

if(MODE STREQUAL "no_lost_pass")
  file(REMOVE_RECURSE "${WORK}")
  file(MAKE_DIRECTORY "${WORK}")
  set(mutated_scoreboard "${WORK}/scoreboard.txt")
  file(READ "${scoreboard}" scoreboard_text)
  string(REPLACE "accuracycoin/Absolute Indexed\tpass\t0x01\t-\t-\n" ""
    mutated_text "${scoreboard_text}")
  if(mutated_text STREQUAL scoreboard_text)
    message(FATAL_ERROR "no-lost-pass control could not remove the named protected row")
  endif()
  file(WRITE "${mutated_scoreboard}" "${mutated_text}")
  set(expected "candidate scoreboard lost a protected key")
elseif(MODE STREQUAL "ci_empty_baseline")
  file(REMOVE_RECURSE "${WORK}")
  file(MAKE_DIRECTORY "${WORK}")
  set(empty_baseline "${WORK}/empty-scoreboard.txt")
  file(WRITE "${empty_baseline}" "")
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env CI=true
      "NESTURBATOR_SCOREBOARD_BASELINE=${empty_baseline}"
      "NESTURBATOR_SCOREBOARD_BASELINE_METADATA=${metadata}"
      "NESTURBATOR_SCOREBOARD_BASELINE_REVISION=${revision}"
      "${SCOREBOARD_TEST}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error)
  if("${result}" STREQUAL "0")
    message(FATAL_ERROR "empty protected authority was accepted")
  endif()
  set(combined "${error}${output}")
  if(NOT combined MATCHES "scoreboard is empty")
    message(FATAL_ERROR "empty authority failed for the wrong reason (${result}): ${combined}")
  endif()
  return()
elseif(MODE STREQUAL "ci_missing_baseline")
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env --unset=NESTURBATOR_SCOREBOARD_BASELINE
      --unset=NESTURBATOR_SCOREBOARD_BASELINE_METADATA
      --unset=NESTURBATOR_SCOREBOARD_BASELINE_REVISION
      CI=true "${SCOREBOARD_TEST}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error)
  if("${result}" STREQUAL "0")
    message(FATAL_ERROR "CI accepted a missing protected authority handoff")
  endif()
  set(combined "${error}${output}")
  if(NOT combined MATCHES "CI requires protected scoreboard, metadata, and revision handoff")
    message(FATAL_ERROR "missing authority failed for the wrong reason (${result}): ${combined}")
  endif()
  return()
else()
  message(FATAL_ERROR "unsupported scoreboard control mode: ${MODE}")
endif()

execute_process(
  COMMAND "${SCOREBOARD_TEST}" --verify
    "${baseline}" "${metadata}" "${revision}" "${mutated_scoreboard}"
    "${rom}" "${manifest}" "${migration}" "${EVIDENCE_ROOT}"
  RESULT_VARIABLE result
  OUTPUT_VARIABLE output
  ERROR_VARIABLE error)
if("${result}" STREQUAL "0")
  message(FATAL_ERROR "production verifier accepted a candidate scoreboard that lost a protected PASS")
endif()
set(combined "${error}${output}")
if(NOT combined MATCHES "${expected}")
  message(FATAL_ERROR "protected scoreboard regression failed for the wrong reason (${result}): ${combined}")
endif()
