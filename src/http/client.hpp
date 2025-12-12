#pragma once

#include <cstdint>
#include "http/request.hpp"
#include "http/response.hpp"

namespace surge::http {

class Client {
public:
    Client() = default;
    ~Client() = default;
    
    // Disable copy
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
    
    // Make an HTTP request
    Response execute(const Request& request);

private:
    struct ParsedUrl {
        std::string host;
        std::uint16_t port;
        std::string path;
    };
    
    ParsedUrl parse_url(const std::string& url);
    std::string build_request_string(const Request& request, 
                                     const std::string& host);
    Response parse_response(const std::string& raw_response);
};

}  // namespace surge::http