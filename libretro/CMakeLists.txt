# The libretro core: a module RetroArch loads at run time. Hidden visibility
# here and in the two static libraries it links leaves only the retro_*
# functions, which libretro.h marks with default visibility or dllexport,
# in the export table.
add_library(nesturbator_libretro MODULE libretro.c)
target_include_directories(nesturbator_libretro PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(nesturbator_libretro PRIVATE nesturbator::nesturbator nesturbator_host)
# No "lib" prefix: the frontend and the release name it nesturbator_libretro.
# CMake gives a MODULE the suffix .so on macOS; RetroArch expects .dylib.
set_target_properties(nesturbator_libretro PROPERTIES
  PREFIX ""
  C_VISIBILITY_PRESET hidden)
if(APPLE)
  set_target_properties(nesturbator_libretro PROPERTIES SUFFIX ".dylib")
endif()
nesturbator_warnings(nesturbator_libretro)
