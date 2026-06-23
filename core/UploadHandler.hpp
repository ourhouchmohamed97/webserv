#pragma once

#include "../request_response/HttpUtils.hpp"
#include "../config_cgi/LocationConfig.hpp"
#include "../request_response/MultipartParser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

class UploadHandler {
public:
    static HttpResponse handle(const HttpRequest& req, const LocationConfig& loc) {
        HttpResponse res;

        // 1. Verify that a Content-Type header exists and contains a multipart boundary definition
        std::map<std::string, std::string>::const_iterator it = req.headers.find("Content-Type");
        if (it == req.headers.end()) {
            it = req.headers.find("content-type");
        }
        
        if (it == req.headers.end() || it->second.find("multipart/form-data") == std::string::npos) {
            res.statusCode = 400;
            res.headers["Content-Type"] = "text/html";
            res.body = "<html><body><h1>400 Bad Request: Expected Multipart Payload</h1></body></html>";
            return res;
        }

        // 2. Call your static parser directly using the entire HttpRequest object
        UploadedFile uploaded = MultipartParser::parse(req);

        // 3. Extract metadata (Using .content for the file byte stream array)
        std::string filename = uploaded.filename;
        std::string fileData = uploaded.content; // 🌟 Fixed: Changed from .fileData to .content

        if (filename.empty()) {
            res.statusCode = 400;
            res.headers["Content-Type"] = "text/html";
            res.body = "<html><body><h1>400 Bad Request: Target Filename Entry Not Found</h1></body></html>";
            return res;
        }

        // 4. Resolve the location storage destination directory path 
        std::string uploadDir = loc.getRoot();
        if (uploadDir.empty() || uploadDir.at(uploadDir.length() - 1) != '/') {
            uploadDir += "/";
        }

        std::string targetFilePath = uploadDir + filename;

        // 5. Open a raw binary write stream to drop the payload onto disk storage
        std::ofstream outFile(targetFilePath.c_str(), std::ios::binary | std::ios::out);
        if (!outFile.is_open()) {
            res.statusCode = 500;
            res.headers["Content-Type"] = "text/html";
            res.body = "<html><body><h1>500 Internal Server Error: Disk Write Access Blocked</h1></body></html>";
            return res;
        }

        outFile.write(fileData.c_str(), fileData.size());
        outFile.close();

        // 6. Build the clear, successful return package payload string
        res.statusCode = 201; 
        res.headers["Content-Type"] = "text/plain";
        res.body = "File successfully ingested and written to system storage matrix.\n";
        return res;
    }
};