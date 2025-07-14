# Jetson Multimedia API JPEG Compressor

This document describes the Jetson Multimedia API compressor implementation that uses NVIDIA's hardware-accelerated JPEG encoding on Jetson platforms.

## Overview

The `JetsonCompressor` class provides hardware-accelerated JPEG compression using the Jetson Multimedia API (V4L2 interface). This implementation is specifically designed for Jetson platforms and offers better performance than CPU-based compression while being more compatible than nvJPEG on Jetson devices.

## Key Features

- **Hardware Acceleration**: Uses Jetson's dedicated hardware encoders (NVENC/NVDEC)
- **V4L2 Interface**: Leverages Video4Linux2 API for device access
- **Same Interface**: Compatible with existing compressor interfaces
- **Power Efficient**: Uses specialized hardware instead of CUDA cores
- **Real-time Capable**: Designed for embedded and real-time applications

## Architecture

### Hardware Components
- **NVENC**: Hardware video encoder (can handle JPEG)
- **NVDEC**: Hardware video decoder
- **VIC**: Video Image Compositor
- **V4L2**: Video4Linux2 driver interface

### Software Stack
```
Application Layer
    ↓
JetsonCompressor (C++)
    ↓
V4L2 API
    ↓
Jetson Multimedia Driver
    ↓
Hardware Encoder (NVENC/VIC)
```

## Usage

### Basic Usage
```cpp
#include "jetson_compressor.hpp"

// Initialize compressor
JetsonCompressor compressor;

// Compress image
std::vector<unsigned char> compressed = compressor.compress(
    rgb_data, width, height, channels, quality
);
```

### Quality Settings
```cpp
compressor.setQuality(90);  // 0-100
compressor.setChromaSubsampling(420);  // 420, 422, or 444
```

## Performance Comparison

| Compressor | Platform | Performance | Power Efficiency | Compatibility |
|------------|----------|-------------|------------------|---------------|
| OpenCV | Any | CPU-based | Low | High |
| nvJPEG | Desktop/Server | GPU-based | Medium | Jetson: No |
| Jetson API | Jetson only | Hardware | High | Jetson: Yes |

## Building

### Prerequisites
- Jetson platform (Orin, Xavier, Nano)
- JetPack 5.x or 6.x
- V4L2 development headers
- OpenCV

### Compilation
```bash
mkdir build && cd build
cmake ..
make test_jetson_pc
make test_compression_comparison
```

## Testing

### Individual Test
```bash
./test_jetson_pc input.png output.jpg
```

### Comparison Test
```bash
./test_compression_comparison input.png
```

## Device Detection

The compressor automatically detects available V4L2 devices:
- `/dev/video0`
- `/dev/video1` 
- `/dev/video2`

It checks for JPEG encoding capabilities using `VIDIOC_QUERYCAP`.

## Error Handling

Common errors and solutions:

1. **"No suitable JPEG encoder device found"**
   - Ensure running on Jetson platform
   - Check V4L2 device permissions
   - Verify JetPack installation

2. **"Failed to set format"**
   - Check image dimensions (must be supported by hardware)
   - Verify pixel format compatibility

3. **"Failed to queue/dequeue buffer"**
   - Check device state
   - Verify buffer allocation

## Limitations

- **Platform Specific**: Only works on Jetson devices
- **Format Support**: Limited to RGB input (3 channels)
- **Resolution**: Hardware-dependent maximum resolutions
- **Quality Range**: May not support full 0-100 quality range

## Advantages over nvJPEG

1. **Jetson Compatibility**: Works on Jetson platforms where nvJPEG doesn't
2. **Power Efficiency**: Uses dedicated hardware instead of CUDA cores
3. **Real-time Performance**: Optimized for embedded applications
4. **Lower Latency**: Direct hardware access without CUDA overhead

## Integration with Pipeline

The Jetson compressor can be used as a drop-in replacement for other compressors in the image processing pipeline:

```cpp
// In your pipeline
JetsonCompressor compressor;
auto compressed = compressor.compress(image_data, width, height, channels, quality);
// Use compressed data for encryption, transmission, etc.
```

## Future Enhancements

- Support for different input formats (YUV, etc.)
- Batch processing capabilities
- Quality optimization algorithms
- Integration with camera pipelines
- Support for different compression standards (JPEG 2000, etc.) 