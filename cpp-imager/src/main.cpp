#include "image_processor.hpp"
#include "nvjpeg_compressor.hpp"
#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_image> <output_jpeg>" << std::endl;
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

        // 4. Use this data with nvJPEG compressor
        NVJPEGCompressor compressor;
        auto compressed_data = compressor.compress(
            raw_data,
            width,
            height,
            channels,
            90  // quality
        );

        // 5. Save the compressed data
        std::ofstream outfile(argv[2], std::ios::binary);
        if (!outfile) {
            std::cerr << "Failed to open output file: " << argv[2] << std::endl;
            return 1;
        }

        outfile.write(reinterpret_cast<const char*>(compressed_data.data()),
                     compressed_data.size());

        std::cout << "Successfully compressed image to: " << argv[2] << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
