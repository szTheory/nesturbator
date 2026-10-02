# palgen writes src/palette_ntsc.c (D-12). A host tool: nothing shipped links
# it and it is not installed. Contraction of a*b+c into one fused operation
# is off, so every compiler writes the same table (01-RESEARCH Pitfall 3).
add_executable(palgen palgen.c)
target_compile_options(palgen PRIVATE
  $<$<C_COMPILER_ID:GNU,Clang,AppleClang>:-ffp-contract=off>
  $<$<C_COMPILER_ID:MSVC>:/fp:precise>)
target_compile_definitions(palgen PRIVATE $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
if(NOT WIN32)
  target_link_libraries(palgen PRIVATE m)
endif()
nesturbator_warnings(palgen)
