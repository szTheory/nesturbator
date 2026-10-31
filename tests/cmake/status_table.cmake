cmake_minimum_required(VERSION 3.25)

# D-07, D-19, D-20: the canonical API guide's status table equals the public
# enum and its native-state section retains the rewind warning. Run with
# -DSOURCE_DIR=<repo> -DDOC=<canonical-api-guide>.

function(status_table_fixture name has_doc doc_text out_root out_doc)
  set(root "${WORK}/status_table_${nonce}/${name}")
  file(MAKE_DIRECTORY "${root}/include" "${root}/docs")
  configure_file("${SOURCE_DIR}/include/nesturbator.h"
    "${root}/include/nesturbator.h" COPYONLY)
  file(WRITE "${root}/README.md" "${stale_readme}")
  set(doc "${root}/docs/api.md")
  if(has_doc)
    file(WRITE "${doc}" "${doc_text}")
  endif()
  set(${out_root} "${root}" PARENT_SCOPE)
  set(${out_doc} "${doc}" PARENT_SCOPE)
endfunction()

function(status_table_run root doc out_result out_text)
  execute_process(
    COMMAND "${CMAKE_COMMAND}" "-DSOURCE_DIR=${root}" "-DDOC=${doc}"
      -P "${CMAKE_CURRENT_LIST_FILE}"
    WORKING_DIRECTORY "${root}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error)
  set(${out_result} "${result}" PARENT_SCOPE)
  set(${out_text} "${output}\n${error}" PARENT_SCOPE)
endfunction()

function(status_table_expect_rejection name result output expected)
  if(result EQUAL 0)
    message(FATAL_ERROR "status table self-test: accepted ${name}")
  endif()
  if(NOT output MATCHES "${expected}")
    message(FATAL_ERROR
      "status table self-test: ${name} failed for the wrong reason: ${output}")
  endif()
endfunction()

if(SELFTEST)
  foreach(required SOURCE_DIR DOC WORK)
    if(NOT DEFINED ${required})
      message(FATAL_ERROR "${required} is required for the status table self-test")
    endif()
  endforeach()

  string(RANDOM LENGTH 16 ALPHABET 0123456789abcdef nonce)
  file(MAKE_DIRECTORY "${WORK}/status_table_${nonce}")
  if(EXISTS "${DOC}")
    file(READ "${DOC}" canonical_text)
  else()
    file(READ "${SOURCE_DIR}/README.md" canonical_text)
  endif()
  # Deliberately keep a complete old contract in every fixture README. Each
  # child invocation must consult only its DOC argument.
  set(stale_readme "${canonical_text}")

  status_table_fixture(valid TRUE "${canonical_text}" valid_root valid_doc)
  status_table_run("${valid_root}" "${valid_doc}" result output)
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "status table self-test: rejected valid canonical guide: ${output}")
  endif()

  status_table_fixture(missing FALSE "" root doc)
  status_table_run("${root}" "${doc}" result output)
  status_table_expect_rejection("missing canonical guide despite stale README text"
    "${result}" "${output}" "canonical API guide does not exist")

  file(STRINGS "${DOC}" canonical_lines)
  set(status_row "")
  foreach(line IN LISTS canonical_lines)
    if(line MATCHES "^\\| ([0-9]+) \\| `(NESTURBATOR_[A-Z_]+)` \\|")
      set(status_row "${line}")
      set(row_value "${CMAKE_MATCH_1}")
      set(row_symbol "${CMAKE_MATCH_2}")
      break()
    endif()
  endforeach()
  if(status_row STREQUAL "")
    message(FATAL_ERROR "status table self-test: no canonical status row to mutate")
  endif()
  set(pair "${row_value}:${row_symbol}")

  string(REPLACE "${status_row}" "" missing_row "${canonical_text}")
  status_table_fixture(missing_row TRUE "${missing_row}" root doc)
  status_table_run("${root}" "${doc}" result output)
  status_table_expect_rejection("missing enum row" "${result}" "${output}"
    "canonical API guide Status codes table lacks ${pair}")

  math(EXPR changed_value "${row_value} + 100")
  string(REPLACE "| ${row_value} |" "| ${changed_value} |" changed_row "${status_row}")
  string(REPLACE "${status_row}" "${changed_row}" changed_table "${canonical_text}")
  status_table_fixture(changed_row TRUE "${changed_table}" root doc)
  status_table_run("${root}" "${doc}" result output)
  status_table_expect_rejection("changed enum value" "${result}" "${output}"
    "canonical API guide Status codes table lacks ${pair}")

  set(duplicate_table "${canonical_text}\n${status_row}\n")
  status_table_fixture(duplicate_row TRUE "${duplicate_table}" root doc)
  status_table_run("${root}" "${doc}" result output)
  status_table_expect_rejection("duplicate enum row" "${result}" "${output}"
    "canonical API guide Status codes table has duplicate rows")

  set(extra_table "${canonical_text}\n| 99 | `NESTURBATOR_EXTRA` | extra status |\n")
  status_table_fixture(extra_row TRUE "${extra_table}" root doc)
  status_table_run("${root}" "${doc}" result output)
  status_table_expect_rejection("extra enum row" "${result}" "${output}"
    "Status codes table has 99:NESTURBATOR_EXTRA")

  string(REPLACE "## Save states" "## Native state" missing_heading "${canonical_text}")
  status_table_fixture(missing_heading TRUE "${missing_heading}" root doc)
  status_table_run("${root}" "${doc}" result output)
  status_table_expect_rejection("missing native-state heading" "${result}" "${output}"
    "canonical API guide lacks the '## Save states' section")

  string(REPLACE "Rewinding past an in-game save reverts the save RAM." ""
    missing_warning "${canonical_text}")
  status_table_fixture(missing_warning TRUE "${missing_warning}" root doc)
  status_table_run("${root}" "${doc}" result output)
  status_table_expect_rejection("missing rewind warning" "${result}" "${output}"
    "canonical API guide lacks 'Rewinding past an in-game save reverts the save")

  message(STATUS "status table self-test: canonical guide and stale-copy controls passed")
  return()
endif()

if(NOT DEFINED SOURCE_DIR)
  message(FATAL_ERROR "SOURCE_DIR is required")
endif()
if(NOT DEFINED DOC)
  message(FATAL_ERROR "DOC is required; pass the canonical API guide path")
endif()
if(NOT EXISTS "${DOC}")
  message(FATAL_ERROR "canonical API guide does not exist: ${DOC}")
endif()

file(READ "${SOURCE_DIR}/include/nesturbator.h" header)
string(FIND "${header}" "enum nesturbator_status {" start)
if(start EQUAL -1)
  message(FATAL_ERROR "enum nesturbator_status not found in the header")
endif()
string(SUBSTRING "${header}" ${start} -1 header_tail)
string(FIND "${header_tail}" "};" stop)
if(stop EQUAL -1)
  message(FATAL_ERROR "enum nesturbator_status has no closing brace in the header")
endif()
string(SUBSTRING "${header_tail}" 0 ${stop} enum_text)

set(enum_pairs "")
string(REGEX MATCHALL "NESTURBATOR_[A-Z_]+ = [0-9]+" enum_items "${enum_text}")
foreach(item IN LISTS enum_items)
  string(REGEX REPLACE "^(NESTURBATOR_[A-Z_]+) = ([0-9]+)$" "\\2:\\1" pair "${item}")
  list(APPEND enum_pairs "${pair}")
endforeach()

file(STRINGS "${DOC}" doc_lines)
set(doc_pairs "")
set(has_section FALSE)
set(has_sentence FALSE)
foreach(line IN LISTS doc_lines)
  if(line MATCHES "^\\| ([0-9]+) \\| `(NESTURBATOR_[A-Z_]+)` \\|")
    list(APPEND doc_pairs "${CMAKE_MATCH_1}:${CMAKE_MATCH_2}")
  endif()
  if(line STREQUAL "## Save states")
    set(has_section TRUE)
  endif()
  if(line MATCHES "Rewinding past an in-game save reverts the save RAM\\.")
    set(has_sentence TRUE)
  endif()
endforeach()

list(SORT enum_pairs COMPARE NATURAL)
list(SORT doc_pairs COMPARE NATURAL)
list(LENGTH enum_pairs enum_count)
list(LENGTH doc_pairs doc_count)
if(enum_count EQUAL 0)
  message(FATAL_ERROR "no enum values parsed from the header")
endif()
foreach(pair IN LISTS enum_pairs)
  if(NOT pair IN_LIST doc_pairs)
    message(FATAL_ERROR "canonical API guide Status codes table lacks ${pair}")
  endif()
endforeach()
foreach(pair IN LISTS doc_pairs)
  if(NOT pair IN_LIST enum_pairs)
    message(FATAL_ERROR "canonical API guide Status codes table has ${pair}, which is not in the enum")
  endif()
endforeach()
if(NOT enum_count EQUAL doc_count)
  message(FATAL_ERROR
    "canonical API guide Status codes table has duplicate rows (${doc_count} rows, ${enum_count} values)")
endif()
if(NOT has_section)
  message(FATAL_ERROR "canonical API guide lacks the '## Save states' section")
endif()
if(NOT has_sentence)
  message(FATAL_ERROR
    "canonical API guide lacks 'Rewinding past an in-game save reverts the save RAM.'")
endif()
message(STATUS "canonical API guide status table matches the enum (${enum_count} values)")
