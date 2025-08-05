#!/bin/bash

# K, Y'all Build Script
# Builds the K, Y'all social posting app for KDE/Plasma

set -e

echo "Building K, Y'all..."

# Create build directory
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
echo "Building..."
make -j$(nproc)

echo "Build complete!"
echo ""
echo "To install system-wide, run:"
echo "  sudo make install"
echo ""
echo "To run from build directory:"
echo "  ./kyall"
