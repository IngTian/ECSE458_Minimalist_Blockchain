cmake_minimum_required(VERSION 3.16)
project(MinimalistBlockChainSystem
    VERSION 1.0.0
    DESCRIPTION "A minimalist blockchain implementation in C"
    LANGUAGES C)

# Project settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Make sure we can find our modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Build configuration
include(cmake/BuildConfig.cmake)

# Global includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Find required packages
include(FindPkgConfig)
find_package(PkgConfig REQUIRED)

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
