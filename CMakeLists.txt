#region Macro Settings.
cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "gcc")
project(MinimalistBlockChainSystem)

include_directories(src)
set(CMAKE_BUILD_TYPE Debug)
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

# Test coverage
if (APPLE)
    set(GCC_COVERAGE_COMPILE_FLAGS "-fprofile-instr-generate -fcoverage-mapping -mllvm -runtime-counter-relocation")
    set(GCC_COVERAGE_LINK_FLAGS "--coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")
endif (APPLE)

# MySQL
pkg_check_modules(LIBMYSQLCLIENT REQUIRED mysqlclient)

foreach (FLAG ${LIBMYSQLCLIENT_CFLAGS_OTHER})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAG}")
endforeach ()

link_directories(${LIBMYSQLCLIENT_LIBRARY_DIRS})

# Glib
pkg_search_module(GLIB REQUIRED glib-2.0)
include_directories(${GTK2_INCLUDE_DIRS})
link_directories(${GTK2_LIBRARY_DIRS})

# Check
find_library(check_library_location check)
add_library(check_library SHARED IMPORTED)
set_target_properties(check_library PROPERTIES IMPORTED_LOCATION ${check_library_location})

# secp256k1
include_directories(/usr/local/include)
include_directories(/usr/local/lib)
find_library(secp256k1_location NAMES libsecp256k1.a)
add_library(secp256k1 STATIC IMPORTED)
set_target_properties(secp256k1 PROPERTIES IMPORTED_LOCATION ${secp256k1_location})
#endregion

#region Add local libraries.
file(GLOB SOCKET_UTIL_SOURCES src/socket/*.c)
file(GLOB SOCKET_UTIL_HEADERS src/socket/*.h)
add_library(BlockChainSocketUtils ${SOCKET_UTIL_SOURCES} ${SOCKET_UTIL_HEADERS})
target_include_directories(BlockChainSocketUtils PRIVATE ${GLIB_INCLUDE_DIRS} ${LIBMYSQLCLIENT_INCLUDE_DIRS})
target_link_libraries(BlockChainSocketUtils BlockChainUtils BlockChainModels ${GLIB_LDFLAGS} secp256k1 ${LIBMYSQLCLIENT_LIBRARIES})

file(GLOB MODEL_SOURCES src/model/**/*.c)
file(GLOB MODEL_HEADERS src/model/**/*.h)
add_library(BlockChainModels ${MODEL_SOURCES} ${MODEL_HEADERS})
target_include_directories(BlockChainModels PRIVATE ${GLIB_INCLUDE_DIRS} ${LIBMYSQLCLIENT_INCLUDE_DIRS})
target_link_libraries(BlockChainModels BlockChainSocketUtils ${GLIB_LDFLAGS} secp256k1 ${LIBMYSQLCLIENT_LIBRARIES})

file(GLOB UTILS_SOURCES src/utils/*.c)
file(GLOB UTILS_HEADERS src/utils/*.h)
add_library(BlockChainUtils ${UTILS_SOURCES} ${UTILS_HEADERS})
target_include_directories(BlockChainUtils PRIVATE ${GLIB_INCLUDE_DIRS} ${LIBMYSQLCLIENT_INCLUDE_DIRS})
target_link_libraries(BlockChainUtils BlockChainSocketUtils ${GLIB_LDFLAGS} BlockChainModels secp256k1 ${LIBMYSQLCLIENT_LIBRARIES})

file(GLOB CLI_SOURCES src/cli/*.c)
file(GLOB CLI_HEADERS src/cli/*.h)
add_library(CliModule ${CLI_SOURCES} ${CLI_HEADERS})
target_include_directories(CliModule PRIVATE ${GLIB_INCLUDE_DIRS} ${LIBMYSQLCLIENT_INCLUDE_DIRS})
target_link_libraries(CliModule BlockChainUtils BlockChainSocketUtils BlockChainModels ${GLIB_LDFLAGS} secp256k1 ${LIBMYSQLCLIENT_LIBRARIES})
#endregion

#region Define executables
if (APPLE)
    add_executable(shell src/cli/shell.c)
    set_target_properties(shell PROPERTIES LINKER_LANGUAGE C)
    target_include_directories(shell PRIVATE ${GLIB_INCLUDE_DIRS} ${LIBMYSQLCLIENT_INCLUDE_DIRS})
    target_link_libraries(shell ${GLIB_LDFLAGS} BlockChainModels BlockChainUtils BlockChainSocketUtils CliModule secp256k1 ${LIBMYSQLCLIENT_LIBRARIES})

    add_executable(test_transaction test/model/transaction_test.c)
    set_target_properties(test_transaction PROPERTIES LINKER_LANGUAGE C)
    target_include_directories(test_transaction PRIVATE ${GLIB_INCLUDE_DIRS} ${LIBMYSQLCLIENT_INCLUDE_DIRS} /opt/homebrew/Cellar/check/0.15.2/include)
    target_link_libraries(test_transaction ${GLIB_LDFLAGS} BlockChainModels BlockChainUtils CliModule secp256k1 check_library ${LIBMYSQLCLIENT_LIBRARIES})

    add_executable(test_block test/model/block_test.c)
    set_target_properties(test_block PROPERTIES LINKER_LANGUAGE C)
    target_include_directories(test_block PRIVATE ${GLIB_INCLUDE_DIRS} ${LIBMYSQLCLIENT_INCLUDE_DIRS} /opt/homebrew/Cellar/check/0.15.2/include)
    target_link_libraries(test_block ${GLIB_LDFLAGS} BlockChainModels BlockChainUtils CliModule secp256k1 check_library ${LIBMYSQLCLIENT_LIBRARIES})
endif (APPLE)

add_executable(main src/main.c)
set_target_properties(main PROPERTIES LINKER_LANGUAGE C)
target_include_directories(main PRIVATE ${GLIB_INCLUDE_DIRS} ${LIBMYSQLCLIENT_INCLUDE_DIRS})
target_link_libraries(main ${GLIB_LDFLAGS} BlockChainModels BlockChainSocketUtils BlockChainUtils CliModule secp256k1 ${LIBMYSQLCLIENT_LIBRARIES})

add_executable(server src/socket/server.c)
set_target_properties(server PROPERTIES LINKER_LANGUAGE C)
target_include_directories(server PRIVATE ${GLIB_INCLUDE_DIRS} ${LIBMYSQLCLIENT_INCLUDE_DIRS})
target_link_libraries(server ${GLIB_LDFLAGS} BlockChainModels BlockChainSocketUtils BlockChainUtils CliModule secp256k1 ${LIBMYSQLCLIENT_LIBRARIES})

add_executable(client src/socket/client.c)
set_target_properties(client PROPERTIES LINKER_LANGUAGE C)
target_include_directories(client PRIVATE ${GLIB_INCLUDE_DIRS} ${LIBMYSQLCLIENT_INCLUDE_DIRS})
target_link_libraries(client ${GLIB_LDFLAGS} BlockChainModels BlockChainSocketUtils BlockChainUtils CliModule secp256k1 ${LIBMYSQLCLIENT_LIBRARIES})
#endregion
