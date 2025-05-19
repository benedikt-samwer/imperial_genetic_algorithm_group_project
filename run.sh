#!/bin/bash
# cd src
rm -rf build
mkdir build
cd build
cmake ..
make

if [ $? -eq 0 ]; then
    echo "编译成功，开始运行："
    ./bin/Circuit_Optimizer
else
    echo "编译失败！"
fi