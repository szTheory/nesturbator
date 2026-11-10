# FRAME-02: a separate project that finds the installed package. The test
# install.consumer builds it against <build>/stage and runs it.
cmake_minimum_required(VERSION 3.25)
project(consumer C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(DEFINED QUALIFIED_PREFIX AND NOT QUALIFIED_PREFIX STREQUAL "")
  get_filename_component(qualified_prefix "${QUALIFIED_PREFIX}" REALPATH)
  find_package(nesturbator CONFIG REQUIRED
    PATHS
      "${qualified_prefix}/lib/cmake/nesturbator"
      "${qualified_prefix}/lib64/cmake/nesturbator"
    NO_DEFAULT_PATH)

  function(require_inside_qualified_prefix value description)
    if(NOT IS_ABSOLUTE "${value}" OR "${value}" MATCHES "\\$<")
      message(FATAL_ERROR "${description} is not a resolved absolute path: ${value}")
    endif()
    get_filename_component(resolved "${value}" REALPATH)
    cmake_path(IS_PREFIX qualified_prefix "${resolved}" NORMALIZE inside_prefix)
    if(NOT inside_prefix)
      message(FATAL_ERROR "${description} escapes QUALIFIED_PREFIX: ${resolved}")
    endif()
  endfunction()

  get_target_property(consumer_includes nesturbator::nesturbator INTERFACE_INCLUDE_DIRECTORIES)
  if(NOT consumer_includes)
    message(FATAL_ERROR "installed target has no public include directory")
  endif()
  foreach(include_dir IN LISTS consumer_includes)
    require_inside_qualified_prefix("${include_dir}" "installed include directory")
  endforeach()

  get_target_property(imported_configs nesturbator::nesturbator IMPORTED_CONFIGURATIONS)
  set(imported_locations)
  foreach(config IN LISTS imported_configs)
    string(TOUPPER "${config}" config_upper)
    get_target_property(location nesturbator::nesturbator "IMPORTED_LOCATION_${config_upper}")
    if(location)
      list(APPEND imported_locations "${location}")
    endif()
  endforeach()
  get_target_property(generic_location nesturbator::nesturbator IMPORTED_LOCATION)
  if(generic_location)
    list(APPEND imported_locations "${generic_location}")
  endif()
  if(NOT imported_locations)
    message(FATAL_ERROR "installed target has no imported library location")
  endif()
  list(REMOVE_DUPLICATES imported_locations)
  foreach(location IN LISTS imported_locations)
    require_inside_qualified_prefix("${location}" "imported library")
  endforeach()

  file(WRITE "${CMAKE_BINARY_DIR}/package-resolution.txt"
    "package=${nesturbator_DIR}\n"
    "prefix=${qualified_prefix}\n"
    "includes=${consumer_includes}\n"
    "libraries=${imported_locations}\n"
    "compiler=${CMAKE_C_COMPILER}\n")
else()
  find_package(nesturbator CONFIG REQUIRED)
endif()

add_executable(consumer main.c)
target_link_libraries(consumer PRIVATE nesturbator::nesturbator)
