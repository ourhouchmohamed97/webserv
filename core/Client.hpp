#pragma once

#include <iostream>

enum ClientState
{
    READING, 
    WRITING,
    CLOSED
};

class Client
{
    public:
        int fd;
        ClientState state;
        std::string requestBuffer;
        std::string responseBuffer;
        Client();
        Client(int fd);
        
};