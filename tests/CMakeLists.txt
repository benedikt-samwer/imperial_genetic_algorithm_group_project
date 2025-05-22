cmake_minimum_required(VERSION 3.10)
project(tests)

# Google Test integration
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# List all your test executables (without the “.cpp”)
set(Tests
    test_circuit_simulator
    test_genetic_algorithm
    test_validity_checker
)

# Where to find your public headers
# Assuming your project's include directory is at ../include relative to this tests directory
# Or it's correctly found via CMAKE_SOURCE_DIR if this CMakeLists.txt is part of the main project structure
include_directories(${CMAKE_SOURCE_DIR}/include)
# Also need to include the src directory for CCircuit.h etc if they are not installed to include path
include_directories(${CMAKE_SOURCE_DIR}/src)

foreach(TEST IN LISTS Tests)
    # Build the test executable from its .cpp
    add_executable(${TEST} ${TEST}.cpp)

    # Link against the GA and simulator libraries
    target_link_libraries(${TEST}
        PRIVATE
          geneticAlgorithm
          circuitSimulator
          gtest_main # Link with Google Test (which provides main)
    )

    # Where to drop the binaries
    set_target_properties(${TEST} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests/bin"
        CXX_STANDARD 17
    )

    # Register it with CTest
    # add_test(NAME ${TEST} COMMAND ${TEST}) # Original way
    # For Google Test, CTest can discover tests automatically if configured correctly
    # or you can use gtest_discover_tests (CMake 3.10+)
    include(GoogleTest)
    gtest_discover_tests(${TEST})
endforeach()

# Also make sure your main Circuit_Optimizer is registered
# This seems like an integration test or a different kind of test.
# We'll leave it for now, but it might need different handling than unit tests.
add_test(NAME Circuit_Optimizer COMMAND Circuit_Optimizer)