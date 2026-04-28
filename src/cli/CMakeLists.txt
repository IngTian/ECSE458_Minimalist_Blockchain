# CLI Module - Command Line Interface for blockchain operations

# Collect source files (excluding shell.{c,cpp} which is built as an executable in apps/)
file(GLOB _CLI_C_FILES "interpreter.c" "interpreter.cpp")
set(CLI_SOURCES ${_CLI_C_FILES})

file(GLOB CLI_HEADERS "*.h")

# Create the CLI module library
add_library(CliModule STATIC ${CLI_SOURCES} ${CLI_HEADERS})

# Set target properties (mixed C / C++ during migration)
set_target_properties(CliModule PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Public headers
target_include_directories(CliModule PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Dependencies
target_link_libraries(CliModule PUBLIC
    BlockChain::Utils
    BlockChain::Models
    MySQL::Client
    GLib::GLib
    secp256k1::secp256k1
)

# Export this target
set_property(TARGET CliModule PROPERTY EXPORT_NAME CLI)
add_library(BlockChain::CLI ALIAS CliModule)