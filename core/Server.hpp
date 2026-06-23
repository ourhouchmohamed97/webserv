#pragma once

#include "Client.hpp"
#include "Request.hpp"
#include "../config_cgi/ServerConfig.hpp"
#include "../config_cgi/cgi.hpp"
#include "../request_response/RequestParser.hpp"
#include "../request_response/RouteMatcher.hpp"
#include "../request_response/StaticFileServer.hpp"
#include "../request_response/HttpUtils.hpp"
#include "../request_response/ChunkDecoder.hpp"
#include "UploadHandler.hpp"
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <exception>
#include <poll.h>
#include <fcntl.h>
#include <vector>
#include <map>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <cstring>

class Server
{
    private:
        std::vector<int> server_fd;
        std::vector<int> ports;


        std::vector<pollfd>     fds;
        std::map<int, Client>   clients;
        std::vector<ServerConfig>   _configs;

        void    setupSocket(int port);
        void    acceptClient(int serverfd);
        void    readFromClient(size_t i);
        void    writeToClient(size_t i);
        void    closeClient(size_t i);
        
        bool    is_serverFd(int fd);
        bool    isReqComplete(const std::string &req);
    public:

    Server(std::vector<int> _port, const std::vector<ServerConfig>& configs);
    ~Server();
    void    run();
};