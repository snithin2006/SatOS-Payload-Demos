#include "opencv_processor.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <image_path>" << std::endl;
        return 1;
    }

    ImageProcessor processor;
    
    // Load the image
    if (!processor.loadImage(argv[1])) {
        std::cerr << "Failed to load image: " << argv[1] << std::endl;
        return 1;
    }

    // Get image dimensions
    int width = processor.getWidth();
    int height = processor.getHeight();
    int channels = processor.getChannels();

    std::cout << "Image dimensions: " << width << "x" << height 
              << " (channels: " << channels << ")" << std::endl;

    // Get raw pixel data
    const unsigned char* pixelData = processor.getRawPixelData();
    
    // Print first few pixels (first row) as an example
    std::cout << "\nFirst row pixel values (BGR format):" << std::endl;
    for (int x = 0; x < std::min(width, 5); x++) {
        std::cout << "Pixel " << x << ": ";
        for (int c = 0; c < channels; c++) {
            std::cout << static_cast<int>(pixelData[x * channels + c]) << " ";
        }
        std::cout << std::endl;
    }

    return 0;
} 