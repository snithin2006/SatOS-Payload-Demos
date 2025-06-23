# Use NVIDIA CUDA base image
FROM nvidia/cuda:12.3.1-devel-ubuntu22.04

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive

# Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libopencv-dev \
    python3-opencv \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy project files
COPY . .

# Create build directory and build the project executables
RUN mkdir -p build && \
    cd build && \
    cmake .. && \
    make test_proc test_proc_comp test_proc_comp_enc

# Command to run when container starts (can be overridden)
CMD ["/bin/bash"]