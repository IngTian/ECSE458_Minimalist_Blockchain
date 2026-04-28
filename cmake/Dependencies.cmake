# External Dependencies for MinimalistBlockChainSystem
# Using vcpkg for dependency management

# Test coverage configuration
if(APPLE AND ENABLE_COVERAGE)
    # Clang coverage flags
    set(COVERAGE_COMPILE_FLAGS "-fprofile-instr-generate -fcoverage-mapping")
    set(COVERAGE_LINK_FLAGS "-fprofile-instr-generate")
    message(STATUS "Enabling Clang code coverage")
    
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COVERAGE_COMPILE_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COVERAGE_COMPILE_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${COVERAGE_LINK_FLAGS}")
endif()

# Find packages through vcpkg
message(STATUS "Finding dependencies through vcpkg...")

# GLib - Portable, general-purpose utility library
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED IMPORTED_TARGET glib-2.0)
if(GLIB_FOUND)
    message(STATUS "GLib found via vcpkg: ${GLIB_VERSION}")
    # Use the PkgConfig target directly
    add_library(GLib::GLib ALIAS PkgConfig::GLIB)
else()
    message(FATAL_ERROR "GLib not found. Please install with: vcpkg install glib")
endif()

# MySQL Client Library
find_package(unofficial-libmysql CONFIG QUIET)
if(unofficial-libmysql_FOUND)
    message(STATUS "MySQL client found via vcpkg")
    add_library(MySQL::Client ALIAS unofficial::libmysql::libmysql)
else()
    # Fallback to libmysql target name
    find_package(libmysql CONFIG QUIET)
    if(libmysql_FOUND)
        message(STATUS "MySQL client found via vcpkg (libmysql target)")
        add_library(MySQL::Client ALIAS libmysql)
    else()
        message(FATAL_ERROR "MySQL client not found. Please install with: vcpkg install libmysql")
    endif()
endif()

# secp256k1 cryptographic library
find_package(unofficial-secp256k1 CONFIG QUIET)
if(unofficial-secp256k1_FOUND)
    message(STATUS "secp256k1 found via vcpkg")
    # secp256k1 needs both the main library and precomputed tables
    if(TARGET unofficial::secp256k1 AND TARGET unofficial::secp256k1_precomputed)
        add_library(secp256k1_combined INTERFACE)
        target_link_libraries(secp256k1_combined INTERFACE 
            unofficial::secp256k1 
            unofficial::secp256k1_precomputed
        )
        add_library(secp256k1::secp256k1 ALIAS secp256k1_combined)
    elseif(TARGET unofficial::secp256k1)
        add_library(secp256k1::secp256k1 ALIAS unofficial::secp256k1)
    elseif(TARGET unofficial::secp256k1_precomputed) 
        add_library(secp256k1::secp256k1 ALIAS unofficial::secp256k1_precomputed)
    else()
        message(FATAL_ERROR "secp256k1 targets not found in vcpkg package")
    endif()
else()
    # Try alternative target names
    find_package(secp256k1 CONFIG QUIET)
    if(secp256k1_FOUND)
        message(STATUS "secp256k1 found via vcpkg (secp256k1 target)")
        add_library(secp256k1::secp256k1 ALIAS secp256k1)
    else()
        message(FATAL_ERROR "secp256k1 not found. Please install with: vcpkg install secp256k1")
    endif()
endif()

# spdlog (header-only / static, fmt-style logging)
find_package(spdlog CONFIG REQUIRED)
message(STATUS "spdlog found via vcpkg: ${spdlog_VERSION}")

# fmt (separate target — spdlog's bundled fmt is exposed via spdlog::spdlog)
find_package(fmt CONFIG REQUIRED)
message(STATUS "fmt found via vcpkg: ${fmt_VERSION}")

# Check testing framework (for unit tests)
find_package(check CONFIG QUIET)
if(check_FOUND)
    message(STATUS "Check testing framework found via vcpkg")
    # Check provides both Check::check and Check::checkShared targets
    if(TARGET Check::check)
        add_library(Check::Check ALIAS Check::check)
    elseif(TARGET Check::checkShared)
        add_library(Check::Check ALIAS Check::checkShared)
    else()
        add_library(Check::Check ALIAS check)
    endif()
else()
    message(WARNING "Check testing framework not found. Tests may not build.")
    message(STATUS "To install: vcpkg install check")
endif()