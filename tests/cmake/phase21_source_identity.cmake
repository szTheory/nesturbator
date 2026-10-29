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
get_filename_component(manifest_dir "${MANIFEST}" DIRECTORY)
file(MAKE_DIRECTORY "${manifest_dir}")
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
execute_process(COMMAND git ls-tree -r HEAD
  WORKING_DIRECTORY "${ROOT}" RESULT_VARIABLE committed_result
  OUTPUT_VARIABLE committed_files ERROR_VARIABLE committed_error)
if(NOT "${head_result}" STREQUAL "0" OR NOT "${tree_result}" STREQUAL "0" OR
   NOT "${files_result}" STREQUAL "0" OR NOT "${committed_result}" STREQUAL "0")
  message(FATAL_ERROR "Phase 21 source identity Git query failed: ${head_error}${tree_error}${files_error}${committed_error}")
endif()

# Keep committed, index and raw worktree identities independent, but query each
# inventory in bulk. Starting Git three times per path consumed the controls'
# unchanged 120-second cap on slower process-launch hosts.
string(REPLACE "\r\n" "\n" committed_files "${committed_files}")
string(REPLACE "\n" ";" committed_lines "${committed_files}")
foreach(line IN LISTS committed_lines)
  if(line MATCHES "^([0-9]+) blob ([0-9a-f]+)\t(.+)$")
    set("committed_mode_${CMAKE_MATCH_3}" "${CMAKE_MATCH_1}")
    set("committed_blob_${CMAKE_MATCH_3}" "${CMAKE_MATCH_2}")
  endif()
endforeach()

string(REPLACE "\r\n" "\n" staged_files "${staged_files}")
string(REPLACE "\n" ";" staged_lines "${staged_files}")
set(manifest "schema=1\nhead=${head_sha}\ntree=${tree_sha}\nexcluded=.planning/**;tests/accuracy/measurements.md\n")
set(included_count 0)
set(regular_paths "")
set(regular_inputs "")
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

  set(absolute "${ROOT}/${relative}")
  if(NOT EXISTS "${absolute}" OR IS_DIRECTORY "${absolute}")
    message(FATAL_ERROR "Phase 21 current checkout is missing a tracked input: ${relative}")
  endif()
  list(APPEND included_paths "${relative}")
  if(NOT IS_SYMLINK "${absolute}")
    # --stdin-paths accepts Git's double-quoted pathname syntax, including spaces.
    string(REPLACE "\\" "\\\\" quoted "${absolute}")
    string(REPLACE "\"" "\\\"" quoted "${quoted}")
    string(APPEND regular_inputs "\"${quoted}\"\n")
    list(APPEND regular_paths "${relative}")
  else()
    file(READ_SYMLINK "${absolute}" symlink_target)
    set(hash_input "${MANIFEST}.symlink-input")
    file(WRITE "${hash_input}" "${symlink_target}")
    execute_process(COMMAND git hash-object --no-filters --stdin
      WORKING_DIRECTORY "${ROOT}" INPUT_FILE "${hash_input}"
      RESULT_VARIABLE work_result OUTPUT_VARIABLE work_blob
      OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_VARIABLE work_error)
    file(REMOVE "${hash_input}")
    if(NOT "${work_result}" STREQUAL "0")
      message(FATAL_ERROR "Phase 21 current checkout content cannot be hashed at ${relative}: ${work_error}")
    endif()
    set("work_blob_${relative}" "${work_blob}")
  endif()
  set("index_mode_${relative}" "${index_mode}")
  set("index_blob_${relative}" "${index_blob}")
endforeach()

if(regular_paths)
  set(hash_input "${MANIFEST}.path-input")
  get_filename_component(manifest_dir "${MANIFEST}" DIRECTORY)
  file(MAKE_DIRECTORY "${manifest_dir}")
  file(WRITE "${hash_input}" "${regular_inputs}")
  execute_process(COMMAND git hash-object --no-filters --stdin-paths
    WORKING_DIRECTORY "${ROOT}" INPUT_FILE "${hash_input}"
    RESULT_VARIABLE work_result OUTPUT_VARIABLE work_blobs
    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_VARIABLE work_error)
  file(REMOVE "${hash_input}")
  string(REPLACE "\n" ";" work_blobs "${work_blobs}")
  list(LENGTH regular_paths regular_count)
  list(LENGTH work_blobs hash_count)
  if(NOT "${work_result}" STREQUAL "0" OR NOT regular_count EQUAL hash_count)
    message(FATAL_ERROR "Phase 21 current checkout content cannot be hashed: ${work_error}")
  endif()
  foreach(relative work_blob IN ZIP_LISTS regular_paths work_blobs)
    string(STRIP "${work_blob}" work_blob)
    set("work_blob_${relative}" "${work_blob}")
  endforeach()
endif()

foreach(relative IN LISTS included_paths)
  set(committed_mode "${committed_mode_${relative}}")
  set(committed_blob "${committed_blob_${relative}}")
  if(committed_mode STREQUAL "")
    set(committed_mode "missing")
    set(committed_blob "missing")
  endif()
  string(APPEND manifest
    "${committed_mode}\t${committed_blob}\t${index_mode_${relative}}\t${index_blob_${relative}}\t${work_blob_${relative}}\t${relative}\n")
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
