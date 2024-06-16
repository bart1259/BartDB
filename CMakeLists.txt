# Set minimum cmake version
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
# Define project and language
project(bartdb LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 14)

add_subdirectory(src/btreelib)
add_subdirectory(test)