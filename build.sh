#!/usr/bin/env bash
set -euo pipefail

# 0) Clean root directory CMake cache if exists (prevents generator conflicts)
echo "Cleaning any CMake cache in root directory..."
rm -f CMakeCache.txt
rm -rf CMakeFiles
rm -f cmake_install.cmake
rm -f CTestTestfile.cmake
rm -rf Testing
rm -rf _deps

# 1) Clean & create build dir if not exists
if [[ ! -d build ]]; then
    mkdir -p build
else
    # Clean CMake cache to avoid generator conflicts
    rm -f build/CMakeCache.txt
    rm -rf build/CMakeFiles
fi
cd build

# 2) On macOS switch to Homebrew GCC-14 for OpenMP
if [[ "$(uname -s)" == "Darwin" ]]; then
    echo "macOS → using Homebrew GCC-14 for OpenMP"
    export CC=/opt/homebrew/bin/gcc-14
    export CXX=/opt/homebrew/bin/g++-14
else
    echo "Non-macOS → using default \$CC/\$CXX"
fi

# 3) Configure with Ninja
echo "Configuring with Ninja..."
cmake -G Ninja ..

# 4) Build (parallel)
echo "Building..."
cmake --build . --parallel

echo "Build complete."