# FRAME-06 / DEC.12: the core is integer-only and links only the C memory
# functions, checked three ways under preset nofp (label abi). Each check has
# a fixture that shows it can fail.
if(NOT CMAKE_NM)
  message(FATAL_ERROR "NESTURBATOR_NOFP needs nm (CMAKE_NM is not set)")
endif()
set(scripts ${PROJECT_SOURCE_DIR}/tests/cmake)

# A text scan of the core's sources for float, double and floating literals.
add_test(NAME abi.float_scan
  COMMAND ${CMAKE_COMMAND}
    "-DROOTS=${PROJECT_SOURCE_DIR}/src;${PROJECT_SOURCE_DIR}/include"
    -P ${scripts}/float_scan.cmake)
add_test(NAME abi.float_scan.selftest
  COMMAND ${CMAKE_COMMAND} -DSELFTEST=ON -DWORK=${CMAKE_CURRENT_BINARY_DIR}
    -P ${scripts}/float_scan.cmake)

# Every undefined symbol of the release library is on the allowlist.
add_test(NAME abi.undefined_symbols
  COMMAND ${CMAKE_COMMAND} -DNM=${CMAKE_NM} -DLIB=$<TARGET_FILE:nesturbator>
    -DAPPLE=${APPLE} -P ${scripts}/undefined_symbols.cmake)

# No writable data symbol in the release library.
add_test(NAME abi.global_symbols
  COMMAND ${CMAKE_COMMAND} -DNM=${CMAKE_NM} -DLIB=$<TARGET_FILE:nesturbator>
    -P ${scripts}/global_symbols.cmake)

# A double multiply built with the core's flag: built only by the test, which
# passes if the compiler or the undefined-symbol check rejects it.
add_library(nesturbator_float_fixture STATIC EXCLUDE_FROM_ALL float_fixture.c)
target_compile_options(nesturbator_float_fixture PRIVATE -mgeneral-regs-only)
add_test(NAME abi.float_fixture
  COMMAND ${CMAKE_COMMAND} -DBIN=${PROJECT_BINARY_DIR} -DNM=${CMAKE_NM}
    -DLIB=$<TARGET_FILE:nesturbator_float_fixture> -DAPPLE=${APPLE}
    -DSCRIPTS=${scripts} -P ${scripts}/float_fixture.cmake)

set_tests_properties(abi.float_scan abi.float_scan.selftest
  abi.undefined_symbols abi.global_symbols abi.float_fixture
  PROPERTIES LABELS abi)
