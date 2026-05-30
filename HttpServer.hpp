#pragma once

#include "HttpUtils.hpp"
#include "RequestParser.hpp"
#include "StaticFileServer.hpp"
#include "Config.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

class HttpServer {
private:
    int serverFd;
    int port;
    ServerConfig config;
    std::unordered_map<std::string, RedirectRule> redirects;

public:
    HttpServer(int portNum, const ServerConfig& serverConfig, const std::unordered_map<std::string, RedirectRule>& redirectRules) 
        : serverFd(-1), port(portNum), config(serverConfig), redirects(redirectRules) {}

    ~HttpServer() {
        if (serverFd != -1) {
            close(serverFd);
        }
    }

    void init() {
        serverFd = socket(AF_INET, SOCK_STREAM, 0);
        if (serverFd < 0) {
            perror("Socket generation failed");
            exit(EXIT_FAILURE);
        }

        int opt = 1;
        setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        struct sockaddr_in address;
        std::memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        if (bind(serverFd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            perror("Bind operation failed");
            exit(EXIT_FAILURE);
        }

        if (listen(serverFd, 10) < 0) {
            perror("Listen operation failed");
            exit(EXIT_FAILURE);
        }

        std::cout << "Server initialized safely. Listening on port " << port << "...\n";
    }

    void start() {
        struct sockaddr_in clientAddress;
        socklen_t clientLen = sizeof(clientAddress);

        while (true) {
            int clientFd = accept(serverFd, (struct sockaddr*)&clientAddress, &clientLen);
            if (clientFd < 0) {
                perror("Accepting connection failed");
                continue;
            }
            handleClient(clientFd);
        }
    }

private:
    void handleClient(int clientFd) {
        char buffer[4096] = {0};
        ssize_t bytesRead = read(clientFd, buffer, sizeof(buffer) - 1);
        if (bytesRead <= 0) {
            close(clientFd);
            return;
        }

        std::string rawRequest(buffer, bytesRead);
        
        try {
            HttpRequest req = RequestParser::parse(rawRequest);
            HttpResponse res = StaticFileServer::serveFile(req, config, redirects);
            std::string serializedOutput = res.toString();
            write(clientFd, serializedOutput.c_str(), serializedOutput.length());
        } catch (const std::exception& e) {
            HttpResponse errRes(400, "Bad Request");
            errRes.setHeader("Content-Type", "text/html");
            errRes.setBody(ErrorPageFactory::getErrorPage(400));
            std::string serializedOutput = errRes.toString();
            write(clientFd, serializedOutput.c_str(), serializedOutput.length());
        }

        close(clientFd);
    }
};
