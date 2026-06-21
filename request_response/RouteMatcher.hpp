#pragma once

#include "config & tokenize/ServerConfig.hpp"
#include "config & tokenize/LocationConfig.hpp"
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
        if (!found) {
            bestMatch.setPath("/");
            bestMatch.setRoot(serverConfig.getRoot().empty() ? "./www" : serverConfig.getRoot());
            bestMatch.setIndex(serverConfig.getIndex().empty() ? "index.html" : serverConfig.getIndex());
            bestMatch.setAutoindex(false);
        }

        return bestMatch;
    }
}