cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED RUNNER OR NOT DEFINED ROM OR NOT DEFINED WORK OR
   NOT DEFINED SOURCE_DIR)
  message(FATAL_ERROR "accuracycoin campaign needs RUNNER, ROM, WORK and SOURCE_DIR")
endif()
if(NOT DEFINED BOOT_CAP)
  set(BOOT_CAP 120)
endif()
if(NOT DEFINED NAVIGATION_CAP)
  set(NAVIGATION_CAP 5)
endif()
if(NOT DEFINED EXECUTION_CAP)
  set(EXECUTION_CAP 1800)
endif()
if(NOT DEFINED PAGE_TIMEOUT)
  set(PAGE_TIMEOUT 10)
endif()
if(NOT DEFINED COLLECT_TIMEOUT)
  set(COLLECT_TIMEOUT 20)
endif()

file(MAKE_DIRECTORY "${WORK}")
file(REMOVE "${WORK}/outer-outcomes.tsv" "${WORK}/inventory.tsv"
  "${WORK}/attempts.tsv" "${WORK}/observations.tsv" "${WORK}/report.tsv"
  "${WORK}/summary.txt" "${WORK}/metadata.txt" "${WORK}/collection.stdout"
  "${WORK}/collection.stderr")
file(WRITE "${WORK}/outer-outcomes.tsv"
  "page\toutcome\texit_code\tdeadline_seconds\n")

if(DEFINED TEST_TIMEOUT_PAGE OR DEFINED TEST_PARTIAL_FAILURE_PAGE)
  execute_process(COMMAND "${RUNNER}" --accuracycoin-inventory --rom "${ROM}"
    RESULT_VARIABLE inventory_result OUTPUT_VARIABLE inventory_text
    ERROR_VARIABLE inventory_error)
  if(NOT "${inventory_result}" STREQUAL "0")
    message(FATAL_ERROR "cannot obtain pinned test inventory for fault controls: ${inventory_error}")
  endif()
  string(ASCII 9 TAB)
  string(REPLACE "\n" ";" inventory_lines "${inventory_text}")
  set(FAULT_KEY "")
  set(FAULT_ADDRESS "")
  foreach(line IN LISTS inventory_lines)
    string(REPLACE "${TAB}" ";" fields "${line}")
    list(LENGTH fields field_count)
    if(field_count EQUAL 6)
      list(GET fields 0 candidate_page)
      list(GET fields 1 candidate_row)
      list(GET fields 2 candidate_kind)
      if(candidate_page STREQUAL "2" AND candidate_row STREQUAL "1" AND
         candidate_kind STREQUAL "TEST")
        list(GET fields 3 FAULT_KEY)
        list(GET fields 4 FAULT_ADDRESS)
      endif()
    endif()
  endforeach()
  if(FAULT_KEY STREQUAL "" OR FAULT_ADDRESS STREQUAL "")
    message(FATAL_ERROR "could not locate page 2 row 1 in the authenticated inventory")
  endif()
endif()

foreach(page RANGE 1 22)
  if(page LESS 10)
    set(page_name "page-0${page}")
    set(attempt_name "page-0${page}-cold-01")
  else()
    set(page_name "page-${page}")
    set(attempt_name "page-${page}-cold-01")
  endif()
  set(page_dir "${WORK}/${page_name}")
  file(MAKE_DIRECTORY "${page_dir}")
  file(REMOVE "${page_dir}/raw.tsv" "${page_dir}/diagnostics.tsv"
    "${page_dir}/report.tsv" "${page_dir}/metadata.txt"
    "${page_dir}/stdout.txt" "${page_dir}/stderr.txt"
    "${WORK}/launched-${page_name}.txt")
  file(WRITE "${WORK}/launched-${page_name}.txt" "${page}\n")

  if(DEFINED TEST_TIMEOUT_PAGE OR DEFINED TEST_PARTIAL_FAILURE_PAGE)
    if(DEFINED TEST_TIMEOUT_PAGE AND page EQUAL TEST_TIMEOUT_PAGE)
      execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 3
        TIMEOUT "${PAGE_TIMEOUT}" RESULT_VARIABLE run_result
        OUTPUT_VARIABLE run_stdout ERROR_VARIABLE run_stderr)
    elseif(DEFINED TEST_PARTIAL_FAILURE_PAGE AND page EQUAL TEST_PARTIAL_FAILURE_PAGE)
      execute_process(COMMAND "${CMAKE_COMMAND}"
        "-DOUTPUT_DIR=${page_dir}" "-DPAGE=${page}"
        "-DKEY=${FAULT_KEY}" "-DADDRESS=${FAULT_ADDRESS}"
        -P "${CMAKE_CURRENT_LIST_DIR}/accuracycoin_partial_failure.cmake"
        RESULT_VARIABLE run_result OUTPUT_VARIABLE run_stdout ERROR_VARIABLE run_stderr)
    else()
      execute_process(COMMAND "${CMAKE_COMMAND}" -E echo "launched page ${page}"
        RESULT_VARIABLE run_result OUTPUT_VARIABLE run_stdout ERROR_VARIABLE run_stderr)
    endif()
  else()
    math(EXPR total_frames
      "${BOOT_CAP} + ${NAVIGATION_CAP} * (${page} - 1) + 2 + ${EXECUTION_CAP}")
    execute_process(
      COMMAND "${RUNNER}" --accuracycoin-observe "${page}" --rom "${ROM}"
        --frames "${total_frames}" --accuracycoin-boot-cap "${BOOT_CAP}"
        --accuracycoin-navigation-cap "${NAVIGATION_CAP}"
        --accuracycoin-execution-cap "${EXECUTION_CAP}"
        --accuracycoin-output "${page_dir}"
      WORKING_DIRECTORY "${SOURCE_DIR}" TIMEOUT "${PAGE_TIMEOUT}"
      RESULT_VARIABLE run_result OUTPUT_VARIABLE run_stdout ERROR_VARIABLE run_stderr)
  endif()
  file(WRITE "${page_dir}/stdout.txt" "${run_stdout}")
  file(WRITE "${page_dir}/stderr.txt" "${run_stderr}")

  if("${run_result}" STREQUAL "0")
    set(outcome "EXIT_ZERO")
    set(exit_code "0")
  elseif("${run_result}" MATCHES "^[0-9]+$")
    set(outcome "CHILD_FAILURE")
    set(exit_code "${run_result}")
  elseif("${run_result}" MATCHES "[Tt]imeout")
    set(outcome "OUTER_TIMEOUT")
    set(exit_code "-")
  else()
    set(outcome "CHILD_FAILURE")
    set(exit_code "-")
  endif()
  file(APPEND "${WORK}/outer-outcomes.tsv"
    "${page}\t${outcome}\t${exit_code}\t${PAGE_TIMEOUT}\n")
endforeach()

execute_process(
  COMMAND "${RUNNER}" --accuracycoin-collect "${WORK}" --rom "${ROM}"
  WORKING_DIRECTORY "${SOURCE_DIR}" TIMEOUT "${COLLECT_TIMEOUT}"
  RESULT_VARIABLE collect_result OUTPUT_VARIABLE collect_stdout ERROR_VARIABLE collect_stderr)
file(WRITE "${WORK}/collection.stdout" "${collect_stdout}")
file(WRITE "${WORK}/collection.stderr" "${collect_stderr}")

if(DEFINED TEST_TIMEOUT_PAGE OR DEFINED TEST_PARTIAL_FAILURE_PAGE)
  if("${collect_result}" STREQUAL "0")
    message(FATAL_ERROR "production collector accepted an injected outer failure")
  endif()
  foreach(page RANGE 1 22)
    if(page LESS 10)
      set(page_name "page-0${page}")
    else()
      set(page_name "page-${page}")
    endif()
    if(NOT EXISTS "${WORK}/launched-${page_name}.txt")
      message(FATAL_ERROR "outer failure prevented later independent launch ${page}")
    endif()
  endforeach()
  file(READ "${WORK}/outer-outcomes.tsv" outcomes)
  if(DEFINED TEST_TIMEOUT_PAGE)
    string(FIND "${outcomes}" "${TEST_TIMEOUT_PAGE}\tOUTER_TIMEOUT\t-\t${PAGE_TIMEOUT}\n" timeout_row)
    if(timeout_row EQUAL -1)
      message(FATAL_ERROR "outer timeout was not preserved as OUTER_TIMEOUT: ${outcomes}")
    endif()
  endif()
  if(DEFINED TEST_PARTIAL_FAILURE_PAGE)
    string(FIND "${outcomes}" "${TEST_PARTIAL_FAILURE_PAGE}\tCHILD_FAILURE\t1\t${PAGE_TIMEOUT}\n" failure_row)
    if(failure_row EQUAL -1)
      message(FATAL_ERROR "partial child exit was not preserved as CHILD_FAILURE: ${outcomes}")
    endif()
    file(READ "${WORK}/observations.tsv" observations)
    string(FIND "${observations}" "page-02-cold-01\tbefore-start\t2\t1\t${FAULT_KEY}\t${FAULT_ADDRESS}\t0\t-\n" partial_row)
    if(partial_row EQUAL -1)
      message(FATAL_ERROR "collector discarded the genuine partial unobserved snapshot")
    endif()
  endif()
  file(READ "${WORK}/metadata.txt" campaign_metadata)
  if(NOT campaign_metadata MATCHES "trustworthy_observation=0" OR
     NOT campaign_metadata MATCHES "infrastructure_verdict=failed")
    message(FATAL_ERROR "outer failure was reported as trustworthy: ${campaign_metadata}")
  endif()
else()
  if(NOT "${collect_result}" STREQUAL "0")
    message(FATAL_ERROR "production AccuracyCoin collection failed (${collect_result}): ${collect_stderr}")
  endif()
  foreach(name IN ITEMS inventory.tsv attempts.tsv observations.tsv report.tsv summary.txt metadata.txt)
    if(NOT EXISTS "${WORK}/${name}")
      message(FATAL_ERROR "campaign collection omitted ${name}")
    endif()
  endforeach()
  file(STRINGS "${WORK}/attempts.tsv" attempt_lines)
  file(STRINGS "${WORK}/report.tsv" report_lines)
  list(LENGTH attempt_lines attempt_count)
  list(LENGTH report_lines report_count)
  if(NOT attempt_count EQUAL 23 OR NOT report_count EQUAL 152)
    message(FATAL_ERROR "campaign expected 22 attempts and 151 inventory rows, got ${attempt_count} and ${report_count}")
  endif()
  file(READ "${WORK}/summary.txt" summary)
  file(READ "${WORK}/metadata.txt" metadata)
  foreach(expected IN ITEMS
      "scored_total=146" "draw_inventory_only=5" "pages_launched=22/22"
      "observed_scored_rows=" "terminal_scored_rows=")
    string(FIND "${summary}" "${expected}" found)
    if(found EQUAL -1)
      message(FATAL_ERROR "campaign summary omitted ${expected}")
    endif()
  endforeach()
  foreach(expected IN ITEMS
      "rom_pin=6bee919529289c555803b612b8c9e4f7eaaeeabb"
      "rom_sha256=8ac8904a3cf26fbdfa6be501e414d45886ff4478d30dee8e58fa436cf5fd5adc"
      "pages_launched=22" "scored_total=146" "draw=5"
      "trustworthy_observation=1" "infrastructure_verdict=complete")
    string(FIND "${metadata}" "${expected}" found)
    if(found EQUAL -1)
      message(FATAL_ERROR "campaign metadata omitted ${expected}")
    endif()
  endforeach()
endif()
