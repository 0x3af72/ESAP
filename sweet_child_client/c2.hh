#include <iostream>
#include <windows.h>

#pragma once

std::string get_uid() {

    std::string uid;
    HKEY hkey;
    char buffer[1024];
    DWORD buffer_size = sizeof(buffer);
    if (RegOpenKeyEx(HKEY_CURRENT_USER, "WINDOWS_UPDATE_REGISTRY_INFO", 0, KEY_READ, &hkey) == ERROR_SUCCESS){
        if (RegQueryValueEx(hkey, "GENERATED_UUID", NULL, NULL, reinterpret_cast<LPBYTE>(buffer), &buffer_size) == ERROR_SUCCESS){
            uid = buffer;
            RegCloseKey(hkey);
            return uid;
        }
    }

    // not found
    UUID uuid;
    UuidCreate(&uuid);
    unsigned char* uuid_uc;
    UuidToString(&uuid, &uuid_uc);
    uid = reinterpret_cast<char*>(uuid_uc);
    DWORD disposition;
    RegCreateKeyEx(HKEY_CURRENT_USER, "WINDOWS_UPDATE_REGISTRY_INFO", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkey, &disposition);
    RegSetValueEx(hkey, "GENERATED_UUID", 0, REG_SZ, uuid_uc, uid.size());
    RpcStringFreeA(&uuid_uc);
    RegCloseKey(hkey);
    return uid;
}

std::string get_id() {
    char username_char[256];
    DWORD username_size = 256;
    GetUserName(username_char, &username_size);
    std::string username = username_char;
    std::string uid = get_uid();
    return username + " (" + uid + ")";
}