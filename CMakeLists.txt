cmake_minimum_required(VERSION 3.25)

# The single version source; release-please updates version.txt.
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/version.txt" NESTURBATOR_VERSION LIMIT_COUNT 1)
project(nesturbator VERSION ${NESTURBATOR_VERSION} LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Warning set from ENGINEERING section 1. Presets make warnings fatal through
# CMAKE_COMPILE_WARNING_AS_ERROR, so an embedding build is unaffected.
function(nesturbator_warnings target)
  target_compile_options(${target} PRIVATE
    $<$<C_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Wstrict-prototypes -Wmissing-prototypes -Wvla>
    $<$<C_COMPILER_ID:GNU>:-Wduplicated-cond -Wlogical-op>
    $<$<C_COMPILER_ID:MSVC>:/W4>)
endfunction()

# The core library. It links nothing: only the C memory functions are used.
add_library(nesturbator STATIC
  src/instance.c
  src/frame.c
  src/palette.c
  src/palette_ntsc.c
  src/testcard.c)
add_library(nesturbator::nesturbator ALIAS nesturbator)
target_include_directories(nesturbator PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
set_target_properties(nesturbator PROPERTIES
  C_VISIBILITY_PRESET hidden
  POSITION_INDEPENDENT_CODE ON
  VERSION ${PROJECT_VERSION}
  SOVERSION 1) # D-07: SOVERSION equals NESTURBATOR_ABI_VERSION
nesturbator_warnings(nesturbator)

# D-15: the native-to-XRGB8888 conversion the runner and the libretro adapter
# share. Not part of the core and not installed. Hidden visibility keeps its
# symbol out of the libretro module's exports; PIC lets it link into that
# module.
add_library(nesturbator_host STATIC host/convert.c)
target_include_directories(nesturbator_host PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/host)
set_target_properties(nesturbator_host PROPERTIES
  C_VISIBILITY_PRESET hidden
  POSITION_INDEPENDENT_CODE ON)
nesturbator_warnings(nesturbator_host)

enable_testing()
add_subdirectory(runner)
add_subdirectory(tools/palgen)
add_subdirectory(tests)
