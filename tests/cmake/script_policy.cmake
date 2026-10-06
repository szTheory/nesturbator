# Every script in tests/cmake sets cmake_minimum_required. A -P script starts
# with no policies set, so without it CMake 3.x reads IN_LIST and other
# constructs with their old meanings, which differ between CMake versions
# (measured on the CI runners' CMake 3.31, commit 1d9c997).
#
#   cmake -DDIR=<tests/cmake> -P script_policy.cmake

cmake_minimum_required(VERSION 3.25)

file(GLOB scripts "${DIR}/*.cmake")
if(NOT scripts)
  message(FATAL_ERROR "script_policy: no scripts found in ${DIR}")
endif()
set(missing "")
foreach(script IN LISTS scripts)
  file(STRINGS "${script}" line REGEX "^cmake_minimum_required\\(VERSION 3\\.25\\)$" LIMIT_COUNT 1)
  if(NOT line)
    cmake_path(GET script FILENAME name)
    list(APPEND missing "${name}")
  endif()
endforeach()
if(missing)
  list(JOIN missing ", " missing)
  message(FATAL_ERROR "script_policy: no cmake_minimum_required(VERSION 3.25) in ${missing}")
endif()
list(LENGTH scripts count)
message(STATUS "${count} scripts set the policy version")
