cmake_minimum_required(VERSION 3.10)
project(tests)

# List all your test executables (without the “.cpp”)
set(Tests
    test_circuit_simulator
    test_genetic_algorithm
    test_validity_checker
)

# Where to find your public headers
include_directories(${CMAKE_SOURCE_DIR}/include)

foreach(TEST IN LISTS Tests)
    # Build the test executable from its .cpp
    add_executable(${TEST} ${TEST}.cpp)

    # Link against the GA and simulator libraries
    target_link_libraries(${TEST}
        PRIVATE
          geneticAlgorithm
          circuitSimulator
    )

    # Where to drop the binaries
    set_target_properties(${TEST} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests/bin"
        CXX_STANDARD 17
    )

    # Register it with CTest
    add_test(NAME ${TEST} COMMAND ${TEST})
endforeach()

# Also make sure your main Circuit_Optimizer is registered
add_test(NAME Circuit_Optimizer COMMAND Circuit_Optimizer)