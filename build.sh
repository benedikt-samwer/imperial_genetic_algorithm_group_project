#!/usr/bin/env bash
set -euo pipefail

# 1) Clean & create build dir
rm -rf build
mkdir -p build
cd build

# 2) On macOS switch to Homebrew GCC-14 for OpenMP
if [[ "$(uname -s)" == "Darwin" ]]; then
  echo "macOS → using Homebrew GCC-14 for OpenMP"
  export CC=/opt/homebrew/bin/gcc-14
  export CXX=/opt/homebrew/bin/g++-14
else
  echo "Non-macOS → using default \$CC/\$CXX"
fi

# 3) Configure
echo "Configuring…"
cmake ..

# 4) Build (parallel)
echo "Building…"
cmake --build . -- -j"$(getconf _NPROCESSORS_ONLN)"

echo "Build complete."