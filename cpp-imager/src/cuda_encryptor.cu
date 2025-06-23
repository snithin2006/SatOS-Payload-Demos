#include "include/cuda_encryptor.hpp"
#include <cuda_runtime.h>
#include <stdexcept>
#include <cstring>

CudaEncryptor::CudaEncryptor(const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv, AESMode mode)
    : key_(key), iv_(iv), mode_(mode), d_key_(nullptr), d_iv_(nullptr) {
    allocateDeviceMemory();
    copyKeyAndIVToDevice();
}

CudaEncryptor::~CudaEncryptor() {
    freeDeviceMemory();
}

void CudaEncryptor::allocateDeviceMemory() {
    cudaMalloc(&d_key_, key_.size());
    cudaMalloc(&d_iv_, iv_.size());
}

void CudaEncryptor::freeDeviceMemory() {
    if (d_key_) cudaFree(d_key_);
    if (d_iv_) cudaFree(d_iv_);
}

void CudaEncryptor::copyKeyAndIVToDevice() {
    cudaMemcpy(d_key_, key_.data(), key_.size(), cudaMemcpyHostToDevice);
    cudaMemcpy(d_iv_, iv_.data(), iv_.size(), cudaMemcpyHostToDevice);
}

// Placeholder AES kernel (to be implemented)
__global__ void aes_encrypt_kernel(const uint8_t* plaintext, uint8_t* ciphertext, const uint8_t* key, const uint8_t* iv, size_t data_size, AESMode mode) {
    // TODO: Implement AES encryption logic per block
}

std::vector<uint8_t> CudaEncryptor::encrypt(const std::vector<uint8_t>& plaintext) {
    // 1. Copy plaintext to device
    uint8_t* d_plaintext;
    size_t data_size = plaintext.size();
    cudaMalloc(&d_plaintext, data_size);
    cudaMemcpy(d_plaintext, plaintext.data(), data_size, cudaMemcpyHostToDevice);

    // 2. Allocate output buffer on device
    uint8_t* d_ciphertext;
    cudaMalloc(&d_ciphertext, data_size);

    // 3. Launch AES kernel (placeholder)
    int threads = 256;
    int blocks = (data_size + 15) / 16 / threads + 1;
    aes_encrypt_kernel<<<blocks, threads>>>(d_plaintext, d_ciphertext, d_key_, d_iv_, data_size, mode_);
    cudaDeviceSynchronize();

    // 4. Copy encrypted data back to host
    std::vector<uint8_t> ciphertext(data_size);
    cudaMemcpy(ciphertext.data(), d_ciphertext, data_size, cudaMemcpyDeviceToHost);

    // 5. Free device memory
    cudaFree(d_plaintext);
    cudaFree(d_ciphertext);

    return ciphertext;
} 