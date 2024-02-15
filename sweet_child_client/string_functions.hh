#include <iostream>
#include <vector>

#pragma once

// obfuscation
#define cs(chars) (std::string(chars))

std::vector<std::string> split_string(std::string str, std::string delim) {
    std::vector<std::string> res;
    int end;
    while ((end = str.find(delim)) != std::string::npos) {
        res.push_back(str.substr(0, end));
        str = str.substr(end + delim.size(), str.size());
    }
    res.push_back(str);
    return res;
}

std::string replace_chars(std::string str, char from, char to) {
    for (char& c: str) {
        if (c == from) c = to;
    }
    return str;
}