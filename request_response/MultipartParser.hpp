#pragma once

#include "HttpUtils.hpp"
#include <string>
#include <iostream>
#include <map>

struct UploadedFile {
    std::string filename;
    std::string content;
    bool success;

    // C++98 constructor initialization
    UploadedFile() : success(false) {}
};

class MultipartParser {
public:
    static UploadedFile parse(const HttpRequest& req) {
        UploadedFile fileResult;

        // Extract boundary from Content-Type header explicitly
        std::map<std::string, std::string>::const_iterator it = req.headers.find("Content-Type");
        if (it == req.headers.end() || it->second.find("boundary=") == std::string::npos) {
            fileResult.filename = "raw_payload.txt";
            fileResult.content = req.body;
            fileResult.success = true;
            return fileResult;
        }

        std::string contentType = it->second;
        size_t boundaryPos = contentType.find("boundary=");
        if (boundaryPos == std::string::npos)
            return fileResult;

        std::string boundary = "--" + contentType.substr(boundaryPos + 9);

        size_t partStart = req.body.find(boundary);
        if (partStart == std::string::npos)
            return fileResult;

        size_t nextLine = req.body.find("\r\n", partStart);
        if (nextLine == std::string::npos)
            return fileResult;
        size_t fileHeaderStart = nextLine + 2;

        size_t innerHeaderEnd = req.body.find("\r\n\r\n", fileHeaderStart);
        if (innerHeaderEnd == std::string::npos)
            return fileResult;

        std::string fileHeaders = req.body.substr(fileHeaderStart, innerHeaderEnd - fileHeaderStart);

        size_t filenamePos = fileHeaders.find("filename=\"");
        if (filenamePos != std::string::npos) {
            size_t start = filenamePos + 10;
            size_t end = fileHeaders.find("\"", start);
            if (end != std::string::npos) {
                fileResult.filename = fileHeaders.substr(start, end - start);
            }
        }
        if (fileResult.filename.empty())
            fileResult.filename = "unknown_upload.bin";

        size_t dataStart = innerHeaderEnd + 4;

        size_t dataEnd = req.body.find(boundary, dataStart);
        if (dataEnd == std::string::npos)
            return fileResult;

        if (dataEnd >= 2 && req.body.substr(dataEnd - 2, 2) == "\r\n") {
            dataEnd -= 2;
        }

        fileResult.content = req.body.substr(dataStart, dataEnd - dataStart);
        fileResult.success = true;

        return fileResult;
    }
};