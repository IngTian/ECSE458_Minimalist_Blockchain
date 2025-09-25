# External Dependencies for MinimalistBlockChainSystem

# Test coverage (Apple specific)
if(APPLE)
    set(GCC_COVERAGE_COMPILE_FLAGS "-fprofile-instr-generate -fcoverage-mapping")
    set(GCC_COVERAGE_LINK_FLAGS "--coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")
endif()

# MySQL Client
pkg_check_modules(MYSQL REQUIRED mysqlclient)
if(MYSQL_FOUND)
    message(STATUS "MySQL client found: ${MYSQL_VERSION}")
    # Create imported target
    add_library(MySQL::Client INTERFACE IMPORTED)
    target_include_directories(MySQL::Client INTERFACE ${MYSQL_INCLUDE_DIRS})
    target_link_libraries(MySQL::Client INTERFACE ${MYSQL_LIBRARIES})
    target_compile_options(MySQL::Client INTERFACE ${MYSQL_CFLAGS_OTHER})
    target_link_directories(MySQL::Client INTERFACE ${MYSQL_LIBRARY_DIRS})
endif()

# GLib
pkg_check_modules(GLIB REQUIRED glib-2.0)
if(GLIB_FOUND)
    message(STATUS "GLib found: ${GLIB_VERSION}")
    # Create imported target
    add_library(GLib::GLib INTERFACE IMPORTED)
    target_include_directories(GLib::GLib INTERFACE ${GLIB_INCLUDE_DIRS})
    target_link_libraries(GLib::GLib INTERFACE ${GLIB_LIBRARIES})
    target_compile_options(GLib::GLib INTERFACE ${GLIB_CFLAGS_OTHER})
    target_link_directories(GLib::GLib INTERFACE ${GLIB_LIBRARY_DIRS})
endif()

# secp256k1 cryptographic library
find_library(SECP256K1_LIBRARIES 
    NAMES secp256k1 
    PATHS /usr/local/lib /opt/homebrew/lib
    REQUIRED)

if(SECP256K1_LIBRARIES)
    message(STATUS "secp256k1 found: ${SECP256K1_LIBRARIES}")
    # Find include directories
    find_path(SECP256K1_INCLUDE_DIRS 
        NAMES secp256k1.h 
        PATHS /usr/local/include /opt/homebrew/include)
    
    # Create imported target
    add_library(secp256k1::secp256k1 SHARED IMPORTED)
    set_target_properties(secp256k1::secp256k1 PROPERTIES
        IMPORTED_LOCATION ${SECP256K1_LIBRARIES}
        INTERFACE_INCLUDE_DIRECTORIES ${SECP256K1_INCLUDE_DIRS})
endif()

# Check testing framework (for unit tests)
find_library(CHECK_LIBRARIES 
    NAMES check 
    PATHS /opt/homebrew/lib /usr/local/lib)
find_path(CHECK_INCLUDE_DIRS 
    NAMES check.h 
    PATHS /opt/homebrew/include /usr/local/include)

if(CHECK_LIBRARIES AND CHECK_INCLUDE_DIRS)
    message(STATUS "Check testing framework found: ${CHECK_LIBRARIES}")
    # Create imported target
    add_library(Check::Check SHARED IMPORTED)
    set_target_properties(Check::Check PROPERTIES
        IMPORTED_LOCATION ${CHECK_LIBRARIES}
        INTERFACE_INCLUDE_DIRECTORIES ${CHECK_INCLUDE_DIRS})
else()
    message(STATUS "Check testing framework not found")
endif()