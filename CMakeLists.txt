cmake_minimum_required(VERSION 3.10)
project(MINIMALIST_BLOCKCHAIN_SYSTEM)

include_directories(src)

set(CMAKE_C_STANDARD 11)
set(SOURCE_FILES ./src)

add_executable(cli src/cli.c src/cli.h)
set_target_properties(cli PROPERTIES LINKER_LANGUAGE C)