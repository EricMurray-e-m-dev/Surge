#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace surge::cli {

    struct Config {
        // Target URL
        std::string url;

        // Number of concurrent workers
        std::uint32_t concurrency = 10;

        // Total requests to make
        std::uint32_t requests = 0;

        // Duration limit in seconds 
        std::uint32_t duration_seconds = 0;

        // HTTP Method
        std::optional<std::string> method = "GET";

        // Verbose output
        bool verbose = true;
    };
}