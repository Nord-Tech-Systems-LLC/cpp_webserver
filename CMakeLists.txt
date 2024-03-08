file(GLOB_RECURSE SOURCES
    header-folder/*.h
    source-folder/*.cpp
)

add_executable(yourProj ${SOURCES})