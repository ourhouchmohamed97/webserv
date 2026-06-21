#pragma once

#include "Config.hpp"
#include <string>

class RouteMatcher {
public:
    static std::string match(const std::string& requestPath, const ServerConfig& config) {
        std::string bestMatch = "";
        size_t longestMatchLength = 0;

        for (const auto& [routePrefix, routeOpts] : config.routes) {
            if (requestPath.rfind(routePrefix, 0) == 0) {
                if (routePrefix.length() > longestMatchLength) {
                    longestMatchLength = routePrefix.length();
                    bestMatch = routePrefix;
                }
            }
        }
        return bestMatch;
    }
};
