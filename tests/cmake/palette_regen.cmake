# Runs palgen into OUT and compares OUT with the checked-in table REF.
#   cmake -DPALGEN=<palgen> -DOUT=<file> -DREF=<file> -P palette_regen.cmake
execute_process(COMMAND "${PALGEN}" "${OUT}" RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
  message(FATAL_ERROR "palgen exited with ${rc}")
endif()
execute_process(COMMAND "${CMAKE_COMMAND}" -E compare_files --ignore-eol "${OUT}" "${REF}"
  RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
  message(FATAL_ERROR "${OUT} differs from ${REF}; run palgen and review the change")
endif()
