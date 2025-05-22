#!/bin/bash
# cd src
rm -rf build
mkdir build
cd build
cmake ..
make

if [ $? -eq 0 ]; then
    echo "Compile success!"
    echo "Running the program..."
    ./bin/Circuit_Optimizer
    echo "Running the Python script (main.py -f)..."
    cd ../plotting
    python3 main.py -f
else
    echo "Compile failed!"
fi