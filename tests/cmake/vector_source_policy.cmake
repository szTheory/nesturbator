cmake_minimum_required(VERSION 3.25)

# D-01 and D-02: keep the vector conversion/fetch boundary owned, dependency
# free, and independent of CMake's JSON parser. This deliberately does not scan
# release configuration scripts, which may use string(JSON).
function(classify_vector_source contents relative out_var)
  set(problems)
  if(contents MATCHES "(^|[ \t;])python([0-9.]*)?([ \t]|$)|Python(3)?_EXECUTABLE|find_package[ \t]*\\([ \t]*[Pp]ython|find_package[ \t]*\\([^)]*[Jj][Ss][Oo][Nn]|FetchContent_Declare[ \t]*\\([^)]*(json|rapidjson|simdjson|cjson)|nlohmann/json|jansson|json-c|rapidjson|simdjson|cJSON")
    list(APPEND problems "external JSON/Python parser in ${relative}")
  endif()
  if(contents MATCHES "string[ \t]*\\([ \t]*JSON")
    list(APPEND problems "CMake string(JSON) in ${relative}")
  endif()
  set(${out_var} "${problems}" PARENT_SCOPE)
endfunction()

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
    classify_vector_source("${contents}" "${relative}" source_problems)
    list(APPEND problems ${source_problems})
  endforeach()
  set(${out_var} "${problems}" PARENT_SCOPE)
endfunction()

if(SELFTEST)
  if(NOT DEFINED WORK)
    message(FATAL_ERROR "SELFTEST requires WORK")
  endif()
  file(MAKE_DIRECTORY "${WORK}")
  set(fixture "${WORK}/vector-source-policy-fixture.cmake")
  file(WRITE "${fixture}" "set(value 1)\n")
  file(READ "${fixture}" contents)
  classify_vector_source("${contents}" fixture clean_problems)
  if(clean_problems)
    message(FATAL_ERROR "clean source fixture rejected: ${clean_problems}")
  endif()
  foreach(source "string(JSON value GET input 0)" "execute_process(COMMAND python3 parse.py)" "find_package(RapidJSON CONFIG REQUIRED)")
    file(WRITE "${fixture}" "${source}\n")
    file(READ "${fixture}" contents)
    classify_vector_source("${contents}" fixture violations)
    if(NOT violations)
      message(FATAL_ERROR "self-test checker did not detect: ${source}")
    endif()
  endforeach()
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
