# The runner.stop.* cases: nesturbator-run stopped by its own signal (--raise-after) on the
# synthetic cartridges from runner.save_rom.
#
#   cmake -DRUNNER=<nesturbator-run> -DFIXTURE=<runner.save_rom output dir>
#         -DWORK=<scratch dir, recreated> -DCASE=<case> -P runner_stop.cmake

cmake_minimum_required(VERSION 3.25)

foreach(var RUNNER FIXTURE WORK CASE)
  if(NOT DEFINED ${var} OR "${${var}}" STREQUAL "")
    message(FATAL_ERROR "runner_stop: -D${var}=... is required")
  endif()
endforeach()

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")

# Runs the runner with ARGN; sets run_exit, run_out and run_err in the caller.
function(run_runner)
  execute_process(COMMAND "${RUNNER}" ${ARGN}
    OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE rc)
  set(run_exit "${rc}" PARENT_SCOPE)
  set(run_out "${out}" PARENT_SCOPE)
  set(run_err "${err}" PARENT_SCOPE)
endfunction()

function(expect_exit want)
  if(NOT "${run_exit}" STREQUAL "${want}")
    message(FATAL_ERROR "runner_stop ${CASE}: exit ${run_exit}, wanted ${want}\n${run_err}")
  endif()
endfunction()

function(expect_absent path)
  if(EXISTS "${path}")
    message(FATAL_ERROR "runner_stop ${CASE}: ${path} must not exist")
  endif()
endfunction()

function(expect_stop_line n)
  string(FIND "${run_err}" "nesturbator-run: stopped by signal after frame ${n}\n" at)
  if(at LESS 0)
    message(FATAL_ERROR "runner_stop ${CASE}: no stop line for frame ${n} in:\n${run_err}")
  endif()
endfunction()

set(battery "${FIXTURE}/battery.nes")

if(CASE MATCHES "^frame_exact\\.(.+)$")
  set(sig "${CMAKE_MATCH_1}")
  file(MAKE_DIRECTORY "${WORK}/a" "${WORK}/b")
  run_runner(--rom "${battery}" --frames 4294967295 --hash-frame 5 --raise-after 5:${sig}
    --save-dir "${WORK}/a")
  set(a_out "${run_out}")
  expect_exit(0)
  expect_stop_line(5)
  run_runner(--rom "${battery}" --frames 5 --hash-frame 5 --save-dir "${WORK}/b")
  expect_exit(0)
  if(NOT "${a_out}" STREQUAL "${run_out}")
    message(FATAL_ERROR "runner_stop ${CASE}: stopped stdout differs:\n${a_out}\nvs\n${run_out}")
  endif()
  if(NOT "${a_out}" MATCHES "^frame 5 [^\n]*\n$")
    message(FATAL_ERROR "runner_stop ${CASE}: stdout is not one frame 5 line:\n${a_out}")
  endif()
  file(SHA256 "${WORK}/a/battery.sav" sum_a)
  file(SHA256 "${WORK}/b/battery.sav" sum_b)
  if(NOT sum_a STREQUAL sum_b)
    message(FATAL_ERROR "runner_stop ${CASE}: battery.sav differs between stopped and plain run")
  endif()
  expect_absent("${WORK}/a/battery.sav.tmp")
else()
  message(FATAL_ERROR "runner_stop: unknown case ${CASE}")
endif()
