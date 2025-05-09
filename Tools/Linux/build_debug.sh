#!/bin/bash

cd ../../Build/Debug || exit 1

# Dry run the build to check if anything would be rebuilt
if cmake --build . -- -n | grep -qE "Building|Linking"; then
    echo "Changes detected. Rebuilding..."
    rm -f bin/LouronEditor
    cmake --build . -- -j$(nproc)
else
    echo "No changes detected. Skipping rebuild."
fi