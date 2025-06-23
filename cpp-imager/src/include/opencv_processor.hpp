#pragma once

#include <opencv2/opencv.hpp>
#include <string>
#include <stdexcept>

class ImageProcessor {
public:
    ImageProcessor() = default;
    ~ImageProcessor() = default;

    // 1. Load Image
    // Input: Filepath and optional flags
    // Returns: true if successful, false otherwise
    bool loadImage(const std::string& filepath, int flags = cv::IMREAD_COLOR);

    // 2. Get Raw Pixel Buffer
    // Returns: Pointer to raw pixel data (mat.data)
    const unsigned char* getRawPixelData() const;
    
    // Get image dimensions (mat.cols and mat.rows)
    int getWidth() const;    // mat.cols
    int getHeight() const;   // mat.rows
    int getChannels() const; // number of color channels (3 for BGR)

    // 3. Format Conversion
    // Converts BGR to RGB for nvJPEG compatibility
    void convertBGRtoRGB();

private:
    cv::Mat image_;  // Original image loaded from disk (BGR format)
    cv::Mat rgb_image_;  // RGB converted image
    
    // Helper function to validate image after loading
    void validateImage() const;
};
