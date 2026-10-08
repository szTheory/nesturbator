cmake_minimum_required(VERSION 3.25)

# The single version source; release-please updates version.txt.
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/version.txt" NESTURBATOR_VERSION LIMIT_COUNT 1)
# Only C is needed to build the library. A project that embeds this one with
# add_subdirectory or FetchContent gets the library and its install rules,
# plus the nesturbator_-prefixed helpers below (tests/embed checks the list);
# the runner, the libretro core, palgen, vecconv, the tests (which need C++)
# and the release packaging are added only when this is the top-level project.
project(nesturbator VERSION ${NESTURBATOR_VERSION} LANGUAGES C)

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

# Preset nofp: the core gets no floating-point or vector registers, and the
# abi tests check it (ENGINEERING section 1).
option(NESTURBATOR_NOFP "Build the core with -mgeneral-regs-only and run the abi tests" OFF)
if(NESTURBATOR_NOFP AND MSVC)
  message(FATAL_ERROR "NESTURBATOR_NOFP needs GCC or Clang (-mgeneral-regs-only)")
endif()

# The flags every core target gets (D-09): the public header, hidden
# visibility, position-independent code, the warnings and, under nofp,
# -mgeneral-regs-only.
function(nesturbator_core_flags target)
  target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
  set_target_properties(${target} PROPERTIES
    C_VISIBILITY_PRESET hidden
    POSITION_INDEPENDENT_CODE ON)
  nesturbator_warnings(${target})
  if(NESTURBATOR_NOFP)
    target_compile_options(${target} PRIVATE -mgeneral-regs-only)
  endif()
endfunction()

# The CPU as an object library: the library folds its object in, and the
# vector test links the same object against a test bus (D-08, D-09).
add_library(nesturbator_cpu OBJECT src/cpu.c)
nesturbator_core_flags(nesturbator_cpu)

# The core library. It links nothing: only the C memory functions are used.
add_library(nesturbator STATIC
  src/instance.c
  src/cartridge.c
  src/frame.c
  src/bus.c
  src/palette.c
  src/palette_ntsc.c
  src/testcard.c)
target_sources(nesturbator PRIVATE $<TARGET_OBJECTS:nesturbator_cpu>)
add_library(nesturbator::nesturbator ALIAS nesturbator)
target_include_directories(nesturbator PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
nesturbator_core_flags(nesturbator)
set_target_properties(nesturbator PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1) # D-07: SOVERSION equals NESTURBATOR_ABI_VERSION

# Install components (ENGINEERING section 2): `library` is the static library,
# the header, a relocatable CMake package and nesturbator.pc; `runner` and
# `libretro` are installed from their directories. nesturbator_host, palgen,
# vecconv and the tests are never installed.
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
set(NESTURBATOR_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/nesturbator)
install(TARGETS nesturbator EXPORT nesturbatorTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT library)
install(FILES include/nesturbator.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT library)
install(EXPORT nesturbatorTargets NAMESPACE nesturbator::
  DESTINATION ${NESTURBATOR_CMAKE_DIR} COMPONENT library)
configure_package_config_file(cmake/nesturbatorConfig.cmake.in
  ${PROJECT_BINARY_DIR}/nesturbatorConfig.cmake
  INSTALL_DESTINATION ${NESTURBATOR_CMAKE_DIR})
write_basic_package_version_file(${PROJECT_BINARY_DIR}/nesturbatorConfigVersion.cmake
  COMPATIBILITY SameMinorVersion)
install(FILES
    ${PROJECT_BINARY_DIR}/nesturbatorConfig.cmake
    ${PROJECT_BINARY_DIR}/nesturbatorConfigVersion.cmake
  DESTINATION ${NESTURBATOR_CMAKE_DIR} COMPONENT library)
# nesturbator.pc finds the prefix from its own directory, so the install
# can move.
file(RELATIVE_PATH NESTURBATOR_PC_PREFIX
  /prefix/${CMAKE_INSTALL_LIBDIR}/pkgconfig /prefix)
string(REGEX REPLACE "/$" "" NESTURBATOR_PC_PREFIX "${NESTURBATOR_PC_PREFIX}")
configure_file(cmake/nesturbator.pc.in ${PROJECT_BINARY_DIR}/nesturbator.pc @ONLY)
install(FILES ${PROJECT_BINARY_DIR}/nesturbator.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig COMPONENT library)
install(FILES LICENSE DESTINATION . COMPONENT library)

if(NOT PROJECT_IS_TOP_LEVEL)
  return()
endif()

enable_language(CXX)

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

foreach(component runner libretro)
  install(FILES LICENSE DESTINATION . COMPONENT ${component})
endforeach()
# The libretro module is compiled against the vendored libretro.h.
install(FILES THIRD-PARTY-NOTICES.md DESTINATION . COMPONENT libretro)

enable_testing()
add_subdirectory(runner)
add_subdirectory(libretro)
add_subdirectory(tools/palgen)
add_subdirectory(tools/vecconv)
add_subdirectory(tests)
if(NESTURBATOR_NOFP)
  add_subdirectory(tests/abi)
endif()

# Preset hygiene: the forbidden-content scan, the action-pin check and the
# format check as CTest tests. A missing tool fails configure (ENGINEERING
# section 2). Formatting is checked with clang-format 18 only, the version on
# CI's ubuntu-24.04 image; another major version formats differently.
option(NESTURBATOR_HYGIENE "Register the hygiene tests" OFF)
if(NESTURBATOR_HYGIENE)
  find_program(NESTURBATOR_SH sh REQUIRED)
  find_program(NESTURBATOR_GIT git REQUIRED)
  find_program(NESTURBATOR_CLANG_FORMAT NAMES clang-format-18 clang-format
    HINTS /opt/homebrew/opt/llvm@18/bin /usr/local/opt/llvm@18/bin REQUIRED)
  execute_process(COMMAND ${NESTURBATOR_CLANG_FORMAT} --version
    OUTPUT_VARIABLE clang_format_version)
  if(NOT clang_format_version MATCHES "version 18\\.")
    message(FATAL_ERROR "the hygiene preset needs clang-format 18; "
      "${NESTURBATOR_CLANG_FORMAT} reports: ${clang_format_version}")
  endif()
  add_subdirectory(tests/hygiene)
endif()

# Release archives; included last so every install rule is known.
include(cmake/packaging.cmake)
