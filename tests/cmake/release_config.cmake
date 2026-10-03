# release-please's `release-as` pins the next release to one version. Its
# documentation says to remove the key once that release is merged, or every
# later release PR uses the same version again. This check fails while
# packages."." in release-please-config.json has the key.
#
#   cmake -DSOURCE_DIR=<repository root> -P release_config.cmake

if(NOT DEFINED SOURCE_DIR)
  message(FATAL_ERROR "release_config: pass -DSOURCE_DIR=...")
endif()

set(path "${SOURCE_DIR}/release-please-config.json")
file(READ "${path}" config)
string(JSON root ERROR_VARIABLE err GET "${config}" packages .)
if(err)
  message(FATAL_ERROR "release-please-config.json: no packages.\".\": ${err}")
endif()
string(JSON count ERROR_VARIABLE err LENGTH "${root}")
if(err)
  message(FATAL_ERROR "release-please-config.json: packages.\".\" is not an object: ${err}")
endif()
if(count GREATER 0)
  math(EXPR last "${count} - 1")
  foreach(i RANGE ${last})
    string(JSON key MEMBER "${root}" ${i})
    if(key STREQUAL "release-as")
      message(FATAL_ERROR "release-please-config.json: packages.\".\" has release-as; "
        "remove it after the release it pinned")
    endif()
  endforeach()
endif()
