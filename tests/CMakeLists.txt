project(tests)
 
# Find CUnit package
# find_library(CUNIT_LIBRARY NAMES cunit libcunit)
# if(NOT CUNIT_LIBRARY)
#     message(FATAL_ERROR "CUnit library not found")
# endif()
 
set(CUNIT_INCLUDE_DIR "/opt/homebrew/opt/cunit/include")
set(CUNIT_LIBRARY "/opt/homebrew/opt/cunit/lib/libcunit.dylib")
 
 
list(APPEND Tests test_circuit_simulator
                  test_genetic_algorithm
                  test_validity_checker)
 
foreach(TEST IN LISTS Tests)
    add_executable(${TEST} ${TEST}.cpp)
    target_link_libraries(${TEST} geneticAlgorithm circuitSimulator ${CUNIT_LIBRARY})
    target_include_directories(${TEST} PRIVATE
    ../includes
     ${CUNIT_INCLUDE_DIR}
)
    set_target_properties(${TEST} PROPERTIES
        CXX_STANDARD 17
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests/bin")
    add_test(NAME ${TEST} COMMAND ${TEST})
endforeach()
 
add_test(NAME executable COMMAND Circuit_Optimizer)