#include "output/reporter.hpp"
#include <chrono>
#include <cstdint>
#include <fstream>
#include <ios>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <streambuf>
#include <string>

namespace surge::output {
    // Format microseconds
    std::string Reporter::format_duration(std::chrono::microseconds duration) {
        // String builder
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(3);

        double value = duration.count();

        if (value >= 1'000'000) {
            // Seconds
            oss << (value / 1'000'000.0) << "s";
        } else if (value >= 1'000) {
            // Milliseconds
            oss << (value / 1'000.0) << "ms";
        } else {
            // Microseconds
            oss << value << "μs";
        }

        return oss.str();
    }

    // Format number with thousand seperator
    std::string Reporter::format_number(uint64_t number) {
        std::string num_str = std::to_string(number);
        std::string result;

        int count = 0;
        for (int i = num_str.length() - 1; i >= 0; --i) {
            if (count == 3) {
                result = ',' + result;
                count = 0;
            }
            result = num_str[i] + result;
            count++;
        }

        return result;
    }

    // Format percetage with 2 decimal places
    std::string Reporter::format_percent(double value) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << value << "%";
        return oss.str();
    }

    // Format latency (microseconds)
    std::string Reporter::format_latency(uint64_t microseconds) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);

        if (microseconds >= 1'000) {
            // Show as milliseconds
            oss << (microseconds / 1'000.0) << "ms";
        } else {
            oss << microseconds << "μs";
        }

        return oss.str();
    }

    // Draw horizontal line
    std::string Reporter::line(size_t length, char ch) {
        return std::string(length, ch);
    }

    // Print results to console (plain text)
    void Reporter::print(const core::Results& results) {
        const auto& m = results.metrics;
        const auto& p = results.percentiles;

        std::cout << "\n" << line(60, '=') << "\n";
        std::cout << "\tLOAD TEST RESULTS\n";
        std::cout << line(60, '=') << "\n\n";

        // Summary
        std::cout << "Summary:\n";
        std::cout << "  Duration:        " << format_duration(results.duration) << "\n";
        std::cout << "  Total Requests:  " << format_number(m.total_requests) << "\n";
        std::cout << "  Successful:      " << format_number(m.successful_requests);
        
        if (m.total_requests > 0) {
            double success_rate = (m.successful_requests * 100.0) / m.total_requests;
            std::cout << " (" << format_percent(success_rate) << ")";
        }

        std::cout << "\n";
        std::cout << "  Failed:          " << format_number(m.failed_requests);
    
        if (m.total_requests > 0) {
            double failure_rate = (m.failed_requests * 100.0) / m.total_requests;
            std::cout << " (" << format_percent(failure_rate) << ")";
        }

        std::cout << "\n";
        std::cout << "  Requests/sec:    " << std::fixed << std::setprecision(2) 
              << results.requests_per_second << "\n\n";

        // Latency statistics
        if (m.successful_requests > 0) {
            double avg = m.total_latency.count() / static_cast<double>(m.successful_requests);
            
            std::cout << "Latency:\n";
            std::cout << "  Average:  " << format_latency(static_cast<uint64_t>(avg)) << "\n";
            std::cout << "  Min:      " << format_latency(m.min_latency.count()) << "\n";
            std::cout << "  Max:      " << format_latency(m.max_latency.count()) << "\n";
            std::cout << "  p50:      " << format_latency(p.p50) << "\n";
            std::cout << "  p75:      " << format_latency(p.p75) << "\n";
            std::cout << "  p90:      " << format_latency(p.p90) << "\n";
            std::cout << "  p95:      " << format_latency(p.p95) << "\n";
            std::cout << "  p99:      " << format_latency(p.p99) << "\n";
            std::cout << "  p99.9:    " << format_latency(p.p999) << "\n\n";
        }

        // Status code breakdown
        std::cout << "Status Codes:\n";
        for (const auto& [code, count] : m.status_codes) {
            double percentage = (count * 100.0) / m.total_requests;
            std::cout << "  " << code << ":  " << format_number(count) 
                    << " (" << format_percent(percentage) << ")\n";
        }
        std::cout << "\n" << line(60, '=') << "\n";
    }

    void Reporter::print_coloured(const core::Results &results) {
        const auto& m = results.metrics;
        const auto& p = results.percentiles;

        using namespace Colours;

        // Headers
        std::cout << "\n" << CYAN << BOLD << line(60, '=') << RESET << "\n";
        std::cout << CYAN << BOLD << "\t LOAD TEST RESULTS" << RESET << "\n";
        std::cout << CYAN << BOLD << line(60, '=') << RESET << "\n\n";

        // Summary
        std::cout << BOLD << "Summary:" << RESET << "\n";
        std::cout << "\tDuration:       " << MAGENTA << format_duration(results.duration) << RESET << "\n";
        std::cout << "\tTotal Requests: " << BLUE << format_number(m.total_requests) << RESET << "\n";
        std::cout << "\tSuccessful:     " << GREEN << format_number(m.successful_requests) << RESET;

        if (m.total_requests > 0) {
            double success_rate = (m.successful_requests * 100.0) / m.total_requests;
            std::cout << "\t(" << GREEN << format_percent(success_rate) << RESET << ")";
        }
        std::cout << "\n";

        std::cout << "\tFailed:         ";
        if(m.failed_requests > 0) {
            std::cout << RED << format_number(m.failed_requests) << RESET;
        } else {
            std::cout << GREEN << "0" << RESET;
        }

        if (m.total_requests > 0) {
            double failure_rate = (m.failed_requests * 100.0) / m.total_requests;

            if (m.failed_requests > 0) {
                std::cout << "\t(" << RED << format_percent(failure_rate) << RESET << ")";
            } else {
                std::cout << "\t(" << GREEN << format_percent(failure_rate) << RESET << ")";
            }
        }
        std::cout << "\n";

        std::cout << "\tRequests/sec:   " << YELLOW << std::fixed << std::setprecision(2)
                  << results.requests_per_second << RESET << "\n\n";

        // Latency
        if (m.successful_requests > 0) {
            double avg = m.total_latency.count() / static_cast<double>(m.successful_requests);
            
            std::cout << BOLD << "Latency:" << RESET << "\n";
            std::cout << "  Average:  " << YELLOW << format_latency(static_cast<uint64_t>(avg)) << RESET << "\n";
            std::cout << "  Min:      " << GREEN << format_latency(m.min_latency.count()) << RESET << "\n";
            std::cout << "  Max:      " << RED << format_latency(m.max_latency.count()) << RESET << "\n";
            std::cout << "  p50:      " << BLUE << format_latency(p.p50) << RESET << "\n";
            std::cout << "  p75:      " << BLUE << format_latency(p.p75) << RESET << "\n";
            std::cout << "  p90:      " << YELLOW << format_latency(p.p90) << RESET << "\n";
            std::cout << "  p95:      " << YELLOW << format_latency(p.p95) << RESET << "\n";
            std::cout << "  p99:      " << RED << format_latency(p.p99) << RESET << "\n";
            std::cout << "  p99.9:    " << RED << format_latency(p.p999) << RESET << "\n\n";
        }

        // Status codes
        std::cout << BOLD << "Status Codes:" << RESET << "\n";
        for (const auto& [code, count] : m.status_codes) {
            double percentage = (count * 100.0) / m.total_requests;
            
            // Color based on status code
            std::string color;
            if (code >= 200 && code < 300) {
                color = GREEN;  // 2xx = success
            } else if (code >= 300 && code < 400) {
                color = BLUE;   // 3xx = redirect
            } else if (code >= 400 && code < 500) {
                color = YELLOW; // 4xx = client error
            } else {
                color = RED;    // 5xx = server error
            }
            
            std::cout << "  " << color << code << RESET << ":  " 
                    << format_number(count) << " (" << format_percent(percentage) << ")\n";
        }
        std::cout << "\n" << CYAN << BOLD << line(60, '=') << RESET << "\n";
    }

    bool Reporter::save_to_file(const core::Results &results, const std::string &filepath) {
        std::ofstream file(filepath);

        if (!file.is_open()) {
            std::cerr << "Error: could not open file to write: " << filepath << "\n";
            return false;
        }

        // Redirect cout to file temporarily
        std::streambuf* original_cout = std::cout.rdbuf();
        std::cout.rdbuf(file.rdbuf());
        // Print to file (uses plain print, no colors)
        print(results);
        
        // Restore cout
        std::cout.rdbuf(original_cout);
        
        file.close();
        return true;
    }
}