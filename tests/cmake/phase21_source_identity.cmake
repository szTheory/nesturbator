cmake_minimum_required(VERSION 3.25)

foreach(required IN ITEMS MODE ROOT MANIFEST)
  if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
    message(FATAL_ERROR "Phase 21 source identity requires ${required}")
  endif()
endforeach()
if(NOT MODE STREQUAL "write" AND NOT MODE STREQUAL "verify")
  message(FATAL_ERROR "Phase 21 source identity MODE must be write or verify")
endif()
if(MODE STREQUAL "verify" AND (NOT EXISTS "${MANIFEST}" OR IS_DIRECTORY "${MANIFEST}"))
  message(FATAL_ERROR "Phase 21 source identity modes/blob manifest is missing: ${MANIFEST}")
endif()

if(NOT EXISTS "${ROOT}/.git")
  message(FATAL_ERROR "Phase 21 source identity checkout is not a Git checkout: ${ROOT}")
endif()
execute_process(COMMAND git rev-parse --show-toplevel
  WORKING_DIRECTORY "${ROOT}" RESULT_VARIABLE git_result
  OUTPUT_VARIABLE git_root OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_VARIABLE git_error)
if(NOT "${git_result}" STREQUAL "0" OR NOT git_root STREQUAL ROOT)
  message(FATAL_ERROR "Phase 21 source identity root mismatch: ${git_error}")
endif()
execute_process(COMMAND git rev-parse HEAD
  WORKING_DIRECTORY "${ROOT}" RESULT_VARIABLE head_result
  OUTPUT_VARIABLE head_sha OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_VARIABLE head_error)
execute_process(COMMAND git rev-parse "HEAD^{tree}"
  WORKING_DIRECTORY "${ROOT}" RESULT_VARIABLE tree_result
  OUTPUT_VARIABLE tree_sha OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_VARIABLE tree_error)
execute_process(COMMAND git ls-files --stage
  WORKING_DIRECTORY "${ROOT}" RESULT_VARIABLE files_result
  OUTPUT_VARIABLE staged_files ERROR_VARIABLE files_error)
if(NOT "${head_result}" STREQUAL "0" OR NOT "${tree_result}" STREQUAL "0" OR
   NOT "${files_result}" STREQUAL "0")
  message(FATAL_ERROR "Phase 21 source identity Git query failed: ${head_error}${tree_error}${files_error}")
endif()

string(REPLACE "\r\n" "\n" staged_files "${staged_files}")
string(REPLACE "\n" ";" staged_lines "${staged_files}")
set(manifest "schema=1\nhead=${head_sha}\ntree=${tree_sha}\nexcluded=.planning/**;tests/accuracy/measurements.md\n")
set(included_count 0)
foreach(line IN LISTS staged_lines)
  if(line STREQUAL "")
    continue()
  endif()
  if(NOT line MATCHES "^([0-9]+) ([0-9a-f]+) ([0-9]+)\t(.+)$")
    message(FATAL_ERROR "Phase 21 source identity found malformed Git index row")
  endif()
  set(index_mode "${CMAKE_MATCH_1}")
  set(index_blob "${CMAKE_MATCH_2}")
  set(stage "${CMAKE_MATCH_3}")
  set(relative "${CMAKE_MATCH_4}")
  if(NOT stage STREQUAL "0")
    message(FATAL_ERROR "Phase 21 source identity requires stage-zero index entries: ${relative}")
  endif()
  if(relative MATCHES "^\.planning(/|$)" OR
     relative STREQUAL "tests/accuracy/measurements.md")
    continue()
  endif()
  if(IS_ABSOLUTE "${relative}")
    message(FATAL_ERROR "Phase 21 source identity has an unsafe tracked path: ${relative}")
  endif()

  execute_process(COMMAND git rev-parse "HEAD:${relative}"
    WORKING_DIRECTORY "${ROOT}" RESULT_VARIABLE committed_result
    OUTPUT_VARIABLE committed_blob OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_VARIABLE committed_error)
  set(committed_mode "missing")
  if("${committed_result}" STREQUAL "0")
    execute_process(COMMAND git ls-tree HEAD -- "${relative}"
      WORKING_DIRECTORY "${ROOT}" RESULT_VARIABLE mode_result
      OUTPUT_VARIABLE tree_entry OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_VARIABLE mode_error)
    if("${mode_result}" STREQUAL "0" AND
       tree_entry MATCHES "^([0-9]+) blob ${committed_blob}")
      set(committed_mode "${CMAKE_MATCH_1}")
    else()
      message(FATAL_ERROR "Phase 21 source identity cannot read committed Git mode at ${relative}: ${mode_error}")
    endif()
  else()
    set(committed_blob "missing")
  endif()

  set(absolute "${ROOT}/${relative}")
  if(NOT EXISTS "${absolute}" OR IS_DIRECTORY "${absolute}")
    message(FATAL_ERROR "Phase 21 current checkout is missing a tracked input: ${relative}")
  endif()
  if(IS_SYMLINK "${absolute}")
    file(READ_SYMLINK "${absolute}" symlink_target)
    set(hash_input "${MANIFEST}.symlink-input")
    file(WRITE "${hash_input}" "${symlink_target}")
    execute_process(COMMAND git hash-object --no-filters --stdin
      WORKING_DIRECTORY "${ROOT}" INPUT_FILE "${hash_input}"
      RESULT_VARIABLE work_result OUTPUT_VARIABLE work_blob
      OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_VARIABLE work_error)
    file(REMOVE "${hash_input}")
  else()
    execute_process(COMMAND git hash-object --no-filters -- "${absolute}"
      WORKING_DIRECTORY "${ROOT}" RESULT_VARIABLE work_result
      OUTPUT_VARIABLE work_blob OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_VARIABLE work_error)
  endif()
  if(NOT "${work_result}" STREQUAL "0")
    message(FATAL_ERROR "Phase 21 current checkout content cannot be hashed at ${relative}: ${work_error}")
  endif()
  string(APPEND manifest
    "${committed_mode}\t${committed_blob}\t${index_mode}\t${index_blob}\t${work_blob}\t${relative}\n")
  math(EXPR included_count "${included_count} + 1")
endforeach()
if(included_count EQUAL 0)
  message(FATAL_ERROR "Phase 21 source identity effective input inventory is empty")
endif()

if(MODE STREQUAL "write")
  get_filename_component(manifest_dir "${MANIFEST}" DIRECTORY)
  file(MAKE_DIRECTORY "${manifest_dir}")
  file(WRITE "${MANIFEST}" "${manifest}")
else()
  file(READ "${MANIFEST}" recorded_manifest)
  if(NOT recorded_manifest STREQUAL manifest)
    message(FATAL_ERROR "Phase 21 source identity modes/blob manifest differs from current checkout")
  endif()
endif()
file(SHA256 "${MANIFEST}" manifest_sha)
message(STATUS
  "Phase 21 tracked source identity ${MODE} verified: head=${head_sha}, inputs=${included_count}, sha256=${manifest_sha}")
