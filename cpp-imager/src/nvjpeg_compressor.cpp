#include "nvjpeg_compressor.hpp"
#include <iostream>

NVJPEGCompressor::NVJPEGCompressor() : initialized_(false)
{
    initialize();
}

NVJPEGCompressor::~NVJPEGCompressor()
{
    cleanup();
}

void NVJPEGCompressor::initialize()
{
    try
    {
        // Create nvJPEG handle
        checkNVJPEGError(nvjpegCreate(NVJPEG_BACKEND_DEFAULT, NULL, &handle_),
                         "Failed to create nvJPEG handle");

        // Create encoder state (add cudaStream_t argument)
        checkNVJPEGError(nvjpegEncoderStateCreate(handle_, &encoder_state_, 0),
                         "Failed to create encoder state");

        // Create encoder parameters (add cudaStream_t argument)
        checkNVJPEGError(nvjpegEncoderParamsCreate(handle_, &encoder_params_, 0),
                         "Failed to create encoder parameters");

        initialized_ = true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Initialization error: " << e.what() << std::endl;
        cleanup();
        throw;
    }
}

std::vector<unsigned char> NVJPEGCompressor::compress(
    const unsigned char *input_data,
    int width,
    int height,
    int channels,
    int quality,
    nvjpegChromaSubsampling_t subsampling)
{
    if (!initialized_)
    {
        throw std::runtime_error("Compressor not initialized");
    }

    try
    {
        // Set quality
        checkNVJPEGError(nvjpegEncoderParamsSetQuality(encoder_params_, quality, NULL),
                         "Failed to set quality");

        // Set sampling factors (use the subsampling argument)
        checkNVJPEGError(nvjpegEncoderParamsSetSamplingFactors(encoder_params_, subsampling, NULL),
                         "Failed to set sampling factors");

        // Create input image descriptor
        nvjpegImage_t input_image;
        for (int i = 0; i < NVJPEG_MAX_COMPONENT; ++i)
        {
            input_image.channel[i] = nullptr;
            input_image.pitch[i] = 0;
        }
        input_image.channel[0] = const_cast<unsigned char *>(input_data);
        input_image.pitch[0] = width * channels;

        // Print all channels and pitches before encoding
        std::cout << "nvjpegImage_t setup before encoding:" << std::endl;
        for (int i = 0; i < NVJPEG_MAX_COMPONENT; ++i)
        {
            std::cout << "  channel[" << i << "]: " << static_cast<void *>(input_image.channel[i])
                      << ", pitch[" << i << "]: " << input_image.pitch[i] << std::endl;
        }

        // Encode image (no output buffer here, just pass 0 as stream)
        checkNVJPEGError(nvjpegEncodeImage(handle_, encoder_state_, encoder_params_,
                                           &input_image, NVJPEG_INPUT_RGB,
                                           width, height, 0),
                         "Failed to encode image");

        // Retrieve the bitstream
        size_t bitstream_length = 0;
        checkNVJPEGError(nvjpegEncodeRetrieveBitstream(handle_, encoder_state_, NULL, &bitstream_length, 0),
                         "Failed to get bitstream length");

        std::vector<unsigned char> output_buffer(bitstream_length);
        checkNVJPEGError(nvjpegEncodeRetrieveBitstream(handle_, encoder_state_, output_buffer.data(), &bitstream_length, 0),
                         "Failed to retrieve bitstream");

        output_buffer.resize(bitstream_length);
        return output_buffer;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Compression error: " << e.what() << std::endl;
        throw;
    }
}

bool NVJPEGCompressor::isInitialized() const
{
    return initialized_;
}

void NVJPEGCompressor::checkCudaError(cudaError_t error, const char *msg)
{
    if (error != cudaSuccess)
    {
        throw std::runtime_error(std::string(msg) + ": " + cudaGetErrorString(error));
    }
}

void NVJPEGCompressor::checkNVJPEGError(nvjpegStatus_t status, const char *msg)
{
    if (status != NVJPEG_STATUS_SUCCESS)
    {
        throw std::runtime_error(std::string(msg) + ": " + std::to_string(status));
    }
}

void NVJPEGCompressor::cleanup()
{
    if (initialized_)
    {
        if (encoder_params_)
        {
            nvjpegEncoderParamsDestroy(encoder_params_);
            encoder_params_ = nullptr;
        }
        if (encoder_state_)
        {
            nvjpegEncoderStateDestroy(encoder_state_);
            encoder_state_ = nullptr;
        }
        if (handle_)
        {
            nvjpegDestroy(handle_);
            handle_ = nullptr;
        }
        initialized_ = false;
    }
}