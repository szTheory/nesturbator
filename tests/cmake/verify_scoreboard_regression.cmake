cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SCOREBOARD_TEST OR NOT DEFINED REGRESSION_BASELINE)
  message(FATAL_ERROR "scoreboard regression test requires its executable and baseline")
endif()

execute_process(
  COMMAND "${SCOREBOARD_TEST}" "${REGRESSION_BASELINE}"
  RESULT_VARIABLE result
  OUTPUT_VARIABLE output
  ERROR_VARIABLE error)
if("${result}" STREQUAL "0")
  message(FATAL_ERROR "assertion failed: the candidate accepted a scoreboard that lost a prior PASS")
endif()
if(NOT error MATCHES "scoreboard lost protected PASS: accuracycoin/Removed")
  message(FATAL_ERROR "scoreboard regression failed for the wrong reason (${result}): ${error}${output}")
endif()
