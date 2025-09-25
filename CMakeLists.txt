cmake_minimum_required(VERSION 3.16)

# Set up vcpkg integration before project() call
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# Force Clang toolchain
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

project(
    MinimalistBlockChainSystem
    VERSION 0.0.1
    DESCRIPTION "A minimalist blockchain implementation in C and C++"
    LANGUAGES C CXX
)

# Project settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Make sure we can find our modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Build configuration
include(cmake/BuildConfig.cmake)

# Global includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# External dependencies
include(cmake/Dependencies.cmake)

# Add subdirectories for each module
add_subdirectory(src/utils)
add_subdirectory(src/model)
add_subdirectory(src/cli)

# Main executables
add_subdirectory(apps)

# Tests
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()
