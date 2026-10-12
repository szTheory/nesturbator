cmake_minimum_required(VERSION 3.25)
execute_process(
  COMMAND "${CTEST}" --test-dir "${BUILD_DIR}" --show-only=json-v1
  RESULT_VARIABLE result
  OUTPUT_VARIABLE inventory
  ERROR_VARIABLE error)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "could not inspect CTest inventory: ${error}")
endif()
foreach(required fuzz.regress fuzz.state fuzz.state.regress)
  string(REPLACE "." "\\." pattern "${required}")
  if(NOT inventory MATCHES "\"name\"[ \t]*:[ \t]*\"${pattern}\"")
    message(FATAL_ERROR "CTest does not register the required ${required} test")
  endif()
endforeach()
