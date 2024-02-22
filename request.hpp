#include <iostream>
#include <mutex>

#include "extern/json.hpp"
#include "extern/httprequest.hpp"

#pragma once

using json = nlohmann::json;

std::string GET_Request(std::string url) {
    try {
        http::Request request {url};
        auto response = request.send("GET");
        return std::string(response.body.begin(), response.body.end());
    } catch (const std::exception& e) {
        std::cout << e.what() << "\n";
    }
    return "";
}

std::string POST_Request(std::string url, json data) {
    try {
        http::Request request {url};
        auto response = request.send("POST", data.dump(), {
            {"Content-Type", "application/json"}
        });
        return std::string(response.body.begin(), response.body.end());
    } catch (const std::exception& e) {
        std::cout << e.what() << "\n";
    }
    return "";
}