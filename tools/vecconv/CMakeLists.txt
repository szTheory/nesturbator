# vecconv converts one upstream 65x02 vector file to N65V and reads the result
# back through the reader the tests use (D-01). A host tool: nothing shipped
# links it, it is not installed, and an embedding project does not build it.
add_executable(vecconv vecconv.c ${PROJECT_SOURCE_DIR}/tests/vectors/n65v.c)
target_include_directories(vecconv PRIVATE ${PROJECT_SOURCE_DIR}/tests/vectors)
target_compile_definitions(vecconv PRIVATE $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
nesturbator_warnings(vecconv)
