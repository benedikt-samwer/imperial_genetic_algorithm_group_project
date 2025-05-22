cmake_minimum_required(VERSION 3.10)
project(Genetic_Algorithm)

# Fine OpenMP Package

find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
  message(STATUS "OpenMP found: ${OpenMP_CXX_VERSION}")
else()
  message(FATAL_ERROR "OpenMP not found")
endif()

# Force all executables into <build>/bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Copy source/parameters.txt → build/parameters.txt
configure_file(
  "${CMAKE_SOURCE_DIR}/parameters.txt"
  "${CMAKE_BINARY_DIR}/parameters.txt"
  COPYONLY
)

# Copy source/parameters.txt → build/bin/parameters.txt
configure_file(
  "${CMAKE_SOURCE_DIR}/parameters.txt"
  "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/parameters.txt"
  COPYONLY
)

# Include paths for headers
include_directories("${CMAKE_SOURCE_DIR}/include")

# Build your libraries / main
add_subdirectory(src)

# Tests
include(CTest)
enable_testing()
add_subdirectory(tests)