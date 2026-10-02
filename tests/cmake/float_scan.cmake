# The core is integer-only (ENGINEERING section 1). This text scan is the
# check for floating-point literals: constant folding leaves no trace of them
# in an object file, so the symbol test cannot see them. It reports every
# line of a .c or .h file under ROOTS that, once hex literals are removed,
# contains the keyword float or double or a decimal floating literal:
# 1.5, 1.e3 or 1e3. Comments count, so they write numbers as integers.
#
#   cmake "-DROOTS=<dir>;<dir>" -P float_scan.cmake
#   cmake -DSELFTEST=ON -DWORK=<dir> -P float_scan.cmake
#
# SELFTEST writes a file holding `x = 1.5;` beside a hex literal and fails
# unless exactly that line is reported.

# Appends "file:line: text" for each finding in FILE to the list OUT.
function(float_scan_file file out)
  file(READ "${file}" text)
  # Make each line one list element: neutralise the characters that would
  # split or join list elements, then split on newlines.
  string(REPLACE "\r" "" text "${text}")
  string(REPLACE "\\" "/" text "${text}")
  string(REPLACE ";" "," text "${text}")
  string(REPLACE "[" "(" text "${text}")
  string(REPLACE "]" ")" text "${text}")
  string(REPLACE "\n" ";" text "${text}")
  set(found ${${out}})
  set(n 0)
  foreach(line IN LISTS text)
    math(EXPR n "${n} + 1")
    string(REGEX REPLACE "0[xX][0-9a-fA-F]+" "" s " ${line} ")
    if(s MATCHES "[^A-Za-z0-9_](float|double)[^A-Za-z0-9_]"
        OR s MATCHES "[0-9]\\.[0-9]"
        OR s MATCHES "[0-9]\\.[eE]"
        OR s MATCHES "[^A-Za-z0-9_][0-9]+[eE][+-]?[0-9]")
      list(APPEND found "${file}:${n}: ${line}")
    endif()
  endforeach()
  set(${out} "${found}" PARENT_SCOPE)
endfunction()

if(SELFTEST)
  set(dir "${WORK}/float_scan_selftest")
  file(REMOVE_RECURSE "${dir}")
  file(WRITE "${dir}/sample.c" "int y = 0x1E5;\ndouble_free = 2;\nx = 1.5;\n")
  set(found)
  float_scan_file("${dir}/sample.c" found)
  if(NOT found STREQUAL "${dir}/sample.c:3: x = 1.5,")
    message(FATAL_ERROR "self-test: expected only line 3 of sample.c, got: ${found}")
  endif()
  message(STATUS "self-test reported: ${found}")
  return()
endif()

set(found)
set(count 0)
foreach(root IN LISTS ROOTS)
  if(NOT IS_DIRECTORY "${root}")
    message(FATAL_ERROR "${root} is not a directory")
  endif()
  file(GLOB_RECURSE files "${root}/*.c" "${root}/*.h")
  foreach(f IN LISTS files)
    float_scan_file("${f}" found)
    math(EXPR count "${count} + 1")
  endforeach()
endforeach()
if(found)
  foreach(line IN LISTS found)
    message("${line}")
  endforeach()
  message(FATAL_ERROR "floating point in the core: see the lines above")
endif()
message(STATUS "${count} files scanned, no floating point")
