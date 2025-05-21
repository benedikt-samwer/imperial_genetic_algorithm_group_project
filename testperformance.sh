rm -rf build
mkdir build
cd build
cmake ..
make test_circuit_simulator   
 ./tests/bin/test_circuit_simulator