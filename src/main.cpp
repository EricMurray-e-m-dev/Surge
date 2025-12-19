#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include "core/engine.hpp"
#include "cli/config.hpp"
#include "cli/parser.hpp"
#include "stats/metrics.hpp"

int main(int argc, char* argv[]) {
    std::vector<std::string> args(argv, argv + argc);

    std::cout << "Surge v0.2.0\n\n";
    
    surge::cli::Config config;
    if (!surge::cli::parse_arguments(args, config)) {
        return 1;
    }

    // Print parsed configuration
    std::cout << "\nStarting Load Test:\n";
    std::cout << "\tURL:         " << config.url << "\n";
    std::cout << "\tConcurrency: " << config.concurrency << "\n";
    std::cout << "\tRequests:    " << config.requests << "\n";
    std::cout << "\tVerbose:     " << (config.verbose ? "yes" : "no") << "\n";
    if (config.duration_seconds > 0) {
        std::cout << "\tDuration:    " << config.duration_seconds << "s\n";
    }
    std::cout << "\n";

    // Create and run engine
    surge::core::Engine engine(config);
    surge::core::Results results = engine.run();

    // Print results
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << " RESULTS\n";
    std::cout << std::string(50, '=') << "\n";
    std::cout << "  Duration:          " << (results.duration.count() / 1'000'000.0) << "s\n";
    std::cout << "  Total requests:    " << results.metrics.total_requests << "\n";
    std::cout << "  Successful:        " << results.metrics.successful_requests << "\n";
    std::cout << "  Failed:            " << results.metrics.failed_requests << "\n";
    std::cout << std::string(50, '=') << "\n";

    if (results.metrics.successful_requests > 0) {
        double avg_latency = results.metrics.total_latency.count() / 
                            static_cast<double>(results.metrics.successful_requests);
        std::cout << "  Latency:\n";
        std::cout << "    Average:  " << (avg_latency / 1000.0) << " ms\n";
        std::cout << "    Min:      " << (results.metrics.min_latency.count() / 1000.0) << " ms\n";
        std::cout << "    Max:      " << (results.metrics.max_latency.count() / 1000.0) << " ms\n";
        std::cout << "    p50:      " << (results.percentiles.p50 / 1000.0) << " ms\n";
        std::cout << "    p90:      " << (results.percentiles.p90 / 1000.0) << " ms\n";
        std::cout << "    p99:      " << (results.percentiles.p99 / 1000.0) << " ms\n";
    }

    std::cout << std::string(50, '=') << "\n";

    std::cout << "Status Codes:\n";
    for (const auto& [code, count] : results.metrics.status_codes) {
        double percentage = (count * 100.0) / results.metrics.total_requests;
        std::cout << "\t" << code << ": " << count << " (" << percentage << "%)\n";
    }

    std::cout << std::string(50, '=') << "\n";

    return 0;
}