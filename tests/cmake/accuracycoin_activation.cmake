cmake_minimum_required(VERSION 3.25)

foreach(required IN ITEMS SOURCE_DIR RUNNER CONFORMANCE NEW_ROM OLD_ROM SCOREBOARD
    BASELINE_METADATA OLD_SCOREBOARD MIGRATION FULL_ENABLED FULL_REPORT)
  if(NOT DEFINED ${required})
    message(FATAL_ERROR "AccuracyCoin activation check needs ${required}")
  endif()
endforeach()

include("${CMAKE_CURRENT_LIST_DIR}/accuracycoin_input_fingerprint.cmake")
accuracycoin_compute_input_fingerprint("${SOURCE_DIR}" current_inputs_sha256 current_inputs_count)

function(read_meta name out)
  if(NOT DEFINED META_${name})
    message(FATAL_ERROR "baseline metadata is missing ${name}")
  endif()
  set(${out} "${META_${name}}" PARENT_SCOPE)
endfunction()

foreach(path IN ITEMS "${NEW_ROM}" "${OLD_ROM}" "${SCOREBOARD}" "${BASELINE_METADATA}"
    "${OLD_SCOREBOARD}" "${MIGRATION}")
  if(NOT EXISTS "${path}" OR IS_DIRECTORY "${path}")
    message(FATAL_ERROR "AccuracyCoin activation input is missing: ${path}")
  endif()
endforeach()

file(SHA256 "${NEW_ROM}" new_rom_sha256)
file(SHA256 "${OLD_ROM}" old_rom_sha256)
file(SHA256 "${SCOREBOARD}" canonical_scoreboard_sha256)
if(NOT new_rom_sha256 STREQUAL "8ac8904a3cf26fbdfa6be501e414d45886ff4478d30dee8e58fa436cf5fd5adc" OR
   NOT old_rom_sha256 STREQUAL "4fe8c2bc9abc6f4d418da47b73f62cba89fcacd950fae763097a1681d650e839")
  message(FATAL_ERROR "AccuracyCoin fixture bytes do not match the reviewed old/new pins")
endif()

file(STRINGS "${SOURCE_DIR}/tests/roms/manifest.txt" manifest_lines)
string(ASCII 9 tab)
set(new_manifest_matches 0)
set(old_manifest_matches 0)
foreach(line IN LISTS manifest_lines)
  if(line MATCHES "^#" OR line STREQUAL "")
    continue()
  endif()
  string(REPLACE "${tab}" ";" fields "${line}")
  list(LENGTH fields field_count)
  if(NOT field_count EQUAL 5)
    message(FATAL_ERROR "ROM manifest contains a malformed row")
  endif()
  list(GET fields 0 path)
  list(GET fields 2 pin)
  list(GET fields 3 license)
  list(GET fields 4 digest)
  if(path STREQUAL "tests/roms/accuracycoin.nes")
    if(NOT pin STREQUAL "6bee919529289c555803b612b8c9e4f7eaaeeabb" OR
       NOT license STREQUAL "MIT" OR NOT digest STREQUAL new_rom_sha256)
      message(FATAL_ERROR "canonical AccuracyCoin manifest row does not match its fixture")
    endif()
    math(EXPR new_manifest_matches "${new_manifest_matches} + 1")
  elseif(path STREQUAL "tests/roms/accuracycoin-673ef.nes")
    if(NOT pin STREQUAL "673ef550db296136d52229961e7d39366116882a" OR
       NOT license STREQUAL "MIT" OR NOT digest STREQUAL old_rom_sha256)
      message(FATAL_ERROR "old AccuracyCoin manifest row does not match its fixture")
    endif()
    math(EXPR old_manifest_matches "${old_manifest_matches} + 1")
  endif()
endforeach()
if(NOT new_manifest_matches EQUAL 1 OR NOT old_manifest_matches EQUAL 1)
  message(FATAL_ERROR "manifest must authenticate each AccuracyCoin fixture exactly once")
endif()

execute_process(COMMAND "${CONFORMANCE}" inventory "${NEW_ROM}" "${OLD_ROM}"
  RESULT_VARIABLE inventory_result OUTPUT_VARIABLE inventory_output ERROR_VARIABLE inventory_error)
if(NOT "${inventory_result}" STREQUAL "0")
  message(FATAL_ERROR "parser-derived old/new inventories disagree: ${inventory_error}${inventory_output}")
endif()
execute_process(COMMAND "${CONFORMANCE}" migration "${MIGRATION}" "${OLD_SCOREBOARD}"
  RESULT_VARIABLE migration_result OUTPUT_VARIABLE migration_output ERROR_VARIABLE migration_error)
if(NOT "${migration_result}" STREQUAL "0")
  message(FATAL_ERROR "old protected coverage or migration endpoints disagree: ${migration_error}${migration_output}")
endif()

file(READ "${BASELINE_METADATA}" metadata_text)
set(metadata_keys)
while(NOT metadata_text STREQUAL "")
  string(FIND "${metadata_text}" "\n" line_end)
  if(line_end EQUAL -1)
    set(line "${metadata_text}")
    set(metadata_text "")
  else()
    string(SUBSTRING "${metadata_text}" 0 ${line_end} line)
    math(EXPR next_line "${line_end} + 1")
    string(SUBSTRING "${metadata_text}" ${next_line} -1 metadata_text)
  endif()
  if(line STREQUAL "")
    continue()
  endif()
  if(NOT line MATCHES "^([a-z][a-z0-9_]*)=(.*)$")
    message(FATAL_ERROR "baseline metadata row syntax is invalid: ${line}")
  endif()
  set(name "${CMAKE_MATCH_1}")
  set(value "${CMAKE_MATCH_2}")
  if(name IN_LIST metadata_keys)
    message(FATAL_ERROR "baseline metadata repeats ${name}")
  endif()
  list(APPEND metadata_keys "${name}")
  set("META_${name}" "${value}")
endwhile()

foreach(pair IN ITEMS
    "schema|1"
    "rom_pin|6bee919529289c555803b612b8c9e4f7eaaeeabb"
    "rom_sha256|${new_rom_sha256}"
    "license|MIT"
    "report_schema|1"
    "inventory_schema|1"
    "inventory_sha256|7d022800d01ad81159109bf7457af94ab6f80e4a682fce716ec476f7c454a4a5"
    "inventory_rows|151"
    "inventory_scored_rows|146"
    "inventory_draw_rows|5"
    "scoreboard_schema|2"
    "scoreboard_rows|146"
    "scoreboard_sha256|${canonical_scoreboard_sha256}"
    "pass|101"
    "fail|42"
    "skip|0"
    "not_run|2"
    "timeout|1"
    "draw|5"
    "attempted_scored_rows|146"
    "observed_scored_rows|146"
    "terminal_scored_rows|143"
    "pages_launched|22"
    "pages_start_attributed|22"
    "pages_completed|22"
    "boot_frame_budget|32"
    "navigation_frame_budget_per_page|5"
    "execution_frame_budget|1800"
    "total_frame_budget|41503"
    "behaviour_revision|7"
    "build_inputs_sha256|${current_inputs_sha256}"
    "build_inputs_count|${current_inputs_count}")
  string(REPLACE "|" ";" fields "${pair}")
  list(GET fields 0 name)
  list(GET fields 1 expected)
  read_meta("${name}" actual)
  if(NOT actual STREQUAL expected)
    message(FATAL_ERROR "baseline metadata ${name} is '${actual}', expected '${expected}'")
  endif()
endforeach()

foreach(name IN ITEMS source_revision source_dirty source_dirty_reason input_policy)
  read_meta("${name}" value)
endforeach()
read_meta(campaign_report_sha256 report_sha256)
string(LENGTH "${report_sha256}" report_sha_length)
if(NOT report_sha256 MATCHES "^[0-9a-f]+$" OR NOT report_sha_length EQUAL 64 OR
   NOT META_source_revision MATCHES "^[0-9a-f]+$")
  message(FATAL_ERROR "baseline report/source identity is malformed")
endif()
if(NOT META_source_dirty STREQUAL "0" AND NOT META_source_dirty STREQUAL "1")
  message(FATAL_ERROR "baseline source_dirty must be 0 or 1")
endif()

file(STRINGS "${SCOREBOARD}" scoreboard_lines)
set(scoreboard_keys)
set(pass_count 0)
set(fail_count 0)
set(skip_count 0)
set(not_run_count 0)
set(timeout_count 0)
set(previous_key "")
foreach(line IN LISTS scoreboard_lines)
  string(REPLACE "${tab}" ";" fields "${line}")
  list(LENGTH fields field_count)
  if(NOT field_count EQUAL 5)
    message(FATAL_ERROR "canonical scoreboard row must have five tab-separated fields")
  endif()
  list(GET fields 0 key)
  list(GET fields 1 status)
  list(GET fields 2 raw)
  list(GET fields 3 frames)
  list(GET fields 4 hash)
  if(NOT key MATCHES "^accuracycoin/" OR
     (NOT previous_key STREQUAL "" AND NOT previous_key STRLESS key) OR
     NOT raw MATCHES "^0x[0-9a-f][0-9a-f]$" OR
     NOT frames MATCHES "^[1-9][0-9]*$" OR NOT hash STREQUAL "-")
    message(FATAL_ERROR "canonical scoreboard row is malformed, unsorted or unmeasured: ${line}")
  endif()
  list(APPEND scoreboard_keys "${key}")
  set(previous_key "${key}")
  if(status STREQUAL "pass")
    math(EXPR pass_count "${pass_count} + 1")
  elseif(status STREQUAL "fail")
    math(EXPR fail_count "${fail_count} + 1")
  elseif(status STREQUAL "skip")
    math(EXPR skip_count "${skip_count} + 1")
  elseif(status STREQUAL "not-run")
    math(EXPR not_run_count "${not_run_count} + 1")
  elseif(status STREQUAL "timeout")
    math(EXPR timeout_count "${timeout_count} + 1")
  else()
    message(FATAL_ERROR "canonical scoreboard has unknown result status: ${status}")
  endif()
endforeach()
list(LENGTH scoreboard_keys scoreboard_count)
if(NOT scoreboard_count EQUAL 146 OR NOT pass_count EQUAL 101 OR NOT fail_count EQUAL 42 OR
   NOT skip_count EQUAL 0 OR NOT not_run_count EQUAL 2 OR NOT timeout_count EQUAL 1)
  message(FATAL_ERROR "canonical scoreboard counts do not match the measured campaign")
endif()

if(FULL_ENABLED)
  if(NOT EXISTS "${FULL_REPORT}" OR IS_DIRECTORY "${FULL_REPORT}")
    message(FATAL_ERROR "full-enabled accuracycoin.campaign did not retain report.tsv")
  endif()
  file(SHA256 "${FULL_REPORT}" actual_report_sha256)
  if(NOT actual_report_sha256 STREQUAL report_sha256)
    message(FATAL_ERROR "full campaign report bytes do not match the committed baseline provenance")
  endif()
  file(STRINGS "${FULL_REPORT}" report_lines)
  set(report_count 0)
  set(report_draw_count 0)
  foreach(line IN LISTS report_lines)
    if(line STREQUAL "key${tab}page${tab}row${tab}kind${tab}status${tab}observed${tab}raw${tab}frames${tab}cause${tab}attempt_id")
      continue()
    endif()
    string(REPLACE "${tab}" ";" fields "${line}")
    list(LENGTH fields field_count)
    if(NOT field_count EQUAL 10)
      message(FATAL_ERROR "full campaign report row is malformed: ${line}")
    endif()
    list(GET fields 0 key)
    list(GET fields 3 kind)
    list(GET fields 4 status)
    list(GET fields 5 observed)
    list(GET fields 6 raw)
    list(GET fields 7 frames)
    list(GET fields 9 attempt_id)
    if(kind STREQUAL "draw")
      if(NOT status STREQUAL "draw")
        message(FATAL_ERROR "DRAW inventory row entered the scored status protocol")
      endif()
      math(EXPR report_draw_count "${report_draw_count} + 1")
      continue()
    endif()
    if(NOT kind STREQUAL "test" OR NOT observed STREQUAL "1" OR attempt_id STREQUAL "")
      message(FATAL_ERROR "scored campaign row lacks attempted raw provenance: ${line}")
    endif()
    list(FIND scoreboard_keys "${key}" scoreboard_index)
    if(scoreboard_index LESS 0)
      message(FATAL_ERROR "full campaign row is missing from canonical scoreboard: ${key}")
    endif()
    list(GET scoreboard_lines ${scoreboard_index} scoreboard_line)
    string(REPLACE "${tab}" ";" score_fields "${scoreboard_line}")
    list(GET score_fields 1 score_status)
    list(GET score_fields 2 score_raw)
    list(GET score_fields 3 score_frames)
    if(NOT status STREQUAL score_status OR NOT raw STREQUAL score_raw OR
       NOT frames STREQUAL score_frames)
      message(FATAL_ERROR "canonical score differs from raw campaign report: ${key}")
    endif()
    math(EXPR report_count "${report_count} + 1")
  endforeach()
  if(NOT report_count EQUAL 146 OR NOT report_draw_count EQUAL 5)
    message(FATAL_ERROR "full campaign report coverage is ${report_count} scored/${report_draw_count} DRAW")
  endif()
endif()

message(STATUS "AccuracyCoin activation PASS: canonical 146-row baseline, old 17-row protection, authenticated 151/149 inventories")
