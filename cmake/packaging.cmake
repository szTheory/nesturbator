# Release archives (ENGINEERING section 6): one ZIP per install component,
# named nesturbator-VERSION-COMPONENT-OS-ARCH.zip, with no top-level
# directory. `cpack --preset ci` writes them to <build>/packages.

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(NESTURBATOR_OS linux)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(NESTURBATOR_OS macos)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(NESTURBATOR_OS windows)
else()
  message(FATAL_ERROR "packaging: no archive name for CMAKE_SYSTEM_NAME '${CMAKE_SYSTEM_NAME}'")
endif()

# The architecture the binaries are built for, not the build machine's.
if(MSVC)
  set(nesturbator_arch_raw "${CMAKE_C_COMPILER_ARCHITECTURE_ID}")
else()
  set(nesturbator_arch_raw "${CMAKE_SYSTEM_PROCESSOR}")
  list(LENGTH CMAKE_OSX_ARCHITECTURES nesturbator_osx_arch_count)
  if(APPLE AND nesturbator_osx_arch_count EQUAL 1)
    set(nesturbator_arch_raw "${CMAKE_OSX_ARCHITECTURES}")
  endif()
endif()
if(nesturbator_arch_raw MATCHES "^(x86_64|AMD64|x64)$")
  set(NESTURBATOR_ARCH x64)
elseif(nesturbator_arch_raw MATCHES "^(arm64|aarch64|ARM64)$")
  set(NESTURBATOR_ARCH arm64)
else()
  message(FATAL_ERROR "packaging: no archive name for architecture '${nesturbator_arch_raw}'")
endif()

set(CPACK_GENERATOR ZIP)
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_COMPONENTS_ALL library runner libretro)
set(CPACK_PACKAGE_DIRECTORY ${PROJECT_BINARY_DIR}/packages)
# The component name in these variables is upper case and has no extension
# (cpack-generators(7), CPACK_ARCHIVE_<component>_FILE_NAME).
set(nesturbator_archive_suffix "${NESTURBATOR_OS}-${NESTURBATOR_ARCH}")
set(CPACK_ARCHIVE_LIBRARY_FILE_NAME
  "nesturbator-${PROJECT_VERSION}-library-${nesturbator_archive_suffix}")
set(CPACK_ARCHIVE_RUNNER_FILE_NAME
  "nesturbator-${PROJECT_VERSION}-runner-${nesturbator_archive_suffix}")
set(CPACK_ARCHIVE_LIBRETRO_FILE_NAME
  "nesturbator-${PROJECT_VERSION}-libretro-${nesturbator_archive_suffix}")
include(CPack)
