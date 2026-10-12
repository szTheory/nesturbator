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
target_compile_definitions(frame_index PRIVATE
  $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
nesturbator_warnings(frame_index)

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
set_tests_properties(retroarch.frame_index.dups PROPERTIES TIMEOUT 60)
