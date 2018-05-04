cmake_minimum_required(VERSION 3.5)
project(SIK1)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -O2 -Wpedantic -Wshadow -Wfatal-errors -std=c++17")

add_executable(SIK1 main.cpp Server.cpp Server.h TelnetServer.cpp TelnetServer.h TelnetTerminal.cpp TelnetTerminal.h)
