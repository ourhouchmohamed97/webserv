#include "Server.hpp"

#include <iostream>

Server::Server(const ServerConfig& config) : _config(config){}
Server::~Server(){}
void Server::start(){
    std::cout << "Starting Server on port: " << _config.getPort() << std::endl;
}