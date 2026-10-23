execute_process(
  COMMAND git -C ${PROJECT_SOURCE_DIR} rev-parse --short=12 HEAD
  OUTPUT_VARIABLE NESTURBATOR_SOURCE_REVISION
  OUTPUT_STRIP_TRAILING_WHITESPACE
  RESULT_VARIABLE NESTURBATOR_GIT_REVISION_RESULT)
execute_process(
  COMMAND git -C ${PROJECT_SOURCE_DIR} diff --quiet
  RESULT_VARIABLE NESTURBATOR_GIT_DIRTY_RESULT)
if(NOT NESTURBATOR_GIT_REVISION_RESULT EQUAL 0 OR NOT NESTURBATOR_SOURCE_REVISION MATCHES "^[0-9a-f]+$")
  set(NESTURBATOR_SOURCE_REVISION unknown)
  set(NESTURBATOR_SOURCE_DIRTY unknown)
elseif(NESTURBATOR_GIT_DIRTY_RESULT EQUAL 0)
  set(NESTURBATOR_SOURCE_DIRTY 0)
else()
  set(NESTURBATOR_SOURCE_DIRTY 1)
endif()

add_executable(nesturbator-run main.c accuracycoin.c movie.c sha256.c audio_hash.c ppm.c save.c state_file.c stop.c clock.c bench.c)
target_link_libraries(nesturbator-run PRIVATE nesturbator::nesturbator nesturbator_host)
target_include_directories(nesturbator-run PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_compile_definitions(nesturbator-run PRIVATE
  NESTURBATOR_SOURCE_REVISION="${NESTURBATOR_SOURCE_REVISION}"
  NESTURBATOR_SOURCE_DIRTY="${NESTURBATOR_SOURCE_DIRTY}"
  $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
nesturbator_warnings(nesturbator-run)
install(TARGETS nesturbator-run RUNTIME DESTINATION bin COMPONENT runner)
