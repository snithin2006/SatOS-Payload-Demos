#include "opencv_compressor.hpp"
#include <stdexcept>

JpegCompressor::JpegCompressor(int quality) : quality_(quality) {}

void JpegCompressor::setQuality(int quality)
{
    quality_ = quality;
}

std::vector<uchar> JpegCompressor::compress(const unsigned char *rgb_data, int width, int height, int channels)
{
    (void)channels;
    std::vector<uchar> compressed_data;
    std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, quality_};
    cv::Mat rgb_image(height, width, CV_8UC3, const_cast<unsigned char *>(rgb_data));
    if (!cv::imencode(".jpg", rgb_image, compressed_data, params))
    {
        throw std::runtime_error("Failed to compress image with OpenCV.");
    }
    return compressed_data;
}