#pragma once

#include <vector>
#include <string>
#include <stdexcept>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

class JetsonCompressor
{
public:
    JetsonCompressor();
    ~JetsonCompressor();

    // Initialize the compressor
    void initialize();

    // Set compression quality (0-100)
    void setQuality(int quality);

    // Set chroma subsampling (420, 422, 444)
    void setChromaSubsampling(int subsampling);

    // Main compression function with same interface as other compressors
    std::vector<unsigned char> compress(
        const unsigned char *input_data,
        int width,
        int height,
        int channels,
        int quality = 90);

    // Get compression status
    bool isInitialized() const;

private:
    bool initialized_;
    int quality_;
    int subsampling_;
    int device_fd_;

    // V4L2 structures for JPEG encoding
    struct v4l2_capability cap_;
    struct v4l2_format fmt_;
    struct v4l2_requestbuffers req_;
    struct v4l2_buffer buf_;

    // Buffer management
    unsigned char *input_buffer_;
    unsigned char *output_buffer_;
    size_t input_buffer_size_;
    size_t output_buffer_size_;

    // Helper functions
    void checkV4L2Error(int ret, const char *msg);
    void setupV4L2Device();
    void configureFormat(int width, int height);
    void allocateBuffers(int width, int height);
    void cleanup();
};