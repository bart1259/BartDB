set(CMAKE_BUILD_TYPE Debug)

# create a server executale
add_executable(client client.cpp)

# link the library to the server executable
target_link_libraries(client db_protocols_lib)

# specify the include directories
target_include_directories(db_protocols_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})