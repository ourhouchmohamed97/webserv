#pragma once

#include "Config.hpp"
#include "StaticFileServer.hpp"
#include <string>
#include <map>
#include <sstream>

class HttpServer {
private:
    int serverFd;
    int port;
    ServerConfig config;
    std::map<std::string, RedirectRule> redirects; // Swapped to map

public:
    HttpServer(int portNum, const ServerConfig& serverConfig, const std::map<std::string, RedirectRule>& redirectRules)
        : serverFd(-1), port(portNum), config(serverConfig), redirects(redirectRules) {}
        
    // (Keep your current socket engine logic down here unchanged, just ensure 
    // conversions like stoull look like the custom stringstream method below)
    static unsigned long long parseSize(const std::string& s) {
        std::stringstream ss(s);
        unsigned long long size;
        ss >> size;
        return size;
    }
};