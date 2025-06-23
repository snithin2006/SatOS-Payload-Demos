#include "image_processor.hpp"
#include "include/cuda_encryptor.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <opencv2/opencv.hpp>

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <input_image> <output_jpeg> <output_enc>" << std::endl;
        return 1;
    }

    try {
        // 1. Create and load image
        ImageProcessor processor;
        if (!processor.loadImage(argv[1])) {
            std::cerr << "Failed to load image: " << argv[1] << std::endl;
            return 1;
        }

        // 2. Convert to RGB (needed for nvJPEG)
        processor.convertBGRtoRGB();

        // 3. Get the raw pixel data and dimensions
        const unsigned char* raw_data = processor.getRawPixelData();
        int width = processor.getWidth();
        int height = processor.getHeight();
        int channels = processor.getChannels();

        // 4. Compress with OpenCV
        std::vector<uchar> compressed_data;
        std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 90};
        cv::Mat rgb_image(height, width, CV_8UC3, const_cast<unsigned char*>(raw_data));
        if (!cv::imencode(".jpg", rgb_image, compressed_data, params)) {
            std::cerr << "Failed to compress image with OpenCV." << std::endl;
            return 1;
        }

        // 5. Save the compressed JPEG
        std::ofstream outfile(argv[2], std::ios::binary);
        if (!outfile) {
            std::cerr << "Failed to open output file: " << argv[2] << std::endl;
            return 1;
        }
        outfile.write(reinterpret_cast<const char*>(compressed_data.data()), compressed_data.size());
        outfile.close();

        // 6. Prepare AES key and IV (for demo, generate random 256-bit key and 128-bit IV)
        std::vector<uint8_t> key(32);
        std::vector<uint8_t> iv(16);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<uint8_t> dis(0, 255);
        for (auto& b : key) b = dis(gen);
        for (auto& b : iv) b = dis(gen);

        // 7. Encrypt the JPEG bitstream
        CudaEncryptor encryptor(key, iv, AESMode::CBC);
        std::vector<uint8_t> encrypted = encryptor.encrypt(compressed_data);

        // 8. Save the encrypted output
        std::ofstream encfile(argv[3], std::ios::binary);
        if (!encfile) {
            std::cerr << "Failed to open encrypted output file: " << argv[3] << std::endl;
            return 1;
        }
        encfile.write(reinterpret_cast<const char*>(encrypted.data()), encrypted.size());
        encfile.close();

        std::cout << "Successfully compressed and encrypted image.\n";
        std::cout << "JPEG output: " << argv[2] << std::endl;
        std::cout << "Encrypted output: " << argv[3] << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
} 