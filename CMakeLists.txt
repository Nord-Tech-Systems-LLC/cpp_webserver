cmake_minimum_required(VERSION 3.20)
project(cpp_webserver_main)
set(CMAKE_CXX_STANDARD 20)


# Check the operating system
if(WIN32)
    # Windows specific code
    message(STATUS "Configuring for Windows")
elseif(UNIX)
    # Unix specific code
    message(STATUS "Configuring for Unix")
else()
    message(FATAL_ERROR "Unsupported operating system")
endif()

file(GLOB SOURCES
    src/main.cpp
    src/cpp_files/*.cpp
    src/hpp_files/*.hpp
)

add_executable(cpp_webserver_main ${SOURCES})
