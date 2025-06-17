FROM nvidia/cuda:11.8.0-devel-ubuntu20.04

# Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libopencv-dev \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /workspace

# Copy source files
COPY . .

# Create build directory
RUN mkdir build

# Build the project
WORKDIR /workspace/build
RUN cmake .. && make -j4

# Set the entry point
ENTRYPOINT ["./cpp_imager"]
