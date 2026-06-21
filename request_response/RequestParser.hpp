#pragma once

#include "HttpUtils.hpp"
#include <string>
#include <sstream>
#include <stdexcept>
#include <map>

class RequestParser {
private:
    static std::string trim(std::string str) {
        if (!str.empty() && str.at(str.size() - 1) == '\r') {
            str.resize(str.size() - 1);
        }
        size_t first = str.find_first_not_of(" ");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" ");
        return str.substr(first, (last - first + 1));
    }

public:
    static HttpRequest parse(const std::string &rawRequest) {
        HttpRequest req;
        std::stringstream requestStream(rawRequest);
        std::string line;

        // 1. Parse Request Line
        if (std::getline(requestStream, line)) {
            line = trim(line);
            std::stringstream lineStream(line);
            lineStream >> req.method >> req.path >> req.version;
        }

        // 2. Parse Headers
        while (std::getline(requestStream, line)) {
            line = trim(line);
            if (line.empty()) {
                break; // End of header block
            }

            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                std::string key = trim(line.substr(0, colonPos));
                std::string value = trim(line.substr(colonPos + 1));
                req.headers[key] = value;
            }
        }

        // 3. Extract Body reliably by matching the delimiter string directly
        if (req.method == "POST") {
            std::map<std::string, std::string>::const_iterator it = req.headers.find("Content-Length");
            if (it == req.headers.end()) {
                throw std::runtime_error("400 Bad Request: Missing Content-Length");
            }
            
            int contentLength = 0;
            std::stringstream lengthStream(it->second);
            lengthStream >> contentLength;

            if (contentLength > 0) {
                // Find where the header block officially ends in the raw string
                size_t bodyStart = rawRequest.find("\r\n\r\n");
                if (bodyStart != std::string::npos) {
                    bodyStart += 4; // Advance past the 4 delimiter bytes
                } else {
                    // Fall back to alternate header break representation if needed
                    bodyStart = rawRequest.find("\n\n");
                    if (bodyStart != std::string::npos) {
                        bodyStart += 2;
                    }
                }

                // If a valid start position is found, capture the raw bytes
                if (bodyStart != std::string::npos && bodyStart < rawRequest.length()) {
                    req.body = rawRequest.substr(bodyStart, contentLength);
                }
                
                // Final confirmation step
                if (req.body.length() < static_cast<size_t>(contentLength)) {
                    throw std::runtime_error("400 Bad Request: Body size mismatch.");
                }
            }
        }
        return req;
    }
};