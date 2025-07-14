#include "opencv_processor.hpp"
#include "cuda_encryptor.hpp"
#include "opencv_compressor.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <opencv2/opencv.hpp>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

std::string run_python_cloud_filter(const std::string &image_path)
{
    std::string cmd = "python3 ../tests/cloud_filter.py \"" + image_path + "\"";
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe)
        throw std::runtime_error("popen() failed!");
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
    // Remove trailing newline
    if (!result.empty() && result.back() == '\n')
        result.pop_back();
    return result;
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <input_image> <output_jpeg> <output_enc>" << std::endl;
        return 1;
    }

    try
    {
        // Cloud filter check BEFORE any processing
        std::string image_path = argv[1];
        std::string cloud_status = run_python_cloud_filter(image_path);
        if (cloud_status == "cloudy")
        {
            std::cout << "Image is too cloudy, skipping processing.\n";
            return 0;
        }

        // 1. Create and load image
        ImageProcessor processor;
        if (!processor.loadImage(argv[1]))
        {
            std::cerr << "Failed to load image: " << argv[1] << std::endl;
            return 1;
        }

        // 2. Convert to RGB (needed for nvJPEG)
        processor.convertBGRtoRGB();

        // 3. Get the raw pixel data and dimensions
        const unsigned char *raw_data = processor.getRawPixelData();
        int width = processor.getWidth();
        int height = processor.getHeight();
        int channels = processor.getChannels();

        // 4. Compress with JpegCompressor
        JpegCompressor compressor(90);
        std::vector<uchar> compressed_data = compressor.compress(raw_data, width, height, channels);

        // 5. Save the compressed JPEG
        std::ofstream outfile(argv[2], std::ios::binary);
        if (!outfile)
        {
            std::cerr << "Failed to open output file: " << argv[2] << std::endl;
            return 1;
        }
        outfile.write(reinterpret_cast<const char *>(compressed_data.data()), compressed_data.size());
        outfile.close();

        // 6. Prepare AES key and IV (for demo, generate random 256-bit key and 128-bit IV)
        std::vector<uint8_t> key(32);
        std::vector<uint8_t> iv(16);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<uint8_t> dis(0, 255);
        for (auto &b : key)
            b = dis(gen);
        for (auto &b : iv)
            b = dis(gen);

        // 7. Encrypt the JPEG bitstream
        CudaEncryptor encryptor(key, iv, AESMode::CBC);
        std::vector<uint8_t> encrypted = encryptor.encrypt(compressed_data);

        // 8. Save the encrypted output
        std::ofstream encfile(argv[3], std::ios::binary);
        if (!encfile)
        {
            std::cerr << "Failed to open encrypted output file: " << argv[3] << std::endl;
            return 1;
        }
        encfile.write(reinterpret_cast<const char *>(encrypted.data()), encrypted.size());
        encfile.close();

        std::cout << "Successfully compressed and encrypted image.\n";
        std::cout << "JPEG output: " << argv[2] << std::endl;
        std::cout << "Encrypted output: " << argv[3] << std::endl;

        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}