#include "aes_cuda_lib.hpp"
#include <stdexcept>
#include <cstring>
#include <algorithm>

namespace AESCudaLib
{

    // #cols, #rounds, #keys
    const int Nb_h = 4;
    const int Nr_h = 14;
    const int Nk_h = 8;

    const uint8_t s_h[256] = {
        0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
        0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
        0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
        0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
        0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
        0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
        0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
        0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
        0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
        0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
        0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
        0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
        0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
        0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
        0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
        0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16};

    const uint8_t Rcon_h[256] = {
        0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a,
        0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39,
        0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a,
        0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8,
        0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef,
        0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc,
        0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b,
        0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3,
        0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94,
        0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20,
        0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35,
        0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f,
        0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04,
        0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63,
        0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd,
        0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d};

    __constant__ uint8_t d_sbox[256];
    __constant__ int d_Nb;
    __constant__ int d_Nr;
    __constant__ int d_Nk;
    __constant__ uint32_t d_expanded_key[60];

    static bool g_initialized = false;

    uint32_t subWord(uint32_t word)
    {
        union
        {
            uint32_t word;
            uint8_t bytes[4];
        } subWord_union;
        subWord_union.word = word;

        subWord_union.bytes[3] = s_h[subWord_union.bytes[3]];
        subWord_union.bytes[2] = s_h[subWord_union.bytes[2]];
        subWord_union.bytes[1] = s_h[subWord_union.bytes[1]];
        subWord_union.bytes[0] = s_h[subWord_union.bytes[0]];

        return subWord_union.word;
    }

    uint32_t rotWord(uint32_t word)
    {
        union
        {
            uint8_t bytes[4];
            uint32_t word;
        } rotWord_union;
        rotWord_union.word = word;

        uint8_t B0 = rotWord_union.bytes[3], B1 = rotWord_union.bytes[2],
                B2 = rotWord_union.bytes[1], B3 = rotWord_union.bytes[0];
        rotWord_union.bytes[3] = B1;
        rotWord_union.bytes[2] = B2;
        rotWord_union.bytes[1] = B3;
        rotWord_union.bytes[0] = B0;

        return rotWord_union.word;
    }

    __device__ void subBytes(uint8_t *state)
    {
        for (int i = 0; i < 16; i++)
        {
            state[i] = d_sbox[state[i]];
        }
    }

    __device__ void shiftRows(uint8_t *arr)
    {
        uint8_t out[16];
        out[0] = arr[0];
        out[1] = arr[1];
        out[2] = arr[2];
        out[3] = arr[3];
        out[4] = arr[5];
        out[5] = arr[6];
        out[6] = arr[7];
        out[7] = arr[4];
        out[8] = arr[10];
        out[9] = arr[11];
        out[10] = arr[8];
        out[11] = arr[9];
        out[12] = arr[15];
        out[13] = arr[12];
        out[14] = arr[13];
        out[15] = arr[14];

        for (int i = 0; i < 16; i++)
        {
            arr[i] = out[i];
        }
    }

    __device__ void mixColumns(uint8_t *arr)
    {
        for (int i = 0; i < 4; i++)
        {
            uint8_t a[4];
            uint8_t b[4];
            uint8_t c;
            uint8_t h;
            for (c = 0; c < 4; c++)
            {
                a[c] = arr[(4 * c + i)];
                h = (uint8_t)((signed char)arr[(4 * c + i)] >> 7);
                b[c] = arr[(4 * c + i)] << 1;
                b[c] ^= 0x1B & h;
            }
            arr[(i)] = b[0] ^ a[3] ^ a[2] ^ b[1] ^ a[1];
            arr[(4 + i)] = b[1] ^ a[0] ^ a[3] ^ b[2] ^ a[2];
            arr[(8 + i)] = b[2] ^ a[1] ^ a[0] ^ b[3] ^ a[3];
            arr[(12 + i)] = b[3] ^ a[2] ^ a[1] ^ b[0] ^ a[0];
        }
    }

    __device__ void addRoundKey(uint8_t *state, int round)
    {
        union
        {
            uint32_t word;
            uint8_t bytes[4];
        } kb[4];

        kb[0].word = d_expanded_key[round * 4];
        kb[1].word = d_expanded_key[round * 4 + 1];
        kb[2].word = d_expanded_key[round * 4 + 2];
        kb[3].word = d_expanded_key[round * 4 + 3];

        for (int i = 0; i < 4; i++)
        {
            state[i] = state[i] ^ kb[i].bytes[3];
            state[i + 4] = state[i + 4] ^ kb[i].bytes[2];
            state[i + 8] = state[i + 8] ^ kb[i].bytes[1];
            state[i + 12] = state[i + 12] ^ kb[i].bytes[0];
        }
    }

    __global__ void aes256_ecb_encrypt_kernel(uint8_t *data, size_t num_blocks)
    {
        size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
        if (idx >= num_blocks)
            return;

        uint8_t state[16];
        for (int i = 0; i < 16; i++)
        {
            state[i] = data[(idx * 16) + i];
        }

        addRoundKey(state, 0);

        for (int round = 1; round < 14; round++)
        {
            subBytes(state);
            shiftRows(state);
            mixColumns(state);
            addRoundKey(state, round);
        }

        subBytes(state);
        shiftRows(state);
        addRoundKey(state, 14);

        for (int i = 0; i < 16; i++)
        {
            data[(idx * 16) + i] = state[i];
        }
    }

    __global__ void aes256_cbc_encrypt_kernel(uint8_t *data, const uint8_t *iv, size_t num_blocks)
    {
        size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
        if (idx >= num_blocks)
            return;

        uint8_t state[16];
        for (int i = 0; i < 16; i++)
        {
            state[i] = data[(idx * 16) + i];
        }

        if (idx == 0)
        {
            for (int i = 0; i < 16; i++)
            {
                state[i] ^= iv[i];
            }
        }
        else
        {
            for (int i = 0; i < 16; i++)
            {
                state[i] ^= data[(idx - 1) * 16 + i];
            }
        }

        addRoundKey(state, 0);

        for (int round = 1; round < 14; round++)
        {
            subBytes(state);
            shiftRows(state);
            mixColumns(state);
            addRoundKey(state, round);
        }

        subBytes(state);
        shiftRows(state);
        addRoundKey(state, 14);

        for (int i = 0; i < 16; i++)
        {
            data[(idx * 16) + i] = state[i];
        }
    }

    void expandKey(const uint8_t *key, uint32_t *expanded_key)
    {
        union
        {
            uint8_t bytes[4];
            uint32_t word;
        } temp;
        union
        {
            uint8_t bytes[4];
            uint32_t word;
        } univar[60];

        for (int i = 0; i < Nk_h; i++)
        {
            univar[i].bytes[3] = key[i * 4];
            univar[i].bytes[2] = key[i * 4 + 1];
            univar[i].bytes[1] = key[i * 4 + 2];
            univar[i].bytes[0] = key[i * 4 + 3];
        }

        for (int i = Nk_h; i < Nb_h * (Nr_h + 1); i++)
        {
            temp.word = univar[i - 1].word;
            if (i % Nk_h == 0)
            {
                temp.word = subWord(rotWord(temp.word));
                temp.bytes[3] = temp.bytes[3] ^ (Rcon_h[i / Nk_h]);
            }
            else if (Nk_h > 6 && i % Nk_h == 4)
            {
                temp.word = subWord(temp.word);
            }
            univar[i].word = univar[i - Nk_h].word ^ temp.word;
        }

        for (int i = 0; i < 60; i++)
        {
            expanded_key[i] = univar[i].word;
        }
    }

    void initializeAESConstants()
    {
        if (g_initialized)
            return;

        CUDA_CHECK(cudaMemcpyToSymbol(d_Nk, &Nk_h, sizeof(int), 0, cudaMemcpyHostToDevice));
        CUDA_CHECK(cudaMemcpyToSymbol(d_Nr, &Nr_h, sizeof(int), 0, cudaMemcpyHostToDevice));
        CUDA_CHECK(cudaMemcpyToSymbol(d_Nb, &Nb_h, sizeof(int), 0, cudaMemcpyHostToDevice));
        CUDA_CHECK(cudaMemcpyToSymbol(d_sbox, s_h, 256 * sizeof(uint8_t), 0, cudaMemcpyHostToDevice));

        g_initialized = true;
    }

    void copyExpandedKeyToGPU(const uint32_t *expanded_key)
    {
        CUDA_CHECK(cudaMemcpyToSymbol(d_expanded_key, expanded_key, 60 * sizeof(uint32_t), 0, cudaMemcpyHostToDevice));
    }

    static void pkcs7_pad(std::vector<uint8_t> &data)
    {
        size_t pad_len = AES_BLOCK_SIZE - (data.size() % AES_BLOCK_SIZE);
        if (pad_len == 0)
            pad_len = AES_BLOCK_SIZE;
        data.insert(data.end(), pad_len, static_cast<uint8_t>(pad_len));
    }

    std::vector<uint8_t> encryptAES256(const std::vector<uint8_t> &plaintext,
                                       const std::vector<uint8_t> &key,
                                       const std::vector<uint8_t> &iv,
                                       bool use_cbc)
    {
        if (key.size() != AES_256_KEY_SIZE)
        {
            throw std::runtime_error("Key must be 32 bytes for AES-256");
        }
        if (iv.size() != AES_BLOCK_SIZE)
        {
            throw std::runtime_error("IV must be 16 bytes");
        }

        if (!g_initialized)
        {
            initializeAESConstants();
        }

        std::vector<uint8_t> padded = plaintext;
        pkcs7_pad(padded);
        size_t num_blocks = padded.size() / AES_BLOCK_SIZE;

        uint32_t expanded_key[60];
        expandKey(key.data(), expanded_key);
        copyExpandedKeyToGPU(expanded_key);

        uint8_t *d_data;
        uint8_t *d_iv = nullptr;
        CUDA_CHECK(cudaMalloc(&d_data, padded.size()));

        if (use_cbc)
        {
            CUDA_CHECK(cudaMalloc(&d_iv, AES_BLOCK_SIZE));
            CUDA_CHECK(cudaMemcpy(d_iv, iv.data(), AES_BLOCK_SIZE, cudaMemcpyHostToDevice));
        }

        CUDA_CHECK(cudaMemcpy(d_data, padded.data(), padded.size(), cudaMemcpyHostToDevice));

        int threads = 256;
        int blocks = (num_blocks + threads - 1) / threads;

        if (use_cbc)
        {
            aes256_cbc_encrypt_kernel<<<blocks, threads>>>(d_data, d_iv, num_blocks);
        }
        else
        {
            aes256_ecb_encrypt_kernel<<<blocks, threads>>>(d_data, num_blocks);
        }

        CUDA_CHECK(cudaDeviceSynchronize());

        std::vector<uint8_t> ciphertext(padded.size());
        CUDA_CHECK(cudaMemcpy(ciphertext.data(), d_data, padded.size(), cudaMemcpyDeviceToHost));

        cudaFree(d_data);
        if (d_iv)
            cudaFree(d_iv);

        return ciphertext;
    }

    void cleanup()
    {
        g_initialized = false;
    }

}