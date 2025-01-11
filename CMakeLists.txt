cmake_minimum_required(VERSION 3.29)
project(auth)

set(CMAKE_CXX_STANDARD 20)

add_executable(auth client.cpp
        server.cpp
        messages.h
)
