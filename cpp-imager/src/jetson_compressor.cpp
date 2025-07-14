#include "jetson_compressor.hpp"
#include <iostream>
#include <cstring>
#include <errno.h>

JetsonCompressor::JetsonCompressor()
    : initialized_(false), quality_(90), subsampling_(V4L2_JPEG_CHROMA_SUBSAMPLING_420),
      device_fd_(-1), input_buffer_(nullptr), output_buffer_(nullptr),
      input_buffer_size_(0), output_buffer_size_(0)
{
    initialize();
}

JetsonCompressor::~JetsonCompressor()
{
    cleanup();
}

void JetsonCompressor::initialize()
{
    try
    {
        setupV4L2Device();
        initialized_ = true;
        std::cout << "Jetson compressor initialized successfully" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Initialization error: " << e.what() << std::endl;
        cleanup();
        throw;
    }
}

void JetsonCompressor::setQuality(int quality)
{
    if (quality < 0 || quality > 100)
    {
        throw std::invalid_argument("Quality must be between 0 and 100");
    }
    quality_ = quality;
}

void JetsonCompressor::setChromaSubsampling(int subsampling)
{
    switch (subsampling)
    {
    case 420:
        subsampling_ = V4L2_JPEG_CHROMA_SUBSAMPLING_420;
        break;
    case 422:
        subsampling_ = V4L2_JPEG_CHROMA_SUBSAMPLING_422;
        break;
    case 444:
        subsampling_ = V4L2_JPEG_CHROMA_SUBSAMPLING_444;
        break;
    default:
        throw std::invalid_argument("Invalid subsampling value");
    }
}

std::vector<unsigned char> JetsonCompressor::compress(
    const unsigned char *input_data,
    int width,
    int height,
    int channels,
    int quality)
{
    if (!initialized_)
    {
        throw std::runtime_error("Compressor not initialized");
    }

    if (channels != 3)
    {
        throw std::invalid_argument("Only 3-channel RGB images supported");
    }

    try
    {
        // Set quality if different from current
        if (quality != quality_)
        {
            setQuality(quality);
        }

        // Configure format and allocate buffers
        configureFormat(width, height);
        allocateBuffers(width, height);

        // Copy input data to input buffer
        memcpy(input_buffer_, input_data, input_buffer_size_);

        // Set up buffer for encoding
        memset(&buf_, 0, sizeof(buf_));
        buf_.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        buf_.memory = V4L2_MEMORY_MMAP;
        buf_.index = 0;
        buf_.m.planes = new v4l2_plane[1];
        buf_.length = 1;
        buf_.m.planes[0].m.userptr = (unsigned long)input_buffer_;
        buf_.m.planes[0].length = input_buffer_size_;
        buf_.m.planes[0].bytesused = input_buffer_size_;

        // Queue the buffer
        checkV4L2Error(ioctl(device_fd_, VIDIOC_QBUF, &buf_), "Failed to queue input buffer");

        // Start streaming
        int type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        checkV4L2Error(ioctl(device_fd_, VIDIOC_STREAMON, &type), "Failed to start streaming");

        // Dequeue output buffer
        memset(&buf_, 0, sizeof(buf_));
        buf_.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        buf_.memory = V4L2_MEMORY_MMAP;
        buf_.index = 0;
        buf_.m.planes = new v4l2_plane[1];
        buf_.length = 1;

        checkV4L2Error(ioctl(device_fd_, VIDIOC_DQBUF, &buf_), "Failed to dequeue output buffer");

        // Stop streaming
        checkV4L2Error(ioctl(device_fd_, VIDIOC_STREAMOFF, &type), "Failed to stop streaming");

        // Copy compressed data
        size_t compressed_size = buf_.m.planes[0].bytesused;
        std::vector<unsigned char> result(output_buffer_, output_buffer_ + compressed_size);

        // Clean up
        delete[] buf_.m.planes;

        return result;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Compression error: " << e.what() << std::endl;
        throw;
    }
}

bool JetsonCompressor::isInitialized() const
{
    return initialized_;
}

void JetsonCompressor::checkV4L2Error(int ret, const char *msg)
{
    if (ret < 0)
    {
        throw std::runtime_error(std::string(msg) + ": " + strerror(errno));
    }
}

void JetsonCompressor::setupV4L2Device()
{
    // Try to open the JPEG encoder device
    // On Jetson, this is typically /dev/video0 or /dev/video1
    const char *devices[] = {"/dev/video0", "/dev/video1", "/dev/video2"};

    for (const char *device : devices)
    {
        device_fd_ = open(device, O_RDWR);
        if (device_fd_ >= 0)
        {
            // Check if this device supports JPEG encoding
            if (ioctl(device_fd_, VIDIOC_QUERYCAP, &cap_) == 0)
            {
                if (cap_.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
                {
                    std::cout << "Found JPEG encoder device: " << device << std::endl;
                    return;
                }
            }
            close(device_fd_);
            device_fd_ = -1;
        }
    }

    throw std::runtime_error("No suitable JPEG encoder device found");
}

void JetsonCompressor::configureFormat(int width, int height)
{
    memset(&fmt_, 0, sizeof(fmt_));
    fmt_.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    fmt_.fmt.pix_mp.width = width;
    fmt_.fmt.pix_mp.height = height;
    fmt_.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_JPEG;
    fmt_.fmt.pix_mp.field = V4L2_FIELD_NONE;
    fmt_.fmt.pix_mp.num_planes = 1;
    fmt_.fmt.pix_mp.plane_fmt[0].bytesperline = width * 3; // RGB
    fmt_.fmt.pix_mp.plane_fmt[0].sizeimage = width * height * 3;

    checkV4L2Error(ioctl(device_fd_, VIDIOC_S_FMT, &fmt_), "Failed to set format");
}

void JetsonCompressor::allocateBuffers(int width, int height)
{
    // Request buffers
    memset(&req_, 0, sizeof(req_));
    req_.count = 1;
    req_.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    req_.memory = V4L2_MEMORY_MMAP;

    checkV4L2Error(ioctl(device_fd_, VIDIOC_REQBUFS, &req_), "Failed to request buffers");

    // Allocate input buffer
    input_buffer_size_ = width * height * 3; // RGB
    input_buffer_ = new unsigned char[input_buffer_size_];

    // Allocate output buffer (JPEG compressed data)
    output_buffer_size_ = width * height * 3; // Maximum possible size
    output_buffer_ = new unsigned char[output_buffer_size_];
}

void JetsonCompressor::cleanup()
{
    if (device_fd_ >= 0)
    {
        close(device_fd_);
        device_fd_ = -1;
    }

    if (input_buffer_)
    {
        delete[] input_buffer_;
        input_buffer_ = nullptr;
    }

    if (output_buffer_)
    {
        delete[] output_buffer_;
        output_buffer_ = nullptr;
    }

    initialized_ = false;
}