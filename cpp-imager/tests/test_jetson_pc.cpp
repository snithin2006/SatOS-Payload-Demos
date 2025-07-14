#include "opencv_processor.hpp"
#include "jetson_compressor.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <input_image> <output_jpeg>" << std::endl;
        return 1;
    }

    try
    {
        // 1. Load image using OpenCV
        cv::Mat img = cv::imread(argv[1], cv::IMREAD_UNCHANGED);
        if (img.empty())
        {
            std::cerr << "Failed to load image: " << argv[1] << std::endl;
            return 1;
        }
        std::cout << "Loaded image: " << argv[1] << std::endl;

        // 2. Convert to RGB format
        if (img.channels() == 4)
        {
            cv::cvtColor(img, img, cv::COLOR_BGRA2BGR);
        }
        if (img.channels() == 1)
        {
            cv::cvtColor(img, img, cv::COLOR_GRAY2BGR);
        }
        cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
        std::cout << "Converted to RGB format." << std::endl;

        // 3. Ensure image is 8-bit, 3-channel
        if (img.type() != CV_8UC3)
        {
            img.convertTo(img, CV_8UC3);
        }

        // 4. Get image data
        int width = img.cols;
        int height = img.rows;
        int channels = img.channels();
        const unsigned char *raw_data = img.data;

        std::cout << "Image dimensions: " << width << "x" << height
                  << " (channels: " << channels << ")" << std::endl;

        // 5. Initialize Jetson compressor
        JetsonCompressor compressor;
        if (!compressor.isInitialized())
        {
            std::cerr << "Failed to initialize Jetson compressor" << std::endl;
            return 1;
        }
        std::cout << "Jetson compressor initialized successfully." << std::endl;

        // 6. Compress image
        std::vector<unsigned char> compressed_data = compressor.compress(raw_data, width, height, channels, 90);
        std::cout << "Compression completed. Size: " << compressed_data.size() << " bytes" << std::endl;

        // 7. Save compressed data
        std::ofstream outfile(argv[2], std::ios::binary);
        if (!outfile)
        {
            std::cerr << "Failed to open output file: " << argv[2] << std::endl;
            return 1;
        }
        outfile.write(reinterpret_cast<const char *>(compressed_data.data()), compressed_data.size());
        outfile.close();

        std::cout << "Successfully saved compressed image to: " << argv[2] << std::endl;
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}