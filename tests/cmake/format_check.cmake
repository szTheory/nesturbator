# Checks that every tracked C and C++ source is formatted as .clang-format
# says. The file list comes from git, so a new source is checked without a
# change here. The vendored libretro/libretro.h is left as upstream wrote it.
#
#   cmake -DCLANG_FORMAT=<clang-format> -DGIT=<git> -DROOT=<source dir> -P format_check.cmake
execute_process(COMMAND "${GIT}" ls-files "*.c" "*.h" "*.cpp"
  WORKING_DIRECTORY "${ROOT}"
  OUTPUT_VARIABLE out RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
  message(FATAL_ERROR "git ls-files exited with ${rc}")
endif()
string(REPLACE "\n" ";" files "${out}")
list(FILTER files EXCLUDE REGEX "^$")
list(REMOVE_ITEM files "libretro/libretro.h")
if(NOT files)
  message(FATAL_ERROR "git ls-files found no C sources under ${ROOT}")
endif()
list(LENGTH files count)

execute_process(COMMAND "${CLANG_FORMAT}" --dry-run --Werror ${files}
  WORKING_DIRECTORY "${ROOT}"
  RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
  message(FATAL_ERROR "clang-format reports unformatted code; run clang-format -i on the files above")
endif()
message(STATUS "${count} files formatted")
