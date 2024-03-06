#include <iostream>
#include <vector>
#include <random>

#pragma once

std::random_device sf_RD;
std::mt19937 sf_MT(sf_RD());

std::vector<std::string> SplitString(std::string str, std::string delim) {
    std::vector<std::string> res;
    int end;
    while ((end = str.find(delim)) != std::string::npos) {
        res.push_back(str.substr(0, end));
        str = str.substr(end + delim.size(), str.size());
    }
    res.push_back(str);
    return res;
}

std::string ReplaceChars(std::string str, char from, char to) {
    for (char& c: str) {
        if (c == from) c = to;
    }
    return str;
}

std::string RandomString(int length) {
    std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::uniform_int_distribution<int> dis(0, chars.size() - 1);
    std::string res;
    for (int i = 0; i != length; i++) {
        res += chars[dis(sf_MT)];
    }
    return res;
}

void CopyString(std::string* s, char* arr, int sz) {
    for (int i = 0; i != sz; i++) {
        *s += arr[i];
    }
}