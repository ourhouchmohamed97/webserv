#pragma once

#include "HttpUtils.hpp"
#include <string>
#include <iostream>

struct UploadedFile {
    std::string filename;
    std::string content;
    bool success = false;
};

class MultipartParser {
    public:
        static UploadedFile parse(const HttpRequest& req) {
            UploadedFile fileResult;

            // Extract boundary from Content-Type header
            auto it = req.headers.find("Content-Type");
            if (it == req.headers.end())
                return fileResult; // No Content-Type header

            std::string contentType = it->second;
            size_t boundaryPos = contentType.find("boundary=");
            if (boundaryPos == std::string::npos)
                return fileResult; // No boundary found

            // The real boundary in the body is prefixed with two extra dashes "--"
            std::string boundary = "--" + contentType.substr(boundaryPos + 9);

            // Locate the boundary inside the body payload
            size_t partStart = req.body.find(boundary);
            if (partStart == std::string::npos)
                return fileResult; // Boundary not found in body

            // Move past the boundary string and its trailing \r\n
            size_t nextLine = req.body.find("\r\n", partStart);
            if (nextLine == std::string::npos)
                return fileResult; // Malformed part, no newline after boundary
            size_t fileHeaderStart = nextLine + 2; // Start of the part headers

            // Find where the file's inner headers block ends (\r\n\r\n)
            size_t innerHeaderEnd = req.body.find("\r\n\r\n", fileHeaderStart);
            if (innerHeaderEnd == std::string::npos)
                return  fileResult; // Malformed part, no header-body separator

            std::string fileHeaders = req.body.substr(fileHeaderStart, innerHeaderEnd - fileHeaderStart);

            // Extract the filename from the Content-Disposition header
            size_t filenamePos = fileHeaders.find("filename=\"");
            if (filenamePos != std::string::npos) {
                size_t start = filenamePos + 10; // length of 'filename="'
                size_t end = fileHeaders.find("\"", start);
                if (end != std::string::npos) {
                    fileResult.filename = fileHeaders.substr(start, end - start);
                }
            }
            if (fileResult.filename.empty())
                fileResult.filename = "unknown_upload.bin"; // Fallback name

            // Isolate the raw data payload bytes
            size_t dataStart = innerHeaderEnd + 4; // Skip past the "\r\n\r\n"

            // The data ends right before the next boundary sequence
            size_t dataEnd = req.body.find(boundary, dataStart);
            if (dataEnd == std::string::npos)
                return fileResult; // Malformed part, no closing boundary

            // Strip the trailing "\r\n" that sits right before the closing boundary
            if (dataEnd >= 2 && req.body.substr(dataEnd - 2, 2) == "\r\n") {
                dataEnd -= 2;
            }

            fileResult.content = req.body.substr(dataStart, dataEnd - dataStart);
            fileResult.success = true;

            return fileResult;
        }
};
