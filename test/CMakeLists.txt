set(CMAKE_BUILD_TYPE Debug)

# create the test executable
add_executable(b_tree_tests test.cpp)
add_executable(disk_test disk_test.cpp)
add_executable(benchmark benchmark.cpp)

# link the library to the test executable
target_link_libraries(b_tree_tests b_tree_lib)
target_link_libraries(disk_test b_tree_lib)
target_link_libraries(benchmark b_tree_lib)

# specify the include directories
target_include_directories(b_tree_tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})