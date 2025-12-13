#pragma once

#include <cstddef>                  // size_t
#include <functional>               // std::function
#include <queue>                    // std::queue
#include <thread>                   // std::jthread
#include <mutex>                    // std::mutex
#include <condition_variable>       // std::condition_variable
#include <vector>                   // std::vector

namespace surge::core {
    // A ThreadPool manages a fixed number of worker threads
    // Tasks are submitted to a queue and picked up by free worker threads
    class ThreadPool {
        public:
            // Constructor create N workers
            // num_threads - number of workers to create
            explicit ThreadPool(size_t num_threads);

            // Destructor: automatically stops workers and waits for finish
            ~ThreadPool();

            // Disable copy
            ThreadPool(const ThreadPool&) = delete;
            ThreadPool& operator=(const ThreadPool&) = delete;

            // Submit a task to a worker
            // task - a callable function with void() sig
            void submit(std::function<void()> task);

            // Wait for all to be submitted
            void wait_for_completion();

        private:
            // Internal State

            std::vector<std::jthread> workers_; // Worker threads

            std::queue<std::function<void()>> tasks_;   // Task queue

            std::mutex queue_mutex_;    // Protects access to the queue

            std::condition_variable condition_; // For signaling workers

            std::atomic<size_t> active_tasks_{0};   // Count of running tasks

            bool stop_flag_{false}; // Signal workers to stop

            void worker_loop(); // Worker thread function - runs in loop processing tasks
    };
}