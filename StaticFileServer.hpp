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

class StaticFileServer {
public:
    // Upgraded signature: Now accepts the entire HttpRequest object
    static HttpResponse serveFile(const HttpRequest& req, const ServerConfig& config, const std::unordered_map<std::string, RedirectRule>& redirectConfig) {
        std::string decodedPath = PathResolver::urlDecode(req.path);
        std::string normalized = PathResolver::normalize(decodedPath);

        // 1. Check redirects first
        if (redirectConfig.count(normalized)) {
            const RedirectRule& rule = redirectConfig.at(normalized);
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

        const RouteConfig& matchedRoute = config.routes.at(matchedPrefix);

        // 3. METHOD VALIDATION GUARD <-- NEW LOGIC STEP
        // Check if the current request method is allowed on this route
        const auto& allowed = matchedRoute.allowedMethods;
        if (std::find(allowed.begin(), allowed.end(), req.method) == allowed.end()) {
            HttpResponse errorRes(405, "Method Not Allowed");
            errorRes.setHeader("Content-Type", "text/html");
            
            // Build out an Allow header listing permissible endpoints (RFC standard compliance)
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
            auto it = req.headers.find("Content-Length");
            if (it == req.headers.end()) {
                HttpResponse errorRes(411, "Length Required");
                errorRes.setHeader("Content-Type", "text/html");
                errorRes.setBody(ErrorPageFactory::getErrorPage(411));
                return errorRes;
            }

            try {
                size_t contentLength = std::stoull(it->second);
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

            // Multipart content extraction
            UploadedFile uploadedFile = MultipartParser::parse(req);
            if (!uploadedFile.success) {
                HttpResponse errorRes(400, "Bad Request");
                errorRes.setHeader("Content-Type", "text/html");
                errorRes.setBody(ErrorPageFactory::getErrorPage(400));
                return errorRes;
            }

            // Resolve target upload directory path
            std::string uploadDir = matchedRoute.root;
            if (uploadDir.empty()) {
                uploadDir = "."; // Default fallback to current directory
            }

            // Ensure the folder path has a clean trailing slash operation
            if (uploadDir.back() != '/') {
                uploadDir += '/';
            }

            // Combine path root with extracted filename string
            std::string targetFilePath = uploadDir + uploadedFile.filename;

            // Write binary content payload to local disk
            std::ofstream out(targetFilePath, std::ios::out | std::ios::binary);
            if (!out.is_open()) {
                // If the server fails to open/create the file
                HttpResponse errorRes(500, "Internal Server Error");
                errorRes.setHeader("Content-Type", "text/html");
                errorRes.setBody(ErrorPageFactory::getErrorPage(500));
                return errorRes;
            }

            out.write(uploadedFile.content.data(), uploadedFile.content.length());
            out.close();

            // Success response detailing the isolated data parts
            HttpResponse uploadSuccessRes(201, "Created");
            // Calculate the public URL path for the location header (e.g., /images/cool_picture.png)
            std::string publicUrlPath = normalized;
            if (publicUrlPath.back() != '/') {
                publicUrlPath += "/";
            }
            publicUrlPath += uploadedFile.filename;
            uploadSuccessRes.setHeader("Location", publicUrlPath); 
            uploadSuccessRes.setBody("Successfully Uploaded File!\n"
                                    "Saved to: " + targetFilePath + "\n"
                                    "Size: " + std::to_string(uploadedFile.content.length()) + " bytes\n");
            uploadSuccessRes.setHeader("Content-Type", "text/plain");
            return uploadSuccessRes;
        }
        else if (req.method == "DELETE") {
            std::string relativePath = normalized.substr(matchedPrefix.length());
            std::string fullPath = matchedRoute.root;
            if (!relativePath.empty() && relativePath.front() != '/' && fullPath.back() != '/') {
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
                HttpResponse errorRes(403, "Forbidden (Cannot Delete Directories)");
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

            HttpResponse validationSuccess(200, "OK");
            validationSuccess.setHeader("Content-Type", "text/plain");
            validationSuccess.setBody("Path safely verified! File exists and traversal checks passed.\n"
                                "Target to delete: " + fullPath + "\n");
            return validationSuccess;
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
        if (!relativePath.empty() && relativePath.front() != '/' && fullPath.back() != '/') {
            fullPath += "/";
        }
        fullPath += relativePath;

        std::string mimeLookupPath = normalized;

        // 5. Security Guard: Prevent Traversal Attack
        if (fullPath.find(matchedRoute.root) != 0) {
            HttpResponse errorRes(403, "Forbidden");
            errorRes.setHeader("Content-Type", "text/html");
            errorRes.setBody(ErrorPageFactory::getErrorPage(403));
            return errorRes;
        }

        // 6. Process files and directories
        struct stat s;
        if (stat(fullPath.c_str(), &s) != 0) {
            HttpResponse errorRes(404, "Not Found");
            errorRes.setHeader("Content-Type", "text/html");
            errorRes.setBody(ErrorPageFactory::getErrorPage(404));
            return errorRes;
        }

        if (s.st_mode & S_IFDIR) {
            if (fullPath.back() != '/') fullPath += '/';
            std::string targetIndex = matchedRoute.indexFile.empty() ? "index.html" : matchedRoute.indexFile;
            std::string indexPath = fullPath + targetIndex;

            if (stat(indexPath.c_str(), &s) == 0) {
                fullPath = indexPath;
                mimeLookupPath = (normalized.back() == '/') ? normalized + targetIndex : normalized + "/" + targetIndex;
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

        // 7. Read permission verification
        if (access(fullPath.c_str(), R_OK) == -1) {
            HttpResponse errorRes(403, "Forbidden");
            errorRes.setHeader("Content-Type", "text/html");
            errorRes.setBody(ErrorPageFactory::getErrorPage(403));
            return errorRes;
        }

        return serveFileContent(fullPath, mimeLookupPath);
    }

private:
    static HttpResponse serveFileContent(const std::string& fullPath, const std::string& pathForMime) {
        std::ifstream file(fullPath, std::ios::in | std::ios::binary);
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
};
