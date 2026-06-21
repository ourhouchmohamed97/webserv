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
        int server_fd;
        int port;

        std::vector<pollfd>     fds;
        std::map<int, Client>   clients;

        void    setupSocket();
        void    acceptClient();
        void    readFromClient(size_t i);
        void    writeToClient(size_t i);
        void    closeClient(size_t i);

        bool    isReqComplete(const std::string &req);
    public:

    Server(int _port);
    ~Server();
    void    run();
};