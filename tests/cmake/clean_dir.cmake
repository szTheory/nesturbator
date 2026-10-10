# Empties a directory: removes it and creates it again.
#
#   cmake -DDIR=<directory> -P clean_dir.cmake

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED DIR OR DIR STREQUAL "")
  message(FATAL_ERROR "clean_dir: -DDIR is required")
endif()
file(REMOVE_RECURSE "${DIR}")
file(MAKE_DIRECTORY "${DIR}")
