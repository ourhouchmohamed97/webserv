#pragma once

#include "HttpUtils.hpp"
#include "Config.hpp"
#include "PathResolver.hpp"
#include "RouteMatcher.hpp"
#include "ErrorPageFactory.hpp"
#include "AutoIndex.hpp"
#include "MimeTypeHelper.hpp"
#include "MultipartParser.hpp"
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <string>

class StaticFileServer {
private:
    // C++98 string-to-unsigned-long-long helper using streams instead of std::stoull
    static unsigned long long cxx98_stoull(const std::string& str) {
        std::stringstream ss(str);
        unsigned long long val;
        ss >> val;
        return val;
    }

    static HttpResponse serveFileContent(const std::string& fullPath, const std::string& pathForMime) {
        std::ifstream file(fullPath.c_str(), std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            HttpResponse errorRes(404, "Not Found");
            errorRes.setHeader("Content-Type", "text/html");
            errorRes.setBody(ErrorPageFactory::getErrorPage(404));
            return errorRes;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        HttpResponse res(200, "OK");
        size_t dotPos = pathForMime.find_last_of('.');
        std::string extension = (dotPos != std::string::npos) ? pathForMime.substr(dotPos) : "";
        
        res.setHeader("Content-Type", MimeTypeHelper::getMimeType(extension));
        res.setBody(buffer.str());
        return res;
    }

public:
    static HttpResponse serveFile(const HttpRequest& req, const ServerConfig& config, const std::map<std::string, RedirectRule>& redirectConfig) {
        std::string decodedPath = PathResolver::urlDecode(req.path);
        std::string normalized = PathResolver::normalize(decodedPath);

        // 1. Check redirects first (using .find() and continuous iterators)
        std::map<std::string, RedirectRule>::const_iterator redIt = redirectConfig.find(normalized);
        if (redIt != redirectConfig.end()) {
            const RedirectRule& rule = redIt->second;
            HttpResponse res(rule.statusCode, (rule.statusCode == 301 ? "Moved Permanently" : "Found"));
            res.setHeader("Location", rule.location);
            return res;
        }

        // 2. Longest-Prefix Route Matching
        std::string matchedPrefix = RouteMatcher::match(normalized, config);
        if (matchedPrefix.empty()) {
            HttpResponse errorRes(404, "Not Found");
            errorRes.setHeader("Content-Type", "text/html");
            errorRes.setBody(ErrorPageFactory::getErrorPage(404));
            return errorRes;
        }

        const RouteConfig& matchedRoute = config.routes.find(matchedPrefix)->second;

        // 3. METHOD VALIDATION GUARD
        const std::vector<std::string>& allowed = matchedRoute.allowedMethods;
        if (std::find(allowed.begin(), allowed.end(), req.method) == allowed.end()) {
            HttpResponse errorRes(405, "Method Not Allowed");
            errorRes.setHeader("Content-Type", "text/html");
            
            std::string allowHeaderValue = "";
            for (size_t i = 0; i < allowed.size(); ++i) {
                allowHeaderValue += allowed[i];
                if (i < allowed.size() - 1) allowHeaderValue += ", ";
            }
            errorRes.setHeader("Allow", allowHeaderValue);
            errorRes.setBody(ErrorPageFactory::getErrorPage(405));
            return errorRes;
        }

        // ROUTE METHOD HANDLING BRANCHES
        if (req.method == "POST") {
            std::map<std::string, std::string>::const_iterator it = req.headers.find("Content-Length");
            if (it == req.headers.end()) {
                HttpResponse errorRes(411, "Length Required");
                errorRes.setHeader("Content-Type", "text/html");
                errorRes.setBody(ErrorPageFactory::getErrorPage(411));
                return errorRes;
            }

            try {
                unsigned long long contentLength = cxx98_stoull(it->second);
                if (contentLength > matchedRoute.clientMaxBodySize) {
                    HttpResponse errorRes(413, "Payload Too Large");
                    errorRes.setHeader("Content-Type", "text/html");
                    errorRes.setBody(ErrorPageFactory::getErrorPage(413));
                    return errorRes;
                }
            } catch (const std::exception& e) {
                HttpResponse errorRes(400, "Bad Request");
                errorRes.setHeader("Content-Type", "text/html");
                errorRes.setBody(ErrorPageFactory::getErrorPage(400));
                return errorRes;
            }

            UploadedFile uploadedFile = MultipartParser::parse(req);
            if (!uploadedFile.success) {
                HttpResponse errorRes(400, "Bad Request");
                errorRes.setHeader("Content-Type", "text/html");
                errorRes.setBody(ErrorPageFactory::getErrorPage(400));
                return errorRes;
            }

            std::string uploadDir = matchedRoute.root;
            if (uploadDir.empty()) {
                uploadDir = ".";
            }

            if (uploadDir.at(uploadDir.size() - 1) != '/') {
                uploadDir += '/';
            }

            std::string targetFilePath = uploadDir + uploadedFile.filename;

            std::ofstream out(targetFilePath.c_str(), std::ios::out | std::ios::binary);
            if (!out.is_open()) {
                HttpResponse errorRes(500, "Internal Server Error");
                errorRes.setHeader("Content-Type", "text/html");
                errorRes.setBody(ErrorPageFactory::getErrorPage(500));
                return errorRes;
            }

            out.write(uploadedFile.content.data(), uploadedFile.content.length());
            out.close();

            HttpResponse uploadSuccessRes(201, "Created");
            std::string publicUrlPath = normalized;
            if (publicUrlPath.at(publicUrlPath.size() - 1) != '/') {
                publicUrlPath += "/";
            }
            publicUrlPath += uploadedFile.filename;
            uploadSuccessRes.setHeader("Location", publicUrlPath); 
            
            std::stringstream sizeStream;
            sizeStream << uploadedFile.content.length();
            uploadSuccessRes.setBody("Successfully Uploaded File!\n"
                                    "Saved to: " + targetFilePath + "\n"
                                    "Size: " + sizeStream.str() + " bytes\n");
            uploadSuccessRes.setHeader("Content-Type", "text/plain");
            return uploadSuccessRes;
        }
        else if (req.method == "DELETE") {
            std::string relativePath = normalized.substr(matchedPrefix.length());
            std::string fullPath = matchedRoute.root;
            if (!relativePath.empty() && relativePath.at(0) != '/' && fullPath.at(fullPath.size() - 1) != '/') {
                fullPath += "/";
            }
            fullPath += relativePath;

            if (fullPath.find(matchedRoute.root) != 0) {
                HttpResponse errorRes(403, "Forbidden");
                errorRes.setHeader("Content-Type", "text/html");
                errorRes.setBody(ErrorPageFactory::getErrorPage(403));
                return errorRes;
            }

            struct stat s;
            if (stat(fullPath.c_str(), &s) != 0) {
                HttpResponse errorRes(404, "Not Found");
                errorRes.setHeader("Content-Type", "text/html");
                errorRes.setBody(ErrorPageFactory::getErrorPage(404));
                return errorRes;
            }

            if (s.st_mode & S_IFDIR) {
                HttpResponse errorRes(403, "Forbidden (Cannot DELETE Directories)");
                errorRes.setHeader("Content-Type", "text/html");
                errorRes.setBody(ErrorPageFactory::getErrorPage(403));
                return errorRes;
            }

            if (unlink(fullPath.c_str()) != 0) {
                HttpResponse errorRes(500, "Internal Server Error");
                errorRes.setHeader("Content-Type", "text/html");
                errorRes.setBody(ErrorPageFactory::getErrorPage(500));
                return errorRes;
            }

            HttpResponse deleteSuccessRes(204, "No Content");
            return deleteSuccessRes;
        }
        else if (req.method != "GET") {
            HttpResponse errorRes(405, "Method Not Allowed");
            errorRes.setHeader("Content-Type", "text/html");
            errorRes.setBody(ErrorPageFactory::getErrorPage(405));
            return errorRes;
        }
        
        // 4. Translate URL path to local physical path
        std::string relativePath = normalized.substr(matchedPrefix.length());
        std::string fullPath = matchedRoute.root;
        if (!relativePath.empty() && relativePath.at(0) != '/' && fullPath.at(fullPath.size() - 1) != '/') {
            fullPath += "/";
        }
        fullPath += relativePath;

        std::string mimeLookupPath = normalized;

        if (fullPath.find(matchedRoute.root) != 0) {
            HttpResponse errorRes(403, "Forbidden");
            errorRes.setHeader("Content-Type", "text/html");
            errorRes.setBody(ErrorPageFactory::getErrorPage(403));
            return errorRes;
        }

        // 5. Process files and directories
        struct stat s;
        if (stat(fullPath.c_str(), &s) != 0) {
            HttpResponse errorRes(404, "Not Found");
            errorRes.setHeader("Content-Type", "text/html");
            errorRes.setBody(ErrorPageFactory::getErrorPage(404));
            return errorRes;
        }

        if (s.st_mode & S_IFDIR) {
            if (fullPath.at(fullPath.size() - 1) != '/') fullPath += '/';
            std::string targetIndex = matchedRoute.indexFile.empty() ? "index.html" : matchedRoute.indexFile;
            std::string indexPath = fullPath + targetIndex;

            if (stat(indexPath.c_str(), &s) == 0) {
                fullPath = indexPath;
                mimeLookupPath = (normalized.at(normalized.size() - 1) == '/') ? normalized + targetIndex : normalized + "/" + targetIndex;
            } else if (matchedRoute.autoindex) {
                HttpResponse res(200, "OK");
                res.setHeader("Content-Type", "text/html");
                res.setBody(AutoIndex::generate(fullPath, normalized));
                return res;
            } else {
                HttpResponse errorRes(403, "Forbidden");
                errorRes.setHeader("Content-Type", "text/html");
                errorRes.setBody(ErrorPageFactory::getErrorPage(403));
                return errorRes;
            }
        }

        return serveFileContent(fullPath, mimeLookupPath);
    }
};