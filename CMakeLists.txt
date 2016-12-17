cmake_minimum_required(VERSION 3.3)
project(_RC_FTP_Client)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(_RC_FTP_Client ${SOURCE_FILES})