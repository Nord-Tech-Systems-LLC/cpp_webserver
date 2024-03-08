cmake_minimum_required(VERSION 3.20)
project(cpp_webserver_main)
set(CMAKE_CXX_STANDARD 20)

file(GLOB SOURCES
    src/main.cpp
    src/cpp_files/*.cpp
    src/hpp_files/*.hpp
)

add_executable(cpp_webserver_main ${SOURCES})