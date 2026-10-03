# Checks every line of tests/roms/manifest.txt: five tab-separated fields
# (path, source, pin, licence, sha256), a pin that is a 40-hex-digit commit or
# a release tag, a licence, a sha256 of 64 lowercase hex digits, and a file at
# path whose SHA-256 equals that field. Every failure names the path. Without
# this check a regenerated file with a stale line would go unnoticed.
#
#   cmake -DSOURCE_DIR=<repo> -P manifest_sha256.cmake
#   cmake -DSELFTEST=ON -DWORK=<dir> -P manifest_sha256.cmake
#
# SELFTEST writes a scratch manifest listing a scratch file with a wrong hash
# and passes only if the check reports that file's hash mismatch.

# Appends one message per problem in MANIFEST (paths relative to ROOT) to OUT.
function(manifest_check manifest root out)
  set(found)
  file(STRINGS "${manifest}" lines)
  foreach(line IN LISTS lines)
    if(line STREQUAL "" OR line MATCHES "^#")
      continue()
    endif()
    # A field holding ';' would split the list; no manifest field may.
    if(line MATCHES ";")
      list(APPEND found "manifest line has a ';': ${line}")
      continue()
    endif()
    string(REPLACE "\t" ";" fields "${line}")
    list(LENGTH fields n)
    list(GET fields 0 path)
    if(NOT n EQUAL 5)
      list(APPEND found "${path}: ${n} tab-separated fields, expected 5")
      continue()
    endif()
    list(GET fields 2 pin)
    list(GET fields 3 licence)
    list(GET fields 4 sha)
    # A commit is 40 lowercase hex digits (CMake regex has no {n}, so the
    # length is checked apart); a release tag is like v1.22.2 or 1.2.
    string(LENGTH "${pin}" pin_len)
    if(NOT (pin MATCHES "^[0-9a-f]+$" AND pin_len EQUAL 40)
        AND NOT pin MATCHES "^v?[0-9]+(\\.[0-9]+)+$")
      list(APPEND found "${path}: pin '${pin}' is neither a 40-hex-digit commit nor a release tag")
      continue()
    endif()
    if(licence STREQUAL "")
      list(APPEND found "${path}: no licence")
      continue()
    endif()
    string(LENGTH "${sha}" sha_len)
    if(NOT sha MATCHES "^[0-9a-f]+$" OR NOT sha_len EQUAL 64)
      list(APPEND found "${path}: sha256 field is not 64 lowercase hex digits")
      continue()
    endif()
    if(NOT EXISTS "${root}/${path}" OR IS_DIRECTORY "${root}/${path}")
      list(APPEND found "${path}: file missing")
      continue()
    endif()
    file(SHA256 "${root}/${path}" actual)
    if(NOT actual STREQUAL sha)
      list(APPEND found "${path}: sha256 ${actual}, manifest says ${sha}")
    endif()
  endforeach()
  set(${out} "${found}" PARENT_SCOPE)
endfunction()

if(SELFTEST)
  set(dir "${WORK}/manifest_sha256_selftest")
  file(REMOVE_RECURSE "${dir}")
  file(WRITE "${dir}/data.bin" "not the listed bytes\n")
  set(zero "0000000000000000000000000000000000000000000000000000000000000000")
  file(WRITE "${dir}/manifest.txt"
    "# scratch\ndata.bin\thttps://example.com/x\tv1.0\tMIT\t${zero}\n")
  manifest_check("${dir}/manifest.txt" "${dir}" found)
  file(SHA256 "${dir}/data.bin" actual)
  set(expected "data.bin: sha256 ${actual}, manifest says ${zero}")
  if(NOT found STREQUAL expected)
    message(FATAL_ERROR "self-test: expected '${expected}', got: '${found}'")
  endif()
  message(STATUS "self-test reported: ${found}")
  return()
endif()

if(NOT SOURCE_DIR)
  message(FATAL_ERROR "SOURCE_DIR is required")
endif()
manifest_check("${SOURCE_DIR}/tests/roms/manifest.txt" "${SOURCE_DIR}" found)
if(found)
  foreach(line IN LISTS found)
    message("${line}")
  endforeach()
  message(FATAL_ERROR "tests/roms/manifest.txt: see the lines above")
endif()
