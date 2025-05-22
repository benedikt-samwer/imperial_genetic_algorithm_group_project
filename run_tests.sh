#!/bin/bash

# Clean and reconfigure CMake
rm -rf build && cmake -B build -S . -G Ninja

# Build all targets
cmake --build build --parallel

# Run all tests
cd build && ctest -VV --output-on-failure

# List all available test cases
# cd build && ctest -N

# Methods to run individual test cases:
# 1. Run by test name:
# cd build && ctest -R "CircuitSimulatorTest.AllToTailingsN2" -V
#
# 2. Run by test number:
# cd build && ctest -I 1,1 -V  # Run first test
#
# 3. Run multiple tests using regex:
# cd build && ctest -R "CircuitSimulatorTest.*" -V  # Run all CircuitSimulatorTest tests
#
# 4. Run by test file:
# cd build && ./tests/test_circuit_simulator_test  # Run test executable directly