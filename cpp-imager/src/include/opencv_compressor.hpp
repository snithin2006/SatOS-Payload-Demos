#pragma once
#include <vector>
#include <opencv2/opencv.hpp>

class JpegCompressor {
public:
    JpegCompressor(int quality = 90);
    std::vector<uchar> compress(const unsigned char* rgb_data, int width, int height, int channels);
    void setQuality(int quality);
private:
    int quality_;
}; 