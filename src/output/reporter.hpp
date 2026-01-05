#pragma once

#include "core/engine.hpp"
#include <chrono>
#include <string>

namespace surge::output {
    // ANSI Colour codes for terminal
    namespace Colours {
        // Only use for terminal
        inline const std::string RESET = "\033[0m";
        inline const std::string BOLD = "\033[1m";
        inline const std::string RED = "\033[31m";
        inline const std::string GREEN = "\033[32m";
        inline const std::string YELLOW = "\033[33m";
        inline const std::string BLUE = "\033[34m";
        inline const std::string MAGENTA = "\033[35m";
        inline const std::string CYAN = "\033[36m";
    }

    // Reporter for formatting and displaying load test results
    class Reporter {
        public:
            // Print results to CLI
            static void print(const core::Results& results);

            // Print with colour
            static void print_coloured(const core::Results& results);

            // Save to file
            static bool save_to_file(const core::Results& results, const std::string& filepath);

        private:
            // Helper format duration
            static std::string format_duration(std::chrono::microseconds duration);

            // Helper format number with seperators
            static std::string format_number(uint64_t number);

            // Helper format percentage
            static std::string format_percent(double value);

            // Helper format latency value
            static std::string format_latency(uint64_t microseconds);

            // Helper draw a line
            static std::string line(size_t length, char ch = '=');
    };
}