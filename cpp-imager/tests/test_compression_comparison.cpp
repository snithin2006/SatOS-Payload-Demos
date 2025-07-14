#include "opencv_processor.hpp"
#include "opencv_compressor.hpp"
#include "jetson_compressor.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>

// Function to measure compression time for JetsonCompressor

template <typename Compressor>
double measureCompressionTime(Compressor &compressor, const unsigned char *data,
                              int width, int height, int channels, int quality, int iterations = 10)
{
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i)
    {
        try
        {
            auto result = compressor.compress(data, width, height, channels, quality);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Compression failed: " << e.what() << std::endl;
            return -1.0;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    return duration.count() / (double)iterations; // Average time in microseconds
}

// Overload for JpegCompressor (OpenCV)
double measureCompressionTime(JpegCompressor &compressor, const unsigned char *data,
                              int width, int height, int channels, int /*quality*/, int iterations = 10)
{
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i)
    {
        try
        {
            auto result = compressor.compress(data, width, height, channels);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Compression failed: " << e.what() << std::endl;
            return -1.0;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    return duration.count() / (double)iterations; // Average time in microseconds
}

// Function to save compressed data to file
void saveCompressedData(const std::vector<unsigned char> &data, const std::string &filename)
{
    std::ofstream file(filename, std::ios::binary);
    if (file.is_open())
    {
        file.write(reinterpret_cast<const char *>(data.data()), data.size());
        file.close();
        std::cout << "Saved compressed data to: " << filename << " (size: " << data.size() << " bytes)" << std::endl;
    }
    else
    {
        std::cerr << "Failed to save file: " << filename << std::endl;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <input_image>" << std::endl;
        return 1;
    }

    try
    {
        // Load and preprocess image
        ImageProcessor processor;
        if (!processor.loadImage(argv[1]))
        {
            std::cerr << "Failed to load image: " << argv[1] << std::endl;
            return 1;
        }

        processor.convertBGRtoRGB();

        const unsigned char *raw_data = processor.getRawPixelData();
        int width = processor.getWidth();
        int height = processor.getHeight();
        int channels = processor.getChannels();

        std::cout << "Image dimensions: " << width << "x" << height << " (channels: " << channels << ")" << std::endl;
        std::cout << "Original size: " << (width * height * channels) << " bytes" << std::endl;

        // Test different quality levels
        std::vector<int> qualities = {50, 75, 90};

        for (int quality : qualities)
        {
            std::cout << "\n=== Testing Quality Level: " << quality << " ===" << std::endl;

            // Test OpenCV Compressor
            std::cout << "\n1. OpenCV Compressor:" << std::endl;
            try
            {
                JpegCompressor opencv_compressor(quality);
                double opencv_time = measureCompressionTime(opencv_compressor, raw_data, width, height, channels, quality);

                if (opencv_time > 0)
                {
                    std::cout << "   Average compression time: " << std::fixed << std::setprecision(2)
                              << opencv_time << " microseconds" << std::endl;

                    auto opencv_result = opencv_compressor.compress(raw_data, width, height, channels);
                    std::cout << "   Compressed size: " << opencv_result.size() << " bytes" << std::endl;
                    std::cout << "   Compression ratio: " << std::fixed << std::setprecision(2)
                              << (100.0 * opencv_result.size() / (width * height * channels)) << "%" << std::endl;

                    saveCompressedData(opencv_result, "opencv_compressed_q" + std::to_string(quality) + ".jpg");
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "   OpenCV compression failed: " << e.what() << std::endl;
            }

            // Test Jetson Compressor
            std::cout << "\n2. Jetson Multimedia API Compressor:" << std::endl;
            try
            {
                JetsonCompressor jetson_compressor;
                if (jetson_compressor.isInitialized())
                {
                    double jetson_time = measureCompressionTime(jetson_compressor, raw_data, width, height, channels, quality);

                    if (jetson_time > 0)
                    {
                        std::cout << "   Average compression time: " << std::fixed << std::setprecision(2)
                                  << jetson_time << " microseconds" << std::endl;

                        auto jetson_result = jetson_compressor.compress(raw_data, width, height, channels, quality);
                        std::cout << "   Compressed size: " << jetson_result.size() << " bytes" << std::endl;
                        std::cout << "   Compression ratio: " << std::fixed << std::setprecision(2)
                                  << (100.0 * jetson_result.size() / (width * height * channels)) << "%" << std::endl;

                        saveCompressedData(jetson_result, "jetson_compressed_q" + std::to_string(quality) + ".jpg");
                    }
                }
                else
                {
                    std::cout << "   Jetson compressor not initialized" << std::endl;
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "   Jetson compression failed: " << e.what() << std::endl;
            }
        }

        std::cout << "\n=== Compression Comparison Complete ===" << std::endl;
        std::cout << "Check the generated .jpg files to compare visual quality." << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}