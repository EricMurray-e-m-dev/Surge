#pragma once

#include <cstdint>
#include <chrono>
#include <string>
#include <map>
#include <vector>

namespace surge::stats {
    struct RequestResult {
        bool success;                           // Did request succeed
        std::uint16_t status_code;              // HTTP Status Code
        std::chrono::microseconds latency;      // How long it took
        std::string error_message;              // Any error

        // Constructor
        RequestResult()
            : success(false)
            , status_code(0)
            ,latency(0)
        {}
    };

    struct Metrics {
        // Request counts
        std::uint64_t total_requests = 0;
        std::uint64_t successful_requests = 0;
        std::uint64_t failed_requests = 0;

        // Timings
        std::chrono::microseconds total_latency{0};
        std::chrono::microseconds min_latency{std::chrono::microseconds::max()};
        std::chrono::microseconds max_latency{0};

        // Status Codes
        std::map<std::uint16_t, std::uint64_t> status_codes;

        // Individual latencies for percentile calculations
        std::vector<std::uint64_t> latencies_us;

        // Test duration
        std::chrono::microseconds test_duration{0};
    };

    struct Percentiles {
        std::uint64_t p50;
        std::uint64_t p75;
        std::uint64_t p90;
        std::uint64_t p95;
        std::uint64_t p99;
        std::uint64_t p999;  
    };
}