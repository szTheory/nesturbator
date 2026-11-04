cmake_minimum_required(VERSION 3.25)

function(runner_exit_fixture name has_doc doc_text out_root out_doc)
  set(root "${fixture_root}/${name}")
  file(MAKE_DIRECTORY "${root}/docs" "${root}/runner")
  configure_file("${SOURCE_DIR}/runner/exit_status.h"
    "${root}/runner/exit_status.h" COPYONLY)
  file(WRITE "${root}/README.md" "${stale_readme}")
  set(doc "${root}/docs/runner.md")
  if(has_doc)
    file(WRITE "${doc}" "${doc_text}")
  endif()
  set(${out_root} "${root}" PARENT_SCOPE)
  set(${out_doc} "${doc}" PARENT_SCOPE)
endfunction()

function(runner_exit_run root doc mode out_result out_text)
  execute_process(
    COMMAND "${CMAKE_COMMAND}" "-DSOURCE_DIR=${root}" "-DDOC=${doc}"
      "-DRUNNER=${RUNNER}" "-DMODE=${mode}" -P "${CMAKE_CURRENT_LIST_FILE}"
    WORKING_DIRECTORY "${root}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error)
  set(${out_result} "${result}" PARENT_SCOPE)
  set(${out_text} "${output}\n${error}" PARENT_SCOPE)
endfunction()

function(runner_exit_expect_rejection name result output expected)
  if(result EQUAL 0)
    message(FATAL_ERROR "runner exit self-test: accepted ${name}")
  endif()
  string(FIND "${output}" "${expected}" at)
  if(at EQUAL -1)
    message(FATAL_ERROR "runner exit self-test: ${name} failed for the wrong reason: ${output}")
  endif()
endfunction()

if(SELFTEST)
  foreach(required SOURCE_DIR DOC RUNNER WORK)
    if(NOT DEFINED ${required})
      message(FATAL_ERROR "${required} is required for the runner exit self-test")
    endif()
  endforeach()

  string(RANDOM LENGTH 16 ALPHABET 0123456789abcdef nonce)
  set(fixture_root "${WORK}/runner_exit_${nonce}")
  if(EXISTS "${fixture_root}")
    message(FATAL_ERROR "runner exit self-test scratch path already exists: ${fixture_root}")
  endif()
  file(MAKE_DIRECTORY "${fixture_root}")

  if(EXISTS "${DOC}")
    file(READ "${DOC}" canonical_text)
    set(stale_readme "## The runner\n${canonical_text}\n## The libretro core\n")
  else()
    # RED-phase fixture: the pre-migration README is the complete obsolete copy.
    file(READ "${SOURCE_DIR}/README.md" canonical_text)
    set(stale_readme "${canonical_text}")
  endif()

  runner_exit_fixture(valid TRUE "${canonical_text}" root doc)
  runner_exit_run("${root}" "${doc}" table result output)
  if(NOT result EQUAL 0)
    file(REMOVE_RECURSE "${fixture_root}")
    message(FATAL_ERROR "runner exit self-test: rejected valid canonical guide: ${output}")
  endif()

  runner_exit_fixture(missing_guide FALSE "" root doc)
  runner_exit_run("${root}" "${doc}" table result output)
  if(result EQUAL 0)
    file(REMOVE_RECURSE "${fixture_root}")
    message(FATAL_ERROR
      "runner exit self-test: accepted missing canonical guide despite complete stale README copy")
  endif()
  string(FIND "${output}" "canonical runner guide does not exist" at)
  if(at EQUAL -1)
    file(REMOVE_RECURSE "${fixture_root}")
    message(FATAL_ERROR "runner exit self-test: missing guide failed for the wrong reason: ${output}")
  endif()

  string(FIND "${canonical_text}" "| 0 | done |" at)
  if(at EQUAL -1)
    file(REMOVE_RECURSE "${fixture_root}")
    message(FATAL_ERROR "runner exit self-test: no canonical exit meaning to mutate")
  endif()
  string(REPLACE "| 0 | done |" "| 0 | not done |" changed_meaning "${canonical_text}")
  runner_exit_fixture(changed_meaning TRUE "${changed_meaning}" root doc)
  runner_exit_run("${root}" "${doc}" table result output)
  runner_exit_expect_rejection("changed exit meaning" "${result}" "${output}"
    "canonical runner guide exit rows differ from the header")

  string(FIND "${canonical_text}" "3 and 77 are reserved" at)
  if(at EQUAL -1)
    file(REMOVE_RECURSE "${fixture_root}")
    message(FATAL_ERROR "runner exit self-test: no reserved sentence to mutate")
  endif()
  string(REPLACE "3 and 77 are reserved; the runner never returns them." ""
    missing_reserved "${canonical_text}")
  runner_exit_fixture(missing_reserved TRUE "${missing_reserved}" root doc)
  runner_exit_run("${root}" "${doc}" table result output)
  runner_exit_expect_rejection("missing reserved sentence" "${result}" "${output}"
    "canonical runner guide Exit status lacks the reserved sentence")

  string(FIND "${canonical_text}" "- `--raise-after N[:SIG[:K]]`" at)
  if(at EQUAL -1)
    file(REMOVE_RECURSE "${fixture_root}")
    message(FATAL_ERROR "runner exit self-test: no --raise-after option bullet to mutate")
  endif()
  string(REPLACE "- `--raise-after N[:SIG[:K]]`" "" missing_raise_after "${canonical_text}")
  runner_exit_fixture(missing_raise_after TRUE "${missing_raise_after}" root doc)
  runner_exit_run("${root}" "${doc}" table result output)
  runner_exit_expect_rejection("missing --raise-after before stop section" "${result}" "${output}"
    "canonical runner guide option list lacks --raise-after")

  set(stop_sentences
    "On SIGTERM or SIGINT the runner finishes the current frame, writes the battery save and exits 0, or 1 if the save could not be written."
    "A second signal at any time, including during the flush, ends the runner at once."
    "On Windows the runner handles Ctrl+C, Ctrl+Break and console close the same way."
    "A forced termination (TerminateProcess, taskkill /F, power loss) cannot be handled\; the last committed .sav survives it."
    "A launcher that sends the event to group 0 first registers its own handler that returns TRUE for CTRL_BREAK_EVENT, after attaching and before sending, or the event ends the launcher too.")
  set(stop_diagnostics
    "canonical runner guide lacks stop sentence: On SIGTERM"
    "canonical runner guide lacks stop sentence: A second signal"
    "canonical runner guide lacks stop sentence: On Windows"
    "canonical runner guide lacks stop sentence: A forced termination"
    "canonical runner guide lacks stop sentence: A launcher that sends the event")
  set(stop_number 0)
  foreach(sentence IN LISTS stop_sentences)
    list(GET stop_diagnostics ${stop_number} expected_diagnostic)
    math(EXPR stop_number "${stop_number} + 1")
    string(FIND "${canonical_text}" "${sentence}" at)
    if(at EQUAL -1)
      file(REMOVE_RECURSE "${fixture_root}")
      message(FATAL_ERROR "runner exit self-test: no stop sentence ${stop_number} to mutate")
    endif()
    string(REPLACE "${sentence}" "" mutated_stop "${canonical_text}")
    runner_exit_fixture("stop_${stop_number}" TRUE "${mutated_stop}" root doc)
    runner_exit_run("${root}" "${doc}" table result output)
    runner_exit_expect_rejection("removed stop sentence ${stop_number}" "${result}" "${output}"
      "${expected_diagnostic}")
  endforeach()

  file(REMOVE_RECURSE "${fixture_root}")
  message(STATUS "runner exit self-test: canonical guide and stale-copy controls passed")
  return()
endif()

# RUN-03: runner/exit_status.h, `nesturbator-run --help`, and the canonical runner guide retain
# the same exit table and stop contract. Inputs: -DSOURCE_DIR=<repo> -DDOC=<repo>/docs/runner.md
# -DRUNNER=<nesturbator-run> -DMODE=table|help. MODE=table compares header/help/guide rows;
# MODE=help checks the executable's help output.
foreach(var SOURCE_DIR RUNNER MODE DOC)
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "${var} is required")
  endif()
endforeach()

cmake_path(NORMAL_PATH SOURCE_DIR OUTPUT_VARIABLE source_dir)
cmake_path(NORMAL_PATH DOC OUTPUT_VARIABLE doc_path)
cmake_path(APPEND source_dir "docs/runner.md" OUTPUT_VARIABLE expected_doc)
if(NOT doc_path STREQUAL expected_doc)
  message(FATAL_ERROR "DOC must name the canonical runner guide: ${expected_doc}")
endif()
if(NOT EXISTS "${DOC}")
  message(FATAL_ERROR "canonical runner guide does not exist: ${DOC}")
endif()

function(run_help out_text)
  execute_process(COMMAND "${RUNNER}" --help
    RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)
  if(NOT rc EQUAL 0)
    message(FATAL_ERROR "--help exited ${rc}, expected 0")
  endif()
  if(NOT err STREQUAL "")
    message(FATAL_ERROR "--help wrote to standard error: ${err}")
  endif()
  string(REPLACE "\r" "" out "${out}")
  set(${out_text} "${out}" PARENT_SCOPE)
endfunction()

if(MODE STREQUAL "help")
  run_help(text)
  foreach(needle "usage: nesturbator-run" "Exit status:")
    string(FIND "${text}" "${needle}" at)
    if(at EQUAL -1)
      message(FATAL_ERROR "--help output lacks '${needle}'")
    endif()
  endforeach()
  string(FIND "${text}" "--raise-after N[:SIG[:K]]" at)
  if(at EQUAL -1)
    message(FATAL_ERROR "--help lacks --raise-after")
  endif()
  message(STATUS "--help prints the usage text and exits 0")
  return()
endif()
if(NOT MODE STREQUAL "table")
  message(FATAL_ERROR "MODE must be table or help")
endif()

# The header table and the reserved sentence.
file(READ "${SOURCE_DIR}/runner/exit_status.h" header)
string(REGEX MATCHALL "X\\(RUNNER_EXIT_[A-Z_]+, *[0-9]+, *\"[^\"]*\"\\)" rows "${header}")
set(header_pairs "")
foreach(row IN LISTS rows)
  string(REGEX REPLACE "^X\\(RUNNER_EXIT_[A-Z_]+, *([0-9]+), *\"([^\"]*)\"\\)$" "\\1:\\2" pair "${row}")
  list(APPEND header_pairs "${pair}")
endforeach()
if(NOT header MATCHES "#define RUNNER_EXIT_RESERVED_TEXT \"([^\"]*)\"")
  message(FATAL_ERROR "runner/exit_status.h lacks RUNNER_EXIT_RESERVED_TEXT")
endif()
set(reserved "${CMAKE_MATCH_1}")

# The canonical guide rows in '### Exit status' and its reserved-value sentence.
file(STRINGS "${DOC}" doc_lines)
set(doc_pairs "")
set(in_section FALSE)
set(doc_reserved FALSE)
foreach(line IN LISTS doc_lines)
  if(line STREQUAL "### Exit status")
    set(in_section TRUE)
    continue()
  endif()
  if(in_section AND line MATCHES "^#")
    break()
  endif()
  if(in_section)
    if(line MATCHES "^\\| ([0-9]+) \\| ([^|]+) \\| ")
      string(STRIP "${CMAKE_MATCH_2}" meaning)
      list(APPEND doc_pairs "${CMAKE_MATCH_1}:${meaning}")
    endif()
    string(FIND "${line}" "${reserved}" at)
    if(NOT at EQUAL -1)
      set(doc_reserved TRUE)
    endif()
  endif()
endforeach()
if(NOT doc_reserved)
  message(FATAL_ERROR "canonical runner guide Exit status lacks the reserved sentence '${reserved}'")
endif()

# The --help rows after the 'Exit status:' line, and the reserved line.
run_help(text)
string(REPLACE "\n" ";" help_lines "${text}")
set(help_pairs "")
set(after FALSE)
set(help_reserved FALSE)
foreach(line IN LISTS help_lines)
  if(line STREQUAL "Exit status:")
    set(after TRUE)
    continue()
  endif()
  if(after AND line MATCHES "^  ([0-9]+)  (.+)$")
    list(APPEND help_pairs "${CMAKE_MATCH_1}:${CMAKE_MATCH_2}")
  endif()
  if(after AND line STREQUAL "  ${reserved}")
    set(help_reserved TRUE)
  endif()
endforeach()
if(NOT help_reserved)
  message(FATAL_ERROR "--help lacks the line '  ${reserved}'")
endif()

# 3 and 77 may appear only inside the reserved sentence, never as a row.
foreach(place header doc help)
  foreach(pair IN LISTS ${place}_pairs)
    string(REGEX REPLACE ":.*$" "" number "${pair}")
    if(number EQUAL 3 OR number EQUAL 77)
      message(FATAL_ERROR "${place} has a row for ${number}, which is reserved: ${pair}")
    endif()
  endforeach()
  set(numbers "")
  foreach(pair IN LISTS ${place}_pairs)
    string(REGEX REPLACE ":.*$" "" number "${pair}")
    list(APPEND numbers "${number}")
  endforeach()
  if(NOT numbers STREQUAL "0;1;2;4;5")
    message(FATAL_ERROR "${place} rows are '${numbers}', expected 0;1;2;4;5 in ascending order")
  endif()
endforeach()

foreach(place doc help)
  if(NOT "${${place}_pairs}" STREQUAL "${header_pairs}")
    if(place STREQUAL "doc")
      message(FATAL_ERROR
        "canonical runner guide exit rows differ from the header.\n  header: ${header_pairs}\n  guide: ${doc_pairs}")
    endif()
    message(FATAL_ERROR "${place} rows differ from the header.\n  header: ${header_pairs}\n  ${place}: ${${place}_pairs}")
  endif()
endforeach()
message(STATUS "exit table matches in the header, --help and docs/runner.md: ${header_pairs}")

# Success criterion 4: the canonical guide keeps the stop contract and places --raise-after
# in the option list before the stop section.
file(READ "${DOC}" guide_text)
string(REPLACE "\r" "" guide_text "${guide_text}")
string(FIND "${guide_text}" "\n### Stopping the runner\n" stop_at)
if(stop_at EQUAL -1)
  message(FATAL_ERROR "canonical runner guide lacks the section '### Stopping the runner'")
endif()
string(FIND "${guide_text}" "\n### Exit status\n" exit_at)
if(exit_at EQUAL -1 OR NOT exit_at GREATER stop_at)
  message(FATAL_ERROR "canonical runner guide lacks '### Exit status' after the stop section")
endif()
string(SUBSTRING "${guide_text}" 0 ${stop_at} guide_before_stop)
string(FIND "${guide_before_stop}" "- `--raise-after N[:SIG[:K]]`" at)
if(at EQUAL -1)
  message(FATAL_ERROR "canonical runner guide option list lacks --raise-after before the stop section")
endif()
math(EXPR stop_length "${exit_at} - ${stop_at}")
string(SUBSTRING "${guide_text}" ${stop_at} ${stop_length} stop_section)
set(stop_sentences
  "On SIGTERM or SIGINT the runner finishes the current frame, writes the battery save and exits 0, or 1 if the save could not be written."
  "A second signal at any time, including during the flush, ends the runner at once."
  "On Windows the runner handles Ctrl+C, Ctrl+Break and console close the same way."
  "A forced termination (TerminateProcess, taskkill /F, power loss) cannot be handled\; the last committed .sav survives it."
  "A launcher that sends the event to group 0 first registers its own handler that returns TRUE for CTRL_BREAK_EVENT, after attaching and before sending, or the event ends the launcher too.")
foreach(sentence IN LISTS stop_sentences)
  string(FIND "${stop_section}" "${sentence}" at)
  if(at EQUAL -1)
    message(FATAL_ERROR "canonical runner guide lacks stop sentence: ${sentence}")
  endif()
endforeach()
message(STATUS "canonical runner guide keeps the stop contract and the --raise-after entry")
