#pragma once

#include <string>
#include <map>

class MimeTypeHelper {
public:
    static std::string getMimeType(const std::string &extension) {
        static std::map<std::string, std::string> mimeMap;
        
        // C++98 manual initialization guard
        if (mimeMap.empty()) {
            mimeMap[".html"] = "text/html";
            mimeMap[".css"]  = "text/css";
            mimeMap[".js"]   = "application/javascript";
            mimeMap[".png"]  = "image/png";
            mimeMap[".jpeg"] = "image/jpeg";
            mimeMap[".ico"]  = "image/x-icon";
            mimeMap[".txt"]  = "text/plain";
            mimeMap[".pdf"]  = "application/pdf";
        }

        std::map<std::string, std::string>::const_iterator it = mimeMap.find(extension);
        if (it != mimeMap.end()) {
            return it->second;
        }
        return "application/octet-stream";
    }
};