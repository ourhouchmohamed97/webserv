#pragma once

#include "ServerConfig.hpp"

class Server {
private:
    ServerConfig _config;
public:
    Server (const ServerConfig& config);
    ~Server();
    void start();
};