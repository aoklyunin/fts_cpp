cmake_minimum_required(VERSION 3.9)
project(fts_cpp)

set(CMAKE_CXX_STANDARD 11)


add_executable(fts_cpp main.cpp FTSensor.cpp FTSensor.h time.cpp test.h test.cpp)