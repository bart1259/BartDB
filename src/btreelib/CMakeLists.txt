set(CMAKE_BUILD_TYPE Debug)

# specify the library source files
file(GLOB LIBRARY_SOURCES *.cpp)

# create a static library
add_library(b_tree_lib STATIC ${LIBRARY_SOURCES})

# specify the include directories
target_include_directories(b_tree_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})