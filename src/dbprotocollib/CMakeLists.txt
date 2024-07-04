set(CMAKE_BUILD_TYPE Debug)

# specify the library source files
file(GLOB LIBRARY_SOURCES *.cpp)

# create a static library
add_library(db_protocols_lib STATIC ${LIBRARY_SOURCES})

# link the library to the test executable
target_link_libraries(db_protocols_lib b_tree_lib)

# specify the include directories
target_include_directories(db_protocols_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})