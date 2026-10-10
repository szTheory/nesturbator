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

# SELFTEST_PLACEHOLDER
