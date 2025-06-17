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

# Add executables
add_executable(cpp_imager 
    src/main.cpp
    src/image_processor.cpp
    src/nvjpeg_compressor.cpp
)

add_executable(test_image_processor
    src/test_image_processor.cpp
    src/image_processor.cpp
)

# Link libraries
target_link_libraries(cpp_imager ${OpenCV_LIBS})
target_link_libraries(test_image_processor ${OpenCV_LIBS})

# Print OpenCV status
message(STATUS "OpenCV_INCLUDE_DIRS: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV_LIBS: ${OpenCV_LIBS}")

# Set compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(cpp_imager PRIVATE -Wall -Wextra)
    target_compile_options(test_image_processor PRIVATE -Wall -Wextra)
endif()
