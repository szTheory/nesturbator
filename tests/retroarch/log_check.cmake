# Fails on any RetroArch log line that signals an error, unless the line is in the measured
# allowlist for the savestate level the core claims (D-28 as amended at planning).
#
# Why this is the "no core error" check (success criterion 3): the libretro adapter requests no
# log interface, so a RETRO_LOG_ERROR line from the core cannot appear in RetroArch's log.
# RetroArch writes its own errors as lines that start with `[ERROR]`, so any such line, and any
# rewind or run-ahead warning, is a failure unless tests/retroarch/log_allowlist.txt explains it.
#
# A line is flagged when it matches `^\[ERROR\]` or `^\[(WARN|ERROR)\] \[(Rewind|Run-Ahead)\]`.
# A flagged line passes only if some allowlist entry whose level is `any` or equals LEVEL matches
# it. A line containing `Failed to save state` or `Failed to load state` always fails: no
# allowlist entry can excuse it. With REQUIRE_NETCMD on, a log without a `[NetCMD]` line fails,
# because the state lanes enable the command interface and its absence means the config did not
# take effect.
#
# Direct run:
#   cmake -DLOG=<log> -DLEVEL=<serialized|deterministic> [-DALLOWLIST=<file>]
#         [-DREQUIRE_NETCMD=ON] -P log_check.cmake
# run_retroarch.cmake include()s this file and calls nesturbator_ra_log_check itself.
cmake_minimum_required(VERSION 3.25)

# Splits the text held in the variable named TEXTVAR into its lines without making a list, so a bracket or semicolon in a log line
# cannot merge it with the next one. The lines are stored in the variables
# ${prefix}_0 ... ${prefix}_<n-1> of the caller and the count in ${prefix}_count.
macro(nesturbator_ra_split_lines prefix textvar)
  set(_rest "${${textvar}}")
  set(_n 0)
  while(NOT "${_rest}" STREQUAL "")
    string(FIND "${_rest}" "\n" _nl)
    if(_nl LESS 0)
      set(_line "${_rest}")
      set(_rest "")
    else()
      string(SUBSTRING "${_rest}" 0 ${_nl} _line)
      math(EXPR _next "${_nl} + 1")
      string(SUBSTRING "${_rest}" ${_next} -1 _rest)
    endif()
    string(REGEX REPLACE "\r$" "" _line "${_line}")
    set(${prefix}_${_n} "${_line}")
    math(EXPR _n "${_n} + 1")
  endwhile()
  set(${prefix}_count ${_n})
endmacro()

function(nesturbator_ra_log_check log allowlist level require_netcmd)
  if(NOT level STREQUAL "serialized" AND NOT level STREQUAL "deterministic")
    message(FATAL_ERROR "log_check: LEVEL must be serialized or deterministic, got '${level}'")
  endif()
  if(NOT EXISTS "${log}")
    message(FATAL_ERROR "log_check: log is missing: ${log}")
  endif()
  if(NOT EXISTS "${allowlist}")
    message(FATAL_ERROR "log_check: allowlist is missing: ${allowlist}")
  endif()

  # Allowlist: `<level><TAB><regex>`, each directly below a comment line.
  file(READ "${allowlist}" allow_text)
  nesturbator_ra_split_lines(allow allow_text)
  set(entry_count 0)
  set(previous_comment FALSE)
  set(index 0)
  while(index LESS allow_count)
    set(line "${allow_${index}}")
    math(EXPR index "${index} + 1")
    if(line MATCHES "^#")
      set(previous_comment TRUE)
      continue()
    endif()
    if(line MATCHES "^[ \t]*$")
      set(previous_comment FALSE)
      continue()
    endif()
    string(FIND "${line}" "\t" tab)
    if(tab LESS 1)
      message(FATAL_ERROR "log_check: allowlist entry has no tab between level and regex: ${line}")
    endif()
    string(SUBSTRING "${line}" 0 ${tab} entry_level)
    math(EXPR after_tab "${tab} + 1")
    string(SUBSTRING "${line}" ${after_tab} -1 entry_regex)
    if(NOT entry_level MATCHES "^(any|serialized|deterministic)$")
      message(FATAL_ERROR "log_check: allowlist level '${entry_level}' is not any, serialized or deterministic: ${line}")
    endif()
    if(NOT previous_comment)
      message(FATAL_ERROR "log_check: allowlist entry has no comment line directly above it: ${line}")
    endif()
    set(entry_level_${entry_count} "${entry_level}")
    set(entry_regex_${entry_count} "${entry_regex}")
    set(entry_used_${entry_count} FALSE)
    math(EXPR entry_count "${entry_count} + 1")
    set(previous_comment FALSE)
  endwhile()

  file(READ "${log}" log_text)
  nesturbator_ra_split_lines(rows log_text)
  set(violations "")
  set(saw_netcmd FALSE)
  set(index 0)
  while(index LESS rows_count)
    set(line "${rows_${index}}")
    math(EXPR index "${index} + 1")
    if(line MATCHES "\\[NetCMD\\]")
      set(saw_netcmd TRUE)
    endif()
    string(FIND "${line}" "Failed to save state" failed_save)
    string(FIND "${line}" "Failed to load state" failed_load)
    if(NOT failed_save LESS 0 OR NOT failed_load LESS 0)
      string(APPEND violations "\n  failed state operation (never allowlisted): ${line}")
      continue()
    endif()
    if(NOT line MATCHES "^\\[ERROR\\]" AND
       NOT line MATCHES "^\\[(WARN|ERROR)\\] \\[(Rewind|Run-Ahead)\\]")
      continue()
    endif()
    set(allowed FALSE)
    set(e 0)
    while(e LESS entry_count)
      if(entry_level_${e} STREQUAL "any" OR entry_level_${e} STREQUAL "${level}")
        if(line MATCHES "${entry_regex_${e}}")
          set(allowed TRUE)
          set(entry_used_${e} TRUE)
          break()
        endif()
      endif()
      math(EXPR e "${e} + 1")
    endwhile()
    if(NOT allowed)
      string(APPEND violations "\n  un-allowlisted line: ${line}")
    endif()
  endwhile()

  if(require_netcmd AND NOT saw_netcmd)
    string(APPEND violations "\n  missing [NetCMD] line: the command interface did not come up")
  endif()
  if(NOT "${violations}" STREQUAL "")
    message(FATAL_ERROR "log_check failed for ${log} at level ${level}:${violations}")
  endif()

  set(used "")
  set(e 0)
  while(e LESS entry_count)
    if(entry_used_${e})
      string(APPEND used " [${entry_level_${e}}] ${entry_regex_${e}};")
    endif()
    math(EXPR e "${e} + 1")
  endwhile()
  message(STATUS "log_check passed for ${log} at level ${level}; allowlist entries used:${used}")
endfunction()

# Run as a script (-P log_check.cmake). When run_retroarch.cmake include()s this file,
# CMAKE_SCRIPT_MODE_FILE names run_retroarch.cmake instead, so nothing below runs.
if(CMAKE_SCRIPT_MODE_FILE STREQUAL CMAKE_CURRENT_LIST_FILE)
  if(NOT DEFINED LOG OR "${LOG}" STREQUAL "")
    message(FATAL_ERROR "log_check.cmake needs -DLOG=<log file>")
  endif()
  if(NOT DEFINED ALLOWLIST OR "${ALLOWLIST}" STREQUAL "")
    set(ALLOWLIST "${CMAKE_CURRENT_LIST_DIR}/log_allowlist.txt")
  endif()
  if(NOT DEFINED LEVEL)
    message(FATAL_ERROR "log_check.cmake needs -DLEVEL=serialized or deterministic")
  endif()
  if(NOT DEFINED REQUIRE_NETCMD)
    set(REQUIRE_NETCMD OFF)
  endif()
  nesturbator_ra_log_check("${LOG}" "${ALLOWLIST}" "${LEVEL}" "${REQUIRE_NETCMD}")
endif()
