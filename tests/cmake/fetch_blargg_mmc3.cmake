# Fetches blargg's mmc3_test_2 and mmc3_irq_tests at their pin and checks
# every ROM against tests/mmc3/pins.txt (09 D-10). Runs at test time as the
# fixture mmc3.oracle.fetch, never at configure time. The repository states no
# licence, so the ROMs are fetched only and never committed.
#
#   cmake -DDIR=<dir> -DPINS=<pins.txt> -DSOURCE_DIR=<repo> -DGIT=<git>
#         -P fetch_blargg_mmc3.cmake
#
# The files land in DIR/nes-test-roms. When all 12 already match PINS by size
# and SHA-256 nothing is fetched. Otherwise DIR/nes-test-roms is replaced by a
# sparse, blobless, depth-1 git checkout of the commit, every file is checked,
# the *.nes files of mmc3_test_2/rom_singles and mmc3_irq_tests must be exactly
# the pinned set, and DIR/nes-test-roms/.git is removed. A missing, extra or
# altered file fails with its path; so does no network. Nothing here skips.
# The script deletes DIR/nes-test-roms only when it holds the marker file
# .nesturbator-mmc3 that the script writes when it creates it.
#
# No command that reads blobs (size, log, diff) runs in the blobless tree: each
# would fetch blobs one at a time. Sizes come from file(SIZE) after checkout.

# Script mode starts with no policies set (see fetch_vectors.cmake).
cmake_minimum_required(VERSION 3.25)

foreach(var DIR PINS SOURCE_DIR GIT)
  if(NOT DEFINED ${var} OR "${${var}}" STREQUAL "")
    message(FATAL_ERROR "fetch_blargg_mmc3: ${var} is not set")
  endif()
endforeach()

set(repo_url "https://github.com/christopherpow/nes-test-roms")
set(cone_dirs mmc3_test_2 mmc3_irq_tests)

# DIR must not land in the source tree, except under build/; the logic and its
# reasons are those of fetch_vectors.cmake (REAL_PATH resolves symlinks and
# case; the deepest existing ancestor is resolved when DIR does not exist yet).
function(check_in_source dir_path src_path)
  cmake_path(IS_PREFIX src_path "${dir_path}" NORMALIZE in_source)
  cmake_path(APPEND src_path "build" OUTPUT_VARIABLE build_path)
  cmake_path(IS_PREFIX build_path "${dir_path}" NORMALIZE in_build)
  if(in_source AND NOT in_build)
    message(FATAL_ERROR "fetch_blargg_mmc3: DIR ${dir_path} is inside the source tree but not under build/")
  endif()
endfunction()
cmake_path(ABSOLUTE_PATH DIR NORMALIZE OUTPUT_VARIABLE dir_abs)
cmake_path(ABSOLUTE_PATH SOURCE_DIR NORMALIZE OUTPUT_VARIABLE src_abs)
check_in_source("${dir_abs}" "${src_abs}")
set(probe "${dir_abs}")
set(rest "")
while(NOT EXISTS "${probe}")
  cmake_path(GET probe FILENAME leaf)
  if(rest STREQUAL "")
    set(rest "${leaf}")
  else()
    set(rest "${leaf}/${rest}")
  endif()
  cmake_path(GET probe PARENT_PATH probe)
endwhile()
file(REAL_PATH "${probe}" dir_real)
if(NOT rest STREQUAL "")
  cmake_path(APPEND dir_real "${rest}")
endif()
file(REAL_PATH "${src_abs}" src_real)
if(CMAKE_HOST_WIN32 OR CMAKE_HOST_APPLE)
  string(TOLOWER "${dir_real}" dir_real)
  string(TOLOWER "${src_real}" src_real)
endif()
check_in_source("${dir_real}" "${src_real}")
file(MAKE_DIRECTORY "${dir_abs}")

# The script deletes only a directory it created: DIR/nes-test-roms, which
# holds a marker file from the run that made it. Any other one fails.
set(src "${dir_abs}/nes-test-roms")
set(marker "${src}/.nesturbator-mmc3")
if(EXISTS "${src}" AND NOT EXISTS "${marker}")
  message(FATAL_ERROR "fetch_blargg_mmc3: ${src} exists and was not created by this script; refusing to delete it")
endif()

if(NOT EXISTS "${PINS}")
  message(FATAL_ERROR "fetch_blargg_mmc3: PINS '${PINS}' does not exist")
endif()
file(STRINGS "${PINS}" pin_lines)
list(GET pin_lines 0 head)
if(NOT head MATCHES "^repo ${repo_url} commit ([0-9a-f]+) path mmc3_test_2,mmc3_irq_tests licence none-stated$")
  message(FATAL_ERROR "fetch_blargg_mmc3: ${PINS} line 1 is not the pin line: ${head}")
endif()
set(commit "${CMAKE_MATCH_1}")
string(LENGTH "${commit}" commit_len)
if(NOT commit_len EQUAL 40)
  message(FATAL_ERROR "fetch_blargg_mmc3: commit '${commit}' is not 40 lowercase hex digits")
endif()
list(SUBLIST pin_lines 1 -1 entries)
set(paths)
set(shas)
set(sizes)
foreach(line IN LISTS entries)
  if(NOT line MATCHES "^([0-9a-f]+)  ((mmc3_test_2/rom_singles|mmc3_irq_tests)/[^ /]+\\.nes)  ([0-9]+)$")
    message(FATAL_ERROR "fetch_blargg_mmc3: ${PINS}: bad line: ${line}")
  endif()
  list(APPEND shas "${CMAKE_MATCH_1}")
  list(APPEND paths "${CMAKE_MATCH_2}")
  list(APPEND sizes "${CMAKE_MATCH_4}")
endforeach()
list(LENGTH paths n_pinned)
if(NOT n_pinned EQUAL 12)
  message(FATAL_ERROR "fetch_blargg_mmc3: ${PINS} pins ${n_pinned} files, expected 12")
endif()

# Sets OUT to one message per file that is missing, extra, or differs from the
# pins in size or SHA-256.
function(verify_files out)
  set(found)
  foreach(d mmc3_test_2/rom_singles mmc3_irq_tests)
    file(GLOB present RELATIVE "${src}" "${src}/${d}/*.nes")
    foreach(p IN LISTS present)
      if(NOT p IN_LIST paths)
        list(APPEND found "${p}: not in the pins")
      endif()
    endforeach()
  endforeach()
  set(i 0)
  foreach(p IN LISTS paths)
    list(GET shas ${i} want_sha)
    list(GET sizes ${i} want_size)
    math(EXPR i "${i} + 1")
    if(NOT EXISTS "${src}/${p}")
      list(APPEND found "${p}: missing")
      continue()
    endif()
    file(SIZE "${src}/${p}" size)
    file(SHA256 "${src}/${p}" sha)
    if(NOT size STREQUAL want_size OR NOT sha STREQUAL want_sha)
      list(APPEND found
        "${p}: expected sha256 ${want_sha} size ${want_size}, actual sha256 ${sha} size ${size}")
    endif()
  endforeach()
  set(${out} "${found}" PARENT_SCOPE)
endfunction()

verify_files(found)
if(NOT found)
  message(STATUS "mmc3-oracle: 12 files verify at ${commit}; nothing fetched")
  return()
endif()
list(LENGTH found n)
message(STATUS "mmc3-oracle: ${n} problems in the files; fetching ${commit}")

# A stalled transfer fails after 60 s below 1000 B/s; git never prompts.
set(ENV{GIT_HTTP_LOW_SPEED_LIMIT} 1000)
set(ENV{GIT_HTTP_LOW_SPEED_TIME} 60)
set(ENV{GIT_TERMINAL_PROMPT} 0)

file(REMOVE_RECURSE "${src}")
file(MAKE_DIRECTORY "${src}")
file(TOUCH "${marker}")

function(run_git)
  execute_process(
    COMMAND "${GIT}" ${ARGN}
    WORKING_DIRECTORY "${src}"
    OUTPUT_VARIABLE git_out
    ERROR_VARIABLE git_err
    RESULT_VARIABLE git_rc)
  if(NOT git_rc EQUAL 0)
    message(FATAL_ERROR "fetch_blargg_mmc3: git ${ARGN} exited ${git_rc}\n${git_out}${git_err}")
  endif()
  set(git_out "${git_out}" PARENT_SCOPE)
endfunction()

run_git(-c init.defaultBranch=main init -q)
run_git(remote add origin "${repo_url}.git")
run_git(sparse-checkout set --cone ${cone_dirs})
run_git(-c gc.auto=0 fetch -q --depth 1 --filter=blob:none origin "${commit}")
run_git(-c advice.detachedHead=false checkout -q FETCH_HEAD)
run_git(rev-parse HEAD)
string(STRIP "${git_out}" head_sha)
if(NOT head_sha STREQUAL commit)
  message(FATAL_ERROR "fetch_blargg_mmc3: HEAD is ${head_sha}, expected ${commit}")
endif()
file(REMOVE_RECURSE "${src}/.git")

verify_files(found)
if(found)
  foreach(line IN LISTS found)
    message("${line}")
  endforeach()
  message(FATAL_ERROR "fetch_blargg_mmc3: the files fetched at ${commit} do not match ${PINS}")
endif()
message(STATUS "mmc3-oracle: fetched ${commit}; 12 files verify")
