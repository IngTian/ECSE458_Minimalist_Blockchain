# Applications - Main executables for the blockchain system

# Common link libraries for all executables
set(COMMON_LINK_LIBS
    BlockChain::CLI
    BlockChain::Models
    BlockChain::Utils
    MySQL::Client
    GLib::GLib
    secp256k1::secp256k1
)

# Main blockchain application
add_executable(main ${CMAKE_SOURCE_DIR}/src/main.c)
target_link_libraries(main PRIVATE ${COMMON_LINK_LIBS})

# Link with C++ standard library since MySQL client is built with C++
if(APPLE)
    target_link_libraries(main PRIVATE c++)
else()
    target_link_libraries(main PRIVATE stdc++)
endif()

set_target_properties(main PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Interactive shell
if(APPLE)
    add_executable(shell ${CMAKE_SOURCE_DIR}/src/cli/shell.c)
    target_link_libraries(shell PRIVATE ${COMMON_LINK_LIBS})
    # Link with C++ standard library since MySQL client is built with C++
    if(APPLE)
        target_link_libraries(shell PRIVATE c++)
    else()
        target_link_libraries(shell PRIVATE stdc++)
    endif()
    set_target_properties(shell PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endif()

# Network server
add_executable(server ${CMAKE_SOURCE_DIR}/src/socket/server.c)
target_link_libraries(server PRIVATE ${COMMON_LINK_LIBS})
# Link with C++ standard library since MySQL client is built with C++
if(APPLE)
    target_link_libraries(server PRIVATE c++)
else()
    target_link_libraries(server PRIVATE stdc++)
endif()
set_target_properties(server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Network client
add_executable(client ${CMAKE_SOURCE_DIR}/src/socket/client.c)
target_link_libraries(client PRIVATE ${COMMON_LINK_LIBS})
# Link with C++ standard library since MySQL client is built with C++
if(APPLE)
    target_link_libraries(client PRIVATE c++)
else()
    target_link_libraries(client PRIVATE stdc++)
endif()
set_target_properties(client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Install targets
install(TARGETS main server client
    RUNTIME DESTINATION bin
)

if(APPLE)
    install(TARGETS shell
        RUNTIME DESTINATION bin
    )
endif()