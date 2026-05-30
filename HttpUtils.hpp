#pragma once

#include <string>
#include <unordered_map>
#include <iostream>
#include <sstream>

struct HttpRequest {
    std::string method;
    std::string path;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    void print() const {
        std::cout << "=== HTTP REQUEST ===\n";
        std::cout << "Method: " << method << "\n";
        std::cout << "Path: " << path << "\n";
        std::cout << "Version: " << version << "\n";
        std::cout << "Headers:\n";
        for (const auto &[key, value] : headers) {
            std::cout << " [" << key << "] -> " << value << "\n";
        }
        std::cout << "Body: " << body << "\n";
        std::cout << "=========================\n";
    }
};

class HttpResponse {
private:
    std::string version = "HTTP/1.1";
    int statusCode;
    std::string statusMessage;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

public:
    HttpResponse(int code, std::string message) : statusCode(code), statusMessage(message) {}

    void setHeader(const std::string &key, const std::string &value) {
        headers[key] = value;
    }

    void setBody(const std::string &b) {
        body = b;
        setHeader("Content-Length", std::to_string(body.length()));
    }

    std::string toString() const {
        std::stringstream ss;
        ss << version << " " << statusCode << " " << statusMessage << "\r\n";
        for (const auto &[key, value] : headers) {
            ss << key << ": " << value << "\r\n";
        }
        ss << "\r\n" << body;
        return ss.str();
    }
};
