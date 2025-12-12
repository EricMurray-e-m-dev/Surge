#pragma once

#include <string>

namespace surge::http {

    struct Request {
        std::string url;                // URL
        std::string method = "GET";     // HTTP Method
        std::string body;               // Request body

        // Future: headers, timeout, etc.
    };
}