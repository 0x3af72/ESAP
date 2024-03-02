#include <iostream>
#include <fstream>
#include <vector>
#include <random>

#include "extern/aes.h"

int main(int argc, char* argv[]) {

    // Read contents
    std::cout << "reading file\n";
    std::ifstream pRead(argv[1], std::ios::binary);
    pRead.seekg(0, std::ios::end);
    std::streampos spSize = pRead.tellg();
    pRead.seekg(0);
    std::vector<unsigned char> data(spSize);
    pRead.read(reinterpret_cast<char*>(data.data()), spSize);
    pRead.close();

    // Add junk
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dis(1, 255);
    for (int i = 0; i != 10000000; i++) {
        data.push_back(dis(mt));
    }
    spSize += 10000000;

    // Padding
    int i = spSize;
    while (i % 16 != 0) {
        data.push_back('0');
        std::cout << data[i] << "\n";
        i++;
    }
    
    // AES encryption
    unsigned char* _key = (unsigned char*) "\x36\x0A\x34\xF9\xDD\x4D\xE1\xF5\xF7\x62\xA8\x5B\x34\x82\x7A\x24";
    unsigned char* _iv = (unsigned char*) "\x6B\xB0\x12\xD6\x64\x87\xC8\xD9\x11\x25\x1C\xB8\xFE\xC8\x06\x48";
    std::vector<unsigned char> key(_key, _key + 16);
    std::vector<unsigned char> iv(_iv, _iv + 16);
    AES aes(AESKeyLength::AES_128);
    data = aes.EncryptCBC(data, key, iv);

    // Write back
    std::cout << "writing\n";
    std::ofstream pWrite(argv[1], std::ios::binary);
    pWrite.write(reinterpret_cast<char*>(data.data()), spSize);
    pWrite.close();

    std::cout << "success\n";
    return 0;
}