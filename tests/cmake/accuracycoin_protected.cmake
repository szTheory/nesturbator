cmake_minimum_required(VERSION 3.25)

foreach(required_var IN ITEMS RUNNER VERIFIER SOURCE_DIR OLD_ROM NEW_ROM WORK)
  if(NOT DEFINED ${required_var} OR "${${required_var}}" STREQUAL "")
    message(FATAL_ERROR "accuracycoin protected observation requires ${required_var}")
  endif()
endforeach()

set(boot_cap 32)
set(navigation_cap 5)
set(execution_cap 1800)
if(DEFINED PAGE_WALL_SECONDS)
  set(page_wall_seconds "${PAGE_WALL_SECONDS}")
else()
  set(page_wall_seconds 30)
endif()
set(old_pages 2 14 17)

if(DEFINED BASELINE AND NOT BASELINE STREQUAL "")
  set(baseline "${BASELINE}")
elseif(DEFINED ENV{NESTURBATOR_SCOREBOARD_BASELINE} AND
       NOT "$ENV{NESTURBATOR_SCOREBOARD_BASELINE}" STREQUAL "")
  set(baseline "$ENV{NESTURBATOR_SCOREBOARD_BASELINE}")
else()
  set(baseline "${SOURCE_DIR}/tests/accuracy/scoreboard-main.txt")
endif()
if(DEFINED BASELINE_METADATA AND NOT BASELINE_METADATA STREQUAL "")
  set(baseline_metadata "${BASELINE_METADATA}")
elseif(DEFINED ENV{NESTURBATOR_SCOREBOARD_BASELINE_METADATA} AND
       NOT "$ENV{NESTURBATOR_SCOREBOARD_BASELINE_METADATA}" STREQUAL "")
  set(baseline_metadata "$ENV{NESTURBATOR_SCOREBOARD_BASELINE_METADATA}")
else()
  set(baseline_metadata "${SOURCE_DIR}/tests/accuracy/scoreboard-main-metadata.txt")
endif()
if(DEFINED MIGRATION AND NOT MIGRATION STREQUAL "")
  set(migration "${MIGRATION}")
else()
  set(migration "${SOURCE_DIR}/tests/accuracy/migration.tsv")
endif()

execute_process(
  COMMAND "${VERIFIER}" --protected-pages "${baseline}" "${baseline_metadata}" "${migration}"
  RESULT_VARIABLE page_plan_result
  OUTPUT_VARIABLE page_plan_output
  ERROR_VARIABLE page_plan_error)
if(NOT "${page_plan_result}" STREQUAL "0")
  message(FATAL_ERROR
    "Could not derive fresh new-pin pages from protected PASS keys (${page_plan_result}): ${page_plan_error}${page_plan_output}")
endif()
string(STRIP "${page_plan_output}" page_plan_output)
string(REPLACE "," ";" new_pages "${page_plan_output}")
if(new_pages STREQUAL "")
  message(FATAL_ERROR "Protected PASS keys selected no new-pin pages")
endif()

# A setup fixture must never inherit evidence from an earlier CTest invocation.
file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")

foreach(pin_name IN ITEMS old new)
  if(pin_name STREQUAL "old")
    set(rom "${OLD_ROM}")
    set(pages ${old_pages})
  else()
    set(rom "${NEW_ROM}")
    set(pages ${new_pages})
  endif()
  foreach(page IN LISTS pages)
    if(page LESS 10)
      set(page_name "0${page}")
    else()
      set(page_name "${page}")
    endif()
    set(output_dir "${WORK}/${pin_name}/page-${page_name}")
    math(EXPR maximum_frames
      "${boot_cap} + (${page} - 1) * ${navigation_cap} + 2 + ${execution_cap}")
    file(MAKE_DIRECTORY "${output_dir}")
    execute_process(
      COMMAND "${RUNNER}" --accuracycoin-observe "${page}" --rom "${rom}"
        --frames "${maximum_frames}" --accuracycoin-output "${output_dir}"
        --accuracycoin-boot-cap "${boot_cap}"
        --accuracycoin-navigation-cap "${navigation_cap}"
        --accuracycoin-execution-cap "${execution_cap}"
      RESULT_VARIABLE run_result
      OUTPUT_VARIABLE run_stdout
      ERROR_VARIABLE run_stderr
      TIMEOUT "${page_wall_seconds}")
    if(NOT "${run_result}" STREQUAL "0")
      message(FATAL_ERROR
        "Protected ${pin_name}-pin page ${page} observation failed (${run_result}): ${run_stderr}${run_stdout}")
    endif()
    foreach(evidence IN ITEMS report.tsv raw.tsv diagnostics.tsv metadata.txt)
      if(NOT EXISTS "${output_dir}/${evidence}" OR IS_DIRECTORY "${output_dir}/${evidence}")
        message(FATAL_ERROR "Protected ${pin_name}-pin page ${page} lacks fresh ${evidence}")
      endif()
    endforeach()
  endforeach()
endforeach()
