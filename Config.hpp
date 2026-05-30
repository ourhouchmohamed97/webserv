#pragma once

#include <string>
#include <unordered_map>
#include <vector>

struct RedirectRule {
    int statusCode; // 301 or 302
    std::string location; // The URL to redirect to
};

struct RouteConfig {
    std::string root;       // e.g., "./www/images_folder"
    bool autoindex;         // true or false
    std::string indexFile;  // Default to "index.html"
    std::vector<std::string> allowedMethods; // e.g., {"GET", "POST"}
};

class ServerConfig {
public:
    std::unordered_map<std::string, RouteConfig> routes;

    void addRoute(const std::string& path, const RouteConfig& config) {
        routes[path] = config;
    }
};
