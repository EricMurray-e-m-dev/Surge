#include "stats/collector.hpp"
#include "http/response.hpp"
#include "stats/histogram.hpp"
#include "stats/metrics.hpp"
#include <chrono>
#include <cstdint>
#include <mutex>
#include <vector>

namespace surge::stats {
    // Constructor
    Collector::Collector() {}

    // Record a single HTTP response
    void Collector::record(const http::Response& response) {
        // Acquire lock
        std::lock_guard<std::mutex> lock(mutex_);

        metrics_.total_requests++;

        // Calculate min/max latencies + increment counts
        if (response.success) {
            metrics_.successful_requests++;

            std::chrono::microseconds latency = response.latency;

            metrics_.total_latency += latency;

            if (latency < metrics_.min_latency) {
                metrics_.min_latency = latency;
            }
            if (latency > metrics_.max_latency) {
                metrics_.max_latency = latency;
            }

            metrics_.latencies_us.push_back(latency.count());

            metrics_.status_codes[response.status_code]++;
        } else {
            metrics_.failed_requests++;
        }
    }

    Metrics Collector::get_metrics() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return metrics_;
    }

    void Collector::set_duration(std::chrono::microseconds duration) {
        std::lock_guard<std::mutex> lock(mutex_);
        metrics_.test_duration = duration;
    }

    Percentiles Collector::calculate_percentiles() const {
        std::vector<std::uint64_t> latencies_copy;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            latencies_copy = metrics_.latencies_us;
        }

        return Histogram::calculate(latencies_copy);
    }
}