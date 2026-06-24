#pragma once

#include "../request_response/HttpUtils.hpp"
#include "../config_cgi/LocationConfig.hpp"
#include "../config_cgi/ServerConfig.hpp"
#include "../request_response/MultipartParser.hpp"
#include "../request_response/StaticFileServer.hpp"
#include <sstream>
#include <iostream>

class UploadHandler {
public:
    static HttpResponse handle(const HttpRequest& req, const LocationConfig& loc, const ServerConfig& config) {
        HttpResponse res;

        // 1. Verify Content-Type header integrity definitions
        std::map<std::string, std::string>::const_iterator it = req.headers.find("Content-Type");
        if (it == req.headers.end()) {
            it = req.headers.find("content-type");
        }
        
        if (it == req.headers.end() || it->second.find("multipart/form-data") == std::string::npos) {
            return StaticFileServer::generateErrorResponse(400, config);
        }

        // 2. Call static parsing structures
        UploadedFile uploaded = MultipartParser::parse(req);

        // 3. Extract metadata streams
        std::string filename = uploaded.filename;
        std::string fileData = uploaded.content;

        if (filename.empty()) {
            return StaticFileServer::generateErrorResponse(400, config);
        }

        // 4. Resolve the location storage destination directory path 
        // Using loc.getRoot() as a stable alternative destination root directory definition
        std::string uploadDir = loc.getRoot(); 
        if (uploadDir.empty()) {
            uploadDir = "./www/upload"; 
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
            return StaticFileServer::generateErrorResponse(500, config);
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