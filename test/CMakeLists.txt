# Tests - Unit tests for the blockchain system

# Only build tests on Apple for now (since Check framework paths are configured for it)
if(APPLE AND TARGET Check::Check)
    # Common test link libraries
    set(TEST_LINK_LIBS
        BlockChain::Models
        BlockChain::Utils
        BlockChain::CLI
        MySQL::Client
        GLib::GLib
        secp256k1::secp256k1
        Check::Check
    )
    if(APPLE)
        list(APPEND TEST_LINK_LIBS c++)
    else()
        list(APPEND TEST_LINK_LIBS stdc++)
    endif()

    # Transaction tests
    add_executable(test_transaction model/transaction_test.c)
    target_link_libraries(test_transaction PRIVATE ${TEST_LINK_LIBS})
    set_target_properties(test_transaction PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test
    )

    # Block tests
    add_executable(test_block model/block_test.c)
    target_link_libraries(test_block PRIVATE ${TEST_LINK_LIBS})
    set_target_properties(test_block PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test
    )

    # Register tests with CTest
    add_test(NAME TransactionTests COMMAND test_transaction)
    add_test(NAME BlockTests COMMAND test_block)

    # Install test executables
    install(TARGETS test_transaction test_block
        RUNTIME DESTINATION test
    )
else()
    message(STATUS "Tests disabled - Check framework not found or not on Apple platform")
endif(APPLE AND TARGET Check::Check)