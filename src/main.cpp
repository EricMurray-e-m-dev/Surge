#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include "http/client.hpp"
#include "http/request.hpp"
#include "cli/config.hpp"
#include "cli/parser.hpp"
#include "core/thread_pool.hpp"
#include "http/response.hpp"
#include "stats/collector.hpp"
#include "stats/metrics.hpp"

int main(int argc, char* argv[]) {
    std::vector<std::string> args(argv, argv + argc);

    std::cout << "surge v0.1.3\n\n";
    
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

    // Create collector
    surge::stats::Collector collector;

    // Start timer
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Create thread pool
    surge::core::ThreadPool pool(config.concurrency);

    // Submit HTTP requests as tasks
    for (uint32_t i = 0; i < config.requests; ++i) {
        pool.submit([&config, &collector]() {
            surge::http::Client client;
            surge::http::Request request;
            request.url = config.url;
            request.method = config.method.value_or("GET");

            surge::http::Response response = client.execute(request);

            collector.record(response);

        });
    }

    std::cout << "All tasks submitted, waiting for completion...\n";

    pool.wait_for_completion();

    auto end_timer = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_timer - start_time);

    collector.set_duration(duration);

    // Get Results
    surge::stats::Metrics metrics = collector.get_metrics();
    surge::stats::Percentiles percentiles = collector.calculate_percentiles();

    // Print results
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << " RESULTS\n";
    std::cout << std::string(50, '=') << "\n";
    std::cout << "  Duration:          " << (duration.count() / 1'000'000.0) << "s\n";
    std::cout << "  Total requests:    " << metrics.total_requests << "\n";
    std::cout << "  Successful:        " << metrics.successful_requests << "\n";
    std::cout << "  Failed:            " << metrics.failed_requests << "\n";
    std::cout << std::string(50, '=') << "\n";

    if (metrics.successful_requests > 0) {
        double avg_latency = metrics.total_latency.count() / 
                            static_cast<double>(metrics.successful_requests);
        std::cout << "  Latency:\n";
        std::cout << "    Average:  " << (avg_latency / 1000.0) << " ms\n";
        std::cout << "    Min:      " << (metrics.min_latency.count() / 1000.0) << " ms\n";
        std::cout << "    Max:      " << (metrics.max_latency.count() / 1000.0) << " ms\n";
        std::cout << "    p50:      " << (percentiles.p50 / 1000.0) << " ms\n";
        std::cout << "    p90:      " << (percentiles.p90 / 1000.0) << " ms\n";
        std::cout << "    p99:      " << (percentiles.p99 / 1000.0) << " ms\n";
    }

    std::cout << std::string(50, '=') << "\n";

    std::cout << "Status Codes:\n";
    for (const auto& [code, count] : metrics.status_codes) {
        double percentage = (count * 100.0) / metrics.total_requests;
        std::cout << "\t" << code << ": " << count << " (" << percentage << "%)\n";
    }

    std::cout << std::string(50, '=') << "\n";

    return 0;
}

/* ThreadPool test only
int main() {
    std::cout << "Creating ThreadPool...\n";
    surge::core::ThreadPool pool(4);

    std::cout << "Submitting Tasks...\n";

    for (int i =0; i < 10; ++i) {
        pool.submit([i]() {
            std::cout << "Task " << i << " started on thread " << std::this_thread::get_id() << "\n";

            // Simulate work
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            std::cout << "Task " << i << " completed\n";
        });
    }

    std::cout << "Waiting for completion...\n";
    pool.wait_for_completion();

    std::cout << "All tasks done!\n";

    return 0;
}*/