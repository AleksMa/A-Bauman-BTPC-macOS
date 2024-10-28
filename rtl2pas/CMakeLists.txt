cmake_minimum_required(VERSION 3.5)
project(rtltopas)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

set(SOURCE_FILES main.cpp)
add_executable(rtltopas ${SOURCE_FILES})
