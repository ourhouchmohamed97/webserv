#pragma once

#include "Client.hpp"
#include "Request.hpp"
#include <exception>
#include <poll.h>
#include <fcntl.h>
#include <vector>
#include <map>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

class Server
{
    private:
        std::vector<int> server_fd;
        std::vector<int> ports;


        std::vector<pollfd>     fds;
        std::map<int, Client>   clients;

        void    setupSocket(int port);
        void    acceptClient(int serverfd);
        void    readFromClient(size_t i);
        void    writeToClient(size_t i);
        void    closeClient(size_t i);
        
        bool    is_serverFd(int fd);
        bool    isReqComplete(const std::string &req);
    public:

    Server(std::vector<int> _port);
    ~Server();
    void    run();
};