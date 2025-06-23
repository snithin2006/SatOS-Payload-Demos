#pragma once

#include <vector>
#include <cstdint>
#include <string>

// Supported AES modes
enum class AESMode {
    ECB,
    CBC,
    CTR
};

class CudaEncryptor {
public:
    CudaEncryptor(const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv, AESMode mode);
    ~CudaEncryptor();

    // Encrypts the input buffer (e.g., JPEG bitstream) and returns the encrypted buffer
    std::vector<uint8_t> encrypt(const std::vector<uint8_t>& plaintext);

private:
    std::vector<uint8_t> key_;
    std::vector<uint8_t> iv_;
    AESMode mode_;

    // Device pointers
    uint8_t* d_key_;
    uint8_t* d_iv_;

    void allocateDeviceMemory();
    void freeDeviceMemory();
    void copyKeyAndIVToDevice();
}; 