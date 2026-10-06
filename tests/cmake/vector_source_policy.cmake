cmake_minimum_required(VERSION 3.25)

# D-01 and D-02: keep the vector conversion/fetch boundary owned, dependency
# free, and independent of CMake's JSON parser. This deliberately does not scan
# release configuration scripts, which may use string(JSON).
function(vector_source_policy root out_var)
  set(files
    tools/vecconv/CMakeLists.txt
    tools/vecconv/vecconv.c
    tests/cmake/fetch_vectors.cmake
    tests/cmake/vectors_fixture.cmake
    tests/cmake/vectors_full_run.cmake
    tests/cmake/vectors_regen.cmake
    tests/cmake/vectors_sample_match.cmake
    tests/cmake/vecconv_negative.cmake)
  set(problems)
  foreach(relative IN LISTS files)
    set(path "${root}/${relative}")
    if(NOT EXISTS "${path}")
      list(APPEND problems "missing scoped source ${relative}")
      continue()
    endif()
    file(READ "${path}" contents)
    if(contents MATCHES "(^|[ \t;])python([0-9.]*)?([ \t]|$)|Python(3)?_EXECUTABLE|find_package[ \t]*\\([ \t]*(nlohmann_json|json-c|Jansson)|nlohmann/json|jansson/jansson")
      list(APPEND problems "external JSON/Python parser in ${relative}")
    endif()
    if(contents MATCHES "string[ \t]*\\([ \t]*JSON")
      list(APPEND problems "CMake string(JSON) in ${relative}")
    endif()
  endforeach()
  set(${out_var} "${problems}" PARENT_SCOPE)
endfunction()

if(SELFTEST)
  if(NOT DEFINED WORK)
    message(FATAL_ERROR "SELFTEST requires WORK")
  endif()
  file(MAKE_DIRECTORY "${WORK}")
  set(fixture "${WORK}/vector-source-policy-fixture.cmake")
  file(WRITE "${fixture}" "string(JSON value GET input 0)\n")
  file(READ "${fixture}" contents)
  if(NOT contents MATCHES "string[ \t]*\\([ \t]*JSON")
    message(FATAL_ERROR "self-test fixture was not recognized as a parser violation")
  endif()
  file(WRITE "${fixture}" "execute_process(COMMAND python3 parse.py)\n")
  file(READ "${fixture}" contents)
  if(NOT contents MATCHES "(^|[ \t;])python([0-9.]*)?([ \t]|$)")
    message(FATAL_ERROR "self-test fixture was not recognized as a Python violation")
  endif()
  message(STATUS "vector source policy self-test passed")
elseif(DEFINED SOURCE_DIR)
  vector_source_policy("${SOURCE_DIR}" violations)
  if(violations)
    list(JOIN violations "; " report)
    message(FATAL_ERROR "vector source policy: ${report}")
  endif()
  message(STATUS "vector source policy passed")
else()
  message(FATAL_ERROR "SOURCE_DIR is required")
endif()
