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

        if (std::getline(requestStream, line)) {
            line = trim(line);
            std::stringstream lineStream(line);
            lineStream >> req.method >> req.path >> req.version;
        }

        while (std::getline(requestStream, line)) {
            line = trim(line);
            if (line.empty()) break;

            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                std::string key = trim(line.substr(0, colonPos));
                std::string value = trim(line.substr(colonPos + 1));
                req.headers[key] = value;
            }
        }

        if (req.method == "POST") {
            std::map<std::string, std::string>::const_iterator it = req.headers.find("Content-Length");
            if (it == req.headers.end()) {
                throw std::runtime_error("400 Bad Request: Missing Content-Length");
            }
            
            // C++98 compliant conversion using stringstream
            int contentLength = 0;
            std::stringstream lengthStream(it->second);
            lengthStream >> contentLength;

            if (contentLength > 0) {
                std::ostringstream remainder;
                remainder << requestStream.rdbuf();
                std::string remainingContent = remainder.str();

                if (remainingContent.length() >= static_cast<size_t>(contentLength)) {
                    req.body = remainingContent.substr(0, contentLength);
                } else {
                    throw std::runtime_error("400 Bad Request: Body size mismatch.");
                }
            }
        }
        return req;
    }
};