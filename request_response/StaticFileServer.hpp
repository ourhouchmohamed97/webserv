#pragma once

#include "../request_response/HttpUtils.hpp"
#include "../config_cgi/LocationConfig.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <vector>
#include <cstdio>

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

    static HttpResponse generateErrorResponse(int code, const std::string& msg) {
        HttpResponse res;
        res.statusCode = code;
        res.headers["Content-Type"] = "text/html";
        std::stringstream ss;
        ss << "<html><body><h1>" << code << " " << msg << "</h1></body></html>";
        res.body = ss.str();
        return res;
    }

public:
    static HttpResponse serveFile(const HttpRequest& req, const LocationConfig& loc) {
        HttpResponse res;
        
        if (req.body.length() > loc.getClientMaxBodySize()) {
            return generateErrorResponse(413, "Payload Too Large");
        }

        std::vector<std::string> methods = loc.getAllowedMethods();
        if (!methods.empty() && std::find(methods.begin(), methods.end(), req.method) == methods.end()) {
            return generateErrorResponse(405, "Method Not Allowed");
        }

        std::string fullPath = loc.getRoot() + req.path;

        if (!req.path.empty() && req.path.at(req.path.length() - 1) == '/') {
            if (!loc.getIndex().empty()) {
                fullPath += loc.getIndex();
            } else {
                return generateErrorResponse(403, "Forbidden");
            }
        }

        // 🌟 EXPLICIT DESTRUCTIVE FILE REMOVAL TERMINATOR
        // 🌟 EXPLICIT DESTRUCTIVE FILE REMOVAL TERMINATOR
        if (req.method == "DELETE") {
            // Log this path to your terminal window so you can see exactly what path C++ is trying to delete!
            std::cout << "[DEBUG DELETE] Raw req.path: " << req.path << std::endl;
            std::cout << "[DEBUG DELETE] Location root: " << loc.getRoot() << std::endl;

            std::string fileToDitch = req.path;
            
            // If the request path is "/upload/filename.txt" and your root is already "www/upload",
            // we need to strip out the extra "/upload" prefix so they don't double up!
            if (fileToDitch.find("/upload/") == 0) {
                fileToDitch = fileToDitch.substr(7); // Strips "/upload" leaving just "filename.txt"
            } else if (fileToDitch.find("/") == 0) {
                fileToDitch = fileToDitch.substr(1); // Strips leading slash if necessary
            }

            // Combine clean filename with the designated folder root
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
                return generateErrorResponse(404, "Not Found: File deletion target path unresolvable.");
            }
        }

        // Standard operational logic paths for normal GET/POST content views
        std::ifstream file(fullPath.c_str(), std::ios::binary);
        if (!file.is_open()) {
            return generateErrorResponse(404, "Not Found");
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