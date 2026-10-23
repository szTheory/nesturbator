cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED CONFORMANCE OR NOT DEFINED RUNNER OR NOT DEFINED ROM OR
   NOT DEFINED WORK OR NOT DEFINED SOURCE_DIR)
  message(FATAL_ERROR "accuracycoin.driver needs CONFORMANCE, RUNNER, ROM, WORK and SOURCE_DIR")
endif()

execute_process(COMMAND "${CONFORMANCE}" driver RESULT_VARIABLE driver_result
  OUTPUT_VARIABLE driver_stdout ERROR_VARIABLE driver_stderr)
if(NOT "${driver_result}" STREQUAL "0")
  message(FATAL_ERROR "start-attribution controls failed (${driver_result}): ${driver_stderr}")
endif()

file(MAKE_DIRECTORY "${WORK}")
foreach(cap IN ITEMS 12 13 14)
  set(case_dir "${WORK}/cap-${cap}")
  file(MAKE_DIRECTORY "${case_dir}")
  file(REMOVE "${case_dir}/raw.tsv" "${case_dir}/diagnostics.tsv"
    "${case_dir}/report.tsv" "${case_dir}/metadata.txt")
  math(EXPR total "32 + 5 * 2 + 2 + ${cap}")
  execute_process(
    COMMAND "${RUNNER}" --accuracycoin-observe 3 --rom "${ROM}" --frames "${total}"
      --accuracycoin-boot-cap 32 --accuracycoin-navigation-cap 5
      --accuracycoin-execution-cap "${cap}" --accuracycoin-output "${case_dir}"
    WORKING_DIRECTORY "${SOURCE_DIR}" TIMEOUT 10
    RESULT_VARIABLE result OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr)
  if(NOT "${result}" STREQUAL "0")
    message(FATAL_ERROR "page-3 cap ${cap} observation failed (${result}): ${stderr}")
  endif()
  file(READ "${case_dir}/metadata.txt" metadata)
  if(cap EQUAL 12)
    if(NOT metadata MATCHES "verdict=guest-timeout" OR
       NOT metadata MATCHES "stop_cause=execution-cap-active-test" OR
       NOT metadata MATCHES "active_raw=0x03")
      message(FATAL_ERROR "cap-1 did not preserve a real active timeout: ${metadata}")
    endif()
  else()
    if(NOT metadata MATCHES "verdict=complete")
      message(FATAL_ERROR "cap ${cap} failed to recognize a terminal result: ${metadata}")
    endif()
  endif()
  if(cap EQUAL 13 AND NOT metadata MATCHES "execution_frames=13\n")
    message(FATAL_ERROR "completion on the cap frame was not counted: ${metadata}")
  endif()
endforeach()

set(controller_dir "${WORK}/controller")
file(MAKE_DIRECTORY "${controller_dir}")
file(REMOVE "${controller_dir}/raw.tsv" "${controller_dir}/diagnostics.tsv"
  "${controller_dir}/report.tsv" "${controller_dir}/metadata.txt")
execute_process(
  COMMAND "${RUNNER}" --accuracycoin-observe 14 --rom "${ROM}" --frames 1899
    --accuracycoin-boot-cap 32 --accuracycoin-navigation-cap 5
    --accuracycoin-execution-cap 1800 --accuracycoin-output "${controller_dir}"
  WORKING_DIRECTORY "${SOURCE_DIR}" TIMEOUT 10
  RESULT_VARIABLE controller_result OUTPUT_VARIABLE controller_stdout
  ERROR_VARIABLE controller_stderr)
if(NOT "${controller_result}" STREQUAL "0")
  message(FATAL_ERROR "controller-sensitive page observation failed: ${controller_stderr}")
endif()
file(READ "${controller_dir}/metadata.txt" controller_metadata)
file(READ "${controller_dir}/report.tsv" controller_report)
string(FIND "${controller_report}" "accuracycoin/Controller Clocking\t14" controller_test)
if(NOT controller_metadata MATCHES "start_attributed=1" OR
   NOT controller_metadata MATCHES "a_press_observed=1" OR
   NOT controller_metadata MATCHES "a_release_observed=1" OR
   NOT controller_metadata MATCHES "release_frames=1" OR controller_test EQUAL -1)
  message(FATAL_ERROR "controller run lacked attributed start/release evidence: ${controller_metadata}")
endif()

set(draw_dir "${WORK}/draw-page16")
file(MAKE_DIRECTORY "${draw_dir}")
execute_process(
  COMMAND "${RUNNER}" --accuracycoin-observe 16 --rom "${ROM}" --frames 1909
    --accuracycoin-boot-cap 32 --accuracycoin-navigation-cap 5
    --accuracycoin-execution-cap 1800 --accuracycoin-output "${draw_dir}"
  WORKING_DIRECTORY "${SOURCE_DIR}" TIMEOUT 10
  RESULT_VARIABLE draw_result OUTPUT_VARIABLE draw_stdout ERROR_VARIABLE draw_stderr)
if(NOT "${draw_result}" STREQUAL "0")
  message(FATAL_ERROR "DRAW-only page observation failed: ${draw_stderr}")
endif()
file(READ "${draw_dir}/metadata.txt" draw_metadata)
if(NOT draw_metadata MATCHES "start_attributed=1" OR
   NOT draw_metadata MATCHES "result_transition_observed=0" OR
   NOT draw_metadata MATCHES "draw_cursor_transition_observed=1" OR
   NOT draw_metadata MATCHES "menu_returned_to_header_observed=1" OR
   NOT draw_metadata MATCHES "execution_frames=15\n")
  message(FATAL_ERROR "DRAW-only start/completion lacked source-defined cursor evidence: ${draw_metadata}")
endif()

set(active_dir "${WORK}/active-page19")
file(MAKE_DIRECTORY "${active_dir}")
execute_process(
  COMMAND "${RUNNER}" --accuracycoin-observe 19 --rom "${ROM}" --frames 1924
    --accuracycoin-boot-cap 32 --accuracycoin-navigation-cap 5
    --accuracycoin-execution-cap 1800 --accuracycoin-output "${active_dir}"
  WORKING_DIRECTORY "${SOURCE_DIR}" TIMEOUT 10
  RESULT_VARIABLE active_result OUTPUT_VARIABLE active_stdout ERROR_VARIABLE active_stderr)
if(NOT "${active_result}" STREQUAL "0")
  message(FATAL_ERROR "source-qualified active-row observation failed: ${active_stderr}")
endif()
file(READ "${active_dir}/metadata.txt" active_metadata)
file(READ "${active_dir}/report.tsv" active_report)
file(STRINGS "${active_dir}/diagnostics.tsv" active_diagnostic_lines)
list(GET active_diagnostic_lines -1 active_last_diagnostic)
string(ASCII 9 TAB)
string(REPLACE "${TAB}" ";" active_last_diagnostic "${active_last_diagnostic}")
string(FIND "${active_report}"
  "accuracycoin/Arbitrary Sprite zero${TAB}19${TAB}6${TAB}timeout${TAB}1${TAB}0x03" active_row)
string(FIND "${active_report}"
  "accuracycoin/Misaligned OAM behavior${TAB}19${TAB}7${TAB}not-run${TAB}1${TAB}0x03${TAB}1800${TAB}noncurrent-running-marker" stale_marker)
if(NOT active_metadata MATCHES "verdict=guest-timeout" OR
   NOT active_metadata MATCHES "stop_cause=execution-cap-active-test" OR
   NOT active_metadata MATCHES "active_key=accuracycoin/Arbitrary Sprite zero" OR
   NOT active_metadata MATCHES "active_raw=0x03" OR
   NOT active_last_diagnostic MATCHES "^observe;[0-9]+;10;18;5;1;0;1$" OR
   active_row EQUAL -1 OR stale_marker EQUAL -1)
  message(FATAL_ERROR "page-19 active identity was not tied to source state: ${active_metadata}\n${active_report}")
endif()
