# CMake Gloabl Config
cmake_minimum_required(VERSION 3.10)
project(cpp_webserver_main) # Set the project name
set(CMAKE_CXX_STANDARD 20) # Specify the C++ standard
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Link Dependencies
set(LIB_DIR "${CMAKE_SOURCE_DIR}/target/debug/deps") # Set the path to the C++ libraries
link_directories(${LIB_DIR}) # Add the path to the linker

# Add include directory to global include paths
include_directories(include)

# Source and include directories
set(SrcDir src)
file(GLOB_RECURSE SOURCES "${SrcDir}/*.cpp")

# Create executeable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/target/debug/build") # Set the output directory for the executable
add_executable(cpp_webserver_main ${SOURCES}) # Add the executable

##################################
# TODO: Add libraries that you're using
##################################

# Add each library as a subdirectory
# add_subdirectory(${LIB_DIR}/json)          # nlohmann_json
# add_subdirectory(${LIB_DIR}/fmt)           # fmt library

# List your libraries to link
# set(MY_LIBRARIES
#     nlohmann_json::nlohmann_json        # From json subdirectory
#     fmt::fmt                            # From fmt library
# )

# Link each library in MY_LIBRARIES
# message("\n\nLibraries Linked:")
# foreach(CPP_LIB ${MY_LIBRARIES})
#     message("Library Name: ${CPP_LIB}")
#     target_link_libraries(MyExecutable PRIVATE ${CPP_LIB})
# endforeach()
# message("\n\n")

# Include directories for headers if necessary
# include_directories(${CMAKE_SOURCE_DIR}/path/to/headers)

