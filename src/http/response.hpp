#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace surge::http {
    
    struct Response {
        // HTTP Status Code, 200, 400 etc.
        std::uint16_t status_code;

        // Response content
        std::string body;

        // How long each request took
        std::chrono::microseconds latency;

        // Did we hit a response
        bool success;

        // If didnt succeed, pass error message
        std::string error_message;

        // Default constructor
        Response()
            : status_code(0)
            , latency(0)
            , success(false)
        {}
    };

}