#region Macro Settings.
cmake_minimum_required(VERSION 3.10)
project(MinimalistBlockChainSystem)

include_directories(src)

set(CMAKE_C_STANDARD 11)
set(SOURCE_FILES ./src)
#endregion

#region Add global libraries.
# Standard packages
include (CMakeForceCompiler)
include (CheckCSourceCompiles)
include (CheckCCompilerFlag)
include (CheckIncludeFile)
include (CheckIncludeFiles)
include (CheckFunctionExists)
include (CheckSymbolExists)
include (CheckTypeSize)
include (CheckLibraryExists)
include (FindPkgConfig)

# Glib
pkg_search_module(GLIB REQUIRED glib-2.0)
include_directories (${GTK2_INCLUDE_DIRS})
link_directories (${GTK2_LIBRARY_DIRS})
#endregion

#region Add local libraries.
file(GLOB MODEL_SOURCES src/model/**/*.c)
file(GLOB MODEL_HEADERS src/model/**/*.h)
add_library(BlockChainModels ${MODEL_SOURCES} ${MODEL_HEADERS})
#endregion

#region Define executables
add_executable(cli src/cli.c src/cli.h)
set_target_properties(cli PROPERTIES LINKER_LANGUAGE C)
target_include_directories(cli PRIVATE ${GLIB_INCLUDE_DIRS})
target_link_libraries (cli ${GLIB_LDFLAGS})
#endregion