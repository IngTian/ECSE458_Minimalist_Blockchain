# CLI Module - Command Line Interface for blockchain operations

# Collect source files (excluding shell.c which will be an executable)
set(CLI_SOURCES
    interpreter.c
)

file(GLOB CLI_HEADERS "*.h")

# Create the CLI module library
add_library(CliModule STATIC ${CLI_SOURCES} ${CLI_HEADERS})

# Set target properties
set_target_properties(CliModule PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
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