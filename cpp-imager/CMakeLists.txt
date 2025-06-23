cmake_minimum_required(VERSION 3.10)
project(cpp_imager)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV package
find_package(OpenCV REQUIRED)

# Find CUDA for encryption
find_package(CUDA REQUIRED)

# Add include directories
include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/include
)

# Add executables for your actual test files
add_executable(test_proc src/test_proc.cpp src/image_processor.cpp)
add_executable(test_proc_comp src/test_proc_comp.cpp src/image_processor.cpp)
add_executable(test_proc_comp_enc src/test_proc_comp_enc.cpp src/image_processor.cpp src/cuda_encryptor.cu)

# Link OpenCV to all targets
foreach(target test_proc test_proc_comp test_proc_comp_enc)
    target_link_libraries(${target} ${OpenCV_LIBS})
endforeach()

# Link CUDA to the encryption target
set_target_properties(test_proc_comp_enc PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
target_link_libraries(test_proc_comp_enc ${OpenCV_LIBS} ${CUDA_LIBRARIES})

# Print OpenCV status
message(STATUS "OpenCV_INCLUDE_DIRS: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV_LIBS: ${OpenCV_LIBS}")

# Set compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_proc PRIVATE -Wall -Wextra)
    target_compile_options(test_proc_comp PRIVATE -Wall -Wextra)
    target_compile_options(test_proc_comp_enc PRIVATE -Wall -Wextra)
endif()

enable_language(CUDA)
