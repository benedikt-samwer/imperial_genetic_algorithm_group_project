#!/bin/bash
set -euo pipefail

# Parse command line arguments
CLEAN_BUILD=0
VERBOSE=0

while getopts "cv" opt; do
    case $opt in
        c) CLEAN_BUILD=1 ;;
        v) VERBOSE=1 ;;
        *) echo "Usage: $0 [-c] [-v]" >&2
           echo "  -c: Clean build before testing"
           echo "  -v: Verbose output"
           exit 1 ;;
    esac
done

# Clean root directory CMake cache (prevents generator conflicts)
echo "[Root] Cleaning any CMake cache in root directory..."
rm -f CMakeCache.txt
rm -rf CMakeFiles
rm -f cmake_install.cmake
rm -f CTestTestfile.cmake
rm -rf Testing
rm -rf _deps

# Clean and reconfigure if requested
if [ $CLEAN_BUILD -eq 1 ]; then
    echo "[Build] Cleaning build directory..."
    rm -rf build
    echo "[Config] Configuring CMake with Ninja..."
    cmake -B build -S . -G Ninja
fi

# Create build directory if it doesn't exist
if [ ! -d build ]; then
    echo "[Build] Creating build directory..."
    cmake -B build -S . -G Ninja
fi

# Build all targets
echo "[Build] Building tests..."
cmake --build build --parallel

# Run all tests
cd build
if [ $VERBOSE -eq 1 ]; then
    echo "[Test] Running tests with verbose output..."
    ctest -VV --output-on-failure
else
    echo "[Test] Running tests..."
    ctest --output-on-failure
fi

# Available test commands (commented out by default)
: '
# List all available test cases:
ctest -N

# Run specific test case:
# ctest -R "CircuitSimulatorTest.AllToTailingsN2" -V

# Run by test number:
# ctest -I 1,1 -V  # Run first test

# Run multiple tests using regex:
# ctest -R "CircuitSimulatorTest.*" -V

# Run test executable directly:
# ./tests/test_circuit_simulator_test
'