#include "cli/parser.hpp"
#include <iostream>
#include <string_view>

namespace surge::cli {

bool is_flag(std::string_view arg) {
    return arg.size() >= 2 && arg[0] == '-' && arg[1] == '-';
}

bool parse_arguments(const std::vector<std::string>& args, Config& config) {
    for (size_t i = 1; i < args.size(); ++i) {
        std::string_view arg = args[i];
        
        if (arg == "--help" || arg == "-h") {
            print_usage();
            return false;
        }
        
        if (arg == "--url") {
            if (i + 1 >= args.size()) {
                std::cerr << "Error: --url requires a value\n";
                return false;
            }
            config.url = args[++i];
            
        } else if (arg == "--concurrency" || arg == "-c") {
            if (i + 1 >= args.size()) {
                std::cerr << "Error: --concurrency requires a value\n";
                return false;
            }
            try {
                int value = std::stoi(args[++i]);
                if (value <= 0) {
                    std::cerr << "Error: concurrency must be positive\n";
                    return false;
                }
                config.concurrency = static_cast<std::uint32_t>(value);
            } catch (const std::invalid_argument&) {
                std::cerr << "Error: invalid concurrency value\n";
                return false;
            } catch (const std::out_of_range&) {
                std::cerr << "Error: concurrency value too large\n";
                return false;
            }
            
        } else if (arg == "--requests" || arg == "-r") {
            if (i + 1 >= args.size()) {
                std::cerr << "Error: --requests requires a value\n";
                return false;
            }
            try {
                int value = std::stoi(args[++i]);
                if (value < 0) {
                    std::cerr << "Error: requests cannot be negative\n";
                    return false;
                }
                config.requests = static_cast<std::uint32_t>(value);
            } catch (const std::invalid_argument&) {
                std::cerr << "Error: invalid requests value\n";
                return false;
            } catch (const std::out_of_range&) {
                std::cerr << "Error: requests value too large\n";
                return false;
            }
            
        } else if (arg == "--verbose" || arg == "-v") {
            config.verbose = true;
            
        } else {
            std::cerr << "Error: unknown argument '" << arg << "'\n";
            std::cerr << "Use --help for usage information\n";
            return false;
        }
    }
    
    if (config.url.empty()) {
        std::cerr << "Error: --url is required\n";
        return false;
    }
    
    return true;
}

void print_usage() {
    std::cout << R"(Usage: surge [OPTIONS]

A high-performance HTTP load testing tool

OPTIONS:
    --url <url>              Target URL to test (required)
    -c, --concurrency <n>    Number of concurrent workers (default: 10)
    -r, --requests <n>       Total requests to make (default: 100)
    -v, --verbose            Enable verbose output
    -h, --help               Show this help message

EXAMPLES:
    surge --url http://localhost:8080
    surge --url http://api.example.com/users -c 50 -r 1000
    surge --url http://localhost:3000/api/test -v
)";
}

} 