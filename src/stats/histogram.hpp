#pragma once

#include <cstddef>
#include <vector>
#include <cstdint>
#include <algorithm>
#include "stats/metrics.hpp"

namespace surge::stats {
    class Histogram {
        public:
            // Calculate percentiles from unsorted latencies
            static Percentiles calculate(std::vector<std::uint64_t> latencies) {
                Percentiles result{};

                if (latencies.empty()) {
                    return result;
                }

                std::sort(latencies.begin(), latencies.end());

                // Calculate each percentile
                result.p50 = percentile_at(latencies, 0.50);
                result.p75 = percentile_at(latencies, 0.75);
                result.p90 = percentile_at(latencies, 0.90);
                result.p95 = percentile_at(latencies, 0.95);
                result.p99 = percentile_at(latencies, 0.99);
                result.p999 = percentile_at(latencies, 0.999);

                return result;
            }

        private:
            // Get value at each percentile from sorted array

            static std::uint64_t percentile_at(const std::vector<std::uint64_t>& sorted_values, double percentile) {
                if (sorted_values.empty()) {
                    return 0;
                }

                // Calculate the index
                double index = percentile * (sorted_values.size() - 1);

                // Get lower and upper indices
                size_t lower = static_cast<size_t>(index);
                size_t upper = lower + 1;

                // If index is exact just return results
                if (upper >= sorted_values.size()) {
                    return sorted_values[lower];
                }

                // Linear interpolation between values
                double fraction = index - lower;

                std::uint64_t lower_value = sorted_values[lower];
                std::uint64_t upper_value = sorted_values[upper];

                // Interpolate: lower + fraction * (upper - lower)
                return static_cast<std::uint64_t>(
                    lower_value + fraction * (upper_value - lower_value)
                );
            }
    };
}