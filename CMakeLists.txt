cmake_minimum_required(VERSION 3.6)
project(PhoBrain)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
    PhoBrain.cpp
    PhoBrain.h
    keywords.txt)

add_executable(PhoBrain ${SOURCE_FILES})