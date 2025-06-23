#!/bin/bash

set -e

echo "Running Python unit tests..."
python3 test_coverage_analyzer.py

echo "Building and running C++ unit tests..."
CXXFLAGS=$(pkg-config --cflags catch2 2>/dev/null || echo "")
LDFLAGS=$(pkg-config --libs catch2 2>/dev/null || echo "")
g++ -std=c++11 $CXXFLAGS test_func_tracer.cpp $LDFLAGS -o test_func_tracer
./test_func_tracer

echo "All tests passed."
