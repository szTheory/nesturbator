# Scoped public report and command contract for docs/reproduction.md.
#
#   MODE=check validates the report/cold sections and generated build metadata.
#   MODE=self_test requires independent broken copies to fail for named reasons.

cmake_minimum_required(VERSION 3.25)

function(reproduction_require_contains content needle diagnostic)
  string(FIND "${content}" "${needle}" found)
  if(found EQUAL -1)
    message(FATAL_ERROR "docs.reproduction: ${diagnostic}")
  endif()
endfunction()

function(reproduction_check doc_path)
  foreach(var SOURCE_DIR BUILD_DIR CTEST)
    if(NOT DEFINED ${var} OR "${${var}}" STREQUAL "")
      message(FATAL_ERROR "docs.reproduction: -D${var}=... is required")
    endif()
  endforeach()
  if(NOT EXISTS "${doc_path}")
    message(FATAL_ERROR "docs.reproduction: report document is missing")
  endif()

  file(READ "${doc_path}" document)
  string(FIND "${document}" "## Report a scene" report_start)
  string(FIND "${document}" "## Permitted fixture example" example_start)
  string(FIND "${document}" "## Cold movie replay" cold_start)
  if(report_start EQUAL -1 OR example_start LESS report_start OR cold_start LESS example_start)
    message(FATAL_ERROR "docs.reproduction: report, fixture example and cold replay sections are out of order")
  endif()
  math(EXPR report_length "${example_start} - ${report_start}")
  string(SUBSTRING "${document}" ${report_start} ${report_length} report_section)
  string(SUBSTRING "${document}" ${cold_start} -1 cold_section)

  set(report_fields
    "Report revision/date:"
    "Title and region:"
    "Content revision or digest:"
    "Loaded core and exact build identity:"
    "Host and host version:"
    "Launch route and relevant settings:"
    "Exact scene or point reached:"
    "Access method and whether it was verified:"
    "Start conditions (cold boot, checkpoint, save, or other):"
    "Inputs in order, including port for each input:"
    "Expected behavior and its source/confidence:"
    "Observed behavior:"
    "Repeatability (attempts and outcomes):"
    "Conditions not tried:"
    "Evidence limits or unknowns:"
    "Attachments withheld from public report:")
  foreach(field IN LISTS report_fields)
    reproduction_require_contains("${report_section}" "${field}" "report field missing: ${field}")
  endforeach()

  foreach(value unknown withheld "not tried" unavailable)
    reproduction_require_contains("${report_section}" "${value}" "uncertainty value missing: ${value}")
  endforeach()
  reproduction_require_contains("${report_section}" "title-level observation" "title-level and exact-scene evidence are not distinguished")
  reproduction_require_contains("${report_section}" "expected behavior comes" "expected-behavior provenance is missing")
  reproduction_require_contains("${report_section}" "how confident that source is" "expected-behavior confidence is missing")
  reproduction_require_contains("${report_section}" "Do not publish commercial game media" "privacy claim missing: commercial game media")
  reproduction_require_contains("${report_section}" "saves" "privacy claim missing: saves")
  reproduction_require_contains("${report_section}" "native checkpoints" "privacy claim missing: native checkpoints")
  reproduction_require_contains("${report_section}" "raw private" "privacy claim missing: raw private logs or captures")
  reproduction_require_contains("${report_section}" "personal paths" "privacy claim missing: personal paths")
  reproduction_require_contains("${report_section}" "personal identifiers" "privacy claim missing: personal identifiers")

  foreach(required_heading "## Permitted fixture example" "## Cold movie replay")
    reproduction_require_contains("${document}" "${required_heading}" "required heading missing: ${required_heading}")
  endforeach()
  reproduction_require_contains("${cold_section}" "two-port NMOVIE1" "two-port NMOVIE1 validation is not documented")
  reproduction_require_contains("${cold_section}" "--frames" "unsupported movie/frame combination is not documented")
  reproduction_require_contains("${cold_section}" "--load-state" "unsupported movie/state combination is not documented")
  reproduction_require_contains("${cold_section}" "Host recording" "host recording limitation is not documented")
  reproduction_require_contains("${cold_section}" "foreign movie formats" "foreign movie conversion limitation is not documented")
  reproduction_require_contains("${cold_section}" "hardware-correctness evidence" "repeatability is overstated as correctness")

  file(READ "${SOURCE_DIR}/README.md" readme)
  reproduction_require_contains("${readme}" "## Troubleshooting" "README troubleshooting heading missing")
  reproduction_require_contains("${readme}" "[Report a scene or reproduce a permitted fixture](docs/reproduction.md)" "README does not link directly to the report")
  foreach(path IN ITEMS
      "docs/runner.md"
      "docs/testing.md"
      "docs/progress.md"
      "tests/runner/game_movie.c")
    if(NOT EXISTS "${SOURCE_DIR}/${path}")
      message(FATAL_ERROR "docs.reproduction: linked file missing: ${path}")
    endif()
  endforeach()
  file(READ "${SOURCE_DIR}/docs/runner.md" runner_guide)
  file(READ "${SOURCE_DIR}/docs/testing.md" testing_guide)
  file(READ "${SOURCE_DIR}/docs/progress.md" progress_guide)
  file(READ "${SOURCE_DIR}/tests/runner/game_movie.c" movie_writer)
  reproduction_require_contains("${runner_guide}" "## Input movies" "runner guide section link is stale")
  reproduction_require_contains("${testing_guide}" "# Testing and engineering" "testing guide heading is stale")
  reproduction_require_contains("${progress_guide}" "# Protecting emulator progress" "progress guide heading is stale")
  reproduction_require_contains("${movie_writer}" "nesrunner-pos" "owned movie scenario is missing")
  reproduction_require_contains("${cold_section}" "[runner guide](runner.md)" "runner guide link is stale")
  reproduction_require_contains("${cold_section}" "[testing guide](testing.md)" "testing guide link is stale")
  reproduction_require_contains("${cold_section}" "[progress guide](progress.md)" "progress guide link is stale")
  reproduction_require_contains("${document}" "../tests/runner/game_movie.c" "movie generator source link is stale")

  # Both shell examples must name the same source-build targets, and CTest's
  # generated command metadata must contain their corresponding executables.
  string(REGEX MATCHALL "cmake --build [^\n\r]*--target [^\n\r]*" build_commands "${cold_section}")
  list(LENGTH build_commands build_command_count)
  if(build_command_count LESS 2)
    message(FATAL_ERROR "docs.reproduction: POSIX and PowerShell build commands are required")
  endif()
  set(documented_targets)
  foreach(command IN LISTS build_commands)
    string(REGEX REPLACE "^.*--target[ \t]+" "" targets_text "${command}")
    string(REPLACE "\"" "" targets_text "${targets_text}")
    string(STRIP "${targets_text}" targets_text)
    string(REPLACE " " ";" targets "${targets_text}")
    list(APPEND documented_targets ${targets})
  endforeach()
  list(REMOVE_DUPLICATES documented_targets)

  string(REGEX MATCH "-R[ \t]+'[^']+'" selector_arg "${cold_section}")
  if(selector_arg STREQUAL "")
    message(FATAL_ERROR "docs.reproduction: bounded CTest selection is missing")
  endif()
  string(REGEX REPLACE "^-R[ \t]+'" "" test_regex "${selector_arg}")
  string(REGEX REPLACE "'$" "" test_regex "${test_regex}")
  string(LENGTH "${test_regex}" regex_length)
  if(regex_length LESS 5)
    message(FATAL_ERROR "docs.reproduction: bounded CTest selection is malformed")
  endif()
  string(SUBSTRING "${test_regex}" 0 2 regex_prefix)
  math(EXPR test_names_length "${regex_length} - 4")
  string(SUBSTRING "${test_regex}" 2 ${test_names_length} test_names_text)
  math(EXPR regex_suffix_start "${regex_length} - 2")
  string(SUBSTRING "${test_regex}" ${regex_suffix_start} 2 regex_suffix)
  if(NOT regex_prefix STREQUAL "^(" OR NOT regex_suffix STREQUAL ")$")
    message(FATAL_ERROR "docs.reproduction: bounded CTest selection must be an anchored name list")
  endif()
  string(REPLACE "|" ";" documented_tests "${test_names_text}")

  execute_process(COMMAND "${CTEST}" --test-dir "${BUILD_DIR}" --show-only=json-v1
    OUTPUT_VARIABLE all_tests_json ERROR_VARIABLE ctest_err RESULT_VARIABLE ctest_rc TIMEOUT 20)
  if(NOT "${ctest_rc}" STREQUAL "0")
    message(FATAL_ERROR "docs.reproduction: generated CTest metadata could not be read")
  endif()
  string(JSON all_test_count LENGTH "${all_tests_json}" tests)
  set(generated_tests)
  foreach(index RANGE 0 ${all_test_count})
    if(index GREATER_EQUAL all_test_count)
      break()
    endif()
    string(JSON test_name GET "${all_tests_json}" tests ${index} name)
    list(APPEND generated_tests "${test_name}")
  endforeach()
  foreach(test_name IN LISTS documented_tests)
    if(NOT test_name IN_LIST generated_tests)
      message(FATAL_ERROR "docs.reproduction: documented CTest test is not generated: ${test_name}")
    endif()
  endforeach()
  foreach(required_test IN ITEMS
      runner.reproduction.cold
      runner.reproduction.compare_self_test
      runner.boot_to_play
      docs.links
      docs.links.self_test
      docs.reproduction
      docs.reproduction.self_test
      cmake.script_policy)
    if(NOT required_test IN_LIST documented_tests)
      message(FATAL_ERROR "docs.reproduction: documented CTest selection omits ${required_test}")
    endif()
  endforeach()

  execute_process(COMMAND "${CTEST}" --test-dir "${BUILD_DIR}" --show-only=json-v1 -R "${test_regex}"
    OUTPUT_VARIABLE selected_tests_json ERROR_VARIABLE selected_err RESULT_VARIABLE selected_rc TIMEOUT 20)
  if(NOT "${selected_rc}" STREQUAL "0")
    message(FATAL_ERROR "docs.reproduction: documented CTest selection could not be evaluated")
  endif()
  string(JSON selected_count LENGTH "${selected_tests_json}" tests)
  if(selected_count LESS 1)
    message(FATAL_ERROR "docs.reproduction: documented CTest selection matches no tests")
  endif()

  string(JSON selected_length LENGTH "${selected_tests_json}" tests)
  set(cold_timeout "")
  foreach(index RANGE 0 ${selected_length})
    if(index GREATER_EQUAL selected_length)
      break()
    endif()
    string(JSON test_name GET "${selected_tests_json}" tests ${index} name)
    if(test_name STREQUAL "runner.reproduction.cold")
      string(JSON property_count LENGTH "${selected_tests_json}" tests ${index} properties)
      foreach(property_index RANGE 0 ${property_count})
        if(property_index GREATER_EQUAL property_count)
          break()
        endif()
        string(JSON property_name GET "${selected_tests_json}" tests ${index} properties ${property_index} name)
        if(property_name STREQUAL "TIMEOUT")
          string(JSON cold_timeout GET "${selected_tests_json}" tests ${index} properties ${property_index} value)
        endif()
      endforeach()
    endif()
  endforeach()
  if(NOT cold_timeout STREQUAL "180.0" AND NOT cold_timeout STREQUAL "180")
    message(FATAL_ERROR "docs.reproduction: generated cold replay timeout is not 180 seconds")
  endif()

  foreach(target IN LISTS documented_targets)
    string(FIND "${all_tests_json}" "/${target}" target_position)
    if(target_position EQUAL -1)
      message(FATAL_ERROR "docs.reproduction: documented build target is not generated: ${target}")
    endif()
  endforeach()
  foreach(required_target IN ITEMS nesturbator-run runner.game_movie runner.boot_to_play)
    if(NOT required_target IN_LIST documented_targets)
      message(FATAL_ERROR "docs.reproduction: documented build command omits ${required_target}")
    endif()
  endforeach()

  message(STATUS "docs.reproduction: report fields, source links, build targets and bounded CTest selection are current")
endfunction()

function(reproduction_expect_mutation root name mutated expected)
  set(mutated_doc "${root}/${name}.md")
  file(WRITE "${mutated_doc}" "${mutated}")
  execute_process(COMMAND "${CMAKE_COMMAND}"
      "-DMODE=check"
      "-DSOURCE_DIR=${SOURCE_DIR}"
      "-DBUILD_DIR=${BUILD_DIR}"
      "-DCTEST=${CTEST}"
      "-DDOC=${mutated_doc}"
      -P "${CMAKE_CURRENT_LIST_FILE}"
    OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE rc TIMEOUT 20)
  if("${rc}" STREQUAL "0")
    message(FATAL_ERROR "docs.reproduction self-test: ${name} mutation was accepted")
  endif()
  string(CONCAT combined "${out}" "${err}")
  string(FIND "${combined}" "${expected}" diagnostic_pos)
  if(diagnostic_pos EQUAL -1)
    string(REGEX MATCH "docs\.reproduction[^\n]*" observed "${combined}")
    message(FATAL_ERROR "docs.reproduction self-test: ${name} expected '${expected}', observed '${observed}'")
  endif()
  message(STATUS "docs.reproduction.self_test: ${name} rejected for the expected reason")
endfunction()

if(NOT DEFINED MODE OR MODE STREQUAL "")
  message(FATAL_ERROR "docs.reproduction: -DMODE=check|self_test is required")
elseif(MODE STREQUAL "check")
  if(NOT DEFINED DOC OR "${DOC}" STREQUAL "")
    message(FATAL_ERROR "docs.reproduction: -DDOC=... is required")
  endif()
  reproduction_check("${DOC}")
elseif(MODE STREQUAL "self_test")
  foreach(var DOC WORK)
    if(NOT DEFINED ${var} OR "${${var}}" STREQUAL "")
      message(FATAL_ERROR "docs.reproduction self-test: -D${var}=... is required")
    endif()
  endforeach()
  reproduction_check("${DOC}")
  file(READ "${DOC}" original)
  string(RANDOM LENGTH 20 ALPHABET 0123456789abcdef suffix)
  set(root "${WORK}/contract-${suffix}")
  file(MAKE_DIRECTORY "${root}")

  set(field_mutation "${original}")
  string(REPLACE "Title and region:\n" "" field_mutation "${field_mutation}")
  reproduction_expect_mutation("${root}" "missing-report-field" "${field_mutation}" "report field missing: Title and region")

  set(test_mutation "${original}")
  string(REPLACE "runner.reproduction.cold" "runner.reproduction.not_cold" test_mutation "${test_mutation}")
  reproduction_expect_mutation("${root}" "stale-test-name" "${test_mutation}" "not generated")

  set(target_mutation "${original}")
  string(REPLACE "runner.game_movie" "runner.game_movie_missing" target_mutation "${target_mutation}")
  reproduction_expect_mutation("${root}" "stale-target-path" "${target_mutation}" "build target is not generated")

  set(safety_mutation "${original}")
  string(REPLACE "Do not publish commercial game media" "Publish commercial game media" safety_mutation "${safety_mutation}")
  reproduction_expect_mutation("${root}" "missing-privacy-claim" "${safety_mutation}" "privacy claim missing")
  message(STATUS "docs.reproduction.self_test: four independent mutations rejected")
else()
  message(FATAL_ERROR "docs.reproduction: unknown mode '${MODE}'")
endif()
