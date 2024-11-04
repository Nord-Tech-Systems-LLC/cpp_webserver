# CMake Global Config
cmake_minimum_required(VERSION 3.10)
project(cpp_webserver)  # Set the project name
set(CMAKE_CXX_STANDARD 20)  # Specify the C++ standard
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Define the library target name
set(CPP_WEBSERVER_TARGET_NAME ${PROJECT_NAME})

# Link Dependencies (Optional, depends on your usage)
set(LIB_DIR "${CMAKE_SOURCE_DIR}/target/debug/deps")  # Set the path to the C++ libraries
link_directories(${LIB_DIR})  # Add the path to the linker

# Source and include directories
set(SrcDir src)
file(GLOB_RECURSE SOURCES "${SrcDir}/*.cpp")

# Define the library target
add_library(${CPP_WEBSERVER_TARGET_NAME} ${SOURCES})

# Set the include directories for the library
target_include_directories(${CPP_WEBSERVER_TARGET_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  # Public include directory for headers
)

# Create an alias for the cpp_webserver library
add_library(${PROJECT_NAME}::${CPP_WEBSERVER_TARGET_NAME} ALIAS ${CPP_WEBSERVER_TARGET_NAME})

# Set output directory for the build (for executables)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/target/debug/build")