# Checks that every `timeout-minutes:` line in the GitHub workflow files carries
# the ID of the run it was measured from (ENGINEERING section 5: a timeout is
# twice a measured cold run).
#
#   cmake -DROOT=<dir> [-DFILE=<name>] -P timeout_comments.cmake
#
# Without FILE, every *.yml and *.yaml file under ROOT is read. A line passes if
# its trailing "#" comment, or the YAML comment lines directly above it, match
# "run <9 or more digits>" or "runs <9 or more digits>". Each offender is
# printed as file:line.

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

set(id_re "runs? [0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9]")
set(bad 0)
set(count 0)
foreach(f IN LISTS files)
  if(NOT EXISTS "${f}")
    message(FATAL_ERROR "${f} does not exist")
  endif()
  file(READ "${f}" text)
  string(REGEX REPLACE "[][;]" "_" text "${text}")
  string(REPLACE "\n" ";" lines "${text}")
  set(n 0)
  # The comment lines directly above the current line, joined; emptied by any
  # line that is not a comment.
  set(above "")
  foreach(line IN LISTS lines)
    math(EXPR n "${n} + 1")
    if(line MATCHES "^[ \t]*#")
      string(APPEND above " ${line}")
      continue()
    endif()
    if(line MATCHES "timeout-minutes:")
      math(EXPR count "${count} + 1")
      set(own "")
      if(line MATCHES "#(.*)$")
        set(own "${CMAKE_MATCH_1}")
      endif()
      if(NOT "${own}" MATCHES "${id_re}" AND NOT "${above}" MATCHES "${id_re}")
        message(STATUS "${f}:${n}: timeout-minutes without a measured run ID (twice a measured cold run, ENGINEERING section 5)")
        set(bad 1)
      endif()
    endif()
    set(above "")
  endforeach()
endforeach()

if(bad)
  message(FATAL_ERROR "timeout-minutes lines without a run ID")
endif()
message(STATUS "${count} timeout-minutes lines carry a run ID")
