/*
FILESYSTEM:
change directory
list current directory
download file
upload file
*/

#include <iostream>
#include <windows.h>
#include <filesystem>

#pragma once

std::string current_directory = std::filesystem::current_path().string();

std::string get_directory() {
    return std::filesystem::current_path().string();
}

bool change_directory(std::string change) {
    try {
        std::filesystem::current_path(change);
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

std::vector<std::string> list_directory() {
    std::vector<std::string> res;
    for (auto& entry: std::filesystem::directory_iterator(std::filesystem::current_path())) {
        res.push_back(entry.path().filename().string());
    }
    return res;
}

// download file: not sure how to implement yet
// upload file: not sure how to implement yet