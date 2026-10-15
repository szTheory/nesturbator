# Check registered libretro fixture inputs before launching the host. Special
# roles are authorized by the exact manifest path and the bytes of the supplied
# file, including the copy that the host will actually open.
cmake_minimum_required(VERSION 3.25)

string(ASCII 9 _tab)

function(fixture_final_component input out)
  set(path "${input}")
  string(FIND "${path}" "/" slash_pos REVERSE)
  set(separator_pos "${slash_pos}")
  if(WIN32)
    string(FIND "${path}" "\\" backslash_pos REVERSE)
    if(backslash_pos GREATER separator_pos)
      set(separator_pos "${backslash_pos}")
    endif()
  endif()
  if(separator_pos LESS 0)
    set(component "${path}")
  else()
    math(EXPR component_pos "${separator_pos} + 1")
    string(SUBSTRING "${path}" ${component_pos} -1 component)
  endif()
  set(${out} "${component}" PARENT_SCOPE)
endfunction()

function(fixture_identity_check input manifest out_reason)
  fixture_final_component("${input}" basename)
  if(basename STREQUAL "nesteroids.nes")
    set(expected_path "tests/roms/nesteroids.nes")
  elseif(basename STREQUAL "M1_P128K_C128K_S8K.nes")
    set(expected_path "tests/roms/hm/M1_P128K_C128K_S8K.nes")
  else()
    set(${out_reason} "" PARENT_SCOPE)
    return()
  endif()

  if(NOT EXISTS "${input}" OR IS_DIRECTORY "${input}")
    set(${out_reason} "FIXTURE_FILE_MISSING: ${input}" PARENT_SCOPE)
    return()
  endif()
  if(NOT EXISTS "${manifest}" OR IS_DIRECTORY "${manifest}")
    set(${out_reason} "FIXTURE_MANIFEST_MISSING: ${manifest}" PARENT_SCOPE)
    return()
  endif()

  file(STRINGS "${manifest}" lines)
  set(matches 0)
  set(expected_sha "")
  foreach(line IN LISTS lines)
    string(FIND "${line}" "${_tab}" path_end)
    if(path_end LESS 0)
      continue()
    endif()
    string(SUBSTRING "${line}" 0 ${path_end} row_path)
    if(row_path STREQUAL expected_path)
      math(EXPR matches "${matches} + 1")
      string(REPLACE "${_tab}" ";" fields "${line}")
      list(LENGTH fields field_count)
      if(field_count EQUAL 5)
        list(GET fields 4 expected_sha)
      else()
        set(expected_sha "")
      endif()
    endif()
  endforeach()
  if(matches EQUAL 0)
    set(${out_reason} "FIXTURE_MANIFEST_MISSING: ${expected_path}" PARENT_SCOPE)
    return()
  elseif(matches GREATER 1)
    set(${out_reason} "FIXTURE_MANIFEST_DUPLICATE: ${expected_path}" PARENT_SCOPE)
    return()
  endif()

  string(LENGTH "${expected_sha}" sha_length)
  if(NOT expected_sha MATCHES "^[0-9a-f]+$" OR NOT sha_length EQUAL 64)
    set(${out_reason} "FIXTURE_MANIFEST_INVALID: ${expected_path}" PARENT_SCOPE)
    return()
  endif()
  file(SHA256 "${input}" actual_sha)
  if(NOT actual_sha STREQUAL expected_sha)
    set(${out_reason} "FIXTURE_IDENTITY: ${input}" PARENT_SCOPE)
    return()
  endif()
  set(${out_reason} "" PARENT_SCOPE)
endfunction()

function(fixture_expect_reason actual prefix)
  string(FIND "${actual}" "${prefix}" prefix_pos)
  if(prefix_pos LESS 0)
    message(FATAL_ERROR "FIXTURE_SELFTEST: expected ${prefix}, got '${actual}'")
  endif()
endfunction()

function(fixture_expect_no_host input expected_reason build_dir)
  set(probe_host "${build_dir}/host must not launch")
  execute_process(
    COMMAND "${CMAKE_COMMAND}"
      "-DHOST=${probe_host}"
      "-DCORE=${CORE}"
      "-DMODE=serialize"
      "-DSOURCE_DIR=${SOURCE_DIR}"
      "-DBUILD_DIR=${build_dir}/nested negative"
      "-DROMS=${input}"
      -P "${_fixture_identity_script}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error)
  set(combined "${output}\n${error}")
  string(FIND "${combined}" "${expected_reason}" reason_pos)
  if(result STREQUAL "0" OR reason_pos LESS 0 OR combined MATCHES "FIXTURE_HOST_EXIT")
    message(FATAL_ERROR
      "FIXTURE_SELFTEST: wrong-copy gate failed for ${input}; result='${result}'\n${combined}")
  endif()
endfunction()

function(fixture_identity_selftest)
  set(work "${BUILD_DIR}/fixture identity selftest")
  set(manifest "${SOURCE_DIR}/tests/roms/manifest.txt")
  file(MAKE_DIRECTORY "${work}/positive copies" "${work}/wrong copies"
    "${work}/generic/nesteroids" "${work}/generic/M1_P128K_C128K_S8K")
  set(cases 0)

  foreach(basename nesteroids.nes M1_P128K_C128K_S8K.nes)
    if(basename STREQUAL "nesteroids.nes")
      set(source "${SOURCE_DIR}/tests/roms/nesteroids.nes")
    else()
      set(source "${SOURCE_DIR}/tests/roms/hm/M1_P128K_C128K_S8K.nes")
    endif()
    set(copy "${work}/positive copies/${basename}")
    file(COPY_FILE "${source}" "${copy}" ONLY_IF_DIFFERENT)
    fixture_identity_check("${copy}" "${manifest}" reason)
    if(NOT reason STREQUAL "")
      message(FATAL_ERROR "FIXTURE_SELFTEST: valid copied input rejected: ${reason}")
    endif()
    math(EXPR cases "${cases} + 1")

    set(wrong_copy "${work}/wrong copies/${basename}")
    file(WRITE "${wrong_copy}" "not the listed fixture bytes\n")
    fixture_identity_check("${wrong_copy}" "${manifest}" reason)
    fixture_expect_reason("${reason}" "FIXTURE_IDENTITY:")
    fixture_expect_no_host("${wrong_copy}" "FIXTURE_IDENTITY:" "${work}/${basename}")
    math(EXPR cases "${cases} + 1")
  endforeach()

  file(COPY_FILE "${SOURCE_DIR}/tests/roms/dabg.nes"
    "${work}/generic/nesteroids/generic.nes" ONLY_IF_DIFFERENT)
  file(COPY_FILE "${SOURCE_DIR}/tests/roms/dabg.nes"
    "${work}/generic/M1_P128K_C128K_S8K/generic.nes" ONLY_IF_DIFFERENT)
  foreach(generic_input
      "${work}/generic/nesteroids/generic.nes"
      "${work}/generic/M1_P128K_C128K_S8K/generic.nes")
    fixture_identity_check("${generic_input}" "${manifest}" reason)
    if(NOT reason STREQUAL "")
      message(FATAL_ERROR "FIXTURE_SELFTEST: misleading parent changed generic role: ${reason}")
    endif()
    math(EXPR cases "${cases} + 1")
  endforeach()

  file(WRITE "${work}/missing manifest.txt" "# no special fixture row\n")
  fixture_identity_check("${work}/positive copies/nesteroids.nes"
    "${work}/missing manifest.txt" reason)
  fixture_expect_reason("${reason}" "FIXTURE_MANIFEST_MISSING:")
  math(EXPR cases "${cases} + 1")

  file(STRINGS "${manifest}" manifest_lines)
  set(nesteroids_row "")
  foreach(line IN LISTS manifest_lines)
    string(FIND "${line}" "${_tab}" path_end)
    if(path_end GREATER_EQUAL 0)
      string(SUBSTRING "${line}" 0 ${path_end} row_path)
      if(row_path STREQUAL "tests/roms/nesteroids.nes")
        set(nesteroids_row "${line}")
        break()
      endif()
    endif()
  endforeach()
  if(nesteroids_row STREQUAL "")
    message(FATAL_ERROR "FIXTURE_SELFTEST: source manifest lacks Nesteroids row")
  endif()
  file(WRITE "${work}/duplicate manifest.txt" "${nesteroids_row}\n${nesteroids_row}\n")
  fixture_identity_check("${work}/positive copies/nesteroids.nes"
    "${work}/duplicate manifest.txt" reason)
  fixture_expect_reason("${reason}" "FIXTURE_MANIFEST_DUPLICATE:")
  math(EXPR cases "${cases} + 1")

  fixture_identity_check("${work}/missing input/nesteroids.nes" "${manifest}" reason)
  fixture_expect_reason("${reason}" "FIXTURE_FILE_MISSING:")
  math(EXPR cases "${cases} + 1")

  # Exercise the production execute_process boundary with the actual C host in
  # argv-capture mode. CMake lists need escaped semicolons inside one ROM path.
  set(argv_work "${work}/argv boundary")
  set(argv_rom_one "${argv_work}/fixture space;semi/generic one.nes")
  set(argv_rom_two "${argv_work}/literal\\backslash;semi/other path.nes")
  set(argv_core "core path;semi\\module")
  string(REPLACE ";" "\\;" argv_rom_one_list "${argv_rom_one}")
  string(REPLACE ";" "\\;" argv_rom_two_list "${argv_rom_two}")
  set(argv_roms "${argv_rom_one_list};${argv_rom_two_list}")
  set(argv_capture "${argv_work}/captured argv.txt")
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env
      "NESTURBATOR_FIXTURE_ARGV_CAPTURE=${argv_capture}"
      "${CMAKE_COMMAND}"
      "-DHOST=${HOST}"
      "-DCORE=${argv_core}"
      -DMODE=serialize
      "-DSOURCE_DIR=${SOURCE_DIR}"
      "-DBUILD_DIR=${argv_work}/nested"
      "-DROMS=${argv_roms}"
      -P "${_fixture_identity_script}"
    RESULT_VARIABLE argv_result
    OUTPUT_VARIABLE argv_output
    ERROR_VARIABLE argv_error)
  if(NOT argv_result STREQUAL "0" OR NOT EXISTS "${argv_capture}")
    message(FATAL_ERROR
      "FIXTURE_ARGV_BOUNDARY: production wrapper did not reach the recording host")
  endif()
  file(READ "${argv_capture}" delivered_argv)
  set(expected_argv "4\n")
  string(LENGTH "--serialize" arg_length)
  string(APPEND expected_argv "${arg_length}:--serialize\n")
  string(LENGTH "${argv_core}" arg_length)
  string(APPEND expected_argv "${arg_length}:${argv_core}\n")
  string(LENGTH "${argv_rom_one}" arg_length)
  string(APPEND expected_argv "${arg_length}:${argv_rom_one}\n")
  string(LENGTH "${argv_rom_two}" arg_length)
  string(APPEND expected_argv "${arg_length}:${argv_rom_two}\n")
  if(NOT "${delivered_argv}" STREQUAL "${expected_argv}")
    message(FATAL_ERROR
      "FIXTURE_ARGV_BOUNDARY: host received split or altered arguments")
  endif()
  math(EXPR cases "${cases} + 1")

  if(NOT cases EQUAL 10)
    message(FATAL_ERROR "FIXTURE_SELFTEST: ${cases} cases ran, expected 10")
  endif()
  message(STATUS
    "libretro.fixture_identity: cases=${cases} positive_copies=2 mismatch_controls=2 generic=2 argv_boundary=1")
endfunction()

set(_fixture_identity_script "${CMAKE_CURRENT_LIST_FILE}")
if(SELFTEST)
  foreach(required HOST CORE SOURCE_DIR BUILD_DIR)
    if(NOT DEFINED ${required})
      message(FATAL_ERROR "FIXTURE_ARGUMENT_MISSING: ${required}")
    endif()
  endforeach()
  fixture_identity_selftest()
  return()
endif()

foreach(required HOST CORE MODE SOURCE_DIR BUILD_DIR ROMS)
  if(NOT DEFINED ${required})
    message(FATAL_ERROR "FIXTURE_ARGUMENT_MISSING: ${required}")
  endif()
endforeach()
set(manifest "${SOURCE_DIR}/tests/roms/manifest.txt")

set(prepared_roms)
set(index 0)
foreach(rom IN LISTS ROMS)
  math(EXPR index "${index} + 1")
  fixture_final_component("${rom}" basename)
  if(basename STREQUAL "nesteroids.nes"
      OR basename STREQUAL "M1_P128K_C128K_S8K.nes")
    if(NOT EXISTS "${rom}" OR IS_DIRECTORY "${rom}")
      message(FATAL_ERROR "FIXTURE_FILE_MISSING: ${rom}")
    endif()
    set(copy_dir "${BUILD_DIR}/${MODE}/${index}")
    file(MAKE_DIRECTORY "${copy_dir}")
    set(actual_rom "${copy_dir}/${basename}")
    file(COPY_FILE "${rom}" "${actual_rom}" ONLY_IF_DIFFERENT)
    fixture_identity_check("${actual_rom}" "${manifest}" reason)
    if(NOT reason STREQUAL "")
      message(FATAL_ERROR "${reason}")
    endif()
  else()
    set(actual_rom "${rom}")
  endif()
  list(APPEND prepared_roms "${actual_rom}")
endforeach()

if(MODE STREQUAL "serialize")
  set(host_arguments --serialize "${CORE}")
elseif(MODE STREQUAL "runahead")
  set(host_arguments --runahead "${CORE}")
  if(SHORT)
    list(APPEND host_arguments --short)
  endif()
  if(STRESS)
    list(APPEND host_arguments --stress)
  endif()
else()
  message(FATAL_ERROR "FIXTURE_MODE_INVALID: ${MODE}")
endif()
list(APPEND host_arguments ${prepared_roms})

execute_process(
  COMMAND "${HOST}" ${host_arguments}
  RESULT_VARIABLE host_result
  ECHO_OUTPUT_VARIABLE
  ECHO_ERROR_VARIABLE)
if(NOT host_result STREQUAL "0")
  message(FATAL_ERROR "FIXTURE_HOST_EXIT: ${host_result}")
endif()
