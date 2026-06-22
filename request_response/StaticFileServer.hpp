#pragma once

#include "../request_response/HttpUtils.hpp"
#include "../config_cgi/LocationConfig.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>

class StaticFileServer {
private:
    // Simple self-contained fallback extension helper to avoid MimeTypeHelper issues
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

    // Inline fallback page generator to bypass ErrorPageFactory completely
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
        
        // 1. Check Max Body Size Limit
        if (req.body.length() > loc.getClientMaxBodySize()) {
            return generateErrorResponse(413, "Payload Too Large");
        }

        // 2. Check Allowed Methods Vector
        std::vector<std::string> methods = loc.getAllowedMethods();
        if (!methods.empty() && std::find(methods.begin(), methods.end(), req.method) == methods.end()) {
            return generateErrorResponse(405, "Method Not Allowed");
        }

        // 3. Resolve File Path System
        std::string fullPath = loc.getRoot() + req.path;

        // 4. Handle Directory Access
        if (!req.path.empty() && req.path.at(req.path.length() - 1) == '/') {
            if (!loc.getIndex().empty()) {
                fullPath += loc.getIndex();
            } else {
                return generateErrorResponse(403, "Forbidden");
            }
        }

        // 5. Serve Static Resource File
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