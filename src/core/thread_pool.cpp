#include "core/thread_pool.hpp"
#include <chrono>
#include <cstddef>
#include <exception>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>

namespace surge::core {
    // Constructor: Create N workers
    ThreadPool::ThreadPool(size_t num_threads) {
        // Reserve space in vector to avoid reallocations
        workers_.reserve(num_threads);

        // Create worker threads
        for (size_t i = 0; i < num_threads; ++i) {
            // emplace_back construct jthread in place
            // Pass a lambda that captures 'this' and calls worker_loop
            workers_.emplace_back([this]() {
                worker_loop();
            });
        }
        // REMOVE AFTER TESTING
        std::cout << "ThreadPool created with " << num_threads << " workers\n";
    }

    // Destructor: Stop workers and wait for them to finish
    ThreadPool::~ThreadPool() {
        // Signal all workers to stop
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            stop_flag_ = true; // Tell workers to exit their loop
        }
        // Lock released here (lock_guard) destructor

        // Wake up all sleeping workers
        condition_.notify_all();

        // REMOVE AFTER TESTING
        std::cout << "ThreadPool destroyed, all workers joined\n";
    }

    // Submit a task to be executed by a worker
    void ThreadPool::submit(std::function<void()> task) {
        // Critical section: modifying a shared queue
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            tasks_.push(task);
        }
        // Lock release

        // Wake up one sleeping worker
        condition_.notify_one();
    }

    // Wait for all tasks to be completed
    void ThreadPool::wait_for_completion() {
        while (true) {
            {
                std::lock_guard<std::mutex> lock(queue_mutex_);

                // Check if all work is done
                if(tasks_.empty() && active_tasks_ == 0) {
                    return; // All finished
                }
            }
            // Lock release

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    // Worker thread main loop
    void ThreadPool::worker_loop() {
        while (true) {
            std::function<void()> task; // Hold the task to execute
            // Critical section: accessing shared state
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);

                condition_.wait(lock, [this]() {
                    return !tasks_.empty() || stop_flag_;
                });

                // If stopping and no tasks left
                if (stop_flag_ && tasks_.empty()) {
                    return; // Exit worker_loop, thread terminates
                }

                // Get task from the queue
                task = tasks_.front();  // Copy the task
                tasks_.pop();           // Remove from queue

                // Increment active tasks
                active_tasks_++;
            }
            // Lock release

            // Execute the task
            try {
                task(); // Call task function
            } catch (const std::exception& e) {
                // Task threw an exception - log but dont crash the worker
                std::cerr << "Task threw exception: " << e.what() << "\n";
            } catch (...) {
                // Unknown exception
                std::cerr << "Task threw unknown exception\n";
            }
            // Task finished, decrement active count
            active_tasks_--;
        }
    }
}