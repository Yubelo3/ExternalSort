cmake_minimum_required(VERSION 3.9)

project(ext_sort)

set( CMAKE_CXX_FLAGS "-std=c++17" )
set(CMAKE_BUILD_TYPE DEBUG)

add_executable(ext_sort src/main.cpp src/Buffer.hpp src/MinLoserTree.hpp src/MinWinnerTree.hpp src/ExternalSort.hpp src/RunPayload.hpp)