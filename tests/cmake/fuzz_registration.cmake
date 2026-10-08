cmake_minimum_required(VERSION 3.25)
execute_process(
  COMMAND "${CTEST}" --test-dir "${BUILD_DIR}" --show-only=json-v1
  RESULT_VARIABLE result
  OUTPUT_VARIABLE inventory
  ERROR_VARIABLE error)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "could not inspect CTest inventory: ${error}")
endif()
if(NOT inventory MATCHES "\\\"name\\\"[ \t]*:[ \t]*\\\"fuzz\\.regress\\\"")
  message(FATAL_ERROR "CTest does not register the required fuzz.regress test")
endif()
