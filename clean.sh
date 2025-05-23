#!/usr/bin/env bash
set -euo pipefail

echo "[Clean] Starting comprehensive cleanup..."

# Clean root directory CMake artifacts
echo "[Root] Removing CMake cache from root directory..."
rm -f CMakeCache.txt
rm -rf CMakeFiles
rm -f cmake_install.cmake
rm -f CTestTestfile.cmake
rm -rf Testing
rm -rf _deps
rm -f DartConfiguration.tcl

# Clean build directory
echo "[Build] Removing build directory..."
rm -rf build

# Clean any other CMake artifacts
echo "[Artifacts] Removing other build artifacts..."
rm -rf bin
rm -rf lib

echo "[Clean] Cleanup complete! You can now run ./build.sh safely." 