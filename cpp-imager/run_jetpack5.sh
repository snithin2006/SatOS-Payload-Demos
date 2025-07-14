#!/bin/bash

# Script to run the JetPack 5.x container for nvJPEG testing

echo "Building and running JetPack 5.x container for nvJPEG testing..."

# Check if Docker is available
if ! command -v docker &> /dev/null; then
    echo "Error: Docker is not installed"
    exit 1
fi

# Check if NVIDIA Container Toolkit is available
if ! command -v nvidia-docker &> /dev/null; then
    echo "Warning: nvidia-docker not found, trying with regular docker..."
    DOCKER_CMD="docker run --gpus all"
else
    DOCKER_CMD="nvidia-docker run"
fi

# Build the container
echo "Building container..."
docker build -f Dockerfile.jetpack5 -t jetson-compression:jetpack5 .

# Run the container
echo "Running container..."
$DOCKER_CMD -it \
    -v $(pwd):/workspace \
    -v $(pwd)/data:/workspace/data \
    --workdir /workspace \
    jetson-compression:jetpack5 \
    /bin/bash

echo "Container exited." 