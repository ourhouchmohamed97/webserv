#pragma once

#include "../config_cgi/ServerConfig.hpp"
#include "../config_cgi/LocationConfig.hpp"
#include <string>
#include <vector>

class RouteMatcher {
public:
    // Matches the longest prefix of reqPath against the server's locations array
    static LocationConfig match(const std::string& reqPath, const ServerConfig& serverConfig) {
        std::vector<LocationConfig> locations = serverConfig.getLocations();
        LocationConfig bestMatch;
        size_t longestMatchLen = 0;
        bool found = false;

        std::vector<LocationConfig>::const_iterator it;
        for (it = locations.begin(); it != locations.end(); ++it) {
            std::string locPath = it->getPath();

            // Check if reqPath starts with locPath
            if (reqPath.find(locPath) == 0) {
                // Ensure it matches a complete path segment boundary
                if (locPath == "/" || reqPath.length() == locPath.length() || reqPath[locPath.length()] == '/') {
                    if (locPath.length() > longestMatchLen) {
                        longestMatchLen = locPath.length();
                        bestMatch = *it;
                        found = true;
                    }
                }
            }
        }

        // Fallback fallback mechanism: if no location block matched, build a basic root default
        if (found) {
    if (bestMatch.getRoot().empty()) {
        bestMatch.setRoot(serverConfig.getRoot());
    }
    if (bestMatch.getIndex().empty()) {
        bestMatch.setIndex(serverConfig.getIndex());
    }
}
        return bestMatch;
    }
};