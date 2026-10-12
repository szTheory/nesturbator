cmake_minimum_required(VERSION 3.25)

# Scratch project for libretro.info_claims.self_test: it registers a dummy CTest for each name
# in the claim table except OMIT, then defers the guard exactly as tests/CMakeLists.txt does.
# -DINFO is the .info to guard; -DOMIT is one test name to leave out (or empty).
project(claim_guard LANGUAGES NONE)
enable_testing()
include(${CMAKE_CURRENT_LIST_DIR}/../info_claims.cmake)

set(names ${NESTURBATOR_CLAIM_TESTS_serialized} ${NESTURBATOR_CLAIM_TESTS_deterministic})
list(REMOVE_DUPLICATES names)
foreach(name IN LISTS names)
  if(NOT name STREQUAL OMIT)
    add_test(NAME ${name} COMMAND ${CMAKE_COMMAND} -E true)
  endif()
endforeach()
cmake_language(DEFER CALL nesturbator_info_claim_guard "${INFO}")
