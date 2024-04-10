cmake_minimum_required(VERSION 3.20)
project(cpp_webserver_main)
set(CMAKE_CXX_STANDARD 20)

# Check the operating system
if(WIN32)
    # Windows specific code
    message(STATUS "Configuring for Windows")
    file(GLOB SOURCES
        windows/src/main.cpp
        windows/src/cpp_files/*.cpp
        windows/src/hpp_files/*.hpp
    )

    add_executable(cpp_webserver_main ${SOURCES})
elseif(UNIX)
    # Unix specific code
    message(STATUS "Configuring for Unix")
    file(GLOB SOURCES
        linux/src/main.cpp
        linux/src/cpp_files/*.cpp
        linux/src/hpp_files/*.hpp
    )

    add_executable(cpp_webserver_main ${SOURCES})
else()
    message(FATAL_ERROR "Unsupported operating system")
endif()