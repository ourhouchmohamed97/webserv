#pragma once

#include "../request_response/HttpUtils.hpp"
#include "../config_cgi/LocationConfig.hpp"
#include "ErrorPageFactory.hpp" // 🌟 Linked to your high-fidelity error styles!
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <vector>
#include <cstdio>
#include <iostream>

class StaticFileServer {
private:
    static std::string getContentType(const std::string& path) {
        size_t dot = path.find_last_of(".");
        if (dot == std::string::npos) return "text/plain";
        std::string ext = path.substr(dot);
        if (ext == ".html" || ext == ".htm") return "text/html";
        if (ext == ".css") return "text/css";
        if (ext == ".js") return "application/javascript";
        if (ext == ".png") return "image/png";
        if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
        return "text/plain";
    }

    // 🌟 LINKED: Now calls your custom dashboard factory instead of printing plain unstyled text
    static HttpResponse generateErrorResponse(int code) {
        HttpResponse res;
        res.statusCode = code;
        res.headers["Content-Type"] = "text/html";
        res.body = ErrorPageFactory::getErrorPage(code);
        return res;
    }

public:
    static HttpResponse serveFile(const HttpRequest& req, const LocationConfig& loc) {
        HttpResponse res;
        
        // 1. Guard Rule: Max Body Size Limit Caps
        if (req.body.length() > loc.getClientMaxBodySize()) {
            return generateErrorResponse(413);
        }

        // 2. Guard Rule: Method Permissions Check
        std::vector<std::string> methods = loc.getAllowedMethods();
        if (!methods.empty() && std::find(methods.begin(), methods.end(), req.method) == methods.end()) {
            return generateErrorResponse(405);
        }

        // 🌟 EXPLICIT DESTRUCTIVE FILE REMOVAL TERMINATOR (DELETE Pathway Execution)
        if (req.method == "DELETE") {
            std::cout << "[DEBUG DELETE] Raw req.path: " << req.path << std::endl;
            std::cout << "[DEBUG DELETE] Location root: " << loc.getRoot() << std::endl;

            std::string fileToDitch = req.path;
            
            if (fileToDitch.find("/upload/") == 0) {
                fileToDitch = fileToDitch.substr(7); // Strips "/upload" leaving just "filename.txt"
            } else if (fileToDitch.find("/") == 0) {
                fileToDitch = fileToDitch.substr(1); // Strips leading slash if necessary
            }

            std::string trueStoragePath = loc.getRoot();
            if (!trueStoragePath.empty() && trueStoragePath.at(trueStoragePath.length() - 1) != '/') {
                trueStoragePath += "/";
            }
            trueStoragePath += fileToDitch;

            std::cout << "[DEBUG DELETE] Attempting to erase target file at: " << trueStoragePath << std::endl;

            if (std::remove(trueStoragePath.c_str()) == 0) {
                res.statusCode = 200; 
                res.headers["Content-Type"] = "text/plain";
                res.body = "File successfully removed from server cluster disk volume.\n";
                return res;
            } else {
                std::cerr << "[DEBUG DELETE] std::remove failed for: " << trueStoragePath << " (Error: " << strerror(errno) << ")" << std::endl;
                return generateErrorResponse(404);
            }
        }

        // 3. Resolve File Path System (For GET / POST static reads)
        std::string fullPath = loc.getRoot() + req.path;

        // 4. Handle Directory Access Requests
        if (!req.path.empty() && req.path.at(req.path.length() - 1) == '/') {
            if (!loc.getIndex().empty()) {
                fullPath += loc.getIndex();
            } else {
                return generateErrorResponse(403);
            }
        }

        // 5. Standard Operational Logic for Asset Data Reads
        std::ifstream file(fullPath.c_str(), std::ios::binary);
        if (!file.is_open()) {
            return generateErrorResponse(404);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        res.body = buffer.str();
        file.close();

        res.statusCode = 200;
        res.headers["Content-Type"] = getContentType(fullPath);
        
        return res;
    }
};