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
