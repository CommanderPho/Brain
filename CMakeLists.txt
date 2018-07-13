cmake_minimum_required(VERSION 3.6)
project(PhoBrain)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
    Brain.cpp
    Brain.h
    keywords.txt
    ThinkGearPacket.cpp
    ThinkGearPacket.h
    ThinkGearPacketProtocol.h)

add_executable(PhoBrain ${SOURCE_FILES})