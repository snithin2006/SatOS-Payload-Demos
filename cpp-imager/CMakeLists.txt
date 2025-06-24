cmake_minimum_required(VERSION 3.10)
project(cpp_imager)

# Enable CUDA language
enable_language(CUDA)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV package
find_package(OpenCV REQUIRED)

# Add include directories
include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/include
    ${CMAKE_SOURCE_DIR}/src/aes_lib
)

# Add executables for each test with their required implementation files
add_executable(test_p tests/test_p.cpp src/opencv_processor.cpp)
add_executable(test_pc tests/test_pc.cpp src/opencv_processor.cpp src/opencv_compressor.cpp)
add_executable(test_pce tests/test_pce.cpp src/opencv_processor.cpp src/opencv_compressor.cpp src/cuda_encryptor.cu src/aes_lib/aes_cuda_lib.cu)
add_executable(test_e tests/test_e.cpp src/cuda_encryptor.cu src/aes_lib/aes_cuda_lib.cu)

# Set CUDA properties for executables that use CUDA
set_target_properties(test_pce PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
set_target_properties(test_e PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

# Link OpenCV to all targets
foreach(tgt test_p test_pc test_pce test_e)
    if(TARGET ${tgt})
        target_link_libraries(${tgt} ${OpenCV_LIBS})
        target_compile_options(${tgt} PRIVATE -Wall -Wextra)
    endif()
endforeach()

# Print OpenCV status
message(STATUS "OpenCV_INCLUDE_DIRS: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV_LIBS: ${OpenCV_LIBS}")

