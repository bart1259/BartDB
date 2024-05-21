# Set minimum cmake version
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
# Define project and language
project(hello LANGUAGES CXX)

# Configure c++
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(a hello.cpp)

