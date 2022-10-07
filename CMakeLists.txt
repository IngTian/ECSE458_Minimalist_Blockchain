# Macro Settings.
cmake_minimum_required(VERSION 3.10)
project(MinimalistBlockChainSystem)

include_directories(src)

set(CMAKE_C_STANDARD 11)
set(SOURCE_FILES ./src)

# Add Libraries.
file(GLOB MODEL_SOURCES src/model/**/*.c)
file(GLOB MODEL_HEADERS src/model/**/*.h)
add_library(BlockChainModels ${MODEL_SOURCES} ${MODEL_HEADERS})

# Define executables.
add_executable(cli src/cli.c src/cli.h)
set_target_properties(cli PROPERTIES LINKER_LANGUAGE C)