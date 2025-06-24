# AES-256 CUDA Library Integration

This document explains the integration of a production-ready AES-256 CUDA library into the image processing pipeline.

## Overview

We've integrated the **cartermc24/AES-Cuda** library, which provides:
- **AES-256 encryption** with full 14-round implementation
- **CBC (Cipher Block Chaining)** mode for secure encryption
- **ECB mode** support (less secure, not recommended for production)
- **PKCS#7 padding** for variable-length data
- **CUDA-optimized** performance

## Files Added

### Core Library Files
- `src/aes_lib/aes_cuda_lib.hpp` - Header file with clean API
- `src/aes_lib/aes_cuda_lib.cu` - Implementation with full AES-256

### Integration Files
- `src/cuda_encryptor.cu` - Updated to use production library
- `src/test_aes.cpp` - Test program for verification

## How It Works

### 1. Key Expansion
The library expands your 32-byte AES-256 key into 60 round keys using the standard AES key schedule.

### 2. Encryption Process
1. **Padding**: Data is padded to 16-byte blocks using PKCS#7
2. **CBC Mode**: Each block is XORed with the previous ciphertext (or IV for first block)
3. **AES Rounds**: 14 rounds of SubBytes, ShiftRows, MixColumns, AddRoundKey
4. **Output**: Encrypted data with padding

### 3. CUDA Parallelization
- Each CUDA thread processes one 16-byte block
- Blocks are processed in parallel for high performance
- Shared memory used for IV in CBC mode

## Usage

### Basic Usage
```cpp
#include "include/cuda_encryptor.hpp"

// Generate 32-byte key and 16-byte IV
std::vector<uint8_t> key(32);
std::vector<uint8_t> iv(16);
// ... fill with random data ...

// Create encryptor
CudaEncryptor encryptor(key, iv, AESMode::CBC);

// Encrypt data
std::vector<uint8_t> plaintext = /* your data */;
std::vector<uint8_t> ciphertext = encryptor.encrypt(plaintext);
```

### In Your Pipeline
```cpp
// After JPEG compression
std::vector<uchar> compressed_jpeg = compressor.compress(...);

// Encrypt the compressed data
CudaEncryptor encryptor(key, iv, AESMode::CBC);
std::vector<uint8_t> encrypted = encryptor.encrypt(compressed_jpeg);
```

## Security Features

### ✅ What's Secure
- **AES-256**: 256-bit key strength
- **CBC Mode**: Prevents pattern leakage
- **Random IV**: Each encryption uses unique IV
- **PKCS#7 Padding**: Standard padding scheme
- **Full Implementation**: All 14 AES rounds implemented

### ⚠️ Security Considerations
- **Key Management**: You must securely generate and store keys
- **IV Generation**: Use cryptographically secure random IVs
- **Mode Selection**: CBC is secure, ECB is not recommended
- **No Decryption**: This implementation only encrypts (add decryption if needed)

## Performance

### Expected Performance
- **Throughput**: ~1-10 GB/s depending on GPU
- **Latency**: ~1-10ms for typical image data
- **Memory**: Efficient GPU memory usage

### Optimization Features
- **Constant Memory**: S-box and round constants in fast memory
- **Shared Memory**: IV sharing in CBC mode
- **Coalesced Access**: Optimized memory access patterns
- **Parallel Processing**: Multiple blocks processed simultaneously

## Building

### Prerequisites
- CUDA toolkit (10.0 or later)
- CMake 3.10+
- OpenCV

### Build Commands
```bash
mkdir build && cd build
cmake ..
make test_aes    # Build AES test
make test_pce    # Build full pipeline test
```

### Testing
```bash
# Test AES integration
./test_aes

# Test full pipeline (image -> compress -> encrypt)
./test_pce input.jpg output.jpeg output.enc
```

## Troubleshooting

### Common Issues

1. **CUDA Not Found**
   - Install CUDA toolkit
   - Set `CUDA_PATH` environment variable

2. **Build Errors**
   - Ensure CMake version >= 3.10
   - Check CUDA compiler availability

3. **Runtime Errors**
   - Verify key size is 32 bytes
   - Verify IV size is 16 bytes
   - Check GPU memory availability

### Debug Mode
```bash
# Build with debug info
cmake -DCMAKE_BUILD_TYPE=Debug ..
make test_aes
```

## Future Enhancements

### Planned Features
- [ ] **Decryption Support**: Add decrypt method
- [ ] **CTR Mode**: Counter mode for parallel encryption
- [ ] **GCM Mode**: Authenticated encryption
- [ ] **Key Derivation**: PBKDF2 or similar
- [ ] **Hardware Acceleration**: Use GPU-specific AES instructions

### Performance Optimizations
- [ ] **Memory Pinning**: Faster host-device transfers
- [ ] **Stream Processing**: Asynchronous encryption
- [ ] **Batch Processing**: Multiple images at once

## References

- **Original Library**: [cartermc24/AES-Cuda](https://github.com/cartermc24/AES-Cuda)
- **AES Standard**: FIPS 197
- **CBC Mode**: NIST SP 800-38A
- **PKCS#7**: RFC 5652

## License

The AES library is MIT licensed. See the original repository for details. 