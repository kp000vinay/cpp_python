#!/bin/bash
# Simple build script for macOS
# This will build cpp_project.exe on macOS

clang++ -std=c++11 -O2 -o cpp_project.exe main.cpp

echo "Build complete! Run with: ./cpp_project.exe"








