cmake_minimum_required(VERSION 3.5)
project(SIK1)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS "-g -Wall -Wextra -O2 -Wpedantic -Wshadow -Wfatal-errors -std=c++17")

set(SOURCE_FILES zadanie1.cc Server.cc Server.h TelnetServer.cc TelnetServer.h TelnetTerminal.cc TelnetTerminal.h)
add_executable(SIK1 ${SOURCE_FILES})
