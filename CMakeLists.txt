cmake_minimum_required(VERSION 3.22)
project(cpp_simple_vector)

set(CMAKE_CXX_STANDARD 14)

include_directories(simple-vector)

add_executable(cpp_simple_vector
        simple-vector/array_ptr.h
        simple-vector/main.cpp
        simple-vector/simple_vector.h)
