#include <iostream>
#include <string>
#include <vector>

#include "cli/config.hpp"

int main(int argc, char* argv[]) {
    std::vector<std::string> args(argv, argv + argc);

    std::cout << "surge v0.1.0\n\n";
    
    surge::cli::Config config;

    std::cout << "Default configuration:\n";
    std::cout << "\tURL: " << config.url << " (empty)\n";
    std::cout << "\tConcurrency: " << config.concurrency << "\n";
    std::cout << "\tRequests: " << config.requests << "\n";
    std::cout << "\tDuration: " << config.duration_seconds << "s\n";

    if (config.method) {
        std::cout << "\tMethod: " << *config.method << "\n";
    } else {
        std::cout << "\tMethod not set\n";
    }

    std::cout << "\tVerbose: " << (config.verbose ? "true" : "false") << "\n";

    config.url = "http://localhost:8080";
    config.concurrency = 50;

    std::cout << "\nAfter modification:\n";
    std::cout << "\tURL: " << config.url << "\n";
    std::cout << "\tConcurrency: " << config.concurrency << "\n";

    return 0;
}