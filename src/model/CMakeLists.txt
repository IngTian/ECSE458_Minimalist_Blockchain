# Model Library - Core blockchain data structures
# Provides block and transaction models with persistence layer

# Add subdirectories for each model type
add_subdirectory(block)
add_subdirectory(transaction)

# Collect all model sources
file(GLOB_RECURSE MODEL_SOURCES "*.c")
file(GLOB_RECURSE MODEL_HEADERS "*.h")

# Create the models library
add_library(BlockChainModels STATIC ${MODEL_SOURCES} ${MODEL_HEADERS})

# Set target properties
set_target_properties(BlockChainModels PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Public headers
target_include_directories(BlockChainModels PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/block
    ${CMAKE_CURRENT_SOURCE_DIR}/transaction
)

# Dependencies
target_link_libraries(BlockChainModels PUBLIC
    BlockChain::Utils
    MySQL::Client
    GLib::GLib
    secp256k1::secp256k1
)

# Export this target
set_property(TARGET BlockChainModels PROPERTY EXPORT_NAME Models)
add_library(BlockChain::Models ALIAS BlockChainModels)