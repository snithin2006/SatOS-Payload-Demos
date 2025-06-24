#pragma once

#include <cstdint>
#include <vector>
#include <cuda_runtime.h>
#include <stdexcept>
#include <string>

// AES constants
#define AES_BLOCK_SIZE 16
#define AES_256_KEY_SIZE 32
#define AES_256_ROUNDS 14

// AES-256 CUDA library functions
namespace AESCudaLib {
    
    // Initialize AES constants on GPU
    void initializeAESConstants();
    
    // Key expansion for AES-256
    void expandKey(const uint8_t* key, uint32_t* expanded_key);
    
    // Copy expanded key to GPU
    void copyExpandedKeyToGPU(const uint32_t* expanded_key);
    
    // AES-256 ECB encryption kernel (single block)
    __global__ void aes256_ecb_encrypt_kernel(uint8_t* data, size_t num_blocks);
    
    // AES-256 CBC encryption kernel
    __global__ void aes256_cbc_encrypt_kernel(uint8_t* data, const uint8_t* iv, size_t num_blocks);
    
    // Helper function to encrypt data with AES-256
    std::vector<uint8_t> encryptAES256(const std::vector<uint8_t>& plaintext, 
                                      const std::vector<uint8_t>& key,
                                      const std::vector<uint8_t>& iv,
                                      bool use_cbc = true);
    
    // Cleanup GPU memory
    void cleanup();
}

// Error checking macro
#define CUDA_CHECK(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char *file, int line) {
    if (code != cudaSuccess) {
        throw std::runtime_error(std::string("CUDA Error: ") + cudaGetErrorString(code) + " at " + file + ":" + std::to_string(line));
    }
} 