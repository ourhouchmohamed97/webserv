#pragma once

#include <string>
#include <map>
#include <sstream>
#include <iostream>

struct HttpRequest {
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers; // Changed to std::map
    std::string body;

    void print() const {
        std::cout << "Method: " << method << " | Path: " << path << "\n";
        std::map<std::string, std::string>::const_iterator it;
        for (it = headers.begin(); it != headers.end(); ++it) {
            std::cout << "[" << it->first << "] = " << it->second << "\n";
        }
        std::cout << "Body: " << body << "\n";
        std::cout << "=========================\n";
    }
};

struct HttpResponse {
    int statusCode;
    std::string statusText;
    std::string version;
    std::map<std::string, std::string> headers; // Changed to std::map
    std::string body;

    // C++98 explicit constructor (No inline value initialization allowed)
    HttpResponse(int code = 200, std::string text = "OK") 
        : statusCode(code), statusText(text), version("HTTP/1.1") {}

    void setHeader(const std::string& key, const std::string& value) {
        headers[key] = value;
    }

    void setBody(const std::string& newBody) {
        body = newBody;
        std::stringstream ss;
        ss << body.length();
        setHeader("Content-Length", ss.str()); // Replaced to_string
    }

    std::string toString() const {
        std::stringstream ss;
        ss << version << " " << statusCode << " " << statusText << "\r\n";
        
        std::map<std::string, std::string>::const_iterator it;
        for (it = headers.begin(); it != headers.end(); ++it) {
            ss << it->first << ": " << it->second << "\r\n";
        }
        ss << "\r\n" << body;
        return ss.str();
    }
};