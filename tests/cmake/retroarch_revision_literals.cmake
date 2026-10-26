# The hosted RetroArch scripts run only on the pinned macOS runner, so a state
# format or behaviour revision written into them as a number goes stale
# unnoticed when the header changes (PR #57: revision 6 was left behind after
# NESTURBATOR_BEHAVIOUR_REVISION became 7). They must read both numbers from
# include/nesturbator.h. This check fails on any "format=<digit>" or
# "revision=<digit>" literal in tests/retroarch/*.cmake.
#
#   cmake -DDIR=<tests/retroarch> -P retroarch_revision_literals.cmake

cmake_minimum_required(VERSION 3.25)

file(GLOB scripts "${DIR}/*.cmake")
if(NOT scripts)
  message(FATAL_ERROR "retroarch_revision_literals: no scripts found in ${DIR}")
endif()
set(found "")
foreach(script IN LISTS scripts)
  file(STRINGS "${script}" lines REGEX "(format|revision)=[0-9]")
  foreach(line IN LISTS lines)
    cmake_path(GET script FILENAME name)
    string(STRIP "${line}" line)
    list(APPEND found "${name}: ${line}")
  endforeach()
endforeach()
if(found)
  list(JOIN found "\n  " found)
  message(FATAL_ERROR
    "retroarch_revision_literals: read the state format and behaviour revision from include/nesturbator.h instead of\n  ${found}")
endif()
list(LENGTH scripts count)
message(STATUS "${count} RetroArch scripts carry no state format or revision literal")
