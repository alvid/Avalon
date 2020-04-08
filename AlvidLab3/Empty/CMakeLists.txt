cmake_minimum_required(VERSION 3.15)
project(Empty)

set(CMAKE_CXX_STANDARD 17)

add_executable(Empty
        Empty.cpp Producer.cpp Producer.hpp Store.cpp Store.hpp Consumer.cpp Consumer.hpp Types.hpp
        ../../Common/Timeter.cpp ../../Common/Timeter.hpp)

include_directories(Empty PUBLIC ../../Common)
