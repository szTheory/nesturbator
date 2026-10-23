cmake_minimum_required(VERSION 3.25)

foreach(required_var IN ITEMS RUNNER OLD_ROM NEW_ROM WORK)
  if(NOT DEFINED ${required_var} OR "${${required_var}}" STREQUAL "")
    message(FATAL_ERROR "accuracycoin protected observation requires ${required_var}")
  endif()
endforeach()

set(boot_cap 32)
set(navigation_cap 5)
set(execution_cap 1800)
set(page_wall_seconds 5)
set(pages 2 14 17)

# A setup fixture must never inherit evidence from an earlier CTest invocation.
file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")

foreach(pin_name IN ITEMS old new)
  if(pin_name STREQUAL "old")
    set(rom "${OLD_ROM}")
  else()
    set(rom "${NEW_ROM}")
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
