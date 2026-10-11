# Checks the guards in fetch_vectors.cmake that keep it from deleting a
# directory it did not create. Runs offline: GIT names a program that does not
# exist, so a guard that failed to stop the script would fail it at its first
# git call instead of fetching.
#
#   cmake -DSCRIPT=<fetch_vectors.cmake> -DSOURCE_DIR=<repo> -DWORK=<dir>
#         [-DSUBDIR=<65x02-src>] [-DMARKER=<.nesturbator-vectors>]
#         [-DPINS=<tests/vectors/pins.txt>] -P fetch_guard.cmake
#
# SUBDIR, MARKER and PINS name the script's checkout directory, marker file and
# pins file, so one guard serves fetch_vectors.cmake and fetch_blargg_mmc3.cmake.
#
# 1. DIR/SUBDIR exists without the marker file: the script refuses and the
#    directory's file survives.
# 2. DIR is a symbolic link to the source tree (not on Windows, where making
#    one needs a privilege): the script refuses it as inside the source tree.
#    So it does a not-yet-existing DIR below that link, and creates none of
#    it in the source tree.
# 3. DIR is the source tree spelled in upper case, on a host whose file system
#    ignores case (the spelling exists): the script refuses it too. REAL_PATH
#    returning the on-disk case is what refuses it; the script's lower-casing
#    is only a fallback, and this case passes without it.

cmake_minimum_required(VERSION 3.25)

foreach(var SCRIPT SOURCE_DIR WORK)
  if(NOT DEFINED ${var} OR "${${var}}" STREQUAL "")
    message(FATAL_ERROR "fetch_guard: ${var} is not set")
  endif()
endforeach()
# The script's checkout directory and marker; the defaults are those of
# fetch_vectors.cmake. PINS names the pins file the script is given.
if(NOT DEFINED SUBDIR)
  set(SUBDIR "65x02-src")
endif()
if(NOT DEFINED MARKER)
  set(MARKER ".nesturbator-vectors")
endif()
if(NOT DEFINED PINS)
  set(PINS "${SOURCE_DIR}/tests/vectors/pins.txt")
endif()

# file(REMOVE) deletes the link itself, never the tree it points to.
file(REMOVE "${WORK}/link")
file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")

# Runs the script with DIR and requires it to fail with WANT in its output.
function(expect_refusal dir want)
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -DDIR=${dir}
      -DPINS=${PINS}
      -DSOURCE_DIR=${SOURCE_DIR} -DGIT=${WORK}/no-such-git
      -P "${SCRIPT}"
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err
    RESULT_VARIABLE rc)
  if(rc EQUAL 0)
    message(FATAL_ERROR "fetch_guard: DIR ${dir} was accepted\n${out}${err}")
  endif()
  # CMake wraps long messages, so compare with the whitespace collapsed.
  string(REGEX REPLACE "[ \t\r\n]+" " " text "${out}${err}")
  string(FIND "${text}" "${want}" at)
  if(at EQUAL -1)
    message(FATAL_ERROR "fetch_guard: DIR ${dir} failed without '${want}'\n${out}${err}")
  endif()
endfunction()

set(foreign "${WORK}/foreign")
file(WRITE "${foreign}/${SUBDIR}/keep.txt" "not the script's\n")
expect_refusal("${foreign}" "was not created by this script; refusing to delete it")
if(NOT EXISTS "${foreign}/${SUBDIR}/keep.txt")
  message(FATAL_ERROR "fetch_guard: ${foreign}/${SUBDIR}/keep.txt was deleted")
endif()

if(NOT CMAKE_HOST_WIN32)
  file(CREATE_LINK "${SOURCE_DIR}" "${WORK}/link" SYMBOLIC)
  expect_refusal("${WORK}/link" "is inside the source tree but not under build/")
  # A DIR that does not exist yet below the link is refused before any of it
  # is created, so the source tree gains no directory.
  set(probe "${SOURCE_DIR}/fetch-guard-probe")
  expect_refusal("${WORK}/link/fetch-guard-probe/deeper" "is inside the source tree but not under build/")
  file(REMOVE "${WORK}/link")
  if(EXISTS "${probe}")
    file(REMOVE_RECURSE "${probe}")
    message(FATAL_ERROR "fetch_guard: a refused DIR created ${probe}")
  endif()
endif()

string(TOUPPER "${SOURCE_DIR}" upper)
if(NOT upper STREQUAL SOURCE_DIR AND EXISTS "${upper}")
  expect_refusal("${upper}" "is inside the source tree but not under build/")
endif()

message(STATUS "fetch_guard: every guard refused")
