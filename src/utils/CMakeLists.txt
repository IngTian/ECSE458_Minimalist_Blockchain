# Utils Library - Core utilities for the blockchain system
# Provides cryptography, logging, JSON parsing, database utilities, etc.

# Collect source files
file(GLOB UTILS_SOURCES "*.c")
file(GLOB UTILS_HEADERS "*.h")

# Create the utils library
add_library(BlockChainUtils STATIC ${UTILS_SOURCES} ${UTILS_HEADERS})

# Set target properties
set_target_properties(BlockChainUtils PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Public headers - these can be included by other modules
target_include_directories(BlockChainUtils PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Private headers - internal to this module
target_include_directories(BlockChainUtils PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link external dependencies
target_link_libraries(BlockChainUtils PUBLIC
    MySQL::Client
    GLib::GLib
    secp256k1::secp256k1
)

# Compiler flags for this target
target_compile_definitions(BlockChainUtils PRIVATE
    _GNU_SOURCE
)

# Export this target for use by other modules
set_property(TARGET BlockChainUtils PROPERTY EXPORT_NAME Utils)
add_library(BlockChain::Utils ALIAS BlockChainUtils)