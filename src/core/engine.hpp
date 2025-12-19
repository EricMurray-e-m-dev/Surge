#pragma once

#include <cstdint>
#include <memory>
#include <atomic>
#include <optional>
#include <chrono>
#include "cli/config.hpp"
#include "core/thread_pool.hpp"
#include "stats/collector.hpp"
#include "stats/metrics.hpp"

namespace surge::core {
    struct Results {
        stats::Metrics metrics;
        stats::Percentiles percentiles;

        std::chrono::microseconds duration;

        double requests_per_second; // Throughput
    };

    class Engine {
        public:
            // Constructor takes config parameter
            explicit Engine(const cli::Config& config);

            ~Engine();

            // Disable copy
            Engine(const Engine&) = delete;
            Engine& operator=(const Engine&) = delete;

            // Run the load test - main entry point
            Results run();

            // Stop the test early
            void stop();

        private:
            // Called by worker threads
            void execute_request();

            // Check if test should continue, false when limits reached
            bool should_continue() const;

            void wait_for_completion();

            cli::Config config_;

            // unique pointer because threadpool is non copy
            std::unique_ptr<core::ThreadPool> pool_;

            // Stats collector
            stats::Collector collector_;

            // State management
            std::atomic<bool> running_{false};
            std::atomic<bool> stop_requested_{false};

            // Timing
            std::chrono::steady_clock::time_point start_time_;
            std::optional<std::chrono::steady_clock::time_point> deadline_;

            // Request tracking 
            std::atomic<std::uint32_t> requests_completed_{0};
    };
}