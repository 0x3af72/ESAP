#include <iostream>
#include <fstream>
#include <sstream>

#pragma once

#define _min(a, b) (a < b ? a : b)

void writeLong(std::ofstream& pWrite, std::string str) {
    const int chunkSz = 1024 * 2;
    int start = 0;
    while (start < str.size()) {
        pWrite.write(str.data() + start, _min(chunkSz, str.size() - start));
        start += chunkSz;
    }
}

void readLong(std::ifstream& pRead, std::string& str) {
    std::stringstream stream;
    stream << pRead.rdbuf();
    str = stream.str();
}