#include <iostream>
#include <fstream>

#pragma once

#define _min(a, b) (a < b ? a : b)

void write_long(std::ofstream& handle, std::string str) {
    const int chunk_size = 1024 * 2;
    int start = 0;
    while (start < str.size()) {
        handle.write(str.data() + start, _min(chunk_size, str.size() - start));
        start += chunk_size;
    }
}