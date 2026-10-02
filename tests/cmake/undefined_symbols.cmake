# The core links only the C memory functions (ENGINEERING section 1). Fails
# if the static library LIB references a symbol that none of its own members
# defines and that is not on the allowlist below.
#
#   cmake -DNM=<nm> -DLIB=<static library> -DAPPLE=<bool> -P undefined_symbols.cmake
#
# nm -u prints "name" (Apple) or "U name" (GNU) per line, plus a "member:"
# header per object; the name is the last token. Mach-O prefixes every C
# symbol with one underscore, which is stripped when APPLE is true.

# The C memory functions; malloc and free for the default allocator (D-08);
# the fortify and stack-protector helpers that hardened toolchains emit
# (01-RESEARCH Pitfall 4, measured with Apple clang 21).
set(allowed
  memcpy memmove memset memcmp
  malloc free
  __memcpy_chk __memmove_chk __memset_chk
  __stack_chk_fail __stack_chk_guard)
# Darwin: Apple clang lowers memset(p, 0, n) to bzero (measured, Apple
# clang 21, -O2).
if(APPLE)
  list(APPEND allowed bzero)
endif()

function(nm_names args out)
  execute_process(COMMAND "${NM}" ${args} "${LIB}"
    RESULT_VARIABLE rc OUTPUT_VARIABLE text ERROR_VARIABLE err)
  if(NOT rc EQUAL 0)
    message(FATAL_ERROR "${NM} ${args} ${LIB} failed (${rc}): ${err}")
  endif()
  string(REPLACE "\n" ";" lines "${text}")
  set(names)
  foreach(line IN LISTS lines)
    string(STRIP "${line}" line)
    if(line STREQUAL "" OR line MATCHES ":$")
      continue()
    endif()
    string(REGEX MATCH "[^ \t]+$" name "${line}")
    if(APPLE)
      string(REGEX REPLACE "^_" "" name "${name}")
    endif()
    list(APPEND names "${name}")
  endforeach()
  set(${out} "${names}" PARENT_SCOPE)
endfunction()

nm_names("-u" undefined)
# Defined symbols: a line with an address, a type letter and a name.
execute_process(COMMAND "${NM}" "${LIB}"
  RESULT_VARIABLE rc OUTPUT_VARIABLE text ERROR_VARIABLE err)
if(NOT rc EQUAL 0)
  message(FATAL_ERROR "${NM} ${LIB} failed (${rc}): ${err}")
endif()
string(REPLACE "\n" ";" lines "${text}")
set(defined)
foreach(line IN LISTS lines)
  if(line MATCHES "^[0-9a-fA-F]+ [A-Za-z] ([^ \t]+)$")
    set(name "${CMAKE_MATCH_1}")
    if(APPLE)
      string(REGEX REPLACE "^_" "" name "${name}")
    endif()
    list(APPEND defined "${name}")
  endif()
endforeach()

set(bad)
foreach(name IN LISTS undefined)
  if(NOT name IN_LIST allowed AND NOT name IN_LIST defined)
    list(APPEND bad "${name}")
  endif()
endforeach()
list(REMOVE_DUPLICATES bad)
if(bad)
  message(FATAL_ERROR "${LIB} needs symbols outside the allowlist: ${bad}")
endif()
list(REMOVE_DUPLICATES undefined)
message(STATUS "${LIB}: undefined symbols ${undefined}; all allowed or defined in the library")
