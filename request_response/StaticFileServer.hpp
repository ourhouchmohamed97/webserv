#pragma once

#include "HttpUtils.hpp"
#include "config & tokenize/LocationConfig.hpp"
#include "ErrorPageFactory.hpp"
#include "AutoIndex.hpp"
#include "MimeTypeHelper.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

class StaticFileServer {
public:
    static HttpResponse serveFile(const HttpRequest& req, const LocationConfig& loc) {
        HttpResponse res;
        
        // 1. Check Max Body Size Limit
        if (req.body.length() > loc.getClientMaxBodySize()) {
            return ErrorPageFactory::create(413, "Payload Too Large");
        }

        // 2. Check Allowed Methods Vector
        std::vector<std::string> methods = loc.getAllowedMethods();
        if (!methods.empty() && std::find(methods.begin(), methods.end(), req.method) == methods.end()) {
            return ErrorPageFactory::create(405, "Method Not Allowed");
        }

        // 3. Resolve File Path System
        std::string fullPath = loc.getRoot() + req.path;

        // 4. Handle Directory Access / AutoIndex Switch
        // (Basic check if path points to directory)
        if (!req.path.empty() && req.path.at(req.path.length() - 1) == '/') {
            if (!loc.getIndex().empty()) {
                fullPath += loc.getIndex();
            } else {
                if (loc.getAutoindex()) {
                    res.statusCode = 200;
                    res.statusMessage = "OK";
                    res.headers["Content-Type"] = "text/html";
                    res.body = AutoIndex::generate(loc.getRoot(), req.path);
                    return res;
                } else {
                    return ErrorPageFactory::create(403, "Forbidden");
                }
            }
        }

        // 5. Serve Static Resource File
        std::ifstream file(fullPath.c_str(), std::ios::binary);
        if (!file.is_open()) {
            return ErrorPageFactory::create(404, "Not Found");
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        res.body = buffer.str();
        file.close();

        res.statusCode = 200;
        res.statusMessage = "OK";
        res.headers["Content-Type"] = MimeTypeHelper::getType(fullPath);
        
        return res;
    }
};