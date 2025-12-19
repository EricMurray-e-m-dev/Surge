#include "core/engine.hpp"
#include "cli/config.hpp"
#include "core/thread_pool.hpp"
#include "http/client.hpp"
#include "http/request.hpp"
#include "http/response.hpp"
#include "stats/metrics.hpp"
#include <chrono>
#include <memory>
#include <thread>

namespace surge::core {
    // Constructor
    Engine::Engine(const cli::Config& config)
        : config_(config)
        , pool_(nullptr)
        , collector_()
        , running_(false)
        , stop_requested_(false)
        , requests_completed_(0)
    {}

    // Destructor
    Engine::~Engine() {
        // Ensure load test is stopped
        stop_requested_ = true;
        running_ = false;

        // pool_ unique_ptr automatically cleaned up
        // Workers all join jthread handles
    }

    // Execute a single HTTP request
    // Called by workers
    void Engine::execute_request() {
        // Create HTTP Client per request - move to connection pooling later for performance
        http::Client client;

        // Build request
        http::Request request;
        request.url = config_.url;
        request.method = config_.method.value_or("GET");

        // Execute request
        http::Response response = client.execute(request);

        // Record result (thread safe)
        collector_.record(response);

        // Increment req completed
        requests_completed_++;
    }

    // Check if test should continue
    // Return false when limits reached
    bool Engine::should_continue() const {
        // Check stop flag
        if (stop_requested_) {
            return false;
        }

        // Check duration limit if set
        if (deadline_.has_value()) {
            auto now = std::chrono::steady_clock::now();
            if (now >= *deadline_) {
                return false; // Time deadline reached
            }
        }

        // Check request limit if set
        if (config_.requests > 0) {
            if (requests_completed_ >= config_.requests) {
                return false;
            }
        }

        return true;
    }

    // Wait for load test to complete
    void Engine::wait_for_completion() {
        // if request based, wait for all tasks to finish
        if (config_.requests > 0 && config_.duration_seconds == 0) {
            // Request based, pool knows when all tasks are done
            pool_->wait_for_completion();
            return;
        }

        while (running_ && !stop_requested_) {
            // Check if deadline reached
            if (deadline_.has_value()) {
                auto now = std::chrono::steady_clock::now();
                if (now >= *deadline_) {
                    running_ = false;
                    break;
                }
            }
            // Sleep for 100ms to avoid wasting CPU
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    // Run the load test (blocking)
    Results Engine::run() {
        // Initialise state
        start_time_ = std::chrono::steady_clock::now();
        running_ = true;
        stop_requested_ = false;
        requests_completed_ = 0;

        // Set deadline
        if (config_.duration_seconds > 0) {
            deadline_ = start_time_ + std::chrono::seconds(config_.duration_seconds);
        }

        // Create thread pool
        pool_ = std::make_unique<ThreadPool>(config_.concurrency);

        // Submit work 
        if (config_.requests > 0) {
            // Request based
            for (uint32_t i = 0; i < config_.requests; ++i) {
                pool_->submit([this]() {
                    execute_request();
                });
            }
        } else {
            // Duration based
            for (uint32_t i = 0; i < config_.concurrency; ++i) {
                pool_->submit([this]() {
                    while (should_continue()) {
                        execute_request();
                    }
                });
            }
        }

        // Wait
        wait_for_completion();

        // Stop workers and cleanup
        running_ = false;
        stop_requested_ = true;
        pool_.reset();

        // Record test
        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time_);
        collector_.set_duration(duration);

        // Gather results
        stats::Metrics metrics = collector_.get_metrics();
        stats::Percentiles percentiles = collector_.calculate_percentiles();

        // Calculate metrics
        double duration_seconds = duration.count() / 1'000'000.0;
        double requests_per_second = metrics.total_requests / duration_seconds;

        // return results
        return Results {
            .metrics = metrics,
            .percentiles = percentiles,
            .duration = duration,
            .requests_per_second = requests_per_second
        };
    }

    // Stop load test early
    void Engine::stop() {
        stop_requested_ = true;
        running_ = false;
    }
}