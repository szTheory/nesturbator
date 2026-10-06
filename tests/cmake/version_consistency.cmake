# version.txt and the NESTURBATOR_VERSION_* macros of the public header must
# name the same version, and so must display_version in the libretro .info
# file; release-please updates all three.
#
#   cmake -DSOURCE_DIR=<repository root> -P version_consistency.cmake

cmake_minimum_required(VERSION 3.25)

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
# The libretro core's .info file shows the same version in RetroArch.
file(READ "${SOURCE_DIR}/libretro/nesturbator_libretro.info" info)
if(NOT info MATCHES "display_version = \"([^\"]*)\"")
  message(FATAL_ERROR "display_version not found in nesturbator_libretro.info")
endif()
if(NOT text_version STREQUAL CMAKE_MATCH_1)
  message(FATAL_ERROR "version.txt is '${text_version}', nesturbator_libretro.info is '${CMAKE_MATCH_1}'")
endif()
message(STATUS "version ${text_version}")
