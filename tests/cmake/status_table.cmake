cmake_minimum_required(VERSION 3.25)

# The first relocation control starts from the old README contract and proves
# that the production consumer rejects a missing canonical guide even when the
# stale copy still has a complete status table and native-state section.
if(SELFTEST)
  foreach(required SOURCE_DIR DOC WORK)
    if(NOT DEFINED ${required})
      message(FATAL_ERROR "${required} is required for the status table self-test")
    endif()
  endforeach()

  string(RANDOM LENGTH 16 ALPHABET 0123456789abcdef nonce)
  set(fixture "${WORK}/canonical_${nonce}")
  file(MAKE_DIRECTORY "${fixture}/include")
  configure_file("${SOURCE_DIR}/include/nesturbator.h"
    "${fixture}/include/nesturbator.h" COPYONLY)

  if(EXISTS "${DOC}")
    file(READ "${DOC}" stale_readme)
  else()
    file(READ "${SOURCE_DIR}/README.md" stale_readme)
  endif()
  file(WRITE "${fixture}/README.md" "${stale_readme}")

  set(missing_doc "${fixture}/docs/api.md")
  execute_process(
    COMMAND "${CMAKE_COMMAND}" "-DSOURCE_DIR=${fixture}" "-DDOC=${missing_doc}"
      -P "${CMAKE_CURRENT_LIST_FILE}"
    WORKING_DIRECTORY "${fixture}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error)
  if(result STREQUAL "0")
    message(FATAL_ERROR
      "stale README copy satisfied a missing canonical API guide; DOC was ignored")
  endif()
  string(CONCAT output "${output}" "${error}")
  if(NOT output MATCHES "canonical API guide does not exist")
    message(FATAL_ERROR
      "missing canonical API guide failed for the wrong reason: ${output}")
  endif()
  message(STATUS "status table rejects a missing canonical API guide despite stale README text")
  return()
endif()

# D-07: the README "Status codes" table lists exactly the values of enum
# nesturbator_status, and the README carries the Save states section and the
# sentence about rewinding past an in-game save. Run with -DSOURCE_DIR=<repo>.
if(NOT DEFINED SOURCE_DIR)
  message(FATAL_ERROR "SOURCE_DIR is required")
endif()

file(READ "${SOURCE_DIR}/include/nesturbator.h" header)
string(FIND "${header}" "enum nesturbator_status {" start)
if(start EQUAL -1)
  message(FATAL_ERROR "enum nesturbator_status not found in the header")
endif()
string(SUBSTRING "${header}" ${start} -1 header_tail)
string(FIND "${header_tail}" "};" stop)
string(SUBSTRING "${header_tail}" 0 ${stop} enum_text)

set(enum_pairs "")
string(REGEX MATCHALL "NESTURBATOR_[A-Z_]+ = [0-9]+" enum_items "${enum_text}")
foreach(item IN LISTS enum_items)
  string(REGEX REPLACE "^(NESTURBATOR_[A-Z_]+) = ([0-9]+)$" "\\2:\\1" pair "${item}")
  list(APPEND enum_pairs "${pair}")
endforeach()

file(STRINGS "${SOURCE_DIR}/README.md" readme_lines)
set(readme_pairs "")
set(has_section FALSE)
set(has_sentence FALSE)
foreach(line IN LISTS readme_lines)
  if(line MATCHES "^\\| ([0-9]+) \\| `(NESTURBATOR_[A-Z_]+)` \\|")
    list(APPEND readme_pairs "${CMAKE_MATCH_1}:${CMAKE_MATCH_2}")
  endif()
  if(line STREQUAL "### Save states")
    set(has_section TRUE)
  endif()
  if(line MATCHES "Rewinding past an in-game save reverts the save RAM\\.")
    set(has_sentence TRUE)
  endif()
endforeach()

list(SORT enum_pairs COMPARE NATURAL)
list(SORT readme_pairs COMPARE NATURAL)
list(LENGTH enum_pairs enum_count)
list(LENGTH readme_pairs readme_count)
if(enum_count EQUAL 0)
  message(FATAL_ERROR "no enum values parsed from the header")
endif()
foreach(pair IN LISTS enum_pairs)
  if(NOT pair IN_LIST readme_pairs)
    message(FATAL_ERROR "README Status codes table lacks ${pair}")
  endif()
endforeach()
foreach(pair IN LISTS readme_pairs)
  if(NOT pair IN_LIST enum_pairs)
    message(FATAL_ERROR "README Status codes table has ${pair}, which is not in the enum")
  endif()
endforeach()
if(NOT enum_count EQUAL readme_count)
  message(FATAL_ERROR "README Status codes table has duplicate rows (${readme_count} rows, ${enum_count} values)")
endif()
if(NOT has_section)
  message(FATAL_ERROR "README.md lacks the '### Save states' section")
endif()
if(NOT has_sentence)
  message(FATAL_ERROR "README.md lacks 'Rewinding past an in-game save reverts the save RAM.'")
endif()
message(STATUS "status table matches the enum (${enum_count} values)")
