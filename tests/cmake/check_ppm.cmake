# Checks a frame image written by nesturbator-run --dump-frame (D-16).
#
#   cmake [-DCMD=<program;arg;...> -DEXPECT=<line;...> -DEXPECT_EXIT=<code>]
#         -DFILE=<path> -DSIZE=<bytes> [-DPIXELS=<x:y:rrggbb;...>]
#         -P check_ppm.cmake
#
# With CMD set, it first removes FILE, then runs CMD with the checks of
# expect_output.cmake. It then checks that FILE has exactly SIZE bytes, starts
# with the P6 header "P6\n256 240\n255\n", that its first pixel is white
# (FFFFFF) and that each pixel listed in PIXELS has the given colour.

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED FILE OR NOT DEFINED SIZE)
  message(FATAL_ERROR "check_ppm: FILE and SIZE must be set")
endif()

if(DEFINED CMD AND NOT CMD STREQUAL "")
  file(REMOVE "${FILE}")
  get_filename_component(dir "${FILE}" DIRECTORY)
  file(MAKE_DIRECTORY "${dir}")
  include("${CMAKE_CURRENT_LIST_DIR}/expect_output.cmake")
endif()

if(NOT EXISTS "${FILE}")
  message(FATAL_ERROR "check_ppm: ${FILE} was not written")
endif()
file(SIZE "${FILE}" actual_size)
if(NOT actual_size EQUAL SIZE)
  message(FATAL_ERROR "check_ppm: ${FILE} is ${actual_size} bytes, expected ${SIZE}")
endif()

# file(READ ... HEX) gives two lowercase hex digits per byte.
file(READ "${FILE}" hex HEX)
set(header_len 15)
# "P6\n256 240\n255\n"
set(expected_header "50360a323536203234300a3235350a")
string(SUBSTRING "${hex}" 0 30 header)
if(NOT header STREQUAL expected_header)
  message(FATAL_ERROR "check_ppm: header is ${header}, expected ${expected_header}")
endif()

function(check_pixel x y rgb)
  math(EXPR offset "(${header_len} + (${y} * 256 + ${x}) * 3) * 2")
  string(SUBSTRING "${hex}" ${offset} 6 actual)
  string(TOLOWER "${rgb}" want)
  if(NOT actual STREQUAL want)
    message(FATAL_ERROR "check_ppm: pixel (${x},${y}) is ${actual}, expected ${want}")
  endif()
endfunction()

check_pixel(0 0 ffffff)
foreach(p IN LISTS PIXELS)
  string(REPLACE ":" ";" parts "${p}")
  list(GET parts 0 px)
  list(GET parts 1 py)
  list(GET parts 2 prgb)
  check_pixel(${px} ${py} ${prgb})
endforeach()
