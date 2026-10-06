# Checks that every `uses:` key in the GitHub workflow files names an action by
# a full commit SHA (ENGINEERING section 5, Pins). The key is matched at any
# indentation, with or without a leading "- ", so a step written as a map is
# not missed.
#
#   cmake -DROOT=<dir> [-DFILE=<name>] -P action_pins.cmake
#
# Without FILE, every *.yml and *.yaml file under ROOT is read. A value passes
# if it is a local path (./...) or owner/repo[/path]@<40 lowercase hex digits>.
# Each offender is printed as file:line.

cmake_minimum_required(VERSION 3.25)

if(DEFINED FILE)
  set(files "${ROOT}/${FILE}")
else()
  file(GLOB_RECURSE files "${ROOT}/*.yml" "${ROOT}/*.yaml")
endif()
if(NOT files)
  message(STATUS "no workflow files under ${ROOT}")
  return()
endif()

set(bad 0)
set(count 0)
foreach(f IN LISTS files)
  if(NOT EXISTS "${f}")
    message(FATAL_ERROR "${f} does not exist")
  endif()
  # One list element per line, empty lines kept so line numbers hold. Brackets
  # and semicolons would upset list splitting; no uses: value contains them.
  file(READ "${f}" text)
  string(REGEX REPLACE "[][;]" "_" text "${text}")
  string(REPLACE "\n" ";" lines "${text}")
  set(n 0)
  foreach(line IN LISTS lines)
    math(EXPR n "${n} + 1")
    if(NOT line MATCHES "^[ \t]*(- )?[ \t]*uses:[ \t]*(.*)$")
      continue()
    endif()
    set(value "${CMAKE_MATCH_2}")
    # Drop a trailing comment and quotes.
    string(REGEX REPLACE "[ \t]+#.*$" "" value "${value}")
    string(STRIP "${value}" value)
    string(REGEX REPLACE "^[\"'](.*)[\"']$" "\\1" value "${value}")
    math(EXPR count "${count} + 1")
    if(value MATCHES "^\\./")
      continue()
    endif()
    if(value MATCHES "^[A-Za-z0-9_.-]+/[A-Za-z0-9_./-]+@[0-9a-f]+$")
      string(REGEX REPLACE "^.*@" "" sha "${value}")
      string(LENGTH "${sha}" len)
      if(len EQUAL 40)
        continue()
      endif()
    endif()
    message("${f}:${n}: action not pinned to a full commit SHA")
    set(bad 1)
  endforeach()
endforeach()

if(bad)
  message(FATAL_ERROR "unpinned actions found")
endif()
message(STATUS "${count} uses: keys checked")
