# A vendored file must be byte-identical to its pinned upstream copy
# (PROVENANCE.md, Vendored files).
#
#   cmake -DFILE=<path> -DSHA256=<expected hex> -P vendored_sha256.cmake
file(SHA256 "${FILE}" actual)
if(NOT actual STREQUAL SHA256)
  message(FATAL_ERROR "${FILE}: SHA-256 is ${actual}, the pin is ${SHA256}")
endif()
message(STATUS "${FILE}: ${actual}")
