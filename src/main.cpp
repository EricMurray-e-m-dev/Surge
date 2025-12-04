#include <iostream>
#include <string>
#include <vector>

#include "cli/config.hpp"
#include "cli/parser.hpp"

int main(int argc, char* argv[]) {
    std::vector<std::string> args(argv, argv + argc);

    std::cout << "surge v0.1.0\n\n";
    
    surge::cli::Config config;

    if (!surge::cli::parse_arguments(args, config)) {
        return 1;
    }

    // Print parsed configuration
    std::cout << "\nConfiguration loaded:\n";
    std::cout << "\tURL:         " << config.url << "\n";
    std::cout << "\tConcurrency: " << config.concurrency << "\n";
    std::cout << "\tRequests:    " << config.requests << "\n";
    std::cout << "\tVerbose:     " << (config.verbose ? "yes" : "no") << "\n";
    
    std::cout << "\n[Load test would run here]\n";

    return 0;
}