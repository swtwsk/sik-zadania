cmake_minimum_required(VERSION 3.5)
project(2)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS "-g -Wall -Wextra -O2 -Wpedantic -Wshadow -Wfatal-errors -std=c++17")

add_executable(2 main.cpp Transmitter.cpp Transmitter.h CtrlPortListener.cpp CtrlPortListener.h)