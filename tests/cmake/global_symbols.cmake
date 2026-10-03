# The core keeps no mutable global or static variable: all state lives in the
# instance (ENGINEERING section 1). Fails if the static library LIB defines a
# writable data symbol, which nm types B b (zero-initialised), D d
# (initialised) and C (common).
#
#   cmake -DNM=<nm> -DLIB=<static library> -P global_symbols.cmake
#
# Names starting with ltmp or l_ are assembler-local labels on Mach-O and are
# ignored. There is no exception for ELF .data.rel.ro: under position-
# independent code a file-scope const table of pointers (to functions or
# strings) is placed there, and nm reports it as d. So the core keeps no such
# table. The default allocator is plain code that stores the static
# malloc/free wrappers into the instance at creation, and const lookup tables
# hold integers only.

cmake_minimum_required(VERSION 3.25)

execute_process(COMMAND "${NM}" "${LIB}"
  RESULT_VARIABLE rc OUTPUT_VARIABLE text ERROR_VARIABLE err)
if(NOT rc EQUAL 0)
  message(FATAL_ERROR "${NM} ${LIB} failed (${rc}): ${err}")
endif()
string(REPLACE "\n" ";" lines "${text}")
set(bad)
foreach(line IN LISTS lines)
  if(line MATCHES "^[0-9a-fA-F]* *([BbDdC]) ([^ \t]+)$")
    set(type "${CMAKE_MATCH_1}")
    set(name "${CMAKE_MATCH_2}")
    if(NOT name MATCHES "^(ltmp|l_)")
      list(APPEND bad "${type} ${name}")
    endif()
  endif()
endforeach()
if(bad)
  foreach(entry IN LISTS bad)
    message("${entry}")
  endforeach()
  message(FATAL_ERROR "${LIB} defines writable data: see the symbols above")
endif()
message(STATUS "${LIB}: no writable data symbol")
