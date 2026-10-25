cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED RUNNER OR NOT DEFINED ROM OR NOT DEFINED WORK)
  message(FATAL_ERROR "accuracycoin.tracer needs RUNNER, ROM and WORK")
endif()

file(MAKE_DIRECTORY "${WORK}")
file(REMOVE "${WORK}/report.tsv" "${WORK}/raw.tsv" "${WORK}/diagnostics.tsv"
  "${WORK}/metadata.txt")
execute_process(
  COMMAND "${RUNNER}" --accuracycoin-observe 3 --rom "${ROM}" --frames 1800
    --accuracycoin-output "${WORK}"
  WORKING_DIRECTORY "${SOURCE_DIR}"
  RESULT_VARIABLE run_result
  OUTPUT_VARIABLE run_stdout
  ERROR_VARIABLE run_stderr)
if(NOT "${run_result}" STREQUAL "0")
  message(FATAL_ERROR "page-3 observation failed (${run_result}): ${run_stderr}")
endif()
if(NOT run_stdout MATCHES "accuracycoin observe page 3 (complete|guest-timeout) \\(7 rows,")
  message(FATAL_ERROR "page-3 observation did not report a bounded terminal verdict: ${run_stdout}")
endif()

foreach(name IN ITEMS report.tsv raw.tsv diagnostics.tsv metadata.txt)
  if(NOT EXISTS "${WORK}/${name}")
    message(FATAL_ERROR "page-3 observation omitted ${name}")
  endif()
endforeach()
file(READ "${WORK}/report.tsv" report)
file(READ "${WORK}/raw.tsv" raw)
file(READ "${WORK}/diagnostics.tsv" diagnostics)
file(READ "${WORK}/metadata.txt" metadata)
string(ASCII 9 TAB)
string(REPLACE "\n" ";" report_lines "${report}")
string(REPLACE "\n" ";" raw_lines "${raw}")
list(LENGTH report_lines report_line_count)
if(NOT report_line_count EQUAL 9)
  message(FATAL_ERROR "page-3 report must have one header and seven rows; got ${report_line_count} lines")
endif()
if(NOT report MATCHES "^key${TAB}page${TAB}row${TAB}status${TAB}observed${TAB}raw${TAB}frames${TAB}cause${TAB}attempt_id")
  message(FATAL_ERROR "page-3 report header is incomplete")
endif()

set(expected_keys
  "accuracycoin/$03   SLO indirect,X"
  "accuracycoin/$07   SLO zeropage"
  "accuracycoin/$0F   SLO absolute"
  "accuracycoin/$13   SLO indirect,Y"
  "accuracycoin/$17   SLO zeropage,X"
  "accuracycoin/$1B   SLO absolute,Y"
  "accuracycoin/$1F   SLO absolute,X")
set(expected_addresses 0x0409 0x040a 0x040b 0x040c 0x040d 0x040e 0x040f)
set(expected_row 0)
foreach(expected_key IN LISTS expected_keys)
  math(EXPR expected_row "${expected_row} + 1")
  math(EXPR address_index "${expected_row} - 1")
  list(GET expected_addresses ${address_index} expected_address)
  set(found 0)
  foreach(line IN LISTS report_lines)
    string(FIND "${line}" "${expected_key}${TAB}" key_offset)
    if(key_offset EQUAL 0)
      set(found 1)
      string(REPLACE "${TAB}" ";" fields "${line}")
      list(LENGTH fields field_count)
      if(NOT field_count EQUAL 9)
        message(FATAL_ERROR "malformed report row for ${expected_key}: ${line}")
      endif()
      list(GET fields 1 page)
      list(GET fields 2 row)
      list(GET fields 3 status)
      list(GET fields 4 observed)
      list(GET fields 5 raw_value)
      list(GET fields 6 frames)
      list(GET fields 7 cause)
      list(GET fields 8 attempt)
      if(NOT page STREQUAL "3" OR NOT row STREQUAL "${expected_row}" OR
         NOT status MATCHES "^(pass|fail|skip|not-run|timeout)$" OR
         NOT observed STREQUAL "1" OR NOT raw_value MATCHES "^0x[0-9a-f][0-9a-f]$" OR
         NOT frames MATCHES "^[0-9]+$" OR cause STREQUAL "" OR
         NOT attempt STREQUAL "page-03-cold-01")
        message(FATAL_ERROR "untrustworthy report evidence for ${expected_key}: ${line}")
      endif()
      set(final_raw "${raw_value}")
      set(final_row "final${TAB}3${TAB}${expected_row}${TAB}${expected_key}${TAB}")
      string(FIND "${raw}" "${final_row}" final_offset)
      if(final_offset EQUAL -1)
        message(FATAL_ERROR "final raw snapshot omitted ${expected_key}")
      endif()
      set(final_raw_found 0)
      foreach(raw_line IN LISTS raw_lines)
        string(FIND "${raw_line}" "${final_row}" raw_row_offset)
        if(raw_row_offset EQUAL 0)
          string(REPLACE "${TAB}" ";" raw_fields "${raw_line}")
          list(LENGTH raw_fields raw_field_count)
          if(NOT raw_field_count EQUAL 7)
            message(FATAL_ERROR "malformed final raw row for ${expected_key}: ${raw_line}")
          endif()
          list(GET raw_fields 4 raw_address)
          list(GET raw_fields 5 raw_observed)
          list(GET raw_fields 6 raw_value)
          if(NOT raw_address STREQUAL expected_address OR NOT raw_observed STREQUAL "1" OR
             NOT raw_value STREQUAL final_raw)
            message(FATAL_ERROR "report raw byte does not link to final snapshot for ${expected_key}")
          endif()
          set(final_raw_found 1)
        endif()
      endforeach()
      if(NOT final_raw_found)
        message(FATAL_ERROR "report raw byte does not link to the final snapshot for ${expected_key}")
      endif()
    endif()
  endforeach()
  if(NOT found)
    message(FATAL_ERROR "page-3 report omitted decoded token key ${expected_key}")
  endif()
endforeach()

string(FIND "${raw}" "before-start${TAB}3${TAB}1${TAB}accuracycoin/$03   SLO indirect,X${TAB}0x0409${TAB}1${TAB}0x00" before_start)
string(FIND "${raw}" "a-press${TAB}3${TAB}1${TAB}accuracycoin/$03   SLO indirect,X${TAB}0x0409${TAB}1${TAB}0x03" press_transition)
if(before_start EQUAL -1 OR press_transition EQUAL -1)
  message(FATAL_ERROR "page-3 start lacks its before/after raw result transition")
endif()

string(REPLACE "\n" ";" diagnostic_lines "${diagnostics}")
set(selected_page_seen 0)
set(start_state_seen 0)
foreach(line IN LISTS diagnostic_lines)
  string(REPLACE "${TAB}" ";" fields "${line}")
  list(LENGTH fields field_count)
  if(field_count EQUAL 8)
    list(GET fields 0 stage)
    list(GET fields 2 menu_ready)
    list(GET fields 3 page_index)
    list(GET fields 4 cursor)
    list(GET fields 5 automate)
    list(GET fields 7 transition)
    if(stage STREQUAL "navigate" AND menu_ready STREQUAL "10" AND
       page_index STREQUAL "2" AND cursor STREQUAL "255")
      set(selected_page_seen 1)
    endif()
    if(stage STREQUAL "a-press" AND menu_ready STREQUAL "10" AND
       page_index STREQUAL "2" AND automate STREQUAL "1" AND transition STREQUAL "1")
      set(start_state_seen 1)
    endif()
  endif()
endforeach()
if(NOT selected_page_seen OR NOT start_state_seen)
  message(FATAL_ERROR "page-3 selection/start did not prove menu state and result transition")
endif()
foreach(expected_metadata IN ITEMS
    "rom_pin=6bee919529289c555803b612b8c9e4f7eaaeeabb"
    "rom_sha256=8ac8904a3cf26fbdfa6be501e414d45886ff4478d30dee8e58fa436cf5fd5adc"
    "inventory_sha256=7d022800d01ad81159109bf7457af94ab6f80e4a682fce716ec476f7c454a4a5"
    "mode=observe"
    "run_state_observed=1"
    "result_transition_observed=1")
  string(FIND "${metadata}" "${expected_metadata}" metadata_offset)
  if(metadata_offset EQUAL -1)
    message(FATAL_ERROR "metadata omitted ${expected_metadata}")
  endif()
endforeach()
if(NOT metadata MATCHES "verdict=(complete|guest-timeout)")
  message(FATAL_ERROR "metadata has no complete or guest-timeout verdict")
endif()
