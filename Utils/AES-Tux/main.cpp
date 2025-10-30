#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "aes.h"

std::vector<unsigned char> read_file(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filepath);
    }
    return std::vector<unsigned char>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

void write_file(const std::string& filepath, const std::vector<unsigned char>& data) {
    std::ofstream file(filepath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot create file: " + filepath);
    }
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
}

void pad(std::vector<unsigned char>& data) {
    size_t block_size = 16;
    size_t padding_len = block_size - (data.size() % block_size);
    if (padding_len == 0) {
        padding_len = block_size;
    }
    for (size_t i = 0; i < padding_len; ++i) {
        data.push_back(static_cast<unsigned char>(padding_len));
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_file.bmp> <output_file.bmp>" << std::endl;
        return 1;
    }

    std::string input_path = argv[1];
    std::string output_path = argv[2];

    std::vector<unsigned char> key = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
    };

    try {
        std::cout << "Reading file: " << input_path << std::endl;
        std::vector<unsigned char> file_data = read_file(input_path);
        
        const size_t bmp_header_size = 54;
        if (file_data.size() < bmp_header_size) {
            throw std::runtime_error("Input is not a valid BMP file or is too small.");
        }
        
        std::vector<unsigned char> pixel_data(file_data.begin() + bmp_header_size, file_data.end());

        pad(pixel_data);
        
        std::vector<unsigned char> encrypted_pixel_data;
        encrypted_pixel_data.reserve(pixel_data.size());

        std::cout << "Encrypting with AES in ECB mode..." << std::endl;
        
        for (size_t i = 0; i < pixel_data.size(); i += 16) {
            std::vector<unsigned char> block(pixel_data.begin() + i, pixel_data.begin() + i + 16);
            std::vector<unsigned char> encrypted_block = aes::encrypt(block, key);
            encrypted_pixel_data.insert(encrypted_pixel_data.end(), encrypted_block.begin(), encrypted_block.end());
        }

        std::vector<unsigned char> encrypted_file_data;
        encrypted_file_data.insert(encrypted_file_data.end(), file_data.begin(), file_data.begin() + bmp_header_size);
        encrypted_file_data.insert(encrypted_file_data.end(), encrypted_pixel_data.begin(), encrypted_pixel_data.end());
        
        write_file(output_path, encrypted_file_data);
        std::cout << "Encryption successful! Output written to: " << output_path << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}