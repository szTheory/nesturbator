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

# D-27 as corrected: frame_index identifies a RetroArch screenshot among the runner's frames
# 1..W (RGB, not the native hash). Its two tests run on every platform; the tool needs the
# library and the host converter. nesteroids repeats frames in adjacent pairs, so a screenshot
# can name two indices.
add_executable(frame_index frame_index.c bmp_ppm.c)
target_link_libraries(frame_index PRIVATE nesturbator nesturbator_host)
target_include_directories(frame_index PRIVATE ${PROJECT_SOURCE_DIR}/tests ${PROJECT_SOURCE_DIR}/src)
target_compile_definitions(frame_index PRIVATE
  $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
nesturbator_warnings(frame_index)

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
add_test(NAME retroarch.progression.controls
  COMMAND frame_index --progression-controls-selftest
    "${CMAKE_CURRENT_BINARY_DIR}/progression artifacts")
set_tests_properties(retroarch.progression.controls PROPERTIES
  FIXTURES_REQUIRED progression_files TIMEOUT 60)
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
