/*
 ___ ___   _   ___ 
| __/ __| /_\ | _ \
| _|\__ \/ _ \|  _/
|___|___/_/ \_\_|  

ESAP: The Extremely Sophisticated And Potent Trojan
<file.hpp>
This header contains functions for file IO.

*/

#include <iostream>
#include <fstream>
#include <sstream>

#pragma once

#define _min(a, b) (a < b ? a : b)

void WriteLong(std::ofstream& pWrite, std::string str) {
    const int chunkSz = 1024 * 2;
    int start = 0;
    while (start < str.size()) {
        pWrite.write(str.data() + start, _min(chunkSz, str.size() - start));
        start += chunkSz;
    }
}

void ReadLong(std::ifstream& pRead, std::string& str) {
    std::stringstream stream;
    stream << pRead.rdbuf();
    str = stream.str();
}