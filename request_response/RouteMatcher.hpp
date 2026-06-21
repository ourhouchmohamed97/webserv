#pragma once

#include "Config.hpp"
#include <string>
#include <map>

class RouteMatcher {
public:
    static std::string match(const std::string& requestPath, const ServerConfig& config) {
        std::string bestMatch = "";
        size_t longestMatchLength = 0;

        std::map<std::string, RouteConfig>::const_iterator it;
        for (it = config.routes.begin(); it != config.routes.end(); ++it) {
            std::string routePrefix = it->first;
            
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