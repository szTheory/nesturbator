add_executable(nesturbator-run main.c sha256.c)
target_link_libraries(nesturbator-run PRIVATE nesturbator::nesturbator)
target_compile_definitions(nesturbator-run PRIVATE $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
nesturbator_warnings(nesturbator-run)
