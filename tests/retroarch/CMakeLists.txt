# FRAME-05 / D-17: compare_frame checks a RetroArch screenshot against the
# runner's frame. Its readers are tested here on synthesised images on every
# platform; these tests carry no label, so ci and asan run them.
add_executable(compare_frame compare_frame.c bmp_ppm.c)
add_executable(test_compare_frame test_compare_frame.c bmp_ppm.c)
foreach(target compare_frame test_compare_frame)
  target_compile_definitions(${target} PRIVATE
    $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
  nesturbator_warnings(${target})
endforeach()

# test_compare_frame writes its images into cmp and leaves the equal pair
# frame.ppm and shot.bmp there for the command-line test.
set(cmp_dir ${CMAKE_CURRENT_BINARY_DIR}/cmp)
file(MAKE_DIRECTORY ${cmp_dir})
add_test(NAME retroarch.compare COMMAND test_compare_frame ${cmp_dir})
set_tests_properties(retroarch.compare PROPERTIES FIXTURES_SETUP cmp_files)
add_test(NAME retroarch.compare.cli
  COMMAND compare_frame ${cmp_dir}/frame.ppm ${cmp_dir}/shot.bmp)
set_tests_properties(retroarch.compare.cli PROPERTIES FIXTURES_REQUIRED cmp_files)

# RetroArch itself is exercised only by the hosted retroarch-e2e job, which
# runs run_retroarch.cmake directly; no local test launches it (TUNE-05).
# So that script reads the state format and behaviour revision from the public header; this
# check keeps a revision bump from leaving a stale number that only the hosted job would see.
add_test(NAME retroarch.revision_literals
  COMMAND ${CMAKE_COMMAND} -DDIR=${CMAKE_CURRENT_SOURCE_DIR}
    -P ${PROJECT_SOURCE_DIR}/tests/cmake/retroarch_revision_literals.cmake)
set_tests_properties(retroarch.revision_literals PROPERTIES TIMEOUT 30)

if(UNIX)
  find_program(NESTURBATOR_PYTHON NAMES python3 REQUIRED)
  add_test(NAME hygiene.push_format.controls
    COMMAND ${NESTURBATOR_PYTHON}
      ${PROJECT_SOURCE_DIR}/tests/cmake/test_push_format.py
      ${PROJECT_SOURCE_DIR}/scripts/check_push_format.py
      ${CMAKE_CURRENT_BINARY_DIR}/push-format-controls)
  set_tests_properties(hygiene.push_format.controls PROPERTIES TIMEOUT 30)
  add_test(NAME ci.session_inventory
    COMMAND ${CMAKE_COMMAND} -DCTEST=${CMAKE_CTEST_COMMAND}
      -DBUILD=${PROJECT_BINARY_DIR} -DSHORT_RUNS=${NESTURBATOR_SHORT_RUNS}
      -P ${PROJECT_SOURCE_DIR}/tests/cmake/session_inventory.cmake)
  set_tests_properties(ci.session_inventory PROPERTIES TIMEOUT 60)
endif()

# D-27 as corrected: frame_index identifies a RetroArch screenshot among the runner's frames
# 1..W (RGB, not the native hash). Its two tests run on every platform; the tool needs the
# library and the host converter. nesteroids repeats frames in adjacent pairs, so a screenshot
# can name two indices.
add_executable(frame_index frame_index.c bmp_ppm.c ${PROJECT_SOURCE_DIR}/runner/clock.c)
target_link_libraries(frame_index PRIVATE nesturbator nesturbator_host)
target_include_directories(frame_index PRIVATE ${PROJECT_SOURCE_DIR}/tests ${PROJECT_SOURCE_DIR}/src
  ${PROJECT_SOURCE_DIR}/runner)
target_compile_definitions(frame_index PRIVATE
  $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
nesturbator_warnings(frame_index)

add_test(NAME retroarch.session_helpers
  COMMAND ${CMAKE_COMMAND} -DFRAME_INDEX=$<TARGET_FILE:frame_index>
    -DWORK=${CMAKE_CURRENT_BINARY_DIR}/session-helper-controls
    -P ${PROJECT_SOURCE_DIR}/tests/cmake/session_helpers.cmake)
set_tests_properties(retroarch.session_helpers PROPERTIES TIMEOUT 30)

add_test(NAME retroarch.marker.controls
  COMMAND frame_index --marker-selftest ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(retroarch.marker.controls PROPERTIES
  PASS_REGULAR_EXPRESSION "marker controls: 36 cases" TIMEOUT 60)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/marker selftest fixtures")
add_test(NAME retroarch.marker.fixtures
  COMMAND frame_index --marker-selftest
    "${CMAKE_CURRENT_BINARY_DIR}/marker selftest fixtures" --fixtures)
set_tests_properties(retroarch.marker.fixtures PROPERTIES TIMEOUT 60)
add_test(NAME retroarch.progression.tracer
  COMMAND frame_index --progression-selftest "${CMAKE_CURRENT_BINARY_DIR}/progression artifacts")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/progression artifacts")
set_tests_properties(retroarch.progression.tracer PROPERTIES
  FIXTURES_SETUP progression_files TIMEOUT 60)
add_test(NAME retroarch.progression.prefix
  COMMAND frame_index --marker-verdict-prefix
    "${CMAKE_CURRENT_BINARY_DIR}/progression artifacts/marker.nes"
    "${CMAKE_CURRENT_BINARY_DIR}/progression artifacts/calibration.txt"
    "${CMAKE_CURRENT_BINARY_DIR}/progression artifacts/progression-prefix.rec")
set_tests_properties(retroarch.progression.prefix PROPERTIES
  FIXTURES_REQUIRED progression_files TIMEOUT 60)
add_test(NAME retroarch.progression.complete_required
  COMMAND frame_index --progression-complete-selftest
    "${CMAKE_CURRENT_BINARY_DIR}/progression artifacts")
set_tests_properties(retroarch.progression.complete_required PROPERTIES
  FIXTURES_REQUIRED progression_files TIMEOUT 60)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/progression control artifacts")
add_test(NAME retroarch.progression.controls
  COMMAND frame_index --progression-selftest
    "${CMAKE_CURRENT_BINARY_DIR}/progression control artifacts" --controls)
set_tests_properties(retroarch.progression.controls PROPERTIES TIMEOUT 120)

# The production state-session driver is exercised with a local fake transport on POSIX hosts.
# The fixture files and expected progression values are independently generated by frame_index.
if(UNIX AND NOT NESTURBATOR_SHORT_RUNS)
  add_test(NAME retroarch.session.source_root
    COMMAND sh "${CMAKE_CURRENT_SOURCE_DIR}/test_state_session.sh"
      "${CMAKE_CURRENT_SOURCE_DIR}/state_session.sh"
      "$<TARGET_FILE:frame_index>"
      "${CMAKE_CURRENT_BINARY_DIR}/session source root"
      "${CMAKE_COMMAND}"
      "${CMAKE_CURRENT_SOURCE_DIR}/run_retroarch.cmake")
  set_tests_properties(retroarch.session.source_root PROPERTIES
    ENVIRONMENT "STATE_SESSION_CONTROL=source-root-preflight" TIMEOUT 120 RUN_SERIAL TRUE)
  add_test(NAME retroarch.session.controls
    COMMAND sh "${CMAKE_CURRENT_SOURCE_DIR}/test_state_session.sh"
      "${CMAKE_CURRENT_SOURCE_DIR}/state_session.sh"
      "$<TARGET_FILE:frame_index>"
      "${CMAKE_CURRENT_BINARY_DIR}/session controls"
      "${CMAKE_COMMAND}"
      "${CMAKE_CURRENT_SOURCE_DIR}/run_retroarch.cmake")
  add_test(NAME retroarch.session.readiness
    COMMAND sh "${CMAKE_CURRENT_SOURCE_DIR}/test_state_session.sh"
      "${CMAKE_CURRENT_SOURCE_DIR}/state_session.sh"
      "$<TARGET_FILE:frame_index>"
      "${CMAKE_CURRENT_BINARY_DIR}/session readiness"
      "${CMAKE_COMMAND}"
      "${CMAKE_CURRENT_SOURCE_DIR}/run_retroarch.cmake")
  # This control has a strict driver-plus-cleanup wall bound; isolate its
  # measurement from concurrent CTest work.
  set_tests_properties(retroarch.session.readiness PROPERTIES
    ENVIRONMENT "STATE_SESSION_CONTROL=readiness" TIMEOUT 60 RUN_SERIAL TRUE)
  # The full local ASan control matrix passed in 555.02 s. Run 38598006408's
  # macOS Intel matrix failed at 510.75 s before readiness, isolation, interruption
  # and full-positive controls: QUIT's 30 s window could not fit after a 179.251 s
  # arrival at boundary 16. Replacing that case adds about 31 s; the unexecuted
  # tail projects to 229.426 s from the completed ASan run's same ordered cases.
  # The readiness correction adds about 6 s (one 1.5 s delayed exact reply and the
  # stalled exact reply's child cap increasing from 1 to 5 s). Projected total is
  # about 777 s, so retain a finite 900 s cap with about 123 s margin; the failed
  # 510.75 s run is not treated as a successful sample.
  # This test measures a cleanup matrix against its primary invocation; other
  # concurrent tests can inflate the measured wall-time delta beyond the fixed
  # budget without changing the cleanup behavior under test.
  set_tests_properties(retroarch.session.controls PROPERTIES TIMEOUT 900 RUN_SERIAL TRUE)
  # Local ci keeps the full matrix. Hosted builds use ci-build and run this
  # label once per OS in dedicated required jobs, without duplicating it in ASan.
  set_tests_properties(retroarch.session.source_root retroarch.session.controls
    retroarch.session.readiness PROPERTIES LABELS retroarch-session)
endif()

set(marker_rom ${CMAKE_CURRENT_BINARY_DIR}/marker.nes)
set(marker_cal ${CMAKE_CURRENT_BINARY_DIR}/calibration.txt)
add_test(NAME retroarch.marker.generate COMMAND frame_index --marker-generate ${marker_rom})
set_tests_properties(retroarch.marker.generate PROPERTIES FIXTURES_SETUP marker_content TIMEOUT 30)
add_test(NAME retroarch.marker.qualify COMMAND frame_index --marker-qualify ${marker_rom} ${marker_cal})
# The permitted absolute capture/rewind/display-lead excursion is 4..10000.
# A wall-clock deadline alone cannot bound the host's frame rate; later observations fail outside it.
set_tests_properties(retroarch.marker.qualify PROPERTIES FIXTURES_REQUIRED marker_content
  PASS_REGULAR_EXPRESSION "marker qualification: 9997 frames" TIMEOUT 600)

set(fi_rom ${PROJECT_SOURCE_DIR}/tests/roms/nesteroids.nes)
set(fi_ppm ${CMAKE_CURRENT_BINARY_DIR}/fi-37.ppm)
add_test(NAME retroarch.frame_index.setup
  COMMAND nesturbator-run --rom ${fi_rom} --frames 37 --dump-frame 37:${fi_ppm})
set_tests_properties(retroarch.frame_index.setup PROPERTIES
  FIXTURES_SETUP fi_dump TIMEOUT 60)
add_test(NAME retroarch.frame_index COMMAND frame_index ${fi_rom} 60 ${fi_ppm})
set_tests_properties(retroarch.frame_index PROPERTIES
  FIXTURES_REQUIRED fi_dump PASS_REGULAR_EXPRESSION " 37[ \n]" TIMEOUT 60)
add_test(NAME retroarch.frame_index.dups COMMAND frame_index --dups ${fi_rom} 1200)
# TIMEOUT 180: 24 s locally under asan, 38 s on a hosted asan runner, over 60 s on a slow one.
set_tests_properties(retroarch.frame_index.dups PROPERTIES TIMEOUT 180)

# D-28 as amended at planning: log_check.cmake fails on an unexplained RetroArch [ERROR] line or
# rewind/run-ahead warning. These tests run it on fixture logs on every platform (no labels), so
# a clean log is shown to pass and each kind of bad line is shown to fail; the expected-failure
# tests match the matcher's own message, so a script crash cannot pass them.
set(lc_dir ${CMAKE_CURRENT_SOURCE_DIR}/fixtures)
set(lc_script ${CMAKE_CURRENT_SOURCE_DIR}/log_check.cmake)
function(nesturbator_log_check_test name log level)
  cmake_parse_arguments(PARSE_ARGV 3 LC "" "ALLOWLIST;EXPECT" "")
  set(args -DLOG=${lc_dir}/${log} -DLEVEL=${level} -DREQUIRE_NETCMD=ON)
  if(LC_ALLOWLIST)
    list(APPEND args -DALLOWLIST=${lc_dir}/${LC_ALLOWLIST})
  endif()
  add_test(NAME ${name} COMMAND ${CMAKE_COMMAND} ${args} -P ${lc_script})
  set_tests_properties(${name} PROPERTIES TIMEOUT 30)
  if(LC_EXPECT)
    set_tests_properties(${name} PROPERTIES PASS_REGULAR_EXPRESSION "${LC_EXPECT}")
  endif()
endfunction()

nesturbator_log_check_test(retroarch.log_check log-clean.txt deterministic)
nesturbator_log_check_test(retroarch.log_check.serialized log-clean.txt serialized)
nesturbator_log_check_test(retroarch.log_check.unlisted_error log-unlisted-error.txt deterministic
  EXPECT "un-allowlisted line: \\[(ERROR|WARN)\\]")
nesturbator_log_check_test(retroarch.log_check.rewind_warn log-rewind-warn.txt deterministic
  EXPECT "un-allowlisted line: \\[(ERROR|WARN)\\]")
# A line allowlisted for serialized only: it passes at serialized and fails at deterministic.
nesturbator_log_check_test(retroarch.log_check.level_serialized log-rewind-warn.txt serialized
  ALLOWLIST log-level-allowlist.txt)
nesturbator_log_check_test(retroarch.log_check.level log-rewind-warn.txt deterministic
  ALLOWLIST log-level-allowlist.txt EXPECT "un-allowlisted line:")
nesturbator_log_check_test(retroarch.log_check.no_netcmd log-no-netcmd.txt serialized
  EXPECT "missing \\[NetCMD\\] line")
nesturbator_log_check_test(retroarch.log_check.failed_load log-failed-load.txt serialized
  ALLOWLIST allow-failed-load.txt EXPECT "failed state operation")
nesturbator_log_check_test(retroarch.log_check.uncommented log-clean.txt serialized
  ALLOWLIST allow-uncommented.txt EXPECT "no comment line directly above it")
