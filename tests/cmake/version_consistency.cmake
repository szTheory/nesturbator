# version.txt and the NESTURBATOR_VERSION_* macros of the public header must
# name the same version; release-please updates both.
#
#   cmake -DSOURCE_DIR=<repository root> -P version_consistency.cmake
file(STRINGS "${SOURCE_DIR}/version.txt" text_version LIMIT_COUNT 1)
file(READ "${SOURCE_DIR}/include/nesturbator.h" header)
set(parts "")
foreach(part MAJOR MINOR PATCH)
  if(NOT header MATCHES "#define NESTURBATOR_VERSION_${part} ([0-9]+)")
    message(FATAL_ERROR "NESTURBATOR_VERSION_${part} not found in nesturbator.h")
  endif()
  list(APPEND parts "${CMAKE_MATCH_1}")
endforeach()
list(JOIN parts "." header_version)
if(NOT text_version STREQUAL header_version)
  message(FATAL_ERROR "version.txt is '${text_version}', nesturbator.h is '${header_version}'")
endif()
message(STATUS "version ${text_version}")
