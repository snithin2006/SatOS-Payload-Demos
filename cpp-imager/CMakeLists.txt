cmake_minimum_required(VERSION 3.10)
project(cpp_imager)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV package
find_package(OpenCV REQUIRED)

# Add include directories
include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/include
)

# Add executables for your actual test files
add_executable(test_p src/test_p.cpp src/opencv_processor.cpp)
add_executable(test_pc src/test_pc.cpp src/opencv_processor.cpp src/opencv_compressor.cpp)

# Only build CUDA-dependent target if CUDA is available
include(CheckLanguage)
check_language(CUDA)
if(CMAKE_CUDA_COMPILER)
    enable_language(CUDA)
    add_executable(test_pce src/test_pce.cpp src/opencv_processor.cpp src/opencv_compressor.cpp src/cuda_encryptor.cu)
    set_target_properties(test_pce PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
    target_link_libraries(test_pce ${OpenCV_LIBS})
endif()

# Link OpenCV to all targets
target_link_libraries(test_p ${OpenCV_LIBS})
target_link_libraries(test_pc ${OpenCV_LIBS})

if(TARGET test_pce)
    target_link_libraries(test_pce ${OpenCV_LIBS})
    target_compile_options(test_pce PRIVATE -Wall -Wextra)
endif()

# Print OpenCV status
message(STATUS "OpenCV_INCLUDE_DIRS: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV_LIBS: ${OpenCV_LIBS}")

# Set compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_p PRIVATE -Wall -Wextra)
    target_compile_options(test_pc PRIVATE -Wall -Wextra)
endif()
