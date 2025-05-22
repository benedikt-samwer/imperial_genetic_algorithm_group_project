#!/usr/bin/env bash
set -euo pipefail

# ✂️ Clean build directory
rm -rf build
mkdir -p build
cd build

# 🖥 Detect OS and set compilers for macOS
if [[ "$(uname -s)" == "Darwin" ]]; then
  echo "macOS detected: switching to Homebrew GCC-14 for OpenMP support"
  export CC=/opt/homebrew/bin/gcc-14
  export CXX=/opt/homebrew/bin/g++-14
else
  echo "Non-macOS detected: using default \$CC/\$CXX"
fi

# 🔧 Configure
echo "Configuring project..."
cmake ..

# 📦 Build
echo "Building project..."
cmake --build . -- -j$(getconf _NPROCESSORS_ONLN)

# ▶️ Run
echo "Running executable..."
# Replace 'Circuit_Optimizer' with your actual executable name if different:
./bin/Circuit_Optimizer

echo "Visualising..."
cd ../plotting
python3 main.py -f