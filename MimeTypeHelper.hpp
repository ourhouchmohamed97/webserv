#pragma once

#include <string>
#include <unordered_map>

class MimeTypeHelper {
public:
    static std::string getMimeType(const std::string &extension) {
        static const std::unordered_map<std::string, std::string> mimeMap = {
            {".html", "text/html"},
            {".css", "text/css"},
            {".js", "application/javascript"},
            {".png", "image/png"},
            {".jpeg", "image/jpeg"},
            {".ico", "image/x-icon"},
            {".txt", "text/plain"},
            {".pdf", "application/pdf"}
        };
        auto it = mimeMap.find(extension);
        if (it != mimeMap.end()) {
            return it->second;
        }
        return "application/octet-stream";
    }
};
