#!/usr/bin/env bash
set -euo pipefail

# 1) Ensure build artifacts exist
if [[ ! -d build ]]; then
  echo "[Error] No build directory found. Run ./build.sh first."
  exit 1
fi

if [[ ! -f build/bin/Circuit_Optimizer ]]; then
  echo "[Error] Circuit_Optimizer binary not found. Run `make build` first."
  exit 1
fi

# 2) Copy parameters file so the binary can see it
echo "[Setup] Copying parameters file..."
cp parameters.txt build/

# 3) Run the optimizer
echo "[Run] Running optimizer…"
./build/bin/Circuit_Optimizer

# 4) Invoke the Python visualization
echo "[Viz] Visualising results…"
pushd plotting >/dev/null
python3 main.py -f
popd >/dev/null

echo "[Done] Complete."