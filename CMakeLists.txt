#region Macro Settings.
cmake_minimum_required(VERSION 3.10)
project(MinimalistBlockChainSystem)

include_directories(src)

set(CMAKE_C_STANDARD 11)
set(SOURCE_FILES ./src)
#endregion

#region Add global libraries.
# Standard packages
include(CMakeForceCompiler)
include(CheckCSourceCompiles)
include(CheckCCompilerFlag)
include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckSymbolExists)
include(CheckTypeSize)
include(CheckLibraryExists)
include(FindPkgConfig)

# Glib
pkg_search_module(GLIB REQUIRED glib-2.0)
include_directories(${GTK2_INCLUDE_DIRS})
link_directories(${GTK2_LIBRARY_DIRS})

# secp256k1
include_directories(/usr/local/include)
include_directories(/usr/local/lib)
find_library(secp256k1_location NAMES libsecp256k1.a)
add_library(secp256k1 STATIC IMPORTED)
set_target_properties(secp256k1 PROPERTIES IMPORTED_LOCATION ${secp256k1_location})
#endregion

#region Add local libraries.
file(GLOB MODEL_SOURCES src/model/**/*.c)
file(GLOB MODEL_HEADERS src/model/**/*.h)
add_library(BlockChainModels ${MODEL_SOURCES} ${MODEL_HEADERS})
target_include_directories(BlockChainModels PRIVATE ${GLIB_INCLUDE_DIRS})
target_link_libraries(BlockChainModels ${GLIB_LDFLAGS} secp256k1)

file(GLOB UTILS_SOURCES src/utils/*.c)
file(GLOB UTILS_HEADERS src/utils/*.h)
add_library(BlockChainUtils ${UTILS_SOURCES} ${UTILS_HEADERS})
target_include_directories(BlockChainUtils PRIVATE ${GLIB_INCLUDE_DIRS})
target_link_libraries(BlockChainUtils ${GLIB_LDFLAGS} secp256k1)
#endregion

#region Define executables
add_executable(cli src/cli.c src/cli.h)
set_target_properties(cli PROPERTIES LINKER_LANGUAGE C)
target_include_directories(cli PRIVATE ${GLIB_INCLUDE_DIRS})
target_link_libraries(cli ${GLIB_LDFLAGS} BlockChainModels BlockChainUtils secp256k1)
#endregion
