cmake_minimum_required(VERSION 3.10)
project(cpp_webserver)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Define the library target name
set(CPP_WEBSERVER_TARGET_NAME ${PROJECT_NAME})

# Source and include directories
set(SrcDir src)
file(GLOB_RECURSE SOURCES "${SrcDir}/*.cpp")

# Define the library target (e.g., cpp_webserver library)
add_library(${CPP_WEBSERVER_TARGET_NAME} ${SOURCES})

# Set the include directories for the library
target_include_directories(${CPP_WEBSERVER_TARGET_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

# Create an alias for easier usage
add_library(${PROJECT_NAME}::${CPP_WEBSERVER_TARGET_NAME} ALIAS ${CPP_WEBSERVER_TARGET_NAME})

# Define libraries to link
set(MY_LIBRARIES
    # nlohmann_json::nlohmann_json  # From json subdirectory (uncomment if needed)
    # fmt::fmt                       # From fmt library
    # CLI11::CLI11                   # From CLI11 library
    # spdlog::spdlog                 # From spdlog library
    ${CPP_WEBSERVER_TARGET_NAME}::${CPP_WEBSERVER_TARGET_NAME}  # Link to cpp_webserver library itself
)

# Link the libraries to cpp_webserver library
foreach(CPP_LIB ${MY_LIBRARIES})
    target_link_libraries(${CPP_WEBSERVER_TARGET_NAME} PRIVATE ${CPP_LIB})
endforeach()

# Define the executable target (e.g., beldum executable)
add_executable(cpp_webserver_run src/main.cpp)  # Use a specific main file or source files for the executable

# Link libraries to executable
foreach(CPP_LIB ${MY_LIBRARIES})
    target_link_libraries(cpp_webserver_run PRIVATE ${CPP_LIB})
endforeach()

# Set output directory for the build (for executables)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/target/debug/build")