#!/bin/bash

build_dir="build"

if [ ! -d "$build_dir" ]; then
    echo "Creating build directory..."
    mkdir "$build_dir"
    echo "Build directory created."
    cd "$build_dir"
    cmake ..
else
    echo "Build directory already exists."
fi

