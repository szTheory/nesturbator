# FRAME-02: a separate project that finds the installed package. The test
# install.consumer builds it against <build>/stage and runs it.
cmake_minimum_required(VERSION 3.25)
project(consumer C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

find_package(nesturbator CONFIG REQUIRED)

add_executable(consumer main.c)
target_link_libraries(consumer PRIVATE nesturbator::nesturbator)
