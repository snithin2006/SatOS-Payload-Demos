# C++ Image Processing and Compression Pipeline

This project implements a GPU-accelerated image processing and compression pipeline using OpenCV and NVIDIA's nvJPEG library.

## Features

- Image loading and preprocessing using OpenCV
- BGR to RGB conversion
- GPU-accelerated JPEG compression using nvJPEG
- Memory-efficient processing pipeline

## Requirements

- NVIDIA GPU with CUDA support
- CUDA Toolkit 11.8 or later
- OpenCV 4.x
- CMake 3.10 or later
- C++17 compatible compiler

## Building with Docker

1. Build the Docker image:
```bash
docker build -t cpp-imager .
```

2. Run the container:
```bash
docker run --gpus all -v $(pwd):/workspace cpp-imager input.jpg output.jpg
```

## Building Locally

1. Create build directory:
```bash
mkdir build && cd build
```

2. Configure with CMake:
```bash
cmake ..
```

3. Build:
```bash
make -j4
```

4. Run:
```bash
./cpp_imager input.jpg output.jpg
```

## Usage

The program takes two command-line arguments:
1. Input image path
2. Output JPEG path

Example:
```bash
./cpp_imager input.png output.jpg
```

## Error Handling

The program includes comprehensive error handling for:
- Image loading failures
- GPU memory allocation issues
- Compression errors
- File I/O problems

## License

This project is licensed under the same terms as the SatOS Payload SDK.
