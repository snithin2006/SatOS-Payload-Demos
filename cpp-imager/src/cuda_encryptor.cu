#include "cuda_encryptor.hpp"
#include "aes_cuda_lib.hpp"
#include <stdexcept>
#include <vector>

// constructor for CudaEncryptor class
// args: key, iv, mode (ECB, CBC)
CudaEncryptor::CudaEncryptor(const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv, AESMode mode)
    : key_(key), iv_(iv), mode_(mode) {

    // validate key and iv sizes
    if (key_.size() != 32) {
        throw std::runtime_error("Key must be 32 bytes for AES-256");
    }
    if (iv_.size() != 16) {
        throw std::runtime_error("IV must be 16 bytes");
    }

    // initialize constants for GPU
    AESCudaLib::initializeAESConstants();
}

// destructor for CudaEncryptor class
CudaEncryptor::~CudaEncryptor() = default;

// encrypt function for CudaEncryptor class
// args: plaintext
// returns: ciphertext
std::vector<uint8_t> CudaEncryptor::encrypt(const std::vector<uint8_t>& plaintext) {
    try {
        // determine if CBC or ECB mode is used
        bool use_cbc = mode_ == AESMode::CBC;

        // encrypt the plaintext given mode
        return AESCudaLib::encryptAES256(plaintext, key_, iv_, use_cbc);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("AES encryption failed: ") + e.what());
    }
} 