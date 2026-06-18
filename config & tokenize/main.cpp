#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "ConfigParser.hpp"
#include "Token.hpp"
#include "Server.hpp"

void startServers(const std::vector<ServerConfig>& servers)
{
    for (size_t i = 0; i < servers.size(); i++)
    {
        int port = servers[i].getPort();
        std::cout << "Starting server on port: " << port << std::endl;
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0)
        {
            std::cerr << "bind failed on port " << port << std::endl;
            continue;
        }
        if (listen(server_fd, 10) < 0)
        {
            std::cerr << "listen failed\n";
            continue;
        }
        std::cout << "Server running on port " << port << std::endl;
        int client_fd = accept(server_fd, NULL, NULL);
        char buffer[30000];
        int bytes = read(client_fd, buffer, sizeof(buffer) - 1);
        if (bytes > 0)
        {
            buffer[bytes] = '\0';
            std::cout << "\n===== REQUEST =====\n";
            std::cout << buffer << "\n";
        }
        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 12\r\n"
            "\r\n"
            "Hello World\n";
        send(client_fd, response.c_str(), response.size(), 0);
        close(client_fd);
        close(server_fd);
    }
}

int main()
{
    ConfigParser parser("webserv.conf");
    std::string content = parser.readFile();
    std::vector<Token> tokens = parser.tokenize(content);
    std::vector<ServerConfig> servers = parser.parse(tokens);
    startServers(servers);
}