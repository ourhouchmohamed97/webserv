#pragma once

#include "../request_response/HttpUtils.hpp"
#include "../config_cgi/LocationConfig.hpp"
#include "../request_response/MultipartParser.hpp"
#include "../request_response/ErrorPageFactory.hpp" // 🌟 Include your design system factory
#include <fstream>
#include <sstream>
#include <iostream>

class UploadHandler {
private:
    // Helper to generate styled error pages quickly
    static HttpResponse generateError(int code) {
        HttpResponse res;
        res.statusCode = code;
        res.headers["Content-Type"] = "text/html";
        res.body = ErrorPageFactory::getErrorPage(code);
        return res;
    }

public:
    static HttpResponse handle(const HttpRequest& req, const LocationConfig& loc) {
        HttpResponse res;

        // 1. Verify Content-Type header integrity definitions
        std::map<std::string, std::string>::const_iterator it = req.headers.find("Content-Type");
        if (it == req.headers.end()) {
            it = req.headers.find("content-type");
        }
        
        if (it == req.headers.end() || it->second.find("multipart/form-data") == std::string::npos) {
            return generateError(400); // 🌟 Modern UI alert
        }

        // 2. Call static parsing structures
        UploadedFile uploaded = MultipartParser::parse(req);

        // 3. Extract metadata streams
        std::string filename = uploaded.filename;
        std::string fileData = uploaded.content;

        if (filename.empty()) {
            return generateError(400); // 🌟 Modern UI alert
        }

        // 4. Resolve the location storage destination directory path 
        // 🌟 STRATEGY: Look for an explicit upload path rule. If empty, fall back safely to location root.
        std::string uploadDir = loc.getUploadStore(); 
        if (uploadDir.empty()) {
            uploadDir = loc.getRoot(); // Fallback strategy safely applied
        }

        // Secure formatting to prevent broken double slashes or missing divider characters
        if (uploadDir.empty()) {
            uploadDir = "./www/upload"; // Hard default fallback protection to preserve execution flow
        }
        if (uploadDir.at(uploadDir.length() - 1) != '/') {
            uploadDir += "/";
        }

        std::string targetFilePath = uploadDir + filename;
        std::cout << "[DEBUG UPLOAD] Target file routing location resolved to: " << targetFilePath << std::endl;

        // 5. Open raw binary stream channels
        std::ofstream outFile(targetFilePath.c_str(), std::ios::binary | std::ios::out);
        if (!outFile.is_open()) {
            std::cerr << "[ERROR UPLOAD] Failed to open target path destination for writing." << std::endl;
            return generateError(500); // 🌟 Launches your dark-themed 500 error page!
        }

        outFile.write(fileData.c_str(), fileData.size());
        outFile.close();

        // 6. Build completion metrics packet return payload
        res.statusCode = 201; 
        res.headers["Content-Type"] = "text/plain";
        res.body = "File successfully ingested and written to system storage matrix.\n";
        return res;
    }
};