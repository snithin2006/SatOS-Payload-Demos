cmake_minimum_required(VERSION 3.10)
project(ImageProcessor CUDA CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenCV REQUIRED)
find_package(CUDA REQUIRED)

# Include directories
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${CUDA_INCLUDE_DIRS})
include_directories(src/include)
include_directories(src/aes_lib)

# Try to find libjpeg-turbo specifically
find_library(JPEG_LIBRARY
    NAMES libjpeg-turbo8 libjpeg
    PATHS /usr/lib/aarch64-linux-gnu /usr/local/lib
    DOC "JPEG library"
)

if(JPEG_LIBRARY)
    message(STATUS "Found JPEG library: ${JPEG_LIBRARY}")
else()
    message(WARNING "JPEG library not found, using default")
    set(JPEG_LIBRARY "")
endif()

# Find nvjpeg library
find_library(NVJPEG_LIBRARY
    NAMES nvjpeg
    PATHS /usr/local/cuda/lib64 /usr/lib/aarch64-linux-gnu/nvidia /usr/local/cuda-12.2/targets/sbsa-linux/lib
    DOC "NVJPEG library"
)

if(NVJPEG_LIBRARY)
    message(STATUS "Found NVJPEG library: ${NVJPEG_LIBRARY}")
else()
    message(WARNING "NVJPEG library not found")
    set(NVJPEG_LIBRARY "")
endif()

# Source files for different components
set(OPENCV_SOURCES
    src/opencv_processor.cpp
    src/opencv_compressor.cpp
)

set(CUDA_SOURCES
    src/cuda_encryptor.cu
    src/aes_lib/aes_cuda_lib.cu
)

set(NVJPEG_SOURCES
    src/nvjpeg_compressor.cpp
)

set(JETSON_SOURCES
    src/jetson_compressor.cpp
)

# Create libraries for different components
add_library(OpenCVProcessor ${OPENCV_SOURCES})
target_link_libraries(OpenCVProcessor ${OpenCV_LIBS} ${JPEG_LIBRARY})

add_library(CudaProcessor ${CUDA_SOURCES})
target_link_libraries(CudaProcessor ${CUDA_LIBRARIES})
set_target_properties(CudaProcessor PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

add_library(NvjpegProcessor ${NVJPEG_SOURCES})
if(NVJPEG_LIBRARY)
    target_link_libraries(NvjpegProcessor ${NVJPEG_LIBRARY} ${JPEG_LIBRARY} ${CUDA_LIBRARIES})
else()
    target_link_libraries(NvjpegProcessor ${JPEG_LIBRARY} ${CUDA_LIBRARIES})
endif()

add_library(JetsonProcessor ${JETSON_SOURCES})
target_link_libraries(JetsonProcessor pthread)

# Test executables
add_executable(test_p tests/test_p.cpp)
target_link_libraries(test_p OpenCVProcessor)

add_executable(test_pc tests/test_pc.cpp)
target_link_libraries(test_pc OpenCVProcessor)

add_executable(test_pce tests/test_pce.cpp)
target_link_libraries(test_pce OpenCVProcessor CudaProcessor)

add_executable(test_e tests/test_e.cpp)
target_link_libraries(test_e CudaProcessor)

add_executable(test_nvjpeg_pc tests/test_nvjpeg_pc.cpp)
if(NVJPEG_LIBRARY)
    target_link_libraries(test_nvjpeg_pc OpenCVProcessor NvjpegProcessor ${CUDA_LIBRARIES})
else()
    target_link_libraries(test_nvjpeg_pc OpenCVProcessor ${CUDA_LIBRARIES})
endif()
set_target_properties(test_nvjpeg_pc PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

add_executable(test_jetson_pc tests/test_jetson_pc.cpp)
target_link_libraries(test_jetson_pc OpenCVProcessor JetsonProcessor)

add_executable(test_compression_comparison tests/test_compression_comparison.cpp)
if(NVJPEG_LIBRARY)
    target_link_libraries(test_compression_comparison OpenCVProcessor NvjpegProcessor JetsonProcessor ${CUDA_LIBRARIES})
else()
    target_link_libraries(test_compression_comparison OpenCVProcessor JetsonProcessor)
endif()

add_executable(test_full_pipeline tests/full_pipeline.cpp)
target_link_libraries(test_full_pipeline OpenCVProcessor CudaProcessor)

# Print configuration info
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "CUDA version: ${CUDA_VERSION}")
message(STATUS "JPEG library: ${JPEG_LIBRARY}")
message(STATUS "NVJPEG library: ${NVJPEG_LIBRARY}")

