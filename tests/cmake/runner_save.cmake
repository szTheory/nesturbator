# The runner.save.* cases: nesturbator-run with --save-dir on synthetic
# cartridges from runner.save_rom.
#
#   cmake -DRUNNER=<nesturbator-run> -DFIXTURE=<runner.save_rom output dir>
#         -DWORK=<scratch dir, recreated> -DCASE=<case> -P runner_save.cmake

cmake_minimum_required(VERSION 3.25)

foreach(var RUNNER FIXTURE WORK CASE)
  if(NOT DEFINED ${var} OR "${${var}}" STREQUAL "")
    message(FATAL_ERROR "runner_save: -D${var}=... is required")
  endif()
endforeach()

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")

# Runs the runner with ARGN; sets run_exit and run_err in the caller.
function(run_runner)
  execute_process(COMMAND "${RUNNER}" ${ARGN}
    OUTPUT_QUIET ERROR_VARIABLE err RESULT_VARIABLE rc)
  set(run_exit "${rc}" PARENT_SCOPE)
  set(run_err "${err}" PARENT_SCOPE)
endfunction()

function(expect_exit want)
  if(NOT "${run_exit}" STREQUAL "${want}")
    message(FATAL_ERROR "runner_save ${CASE}: exit ${run_exit}, wanted ${want}\n${run_err}")
  endif()
endfunction()

function(expect_size path want)
  file(SIZE "${path}" got)
  if(NOT got EQUAL want)
    message(FATAL_ERROR "runner_save ${CASE}: ${path} is ${got} bytes, wanted ${want}")
  endif()
endfunction()

# Hex of LIMIT bytes of path from OFFSET.
function(expect_bytes path offset want)
  string(LENGTH "${want}" hexlen)
  math(EXPR limit "${hexlen} / 2")
  file(READ "${path}" got HEX OFFSET ${offset} LIMIT ${limit})
  if(NOT "${got}" STREQUAL "${want}")
    message(FATAL_ERROR "runner_save ${CASE}: ${path} at ${offset} is ${got}, wanted ${want}")
  endif()
endfunction()

function(expect_absent path)
  if(EXISTS "${path}")
    message(FATAL_ERROR "runner_save ${CASE}: ${path} must not exist")
  endif()
endfunction()

set(sav "${WORK}/battery.sav")
set(battery "${FIXTURE}/battery.nes")

if(CASE STREQUAL "roundtrip")
  # The ROM copies $6100 to $6004, so the seeded $42 at offset 0x100 can only
  # reach offset 4 if the file was loaded before the first frame.
  file(COPY_FILE "${FIXTURE}/seed.sav" "${sav}")
  run_runner(--rom "${battery}" --frames 2 --save-dir "${WORK}")
  expect_exit(0)
  expect_size("${sav}" 8192)
  expect_bytes("${sav}" 0 "5341564542")
  expect_bytes("${sav}" 256 "42")
  expect_absent("${sav}.tmp")
elseif(CASE STREQUAL "fresh")
  run_runner(--rom "${battery}" --frames 2 --save-dir "${WORK}")
  expect_exit(0)
  expect_size("${sav}" 8192)
  expect_bytes("${sav}" 0 "5341564500")
  expect_absent("${sav}.tmp")
else()
  message(FATAL_ERROR "runner_save: unknown CASE ${CASE}")
endif()
