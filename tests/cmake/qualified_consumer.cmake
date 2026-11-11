cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_DIR OR NOT DEFINED BINARY_DIR)
  message(FATAL_ERROR "qualified consumer requires SOURCE_DIR and BINARY_DIR")
endif()

get_filename_component(SOURCE_DIR "${SOURCE_DIR}" ABSOLUTE)
get_filename_component(BINARY_DIR "${BINARY_DIR}" ABSOLUTE)
set(rom "${SOURCE_DIR}/tests/roms/dabg.nes")
if(DEFINED CONTROL_MODE AND CONTROL_MODE STREQUAL "wrong-rom")
  if(NOT DEFINED ROM_OVERRIDE OR NOT EXISTS "${ROM_OVERRIDE}")
    message(FATAL_ERROR "wrong-ROM control requires an existing ROM_OVERRIDE")
  endif()
  file(SHA256 "${ROM_OVERRIDE}" wrong_rom_sha256)
  if(wrong_rom_sha256 STREQUAL "eca79b9d0b546e96c1bc73099e239539fb9c7773ff10a431bb3a9cd6763208ca")
    message(FATAL_ERROR "wrong-ROM control fixture unexpectedly matches the permitted DABG digest")
  endif()
  message(STATUS "wrong-ROM control rejected before consumer invocation: ${wrong_rom_sha256}")
  return()
endif()
if(NOT EXISTS "${rom}")
  message(FATAL_ERROR "permitted DABG ROM is missing: ${rom}")
endif()
file(SHA256 "${rom}" rom_sha256)
if(NOT rom_sha256 STREQUAL "eca79b9d0b546e96c1bc73099e239539fb9c7773ff10a431bb3a9cd6763208ca")
  message(FATAL_ERROR "DABG ROM digest mismatch: ${rom_sha256}")
endif()

string(RANDOM LENGTH 12 ALPHABET 0123456789abcdef run_id)
set(work "${BINARY_DIR}/qualified-consumer-${run_id}")
set(marker "${work}/.qualified-consumer-owned")
set(original_prefix "${work}/original")
set(moved_prefix "${work}/moved package with spaces")
set(consumer_build "${work}/consumer-build")
set(relocated_build "${work}/relocated-consumer-build")
file(MAKE_DIRECTORY "${work}")
file(WRITE "${marker}" "owned qualified consumer scratch\n")

if(DEFINED PREFIX AND NOT PREFIX STREQUAL "")
  get_filename_component(source_prefix "${PREFIX}" ABSOLUTE)
  if(NOT EXISTS "${source_prefix}/include/nesturbator.h")
    message(FATAL_ERROR "PREFIX is not an installed library tree: ${source_prefix}")
  endif()
  set(prefix_entries)
  foreach(entry include lib lib64 LICENSE)
    if(EXISTS "${source_prefix}/${entry}")
      list(APPEND prefix_entries "${source_prefix}/${entry}")
    endif()
  endforeach()
  file(COPY ${prefix_entries} DESTINATION "${original_prefix}")
else()
  set(install_command "${CMAKE_COMMAND}" --install "${BINARY_DIR}"
    --component library --prefix "${original_prefix}")
  if(DEFINED CONFIG AND NOT CONFIG STREQUAL "")
    list(APPEND install_command --config "${CONFIG}")
  endif()
  execute_process(COMMAND ${install_command}
    RESULT_VARIABLE install_result OUTPUT_VARIABLE install_out ERROR_VARIABLE install_err
    TIMEOUT 90)
  if(NOT install_result STREQUAL "0")
    message(FATAL_ERROR "library install failed (${install_result}): ${install_out}${install_err}")
  endif()
endif()

function(configure_consumer build_dir prefix_path strict)
  set(configure_command "${CMAKE_COMMAND}" -S "${SOURCE_DIR}/tests/consumer"
    -B "${build_dir}" "-DCMAKE_PREFIX_PATH=${prefix_path}"
    "-DQUALIFIED_PREFIX=${prefix_path}"
    "-DCMAKE_FIND_USE_PACKAGE_REGISTRY=FALSE"
    "-DCMAKE_FIND_USE_SYSTEM_PACKAGE_REGISTRY=FALSE"
    "-DCMAKE_FIND_USE_CMAKE_ENVIRONMENT_PATH=FALSE"
    "-DCMAKE_FIND_USE_PACKAGE_ROOT_PATH=FALSE"
    "-DCMAKE_FIND_USE_INSTALL_PREFIX=FALSE"
    "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON")
  if(DEFINED GENERATOR AND NOT GENERATOR STREQUAL "")
    list(APPEND configure_command -G "${GENERATOR}")
  endif()
  if(DEFINED C_COMPILER AND NOT C_COMPILER STREQUAL "")
    list(APPEND configure_command "-DCMAKE_C_COMPILER=${C_COMPILER}")
  endif()
  if(DEFINED BUILD_TYPE AND NOT BUILD_TYPE STREQUAL "")
    list(APPEND configure_command "-DCMAKE_BUILD_TYPE=${BUILD_TYPE}")
  endif()
  if(DEFINED C_FLAGS)
    list(APPEND configure_command "-DCMAKE_C_FLAGS=${C_FLAGS}")
  endif()
  if(DEFINED LINK_FLAGS)
    list(APPEND configure_command "-DCMAKE_EXE_LINKER_FLAGS=${LINK_FLAGS}")
  endif()
  if(DEFINED CONFIG AND NOT CONFIG STREQUAL "")
    list(APPEND configure_command "-DCMAKE_CONFIGURATION_TYPES=${CONFIG}")
  endif()
  execute_process(COMMAND ${configure_command}
    RESULT_VARIABLE configure_result OUTPUT_VARIABLE configure_out ERROR_VARIABLE configure_err
    TIMEOUT 90)
  if(NOT configure_result STREQUAL "0")
    message(FATAL_ERROR "${strict} consumer configure failed (${configure_result}): ${configure_out}${configure_err}")
  endif()
endfunction()

function(build_consumer build_dir label)
  set(build_command "${CMAKE_COMMAND}" --build "${build_dir}" --target consumer)
  if(DEFINED CONFIG AND NOT CONFIG STREQUAL "")
    list(APPEND build_command --config "${CONFIG}")
  endif()
  execute_process(COMMAND ${build_command}
    RESULT_VARIABLE build_result OUTPUT_VARIABLE build_out ERROR_VARIABLE build_err
    TIMEOUT 90)
  if(NOT build_result STREQUAL "0")
    message(FATAL_ERROR "${label} consumer build failed (${build_result}): ${build_out}${build_err}")
  endif()
endfunction()

function(find_consumer_executable build_dir output_var)
  set(candidates "${build_dir}/consumer" "${build_dir}/consumer.exe")
  if(DEFINED CONFIG AND NOT CONFIG STREQUAL "")
    list(APPEND candidates "${build_dir}/${CONFIG}/consumer" "${build_dir}/${CONFIG}/consumer.exe")
  endif()
  foreach(candidate IN LISTS candidates)
    if(EXISTS "${candidate}")
      set(${output_var} "${candidate}" PARENT_SCOPE)
      return()
    endif()
  endforeach()
  message(FATAL_ERROR "consumer executable was not found under ${build_dir}")
endfunction()

function(run_consumer executable label)
  execute_process(COMMAND "${executable}"
    RESULT_VARIABLE run_result OUTPUT_VARIABLE run_out ERROR_VARIABLE run_err TIMEOUT 15)
  if(NOT run_result STREQUAL "0")
    message(FATAL_ERROR "${label} no-argument consumer smoke failed (${run_result}): ${run_out}${run_err}")
  endif()
endfunction()

function(evidence_contains_path out evidence expected_path)
  # CMake/Ninja writes Windows drive colons and spaces as "$:" and "$ ".
  # MSVC command evidence uses backslash separators instead. Decode only these
  # documented spellings, then require the complete path with a token boundary.
  string(REPLACE "$ " " " normalized_evidence "${evidence}")
  string(REPLACE "$:" ":" normalized_evidence "${normalized_evidence}")
  string(REPLACE "\\" "/" normalized_evidence "${normalized_evidence}")
  string(REPLACE "\\" "/" normalized_expected "${expected_path}")
  set(found FALSE)
  set(search_from 0)
  string(LENGTH "${normalized_evidence}" evidence_length)
  string(LENGTH "${normalized_expected}" expected_length)
  while(search_from LESS evidence_length)
    string(SUBSTRING "${normalized_evidence}" ${search_from} -1 remaining_evidence)
    string(FIND "${remaining_evidence}" "${normalized_expected}" relative_index)
    if(relative_index LESS 0)
      break()
    endif()
    math(EXPR path_index "${search_from} + ${relative_index}")
    math(EXPR path_end "${path_index} + ${expected_length}")
    set(left_boundary TRUE)
    set(right_boundary TRUE)
    if(path_index GREATER 0)
      math(EXPR left_index "${path_index} - 1")
      string(SUBSTRING "${normalized_evidence}" ${left_index} 1 left_char)
      if(NOT left_char MATCHES "[ \t\r\n\"'=;|]")
        set(left_boundary FALSE)
      endif()
    endif()
    if(path_end LESS evidence_length)
      string(SUBSTRING "${normalized_evidence}" ${path_end} 1 right_char)
      if(NOT right_char MATCHES "[ \t\r\n\"'=;|]")
        set(right_boundary FALSE)
      endif()
    endif()
    if(left_boundary AND right_boundary)
      set(found TRUE)
      break()
    endif()
    set(search_from "${path_end}")
  endwhile()
  set(${out} "${found}" PARENT_SCOPE)
endfunction()

function(compile_commands_contain_path out compile_json expected_path)
  string(JSON command_count ERROR_VARIABLE json_error LENGTH "${compile_json}")
  if(json_error)
    set(${out} FALSE PARENT_SCOPE)
    return()
  endif()
  set(found FALSE)
  if(command_count GREATER 0)
    math(EXPR last_command "${command_count} - 1")
    foreach(command_index RANGE 0 ${last_command})
      string(JSON command ERROR_VARIABLE command_error GET
        "${compile_json}" ${command_index} command)
      if(command_error)
        string(JSON argument_count ERROR_VARIABLE arguments_error LENGTH
          "${compile_json}" ${command_index} arguments)
        if(NOT arguments_error AND argument_count GREATER 0)
          math(EXPR last_argument "${argument_count} - 1")
          set(command "")
          foreach(argument_index RANGE 0 ${last_argument})
            string(JSON argument GET "${compile_json}" ${command_index}
              arguments ${argument_index})
            string(APPEND command " ${argument}")
          endforeach()
        endif()
      endif()
      evidence_contains_path(command_has_path "${command}" "${expected_path}")
      if(command_has_path)
        set(found TRUE)
        break()
      endif()
    endforeach()
  endif()
  set(${out} "${found}" PARENT_SCOPE)
endfunction()

function(check_path_evidence_controls)
  set(prefix "D:/a/nesturbator/nesturbator/build/ci-msvc/qualified-consumer-control/moved package with spaces")
  set(imported_library "${prefix}/lib/nesturbator.lib")
  set(ninja_edge
    "build consumer.exe: C_EXECUTABLE_LINKER__consumer_Release consumer.obj | D$:/a/nesturbator/nesturbator/build/ci-msvc/qualified-consumer-control/moved$ package$ with$ spaces/lib/nesturbator.lib\n")
  set(msvc_link_command
    "link.exe /OUT:consumer.exe \"D:\\a\\nesturbator\\nesturbator\\build\\ci-msvc\\qualified-consumer-control\\moved package with spaces\\lib\\nesturbator.lib\"\n")
  set(old_prefix "D:/a/nesturbator/nesturbator/build/ci-msvc/qualified-consumer-control/original/lib/nesturbator.lib")
  set(other_library "${prefix}/lib/other.lib")
  set(suffix_lookalike "${imported_library}.backup")
  evidence_contains_path(found "${ninja_edge}" "${imported_library}")
  if(NOT found)
    message(FATAL_ERROR "synthetic Ninja drive/space spelling did not resolve the moved imported library")
  endif()
  evidence_contains_path(found "${msvc_link_command}" "${imported_library}")
  if(NOT found)
    message(FATAL_ERROR "synthetic MSVC backslash spelling did not resolve the moved imported library")
  endif()
  foreach(foreign_path IN ITEMS "${old_prefix}" "${other_library}" "${suffix_lookalike}")
    evidence_contains_path(found "${ninja_edge}" "${foreign_path}")
    if(found)
      message(FATAL_ERROR "synthetic Ninja control accepted foreign path ${foreign_path}")
    endif()
  endforeach()
  evidence_contains_path(found "" "${imported_library}")
  if(found)
    message(FATAL_ERROR "synthetic Ninja control accepted missing link evidence")
  endif()

  set(windows_compile_json [=[
    [{"directory":"D:\\build","file":"main.c","command":"cl.exe /I\"D:\\a\\nesturbator\\nesturbator\\build\\ci-msvc\\qualified-consumer-control\\moved package with spaces\\include\" /c main.c"}]
  ]=])
  compile_commands_contain_path(found "${windows_compile_json}" "${prefix}/include")
  if(NOT found)
    message(FATAL_ERROR "synthetic Windows compile_commands.json did not resolve its moved include")
  endif()
  compile_commands_contain_path(found "${windows_compile_json}" "${prefix}/source/include")
  if(found)
    message(FATAL_ERROR "synthetic Windows compile command accepted a foreign include path")
  endif()
  set(source_include_json [=[
    [{"directory":"D:\\build","file":"main.c","command":"cl.exe /I\"D:\\a\\nesturbator\\nesturbator\\include\" /c main.c"}]
  ]=])
  compile_commands_contain_path(found "${source_include_json}"
    "D:/a/nesturbator/nesturbator/include")
  if(NOT found)
    message(FATAL_ERROR "synthetic Windows source-tree include was not detected for rejection")
  endif()
  set(build_include_json [=[
    [{"directory":"D:\\build","file":"main.c","command":"cl.exe /I\"D:\\a\\nesturbator\\nesturbator\\build\\ci-msvc\\include\" /c main.c"}]
  ]=])
  compile_commands_contain_path(found "${build_include_json}"
    "D:/a/nesturbator/nesturbator/build/ci-msvc/include")
  if(NOT found)
    message(FATAL_ERROR "synthetic Windows build-tree include was not detected for rejection")
  endif()
endfunction()

check_path_evidence_controls()

configure_consumer("${consumer_build}" "${original_prefix}" "original")
build_consumer("${consumer_build}" "original")
find_consumer_executable("${consumer_build}" original_executable)
run_consumer("${original_executable}" "original")

if(NOT EXISTS "${marker}" OR EXISTS "${moved_prefix}")
  message(FATAL_ERROR "owned scratch marker missing or moved-prefix destination already exists")
endif()
file(RENAME "${original_prefix}" "${moved_prefix}")
if(EXISTS "${original_prefix}" OR NOT EXISTS "${moved_prefix}/include/nesturbator.h")
  message(FATAL_ERROR "install prefix did not move to the path containing spaces")
endif()

cmake_path(IS_PREFIX work "${consumer_build}" NORMALIZE consumer_build_inside)
if(NOT consumer_build_inside OR NOT EXISTS "${marker}")
  message(FATAL_ERROR "refusing to remove a consumer build outside owned scratch")
endif()
file(REMOVE_RECURSE "${consumer_build}")
if(EXISTS "${consumer_build}")
  message(FATAL_ERROR "earlier consumer build directory survived its removal")
endif()

configure_consumer("${relocated_build}" "${moved_prefix}" "relocated")
build_consumer("${relocated_build}" "relocated")
find_consumer_executable("${relocated_build}" relocated_executable)
run_consumer("${relocated_executable}" "relocated")

set(resolution_file "${relocated_build}/package-resolution.txt")
set(compile_commands_file "${relocated_build}/compile_commands.json")
set(link_file "${relocated_build}/CMakeFiles/consumer.dir/link.txt")
if(EXISTS "${relocated_build}/build.ninja")
  set(link_file "${relocated_build}/build.ninja")
endif()
if(NOT EXISTS "${resolution_file}" OR NOT EXISTS "${compile_commands_file}" OR NOT EXISTS "${link_file}")
  message(FATAL_ERROR "relocated consumer did not retain package, compile and link lookup evidence")
endif()
file(READ "${resolution_file}" resolution_evidence)
file(READ "${compile_commands_file}" compile_evidence)
file(READ "${link_file}" link_evidence)
evidence_contains_path(moved_resolution "${resolution_evidence}" "${moved_prefix}")
compile_commands_contain_path(moved_include "${compile_evidence}" "${moved_prefix}/include")
string(FIND "${resolution_evidence}" "libraries=" imported_libraries_index)
set(imported_library_linked FALSE)
set(imported_libraries_text "")
if(NOT imported_libraries_index LESS 0)
  string(SUBSTRING "${resolution_evidence}" ${imported_libraries_index} -1 imported_libraries_line)
  string(FIND "${imported_libraries_line}" "\n" imported_libraries_line_end)
  if(NOT imported_libraries_line_end LESS 0)
    string(SUBSTRING "${imported_libraries_line}" 0 ${imported_libraries_line_end}
      imported_libraries_line)
  endif()
  string(REGEX REPLACE "^libraries=" "" imported_libraries_text "${imported_libraries_line}")
  set(imported_libraries ${imported_libraries_text})
  foreach(imported_library IN LISTS imported_libraries)
    if(NOT imported_library STREQUAL "")
      evidence_contains_path(imported_library_found "${link_evidence}" "${imported_library}")
      if(imported_library_found)
        set(imported_library_linked TRUE)
      endif()
    endif()
  endforeach()
endif()
compile_commands_contain_path(source_include_present "${compile_evidence}" "${SOURCE_DIR}/include")
compile_commands_contain_path(binary_include_present "${compile_evidence}" "${BINARY_DIR}/include")
if(NOT moved_resolution OR NOT moved_include OR NOT imported_library_linked OR
   source_include_present OR binary_include_present)
  message(FATAL_ERROR
    "relocated consumer evidence failed: moved_resolution=${moved_resolution}, "
    "moved_include=${moved_include}, imported_library_linked=${imported_library_linked}, "
    "source_include_present=${source_include_present}, "
    "binary_include_present=${binary_include_present}, "
    "expected_imported_libraries='${imported_libraries_text}'")
endif()

set(gameplay_report "${work}/gameplay.json")
execute_process(COMMAND "${relocated_executable}" --gameplay "${rom}" --report "${gameplay_report}"
  RESULT_VARIABLE gameplay_result OUTPUT_VARIABLE gameplay_out ERROR_VARIABLE gameplay_err
  TIMEOUT 15)
if(NOT gameplay_result STREQUAL "0")
  message(FATAL_ERROR "relocated gameplay consumer failed (${gameplay_result}): ${gameplay_out}${gameplay_err}")
endif()
if(NOT EXISTS "${gameplay_report}")
  message(FATAL_ERROR "relocated consumer did not write the gameplay evidence report")
endif()
file(READ "${gameplay_report}" runtime_evidence)
string(FIND "${runtime_evidence}" "\"passed\": true" pass_index)
if(pass_index LESS 0)
  message(FATAL_ERROR "relocated gameplay report did not pass its state, motion, pixel and PCM predicates")
endif()

set(no_input_report "${work}/no-input.json")
execute_process(COMMAND "${relocated_executable}" --control no-input --rom "${rom}"
  --report "${no_input_report}"
  RESULT_VARIABLE no_input_result OUTPUT_VARIABLE no_input_out ERROR_VARIABLE no_input_err
  TIMEOUT 15)
if(NOT EXISTS "${no_input_report}")
  message(FATAL_ERROR
    "ASSERTION FAILED: no-input control must complete 110 frames and report its named "
    "input-progress rejection (process status ${no_input_result}: ${no_input_out}${no_input_err})")
endif()
if(NOT no_input_result STREQUAL "0")
  message(FATAL_ERROR "no-input semantic control did not complete successfully (${no_input_result}): ${no_input_out}${no_input_err}")
endif()
file(READ "${no_input_report}" no_input_evidence)
string(FIND "${no_input_evidence}" "\"completed_frames\": 110" no_input_frame_count)
string(FIND "${no_input_evidence}" "\"input_caused_progress\": false" no_input_reason)
string(FIND "${no_input_evidence}" "\"game_state\": false" no_input_state)
string(FIND "${no_input_evidence}" "\"semantic_control_completed\": true" no_input_completed)
if(no_input_frame_count LESS 0 OR no_input_reason LESS 0 OR no_input_state LESS 0 OR no_input_completed LESS 0)
  message(FATAL_ERROR "no-input control did not name its expected semantic failure after 110 frames")
endif()

set(start_only_report "${work}/start-only.json")
execute_process(COMMAND "${relocated_executable}" --control start-only --rom "${rom}"
  --report "${start_only_report}"
  RESULT_VARIABLE start_only_result OUTPUT_VARIABLE start_only_out ERROR_VARIABLE start_only_err
  TIMEOUT 15)
if(NOT start_only_result STREQUAL "0" OR NOT EXISTS "${start_only_report}")
  message(FATAL_ERROR "Start-only semantic control did not complete (${start_only_result}): ${start_only_out}${start_only_err}")
endif()
file(READ "${start_only_report}" start_only_evidence)
string(FIND "${start_only_evidence}" "\"completed_frames\": 110" start_only_frame_count)
string(FIND "${start_only_evidence}" "\"input_caused_progress\": false" start_only_reason)
string(FIND "${start_only_evidence}" "\"game_state\": false" start_only_state)
string(FIND "${start_only_evidence}" "\"semantic_control_completed\": true" start_only_completed)
if(start_only_frame_count LESS 0 OR start_only_reason LESS 0 OR
   start_only_state LESS 0 OR start_only_completed LESS 0)
  message(FATAL_ERROR "Start-only control did not complete and fail the input-progress predicate")
endif()

foreach(synthetic_key IN ITEMS
    "\"missing_game_state_rejected\": true"
    "\"missing_input_progress_rejected\": true"
    "\"missing_pixels_rejected\": true"
    "\"silent_pcm_rejected\": true")
  string(FIND "${runtime_evidence}" "${synthetic_key}" synthetic_index)
  if(synthetic_index LESS 0)
    message(FATAL_ERROR "positive report is missing synthetic verdict control ${synthetic_key}")
  endif()
endforeach()
string(FIND "${runtime_evidence}" "\"refused_load_status\": 7" refusal_status_index)
string(FIND "${runtime_evidence}" "\"refusal_preserved\": true" refusal_preserved_index)
string(FIND "${runtime_evidence}" "\"continuation_equal\": true" continuation_index)
string(FIND "${runtime_evidence}" "\"continuation_frames\": 30" continuation_frames_index)
string(FIND "${runtime_evidence}" "\"buffer_edges_passed\": true" buffer_edges_index)
if(refusal_status_index LESS 0 OR refusal_preserved_index LESS 0 OR
   continuation_index LESS 0 OR continuation_frames_index LESS 0 OR buffer_edges_index LESS 0)
  message(FATAL_ERROR "positive report is missing refused-load continuation or buffer-edge evidence")
endif()

set(wrong_rom "${work}/wrong-rom.nes")
file(WRITE "${wrong_rom}" "wrong ROM control image\n")
execute_process(COMMAND "${CMAKE_COMMAND}"
    "-DSOURCE_DIR=${SOURCE_DIR}" "-DBINARY_DIR=${BINARY_DIR}"
    "-DCONTROL_MODE=wrong-rom" "-DROM_OVERRIDE=${wrong_rom}"
    -P "${CMAKE_CURRENT_LIST_FILE}"
  RESULT_VARIABLE wrong_rom_result OUTPUT_VARIABLE wrong_rom_out ERROR_VARIABLE wrong_rom_err
  TIMEOUT 15)
string(FIND "${wrong_rom_out}${wrong_rom_err}" "wrong-ROM control rejected before consumer invocation" wrong_rom_rejection)
if(NOT wrong_rom_result STREQUAL "0" OR wrong_rom_rejection LESS 0)
  message(FATAL_ERROR "wrong-ROM control did not stop at the digest gate (${wrong_rom_result}): ${wrong_rom_out}${wrong_rom_err}")
endif()

set(incomplete_prefix "${work}/incomplete-prefix")
set(incomplete_build "${work}/incomplete-consumer-build")
file(MAKE_DIRECTORY "${incomplete_prefix}/include")
file(COPY "${SOURCE_DIR}/include/nesturbator.h" DESTINATION "${incomplete_prefix}/include")
set(incomplete_configure "${CMAKE_COMMAND}" -S "${SOURCE_DIR}/tests/consumer"
  -B "${incomplete_build}" "-DQUALIFIED_PREFIX=${incomplete_prefix}"
  "-DCMAKE_PREFIX_PATH=${incomplete_prefix}"
  "-DCMAKE_FIND_USE_CMAKE_ENVIRONMENT_PATH=FALSE"
  "-DCMAKE_FIND_USE_PACKAGE_REGISTRY=FALSE"
  "-DCMAKE_FIND_USE_SYSTEM_PACKAGE_REGISTRY=FALSE")
if(DEFINED GENERATOR AND NOT GENERATOR STREQUAL "")
  list(APPEND incomplete_configure -G "${GENERATOR}")
endif()
execute_process(COMMAND ${incomplete_configure}
  RESULT_VARIABLE incomplete_result OUTPUT_VARIABLE incomplete_out ERROR_VARIABLE incomplete_err
  TIMEOUT 15)
string(FIND "${incomplete_out}${incomplete_err}" "Could not find a package configuration file" incomplete_package_error)
if(incomplete_result STREQUAL "0" OR incomplete_package_error LESS 0)
  message(FATAL_ERROR "incomplete-prefix control did not fail specifically at package discovery (${incomplete_result}): ${incomplete_out}${incomplete_err}")
endif()

file(READ "${relocated_build}/CMakeCache.txt" cache_evidence)
set(original_prefix_absent FALSE)
set(previous_consumer_build_absent FALSE)
if(NOT EXISTS "${original_prefix}")
  set(original_prefix_absent TRUE)
endif()
if(NOT EXISTS "${consumer_build}")
  set(previous_consumer_build_absent TRUE)
endif()
file(WRITE "${work}/configuration-provenance.txt"
  "generator=${GENERATOR}\n"
  "compiler=${C_COMPILER}\n"
  "config=${CONFIG}\n"
  "build_type=${BUILD_TYPE}\n"
  "c_flags=${C_FLAGS}\n"
  "link_flags=${LINK_FLAGS}\n"
  "rom_sha256=${rom_sha256}\n"
  "original_prefix_absent=${original_prefix_absent}\n"
  "moved_prefix=${moved_prefix}\n"
  "previous_consumer_build_absent=${previous_consumer_build_absent}\n"
  "resolution:\n${resolution_evidence}\n"
  "link_file=${link_file}\n"
  "link_evidence:\n${link_evidence}\n"
  "cache_find_package=${cache_evidence}\n"
  "compile_commands=${compile_evidence}\n"
  "runtime_report=${runtime_evidence}\n"
  "no_input_report=${no_input_evidence}\n"
  "start_only_report=${start_only_evidence}\n"
  "wrong_rom_path=${wrong_rom}\n"
  "wrong_rom_result=${wrong_rom_result}\n"
  "wrong_rom_output=${wrong_rom_out}${wrong_rom_err}\n"
  "incomplete_prefix=${incomplete_prefix}\n"
  "incomplete_configure_result=${incomplete_result}\n"
  "incomplete_configure_output=${incomplete_out}${incomplete_err}\n")
message(STATUS "qualified moved-prefix gameplay consumer passed; evidence: ${work}")
