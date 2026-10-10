cmake_minimum_required(VERSION 3.25)

# TUNE-05 / D-05: no registered test may be able to report itself skipped or
# be disabled. The inventory is CTest's own machine-readable listing. There is
# no allowlist: a test that cannot run somewhere is not registered there.

# Sets out_error to the first problem found in the inventory, or to the empty
# string when no test can skip.
function(check_inventory json out_error)
  set(${out_error} "" PARENT_SCOPE)
  string(JSON count ERROR_VARIABLE err LENGTH "${json}" tests)
  if(err)
    set(${out_error} "invalid ctest inventory: ${err}" PARENT_SCOPE)
    return()
  endif()
  if(count EQUAL 0)
    set(${out_error} "ctest inventory is empty" PARENT_SCOPE)
    return()
  endif()
  math(EXPR last_test "${count} - 1")
  foreach(test_index RANGE 0 ${last_test})
    string(JSON test_name GET "${json}" tests ${test_index} name)
    string(JSON property_count LENGTH "${json}" tests ${test_index} properties)
    if(property_count GREATER 0)
      math(EXPR last_property "${property_count} - 1")
      foreach(property_index RANGE 0 ${last_property})
        string(JSON property GET "${json}" tests ${test_index}
          properties ${property_index} name)
        if(property STREQUAL "SKIP_RETURN_CODE"
            OR property STREQUAL "SKIP_REGULAR_EXPRESSION"
            OR property STREQUAL "DISABLED")
          set(${out_error}
            "test can skip or is disabled: ${test_name} (${property})"
            PARENT_SCOPE)
          return()
        endif()
      endforeach()
    endif()
  endforeach()
endfunction()

if(NOT SELFTEST)
  foreach(required CTEST BUILD_DIR)
    if(NOT DEFINED ${required})
      message(FATAL_ERROR "skip_policy: -D${required} is required")
    endif()
  endforeach()
  execute_process(COMMAND "${CTEST}" --test-dir "${BUILD_DIR}" --show-only=json-v1
    RESULT_VARIABLE rc OUTPUT_VARIABLE json ERROR_VARIABLE err)
  if(NOT rc EQUAL 0)
    message(FATAL_ERROR "skip_policy: ctest exited ${rc}: ${err}")
  endif()
  check_inventory("${json}" error)
  if(error)
    message(FATAL_ERROR "skip_policy: ${error}")
  endif()
  string(JSON count LENGTH "${json}" tests)
  message(STATUS "skip_policy: ${count} tests, none can skip")
  return()
endif()

# D-06: mutation self-test. Each case below must be rejected or accepted.
function(expect_rejected label json wanted)
  check_inventory("${json}" error)
  if(NOT error)
    message(FATAL_ERROR "skip_policy self-test: accepted ${label}")
  endif()
  if(NOT "${wanted}" STREQUAL "" AND NOT error MATCHES "${wanted}")
    message(FATAL_ERROR
      "skip_policy self-test: ${label} rejected without naming ${wanted}: ${error}")
  endif()
endfunction()

function(expect_accepted label json)
  check_inventory("${json}" error)
  if(error)
    message(FATAL_ERROR "skip_policy self-test: rejected ${label}: ${error}")
  endif()
endfunction()

foreach(required CTEST WORK_DIR GENERATOR MAKE_PROGRAM)
  if(NOT DEFINED ${required})
    message(FATAL_ERROR "skip_policy self-test: -D${required} is required")
  endif()
endforeach()

set(clean [[{"tests":[
  {"name":"a.one","properties":[{"name":"TIMEOUT","value":60}]},
  {"name":"b.two","properties":[{"name":"LABELS","value":["x"]}]}]}]])
expect_accepted("clean inventory" "${clean}")
expect_rejected("SKIP_RETURN_CODE"
  [[{"tests":[{"name":"a.one","properties":[{"name":"SKIP_RETURN_CODE","value":77}]}]}]]
  "a\\.one")
expect_rejected("SKIP_REGULAR_EXPRESSION"
  [[{"tests":[{"name":"a.one","properties":[{"name":"SKIP_REGULAR_EXPRESSION","value":["skip:"]}]}]}]]
  "a\\.one")
expect_rejected("DISABLED"
  [[{"tests":[{"name":"a.one","properties":[{"name":"DISABLED","value":true}]}]}]]
  "a\\.one")
expect_rejected("invalid JSON" "{not json" "invalid ctest inventory")
expect_rejected("empty list" [[{"tests":[]}]] "empty")

# Real ctest output: CMake configures a throwaway one-test project, so CMake
# itself writes the test file ctest reads. LANGUAGES NONE needs no compiler and
# configure alone is enough.
file(REMOVE_RECURSE "${WORK_DIR}")
function(configure_fixture name with_skip)
  set(src "${WORK_DIR}/${name}-src")
  file(MAKE_DIRECTORY "${src}")
  set(body "cmake_minimum_required(VERSION 3.25)
project(no_skip_fixture LANGUAGES NONE)
enable_testing()
add_test(NAME fixture.skip COMMAND \"\${CMAKE_COMMAND}\" -E true)
")
  if(with_skip)
    string(APPEND body
      "set_tests_properties(fixture.skip PROPERTIES SKIP_RETURN_CODE 77)\n")
  endif()
  file(WRITE "${src}/CMakeLists.txt" "${body}")
  execute_process(COMMAND "${CMAKE_COMMAND}" -S "${src}" -B "${WORK_DIR}/${name}"
    -G "${GENERATOR}" "-DCMAKE_MAKE_PROGRAM=${MAKE_PROGRAM}"
    RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)
  if(NOT rc EQUAL 0)
    message(FATAL_ERROR
      "skip_policy self-test: fixture ${name} configure failed (${rc}):\n${out}\n${err}")
  endif()
  execute_process(COMMAND "${CTEST}" --test-dir "${WORK_DIR}/${name}" --show-only=json-v1
    RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)
  if(NOT rc EQUAL 0)
    message(FATAL_ERROR "skip_policy self-test: ctest on ${name} exited ${rc}: ${err}")
  endif()
  set(fixture_json "${out}" PARENT_SCOPE)
endfunction()

configure_fixture(skip TRUE)
expect_rejected("real ctest output with SKIP_RETURN_CODE" "${fixture_json}"
  "fixture\\.skip")
configure_fixture(clean FALSE)
expect_accepted("real ctest output without skip" "${fixture_json}")

execute_process(COMMAND "${CTEST}" --test-dir "${WORK_DIR}/missing" --show-only=json-v1
  RESULT_VARIABLE rc OUTPUT_QUIET ERROR_QUIET)
if(rc EQUAL 0)
  message(FATAL_ERROR "skip_policy self-test: ctest on a missing directory succeeded")
endif()
message(STATUS "skip_policy self-test: ok")
