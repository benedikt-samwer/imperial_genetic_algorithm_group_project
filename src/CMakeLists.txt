## Add the genetic algorithm library
add_library(geneticAlgorithm Genetic_Algorithm.cpp)

target_link_libraries(geneticAlgorithm PUBLIC OpenMP::OpenMP_CXX)
# Optional: include directory if needed
# include_directories(${CMAKE_SOURCE_DIR}/include)

set_target_properties(geneticAlgorithm
    PROPERTIES
    CXX_STANDARD 17
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# Build the circuit simulator as a testable library
add_library(circuitSimulator CCircuit.cpp CSimulator.cpp CUnit.cpp)
set_target_properties(circuitSimulator
    PROPERTIES
    CXX_STANDARD 17
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# Find OpenMP and enable it if available
find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    target_link_libraries(geneticAlgorithm PUBLIC OpenMP::OpenMP_CXX)
    target_link_libraries(circuitSimulator PUBLIC OpenMP::OpenMP_CXX)
endif()

# Build the executable
add_executable(Circuit_Optimizer main.cpp)
target_link_libraries(Circuit_Optimizer PUBLIC geneticAlgorithm circuitSimulator)

set_target_properties(Circuit_Optimizer
    PROPERTIES
    CXX_STANDARD 17
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)