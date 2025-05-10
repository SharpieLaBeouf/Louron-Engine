#!/bin/bash

cd ../../Build/Debug || exit 1

FORCE_REBUILD=false

# Check for --f argument
for arg in "$@"; do
    if [ "$arg" == "--f" ]; then
        FORCE_REBUILD=true
        break
    fi
done

if $FORCE_REBUILD; then
    echo "Force rebuild triggered..."
    rm -f bin/LouronEditor
    cmake --build . -- -j$(nproc)
else
    # Dry run the build to check if anything would be rebuilt
    if cmake --build . -- -n | grep -qE "Building|Linking"; then
        echo "Changes detected. Rebuilding..."
        rm -f bin/LouronEditor
        cmake --build . -- -j$(nproc)
    else
        echo "No changes detected. Skipping rebuild."
    fi
fi

