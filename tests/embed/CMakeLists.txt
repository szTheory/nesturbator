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
foreach(target nesturbator_host nesturbator-run nesturbator_libretro palgen)
  if(TARGET ${target})
    message(FATAL_ERROR "embed: nesturbator added ${target} to an embedding project")
  endif()
endforeach()
if(EXISTS ${CMAKE_BINARY_DIR}/CPackConfig.cmake)
  message(FATAL_ERROR "embed: nesturbator wrote a CPack configuration")
endif()

add_executable(embed ${CMAKE_CURRENT_SOURCE_DIR}/../consumer/main.c)
target_link_libraries(embed PRIVATE nesturbator::nesturbator)
