#include <iostream>
#include <string>
#include <vector>
#include "cli/config.hpp"
#include "cli/parser.hpp"
#include "core/engine.hpp"
#include "output/reporter.hpp"  // Add this

int main(int argc, char* argv[]) {
    std::vector<std::string> args(argv, argv + argc);
    
    // Parse CLI arguments
    surge::cli::Config config;
    if (!surge::cli::parse_arguments(args, config)) {
        return 1;
    }
    
    std::cout << "\nStarting load test:\n";
    std::cout << "  URL:         " << config.url << "\n";
    std::cout << "  Concurrency: " << config.concurrency << "\n";
    
    if (config.requests > 0) {
        std::cout << "  Requests:    " << config.requests << "\n";
    }
    if (config.duration_seconds > 0) {
        std::cout << "  Duration:    " << config.duration_seconds << "s\n";
    }
    
    std::cout << "\n";
    
    // Create and run engine
    surge::core::Engine engine(config);
    surge::core::Results results = engine.run();
    
    // Print results with colors!
    surge::output::Reporter::print_coloured(results);
    
    return 0;
}