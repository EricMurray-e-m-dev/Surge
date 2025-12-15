#pragma once

#include <chrono>
#include <mutex>
#include "stats/metrics.hpp"
#include "http/response.hpp"

namespace surge::stats {
    // Thread safe stats collector
    // Workers call record() to add results
    // Main thread calls get_metrics() to retrieve aggregated stats
    class Collector {
        public:
            // Constructor
            Collector();

            // Record a single request result
            void record(const http::Response& response);

            // Get aggregated metrics 
            Metrics get_metrics() const;

            // Set test duration (set after test completes)
            void set_duration(std::chrono::microseconds duration);

            // Calculate percentiles from collected latencies
            // Call at the end, expensive operation
            Percentiles calculate_percentiles() const;
        
        private:
            // Protect all shared state
            mutable std::mutex mutex_;
            Metrics metrics_;
    };
}