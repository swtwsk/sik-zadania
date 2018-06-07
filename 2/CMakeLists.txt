cmake_minimum_required(VERSION 3.5)
project(sikradio-sender)

find_package ( Threads REQUIRED )

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS "-g -Wall -Wextra -O2 -Wpedantic -Wshadow -Wfatal-errors -std=c++17")

# http://stackoverflow.com/questions/10555706/
macro (add_executable _name)
    # invoke built-in add_executable
    _add_executable(${ARGV})
    if (TARGET ${_name})
        target_link_libraries(${_name} ${CMAKE_THREAD_LIBS_INIT} )
    endif()
endmacro()

add_executable(sikradio-sender main.cpp Transmitter.cpp Transmitter.h CtrlPortListener.cpp CtrlPortListener.h
        ConcurrentContainers.h ServerException.h TransmitterData.h TransmitterData.cpp)
