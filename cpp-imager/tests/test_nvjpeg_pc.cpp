#include "opencv_processor.hpp"
#include "nvjpeg_compressor.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cuda_runtime.h>

int main(int argc, char *argv[])
{
    // Explicitly set CUDA device to 0
    cudaError_t err = cudaSetDevice(0);
    if (err != cudaSuccess)
    {
        std::cerr << "Failed to set CUDA device: " << cudaGetErrorString(err) << std::endl;
        return 1;
    }
    std::cout << "CUDA device set to 0." << std::endl;

    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <input_image> <output_jpeg>" << std::endl;
        return 1;
    }

    try
    {
        // 1. Load image using OpenCV (force 8-bit, 3-channel)
        cv::Mat img = cv::imread(argv[1], cv::IMREAD_UNCHANGED);
        if (img.empty())
        {
            std::cerr << "Failed to load image: " << argv[1] << std::endl;
            return 1;
        }
        std::cout << "Loaded image: " << argv[1] << std::endl;
        std::cout << "Original OpenCV type: " << img.type() << ", channels: " << img.channels() << std::endl;

        // If image has alpha channel (4 channels), convert to 3-channel BGR
        if (img.channels() == 4)
        {
            cv::cvtColor(img, img, cv::COLOR_BGRA2BGR);
            std::cout << "Converted RGBA to BGR." << std::endl;
        }
        // If image is grayscale, convert to BGR
        if (img.channels() == 1)
        {
            cv::cvtColor(img, img, cv::COLOR_GRAY2BGR);
            std::cout << "Converted grayscale to BGR." << std::endl;
        }
        // Convert BGR to RGB
        cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
        std::cout << "Converted BGR to RGB." << std::endl;

        // Ensure image is 8-bit, 3-channel
        if (img.type() != CV_8UC3)
        {
            img.convertTo(img, CV_8UC3);
            std::cout << "Converted image to CV_8UC3." << std::endl;
        }

        // Ensure data is contiguous
        if (!img.isContinuous())
        {
            img = img.clone();
            std::cout << "Cloned image to ensure contiguous data." << std::endl;
        }

        // Check image format: CV_8UC3, 3 channels, contiguous
        assert(img.type() == CV_8UC3 && img.channels() == 3 && img.isContinuous());
        std::cout << "Image is CV_8UC3, 3 channels, and contiguous." << std::endl;

        // Crop to a small region (e.g., 64x64 from top-left corner)
        cv::Rect roi(0, 0, 64, 64);
        img = img(roi).clone();
        std::cout << "Cropped image to 64x64 for small image test." << std::endl;

        int width = img.cols;
        int height = img.rows;
        int channels = img.channels();
        const unsigned char *raw_data = img.data;

        std::cout << "Image dimensions: " << width << "x" << height
                  << " (channels: " << channels << ")" << std::endl;

        // Print 10 pixels from the center row
        int center_row = height / 2;
        int start_col = width / 2 - 5;
        std::cout << "10 pixels from the center row:" << std::endl;
        for (int i = 0; i < 10; ++i)
        {
            int col = start_col + i;
            int idx = (center_row * width + col) * 3;
            unsigned char r = raw_data[idx];
            unsigned char g = raw_data[idx + 1];
            unsigned char b = raw_data[idx + 2];
            std::cout << "Pixel (" << center_row << "," << col << "): "
                      << "R=" << static_cast<unsigned int>(r) << " "
                      << "G=" << static_cast<unsigned int>(g) << " "
                      << "B=" << static_cast<unsigned int>(b) << std::endl;
        }

        // Print NVJPEG parameters
        std::cout << "NVJPEG parameters:" << std::endl;
        std::cout << "  width: " << width << std::endl;
        std::cout << "  height: " << height << std::endl;
        std::cout << "  channels: " << channels << std::endl;
        std::cout << "  pitch[0]: " << width * 3 << std::endl;
        std::cout << "  channel[0] ptr: " << static_cast<const void *>(raw_data) << std::endl;

        // 4. Use NVJPEGCompressor to compress to JPEG
        std::cout << "Initializing NVJPEG compressor..." << std::endl;
        NVJPEGCompressor compressor;

        if (!compressor.isInitialized())
        {
            std::cerr << "Failed to initialize NVJPEG compressor" << std::endl;
            return 1;
        }
        std::cout << "NVJPEG compressor initialized successfully!" << std::endl;

        // Compress with quality 90
        int quality = 90;
        std::cout << "Compressing with quality " << quality << "..." << std::endl;
        std::vector<unsigned char> compressed_data = compressor.compress(
            raw_data, width, height, channels, quality, NVJPEG_CSS_420);

        // 5. Save the compressed data
        std::ofstream outfile(argv[2], std::ios::binary);
        if (!outfile)
        {
            std::cerr << "Failed to open output file: " << argv[2] << std::endl;
            return 1;
        }

        outfile.write(reinterpret_cast<const char *>(compressed_data.data()),
                      compressed_data.size());

        // Calculate compression stats
        size_t original_size = width * height * channels;
        double compression_ratio = static_cast<double>(original_size) / compressed_data.size();
        double compression_percentage = (1.0 - static_cast<double>(compressed_data.size()) / original_size) * 100.0;

        std::cout << "Successfully compressed image to: " << argv[2] << std::endl;
        std::cout << "Original size: " << original_size << " bytes" << std::endl;
        std::cout << "Compressed size: " << compressed_data.size() << " bytes" << std::endl;
        std::cout << "Compression ratio: " << std::fixed << std::setprecision(2) << compression_ratio << ":1" << std::endl;
        std::cout << "Compression percentage: " << std::fixed << std::setprecision(1) << compression_percentage << "%" << std::endl;

        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}