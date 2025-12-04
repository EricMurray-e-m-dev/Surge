#pragma once

#include <string>
#include <vector>
#include "cli/config.hpp"

namespace surge::cli {
    bool parse_arguments(const std::vector<std::string>& args, Config& config);

    void print_usage();
}