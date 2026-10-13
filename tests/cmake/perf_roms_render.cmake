cmake_minimum_required(VERSION 3.25)

# D-09: the perf job measures instructions per frame, which only means something when the
# PPU is drawing. Inputs: RUNNER, SOURCE_DIR, WORK. For each measured ROM, frame 60 must
# contain at least two distinct pixel values, so a ROM that leaves rendering off fails here.
foreach(var RUNNER SOURCE_DIR WORK)
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "${var} is required")
  endif()
endforeach()
file(MAKE_DIRECTORY "${WORK}")

foreach(rom tests/roms/nesteroids.nes tests/roms/runner.nes tests/roms/hm/M4_P256K_C256K.nes)
  get_filename_component(name "${rom}" NAME_WE)
  set(ppm "${WORK}/${name}.ppm")
  file(REMOVE "${ppm}")
  execute_process(COMMAND "${RUNNER}" --rom "${SOURCE_DIR}/${rom}" --frames 60
      --dump-frame "60:${ppm}"
    RESULT_VARIABLE rc OUTPUT_QUIET ERROR_VARIABLE err)
  if(NOT rc EQUAL 0)
    message(FATAL_ERROR "${rom}: runner exited ${rc}: ${err}")
  endif()
  file(READ "${ppm}" hex HEX)
  # The header "P6\n256 240\n255\n" is 15 bytes, 30 hex digits.
  string(SUBSTRING "${hex}" 30 -1 pixels)
  string(LENGTH "${pixels}" len)
  if(NOT len EQUAL 368640)
    message(FATAL_ERROR "${rom}: frame 60 has ${len} hex digits of pixel data, expected 368640")
  endif()
  string(REGEX MATCHALL "([0-9a-f][0-9a-f])([0-9a-f][0-9a-f])([0-9a-f][0-9a-f])" px "${pixels}")
  list(REMOVE_DUPLICATES px)
  list(LENGTH px distinct)
  if(distinct LESS 2)
    message(FATAL_ERROR "${rom} frame 60 is blank: rendering is off; the perf job would measure an idle PPU (D-09)")
  endif()
  message(STATUS "${rom}: frame 60 has ${distinct} distinct pixel values")
endforeach()
