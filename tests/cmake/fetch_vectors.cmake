# Fetches the full 65x02 nes6502/v1 set at its pin and checks every file
# against tests/vectors/pins.txt (D-19, D-20). Runs at test time as the
# fixture cpu.vectors-full.fetch, never at configure time.
#
#   cmake -DDIR=<dir> -DPINS=<pins.txt> -DSOURCE_DIR=<repo> -DGIT=<git>
#         -P fetch_vectors.cmake
#   cmake -DDIR=<dir> -DPINS=<unused> -DSOURCE_DIR=<repo> -DGIT=<git>
#         -DCOMMIT=<40 hex> -DWRITE_PINS=<pins.txt> -P fetch_vectors.cmake
#
# The files land in DIR/src/nes6502/v1. When all 256 already match PINS by
# size and SHA-256 nothing is fetched. Otherwise DIR/src is replaced by a
# sparse, blobless, depth-1 git checkout of the commit, every file is checked,
# and DIR/src/.git is removed. A missing, extra or altered file fails with its
# path; so does no network. Nothing here skips: the test passes or fails.
#
# WRITE_PINS mode fetches COMMIT and writes the pin line and one
# "<sha256>  nes6502/v1/<xx>.json  <size>" line per file, 00 to ff, LF only.
#
# No command that reads blobs (size, log, diff) runs in the blobless tree: each
# would fetch blobs one at a time.

# Script mode starts with no policies set: CMake 3.x then reads IN_LIST with
# its pre-3.3 meaning and fails (nightly run 37135715964, CMake 3.31).
cmake_minimum_required(VERSION 3.25)

foreach(var DIR SOURCE_DIR GIT)
  if(NOT DEFINED ${var} OR "${${var}}" STREQUAL "")
    message(FATAL_ERROR "fetch_vectors: ${var} is not set")
  endif()
endforeach()

set(repo_url "https://github.com/SingleStepTests/65x02")
set(sub "nes6502/v1")

# DIR must not land in the source tree, except under build/.
cmake_path(ABSOLUTE_PATH DIR NORMALIZE OUTPUT_VARIABLE dir_abs)
cmake_path(ABSOLUTE_PATH SOURCE_DIR NORMALIZE OUTPUT_VARIABLE src_abs)
cmake_path(IS_PREFIX src_abs "${dir_abs}" NORMALIZE in_source)
cmake_path(APPEND src_abs "build" OUTPUT_VARIABLE build_abs)
cmake_path(IS_PREFIX build_abs "${dir_abs}" NORMALIZE in_build)
if(in_source AND NOT in_build)
  message(FATAL_ERROR "fetch_vectors: DIR ${dir_abs} is inside the source tree but not under build/")
endif()

# The 256 file names, 00 to ff.
set(names)
foreach(h 0 1 2 3 4 5 6 7 8 9 a b c d e f)
  foreach(l 0 1 2 3 4 5 6 7 8 9 a b c d e f)
    list(APPEND names ${h}${l})
  endforeach()
endforeach()

# The commit and, outside WRITE_PINS mode, the expected hash and size of each
# file, as variables sha_<xx> and size_<xx>.
if(WRITE_PINS)
  set(commit "${COMMIT}")
else()
  if(NOT DEFINED PINS OR NOT EXISTS "${PINS}")
    message(FATAL_ERROR "fetch_vectors: PINS '${PINS}' does not exist")
  endif()
  file(STRINGS "${PINS}" pin_lines)
  list(GET pin_lines 0 head)
  if(NOT head MATCHES "^repo ${repo_url} commit ([0-9a-f]+) path ${sub} licence MIT$")
    message(FATAL_ERROR "fetch_vectors: ${PINS} line 1 is not the pin line: ${head}")
  endif()
  set(commit "${CMAKE_MATCH_1}")
  list(SUBLIST pin_lines 1 -1 entries)
  foreach(line IN LISTS entries)
    if(NOT line MATCHES "^([0-9a-f]+)  ${sub}/([0-9a-f][0-9a-f])\\.json  ([0-9]+)$")
      message(FATAL_ERROR "fetch_vectors: ${PINS}: bad line: ${line}")
    endif()
    set(sha_${CMAKE_MATCH_2} "${CMAKE_MATCH_1}")
    set(size_${CMAKE_MATCH_2} "${CMAKE_MATCH_3}")
  endforeach()
  foreach(xx IN LISTS names)
    if(NOT DEFINED sha_${xx})
      message(FATAL_ERROR "fetch_vectors: ${PINS} has no line for ${sub}/${xx}.json")
    endif()
  endforeach()
endif()
string(LENGTH "${commit}" commit_len)
if(NOT commit MATCHES "^[0-9a-f]+$" OR NOT commit_len EQUAL 40)
  message(FATAL_ERROR "fetch_vectors: commit '${commit}' is not 40 lowercase hex digits")
endif()

set(src "${dir_abs}/src")
set(files "${src}/${sub}")

# Sets OUT to one message per file that is missing, extra, or differs from
# the pins in size or SHA-256.
function(verify_files out)
  set(found)
  file(GLOB present RELATIVE "${files}" "${files}/*.json")
  foreach(name IN LISTS present)
    string(REGEX REPLACE "\\.json$" "" xx "${name}")
    if(NOT xx IN_LIST names)
      list(APPEND found "${sub}/${name}: not in the pins")
    endif()
  endforeach()
  foreach(xx IN LISTS names)
    set(path "${files}/${xx}.json")
    if(NOT EXISTS "${path}")
      list(APPEND found "${sub}/${xx}.json: missing")
      continue()
    endif()
    file(SIZE "${path}" size)
    file(SHA256 "${path}" sha)
    if(NOT size STREQUAL size_${xx} OR NOT sha STREQUAL sha_${xx})
      list(APPEND found
        "${sub}/${xx}.json: expected sha256 ${sha_${xx}} size ${size_${xx}}, actual sha256 ${sha} size ${size}")
    endif()
  endforeach()
  set(${out} "${found}" PARENT_SCOPE)
endfunction()

if(NOT WRITE_PINS)
  verify_files(found)
  if(NOT found)
    message(STATUS "vectors-full: 256 files verify at ${commit}; nothing fetched")
    return()
  endif()
  list(LENGTH found n)
  message(STATUS "vectors-full: ${n} of 256 files do not verify; fetching ${commit}")
endif()

# A stalled transfer fails after 60 s below 1000 B/s; git never prompts.
set(ENV{GIT_HTTP_LOW_SPEED_LIMIT} 1000)
set(ENV{GIT_HTTP_LOW_SPEED_TIME} 60)
set(ENV{GIT_TERMINAL_PROMPT} 0)

file(REMOVE_RECURSE "${src}")
file(MAKE_DIRECTORY "${src}")

function(run_git)
  execute_process(
    COMMAND "${GIT}" ${ARGN}
    WORKING_DIRECTORY "${src}"
    OUTPUT_VARIABLE git_out
    ERROR_VARIABLE git_err
    RESULT_VARIABLE git_rc)
  if(NOT git_rc EQUAL 0)
    message(FATAL_ERROR "fetch_vectors: git ${ARGN} exited ${git_rc}\n${git_out}${git_err}")
  endif()
  set(git_out "${git_out}" PARENT_SCOPE)
endfunction()

# Upstream has no Git LFS; cone mode also checks out the root LICENSE and
# README.md (02-RESEARCH Pitfall 10: the fetch needs a named remote).
run_git(-c init.defaultBranch=main init -q)
run_git(remote add origin "${repo_url}.git")
run_git(sparse-checkout set --cone "${sub}")
run_git(-c gc.auto=0 fetch -q --depth 1 --filter=blob:none origin "${commit}")
run_git(-c advice.detachedHead=false checkout -q FETCH_HEAD)
run_git(rev-parse HEAD)
string(STRIP "${git_out}" head_sha)
if(NOT head_sha STREQUAL commit)
  message(FATAL_ERROR "fetch_vectors: HEAD is ${head_sha}, expected ${commit}")
endif()
file(REMOVE_RECURSE "${src}/.git")

if(WRITE_PINS)
  set(text "repo ${repo_url} commit ${commit} path ${sub} licence MIT\n")
  set(total 0)
  foreach(xx IN LISTS names)
    set(path "${files}/${xx}.json")
    if(NOT EXISTS "${path}")
      message(FATAL_ERROR "fetch_vectors: ${sub}/${xx}.json: missing")
    endif()
    file(SIZE "${path}" size)
    file(SHA256 "${path}" sha)
    string(APPEND text "${sha}  ${sub}/${xx}.json  ${size}\n")
    math(EXPR total "${total} + ${size}")
  endforeach()
  file(GLOB present "${files}/*.json")
  list(LENGTH present n)
  if(NOT n EQUAL 256)
    message(FATAL_ERROR "fetch_vectors: ${n} .json files at ${commit}, expected 256")
  endif()
  # file(WRITE) writes the bytes as given, so the file has LF endings on
  # every platform.
  file(WRITE "${WRITE_PINS}" "${text}")
  message(STATUS "vectors-full: wrote ${WRITE_PINS}: 256 files, ${total} bytes, commit ${commit}")
  return()
endif()

verify_files(found)
if(found)
  foreach(line IN LISTS found)
    message("${line}")
  endforeach()
  message(FATAL_ERROR "fetch_vectors: the files fetched at ${commit} do not match ${PINS}")
endif()
message(STATUS "vectors-full: fetched ${commit}; 256 files verify")
