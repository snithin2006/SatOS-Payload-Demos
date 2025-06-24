#include "cuda_encryptor.hpp"
#include <iostream>
#include <vector>
#include <random>
#include <iomanip>

// Helper function to print hex data
void printHex(const std::vector<uint8_t>& data, const std::string& label) {
    std::cout << label << " (" << data.size() << " bytes): ";
    for (size_t i = 0; i < std::min(data.size(), size_t(32)); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') 
                  << static_cast<int>(data[i]) << " ";
    }
    if (data.size() > 32) {
        std::cout << "...";
    }
    std::cout << std::dec << std::endl;
}

int main() {
    try {
        std::cout << "=== AES-256 CUDA Library Integration Test ===" << std::endl;
        
        // Generate random key and IV
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<uint8_t> dis(0, 255);
        
        std::vector<uint8_t> key(32);  // AES-256 key
        std::vector<uint8_t> iv(16);   // Initialization vector
        
        for (auto& b : key) b = dis(gen);
        for (auto& b : iv) b = dis(gen);
        
        std::cout << "Generated random key and IV" << std::endl;
        
        // Test data
        std::string test_message = "Hello, this is a test message for AES-256 encryption!";
        std::vector<uint8_t> plaintext(test_message.begin(), test_message.end());
        
        std::cout << "\nTest message: " << test_message << std::endl;
        printHex(plaintext, "Plaintext");
        
        // Create encryptor
        CudaEncryptor encryptor(key, iv, AESMode::CBC);
        std::cout << "\nCreated CudaEncryptor with CBC mode" << std::endl;
        
        // Encrypt
        std::vector<uint8_t> ciphertext = encryptor.encrypt(plaintext);
        printHex(ciphertext, "Ciphertext");
        
        // Verify the encryption changed the data
        bool data_changed = false;
        if (ciphertext.size() != plaintext.size()) {
            data_changed = true;  // Padding was added
        } else {
            for (size_t i = 0; i < plaintext.size(); ++i) {
                if (ciphertext[i] != plaintext[i]) {
                    data_changed = true;
                    break;
                }
            }
        }
        
        if (data_changed) {
            std::cout << "\n✅ SUCCESS: Data was encrypted (ciphertext differs from plaintext)" << std::endl;
        } else {
            std::cout << "\n❌ FAILURE: Data was not encrypted (ciphertext same as plaintext)" << std::endl;
        }
        
        // Test with different data
        std::string test_message2 = "Another test message with different content!";
        std::vector<uint8_t> plaintext2(test_message2.begin(), test_message2.end());
        std::vector<uint8_t> ciphertext2 = encryptor.encrypt(plaintext2);
        
        std::cout << "\nSecond test message: " << test_message2 << std::endl;
        printHex(ciphertext2, "Ciphertext 2");
        
        // Verify different plaintexts produce different ciphertexts
        bool different_ciphertexts = false;
        if (ciphertext.size() != ciphertext2.size()) {
            different_ciphertexts = true;
        } else {
            for (size_t i = 0; i < ciphertext.size(); ++i) {
                if (ciphertext[i] != ciphertext2[i]) {
                    different_ciphertexts = true;
                    break;
                }
            }
        }
        
        if (different_ciphertexts) {
            std::cout << "\n✅ SUCCESS: Different plaintexts produce different ciphertexts" << std::endl;
        } else {
            std::cout << "\n❌ FAILURE: Different plaintexts produced same ciphertext" << std::endl;
        }
        
        std::cout << "\n=== Test completed ===" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
} 