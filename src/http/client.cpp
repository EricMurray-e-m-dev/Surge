#include "http/client.hpp"
#include "http/request.hpp"
#include "http/response.hpp"

// System headers for socket programming
#include <chrono>
#include <sys/socket.h>     // socket(), connect(), send(), recv()
#include <netinet/in.h>     // sockaddr_in struct
#include <arpa/inet.h>      // inet_addr()
#include <netdb.h>          // gethostbyname() for DNS lookup
#include <unistd.h>         // close() for file descriptors

// Standard library
#include <cstring>          // memset(), strlen()
#include <sstream>          // std::istringstream for string parsing

namespace surge::http {
    // Parse URL: "http://example.com:8080/api/v1"
    // Protocol: "http://", Host: "example.com", Port: "8080", Path: "/api/v1"
    Client::ParsedUrl Client::parse_url(const std::string& url) {
        ParsedUrl result;

        // Find "://" ends, skips protocol
        // std::string::find returns position or std::string::npos if not found
        size_t protocol_end = url.find("://");
        if (protocol_end == std::string::npos) {
            // No protocol specified, assume starts at beginning 
            protocol_end = 0;
        } else {
            // Skip past ://
            protocol_end += 3;
        }

        // Find where path starts (first '/' after host)
        size_t path_start = url.find('/', protocol_end);

        // Extract host:port part
        std::string host_port;
        if (path_start == std::string::npos) {
            // No path specified (e.g. "example.com")
            host_port = url.substr(protocol_end);
            result.path = "/";
        } else {
            // Has a path
            host_port = url.substr(protocol_end, path_start - protocol_end);
            result.path = url.substr(path_start);   // Everything from '/' onwards
        }

        // Split host:port
        size_t port_separator = host_port.find(':');
        if (port_separator == std::string::npos) {
            // No port specified use default HTTP
            result.host = host_port;
            result.port = 80;
        } else {
            result.host = host_port.substr(0, port_separator);
            std::string port_str = host_port.substr(port_separator + 1);
            result.port = static_cast<std::uint16_t>(std::stoi(port_str));
        }

        return result;
    }

    std::string Client::build_request_string(const Request& request, const std::string& host) {
        std::string result;

        // Parse url to get path
        ParsedUrl url = parse_url(request.url);

        // Request line: "GET /api/users HTTP/1.1\r\n"
        result += request.method + " " + url.path + " HTTP/1.1\r\n";

        // Host header (Required in HTTP 1.1)
        result += "Host: " + host + "\r\n";

        // Connection header
        result += "Connection: close\r\n";

        // If theres a body, add content length header
        if (!request.body.empty()) {
            result += "Content-Length: " + std::to_string(request.body.length()) + "\r\n";
        }

        // End of headers
        result += "\r\n";

        // Add body if present
        if (!request.body.empty()) {
            result += request.body;
        }

        return result;
    }

    // Parse raw HTTP response into Response struct
    Response Client::parse_response(const std::string& raw_response) {
        Response response;

        // Find end of the status line
        size_t status_line_end = raw_response.find("\r\n");
        if (status_line_end == std::string::npos) {
            response.success = false;
            response.error_message = "Invalid HTTP Response: no status line.";
            return response;
        }

        // Extract status line: "HTTP/1.1 200 OK"
        std::string status_line = raw_response.substr(0, status_line_end);

        // Parse status code (second token)
        std::istringstream status_stream(status_line);
        std::string http_version, status_code_str;
        status_stream >> http_version >> status_code_str;

        // Convert status response to integer
        try {
            response.status_code = static_cast<std::uint16_t>(std::stoi(status_code_str));
        } catch (...) {
            response.success = false;
            response.error_message = "Invalid status code";
            return response;
        }

        // Find where headers end "\r\n\r\n"
        size_t headers_end = raw_response.find("\r\n\r\n");
        if (headers_end == std::string::npos) {
            response.success = false;
            response.error_message = "Invalid HTTP response: no header/body seperator";
            return response;
        }

        // Extract body (everything after headers)
        response.body = raw_response.substr(headers_end + 4);
        response.success = true;

        return response;
    }

    // Main method: execute HTTP request
    Response Client::execute(const Request& request) {
        Response response;

        // Start timing
        auto start_time = std::chrono::high_resolution_clock::now();

        // Parse URL
        ParsedUrl url = parse_url(request.url);

        if (url.host.empty()) {
            response.success = false;
            response.error_message = "Invalid URL: no host specified";
            return response;
        }

        // Resolve hostname to an IP address
        struct hostent* host_info = gethostbyname(url.host.c_str());
        if (host_info == nullptr) {
            response.success = false;
            response.error_message = "Failed to resolve host: " + url.host;
            return response;
        }

        // Create a socket, AF_INET = IPv4, SOCK_STREAM = TCP, 0 = default protocol;
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            response.success = false;
            response.error_message = "Failed to create socket";
            return response;
        }

        // Setup server address structure
        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(url.port);

        // Copy IP address from host info
        memcpy(&server_addr.sin_addr, host_info->h_addr_list[0], host_info->h_length);

        // Connect to the server
        if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            close(sock);    // Clean up socket before returning
            response.success = false;
            response.error_message = "Failed to connect to " + url.host;
            return response;
        }

        // Build & send HTTP request
        std::string request_str = build_request_string(request, url.host);

        ssize_t bytes_sent = send(sock, request_str.c_str(), request_str.length(), 0);
        if (bytes_sent < 0) {
            close(sock);
            response.success = false;
            response.error_message = "Failed to send request";
            return response;
        }

        // Receive response
        std::string response_data;
        char buffer[4096];   // 4KB buffer

        while (true) {
            ssize_t bytes_received = recv(sock, buffer, sizeof(buffer), 0);

            if (bytes_received < 0) {
                close(sock);
                response.success = false;
                response.error_message = "Failed to received response";
                return response;
            }

            if (bytes_received == 0) {
                // Connection closed by server
                break;
            }

            response_data.append(buffer, bytes_received);
        }

        close(sock);

        // Calculate latency
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        response.latency = duration;

        // Parse response
        response = parse_response(response_data);
        response.latency = duration;

        return response;
    }
}