# A C-only project that embeds nesturbator with add_subdirectory, as a host
# using FetchContent would. The test embed.subdirectory builds it and runs it.
# Configuring fails if embedding enabled C++, added a target only the
# top-level project needs, or wrote a CPack configuration into this project.
cmake_minimum_required(VERSION 3.25)
project(embed C)

if(NOT DEFINED NESTURBATOR_SOURCE_DIR)
  message(FATAL_ERROR "embed: set NESTURBATOR_SOURCE_DIR")
endif()
add_subdirectory(${NESTURBATOR_SOURCE_DIR} nesturbator)

get_property(languages GLOBAL PROPERTY ENABLED_LANGUAGES)
if("CXX" IN_LIST languages)
  message(FATAL_ERROR "embed: nesturbator enabled C++ in an embedding project")
endif()
foreach(target nesturbator_host nesturbator-run nesturbator_libretro palgen vecconv)
  if(TARGET ${target})
    message(FATAL_ERROR "embed: nesturbator added ${target} to an embedding project")
  endif()
endforeach()
# Besides the library, embedding adds the CPU object library, the functions
# nesturbator_core_flags and nesturbator_warnings and the option
# NESTURBATOR_NOFP, as the README lists. A new name must be added here and
# there.
get_property(targets DIRECTORY ${NESTURBATOR_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)
if(NOT targets STREQUAL "nesturbator_cpu;nesturbator")
  message(FATAL_ERROR "embed: nesturbator added the targets '${targets}', not 'nesturbator_cpu;nesturbator'")
endif()
foreach(command nesturbator_core_flags nesturbator_warnings)
  if(NOT COMMAND ${command})
    message(FATAL_ERROR "embed: the README lists ${command}, which nesturbator no longer adds")
  endif()
endforeach()
if(NOT DEFINED NESTURBATOR_NOFP)
  message(FATAL_ERROR "embed: the README lists NESTURBATOR_NOFP, which nesturbator no longer adds")
endif()
if(EXISTS ${CMAKE_BINARY_DIR}/CPackConfig.cmake)
  message(FATAL_ERROR "embed: nesturbator wrote a CPack configuration")
endif()

add_executable(embed ${CMAKE_CURRENT_SOURCE_DIR}/../consumer/main.c)
target_link_libraries(embed PRIVATE nesturbator::nesturbator)
