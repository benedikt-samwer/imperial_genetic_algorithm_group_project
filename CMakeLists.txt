cmake_minimum_required(VERSION 3.10)
project(Genetic_Algorithm)

# 1) Force all executables into <build>/bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# 2) copy source/parameters.txt → build/parameters.txt
configure_file(
  "${CMAKE_SOURCE_DIR}/parameters.txt"
  "${CMAKE_BINARY_DIR}/parameters.txt"
  COPYONLY
)

# 3) copy source/parameters.txt → build/bin/parameters.txt
configure_file(
  "${CMAKE_SOURCE_DIR}/parameters.txt"
  "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/parameters.txt"
  COPYONLY
)

# 4) Include paths for headers
include_directories("${CMAKE_SOURCE_DIR}/include")

# 5) Build your libraries / main
add_subdirectory(src)

# 6) Tests
include(CTest)
enable_testing()
add_subdirectory(tests)