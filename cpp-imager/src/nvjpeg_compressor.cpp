#include "include/nvjpeg_compressor.hpp"
#include <iostream>

NVJPEGCompressor::NVJPEGCompressor() : initialized_(false) {
    initialize();
}

NVJPEGCompressor::~NVJPEGCompressor() {
    cleanup();
}

void NVJPEGCompressor::initialize() {
    try {
        // Create nvJPEG handle
        checkNVJPEGError(nvjpegCreate(NVJPEG_BACKEND_DEFAULT, NULL, &handle_),
                        "Failed to create nvJPEG handle");

        // Create encoder state
        checkNVJPEGError(nvjpegEncoderStateCreate(handle_, &encoder_state_),
                        "Failed to create encoder state");

        // Create encoder parameters
        checkNVJPEGError(nvjpegEncoderParamsCreate(handle_, &encoder_params_),
                        "Failed to create encoder parameters");

        initialized_ = true;
    } catch (const std::exception& e) {
        std::cerr << "Initialization error: " << e.what() << std::endl;
        cleanup();
        throw;
    }
}

std::vector<unsigned char> NVJPEGCompressor::compress(
    const unsigned char* input_data,
    int width,
    int height,
    int channels,
    int quality) {
    
    if (!initialized_) {
        throw std::runtime_error("Compressor not initialized");
    }

    try {
        // Set quality
        checkNVJPEGError(nvjpegEncoderParamsSetQuality(encoder_params_, quality, NULL),
                        "Failed to set quality");

        // Set sampling factors
        checkNVJPEGError(nvjpegEncoderParamsSetSamplingFactors(encoder_params_, NVJPEG_CSS_444, NULL),
                        "Failed to set sampling factors");

        // Create input image descriptor
        nvjpegImage_t input_image = {0};
        input_image.channel[0] = const_cast<unsigned char*>(input_data);
        input_image.pitch[0] = width * channels;

        // Get output buffer size
        size_t max_size = 0;
        checkNVJPEGError(nvjpegEncodeGetBufferSize(handle_, encoder_params_, width, height, &max_size),
                        "Failed to get buffer size");

        // Allocate output buffer
        std::vector<unsigned char> output_buffer(max_size);
        unsigned char* output_ptr = output_buffer.data();

        // Encode image
        checkNVJPEGError(nvjpegEncodeImage(handle_, encoder_state_, encoder_params_,
                                         &input_image, NVJPEG_INPUT_RGB,
                                         width, height, &output_ptr, &max_size),
                        "Failed to encode image");

        // Resize output buffer to actual size
        output_buffer.resize(max_size);
        return output_buffer;

    } catch (const std::exception& e) {
        std::cerr << "Compression error: " << e.what() << std::endl;
        throw;
    }
}

bool NVJPEGCompressor::isInitialized() const {
    return initialized_;
}

void NVJPEGCompressor::checkCudaError(cudaError_t error, const char* msg) {
    if (error != cudaSuccess) {
        throw std::runtime_error(std::string(msg) + ": " + cudaGetErrorString(error));
    }
}

void NVJPEGCompressor::checkNVJPEGError(nvjpegStatus_t status, const char* msg) {
    if (status != NVJPEG_STATUS_SUCCESS) {
        throw std::runtime_error(std::string(msg) + ": " + std::to_string(status));
    }
}

void NVJPEGCompressor::cleanup() {
    if (initialized_) {
        if (encoder_params_) {
            nvjpegEncoderParamsDestroy(encoder_params_);
            encoder_params_ = nullptr;
        }
        if (encoder_state_) {
            nvjpegEncoderStateDestroy(encoder_state_);
            encoder_state_ = nullptr;
        }
        if (handle_) {
            nvjpegDestroy(handle_);
            handle_ = nullptr;
        }
        initialized_ = false;
    }
} 