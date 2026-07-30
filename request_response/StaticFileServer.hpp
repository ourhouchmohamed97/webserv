#pragma once

#include "../request_response/HttpUtils.hpp"
#include "../config_cgi/LocationConfig.hpp"
#include "../config_cgi/ServerConfig.hpp"
#include "ErrorPageFactory.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <vector>
#include <cstdio>
#include <iostream>
#include <cerrno>
#include "AutoIndex.hpp"
#include <sys/stat.h>

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

public:
    static HttpResponse generateErrorResponse(int code, const ServerConfig& config) {
        HttpResponse res;
        res.statusCode = code;
        res.headers["Content-Type"] = "text/html";

        std::string customPath = config.getErrorPage(code);
        if (!customPath.empty()) {
            std::ifstream customFile(customPath.c_str(), std::ios::binary);
            if (customFile.is_open()) {
                std::cout << "[DEBUG ERROR] Serving configured error page file: " << customPath << std::endl;
                std::stringstream buffer;
                buffer << customFile.rdbuf();
                res.body = buffer.str();
                customFile.close();

                // 🌟 Set length for custom error page contents
                std::stringstream ssLen;
                ssLen << res.body.length();
                res.headers["Content-Length"] = ssLen.str();
                return res;
            }
            std::cerr << "[WARN ERROR] Configured error_page file unresolvable: " << customPath << ". Falling back to defaults." << std::endl;
        }

        res.body = ErrorPageFactory::getErrorPage(code);
        
        // 🌟 Set length for default error page template contents
        std::stringstream ssLen;
        ssLen << res.body.length();
        res.headers["Content-Length"] = ssLen.str();
        return res;
    }

    static HttpResponse serveFile(const HttpRequest& req, const LocationConfig& loc, const ServerConfig& config) {
        HttpResponse res;
        
        if (req.body.length() > loc.getClientMaxBodySize()) {
            return generateErrorResponse(413, config);
        }

        std::vector<std::string> methods = loc.getAllowedMethods();
        if (!methods.empty() && std::find(methods.begin(), methods.end(), req.method) == methods.end()) {
            return generateErrorResponse(405, config);
        }

        if (req.method == "DELETE") {
            std::cout << "[DEBUG DELETE] Raw req.path: " << req.path << std::endl;
            std::string fileToDitch = req.path;
            
            if (fileToDitch.find("/upload/") == 0) {
                fileToDitch = fileToDitch.substr(7); 
            } else if (fileToDitch.find("/") == 0) {
                fileToDitch = fileToDitch.substr(1); 
            }

            std::string trueStoragePath = loc.getRoot();
            if (!trueStoragePath.empty() && trueStoragePath.at(trueStoragePath.length() - 1) != '/') {
                trueStoragePath += "/";
            }
            trueStoragePath += fileToDitch;

            if (std::remove(trueStoragePath.c_str()) == 0) {
                res.statusCode = 200; 
                res.headers["Content-Type"] = "text/plain";
                res.body = "File successfully removed from server cluster disk volume.\n";
                
                std::stringstream ssLen;
                ssLen << res.body.length();
                res.headers["Content-Length"] = ssLen.str();
                return res;
            } else {
                return generateErrorResponse(404, config);
            }
        }

        // 3. Resolve File Path System
        std::string rootPath = loc.getRoot();
        std::string reqPath = req.path;

        // Strip overlapping slashes safely
        if (!rootPath.empty() && rootPath.at(rootPath.length() - 1) == '/' && !reqPath.empty() && reqPath.at(0) == '/') {
            rootPath = rootPath.substr(0, rootPath.length() - 1);
        }
        std::string fullPath = rootPath + reqPath;

        // 4. Handle Directory Access Requests
        // if (!reqPath.empty() && reqPath.at(reqPath.length() - 1) == '/') {
        //     if (!loc.getIndex().empty()) {
        //         fullPath += loc.getIndex();
        //     } else {
        //         return generateErrorResponse(403, config);
        //     }
        // }
struct stat st;

if (stat(fullPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
{
    std::string dirPath = fullPath;

    if (!dirPath.empty() && dirPath[dirPath.length() - 1] != '/')
        dirPath += "/";

    if (!loc.getIndex().empty())
    {
        std::string indexPath = dirPath + loc.getIndex();

        std::ifstream indexFile(indexPath.c_str(), std::ios::binary);
        if (indexFile.is_open())
        {
            std::stringstream buffer;
            buffer << indexFile.rdbuf();

            res.statusCode = 200;
            res.headers["Content-Type"] = "text/html";
            res.body = buffer.str();
            return res;
        }
    }

    if (loc.getAutoindex())
    {
        res.statusCode = 200;
        res.headers["Content-Type"] = "text/html";
        res.body = AutoIndex::generate(dirPath, req.path);
        return res;
    }

    return generateErrorResponse(403, config);
}   

        // 🌟 Add print trackers to find out exactly where the 404 triggers
        std::cout << "[STATIC SERVER DEBUG] Attempting file read at target path: " << fullPath << std::endl;

        // 5. Standard Operational Logic for Asset Data Reads
        std::ifstream file(fullPath.c_str(), std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "[STATIC SERVER ERROR] Cannot open target file: " << fullPath << " (Error: " << strerror(errno) << ")" << std::endl;
            return generateErrorResponse(404, config);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        res.body = buffer.str();
        file.close();

        res.statusCode = 200;
        res.headers["Content-Type"] = getContentType(fullPath);
        
        // 🌟 CRITICAL FIX: Calculate and assign explicit Content-Length headers!
        std::stringstream ssLen;
        ssLen << res.body.length();
        res.headers["Content-Length"] = ssLen.str();
        
        return res;
    }
};