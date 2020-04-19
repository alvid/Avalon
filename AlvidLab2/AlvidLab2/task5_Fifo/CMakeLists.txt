cmake_minimum_required(VERSION 3.9)
project(task5)

set(CMAKE_CXX_STANDARD 17)
include_directories(../../../Common)

add_executable(task5 task5.cpp ../../../Common/Timeter.cpp)