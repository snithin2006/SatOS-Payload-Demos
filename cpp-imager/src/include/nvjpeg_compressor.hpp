#pragma once

#include <nvjpeg.h>
#include <cuda_runtime.h>
#include <vector>
#include <string>
#include <stdexcept>

class NVJPEGCompressor
{
public:
    NVJPEGCompressor();
    ~NVJPEGCompressor();

    // Initialize the compressor
    void initialize();

    // Add more encoder parameters
    void setHuffmanTable(const std::string &table);
    void setChromaSubsampling(nvjpegChromaSubsampling_t subsampling);

    // Add planar format support
    void setInputFormat(nvjpegInputFormat_t format);

    // Add more detailed compression options
    std::vector<unsigned char> compress(
        const unsigned char *input_data,
        int width,
        int height,
        int channels,
        int quality = 90,
        nvjpegChromaSubsampling_t subsampling = NVJPEG_CSS_444);

    // Get compression status
    bool isInitialized() const;

private:
    nvjpegHandle_t handle_;
    nvjpegEncoderState_t encoder_state_;
    nvjpegEncoderParams_t encoder_params_;
    bool initialized_;

    // Helper functions
    void checkCudaError(cudaError_t error, const char *msg);
    void checkNVJPEGError(nvjpegStatus_t status, const char *msg);
    void cleanup();
};