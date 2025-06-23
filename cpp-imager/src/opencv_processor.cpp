#include "opencv_processor.hpp"
#include <iostream>

bool ImageProcessor::loadImage(const std::string& filepath, int flags) {
    try {
        // a. Function: cv::imread(filepath, flags)
        // b. Reads an image from disk
        // c. Decodes it into a cv::Mat object
        image_ = cv::imread(filepath, flags);
        
        if (image_.empty()) {
            throw std::runtime_error("Failed to load image: " + filepath);
        }

        // Validate the image
        validateImage();
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading image: " << e.what() << std::endl;
        return false;
    }
}

const unsigned char* ImageProcessor::getRawPixelData() const {
    // a. Access pixel data via mat.data
    return image_.data;
}

int ImageProcessor::getWidth() const {
    // b. Image dimensions from mat.cols
    return image_.cols;
}

int ImageProcessor::getHeight() const {
    // b. Image dimensions from mat.rows
    return image_.rows;
}

int ImageProcessor::getChannels() const {
    // c. Interleaved BGR format by default (3 channels: Blue, Green, Red)
    return image_.channels();
}

void ImageProcessor::convertBGRtoRGB() {
    // a. Function: cv::cvtColor()
    // b. BGR to RGB for compatibility with nvJPEG input expectations
    cv::cvtColor(image_, rgb_image_, cv::COLOR_BGR2RGB);
}

void ImageProcessor::validateImage() const {
    if (image_.empty()) {
        throw std::runtime_error("Image is empty");
    }
    
    // Ensure we have a 3-channel BGR image
    if (image_.channels() != 3) {
        throw std::runtime_error("Image must be 3-channel (BGR)");
    }
}
