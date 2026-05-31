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
        std::string rawRequest;
        char chunk[4096];
        ssize_t bytesRead = 0;
        size_t headerEnd = std::string::npos;
        size_t contentLength = 0;
        bool hasContentLength = false;

        // 1. Read until we have at least received the complete header block (\r\n\r\n)
        while (true) {
            bytesRead = read(clientFd, chunk, sizeof(chunk));
            if (bytesRead <= 0) {
                if (rawRequest.empty()) {
                    close(clientFd);
                    return;
                }
                break; // Socket closed or block complete
            }
            rawRequest.append(chunk, bytesRead);

            // Check if we hit the end of the HTTP headers
            headerEnd = rawRequest.find("\r\n\r\n");
            if (headerEnd != std::string::npos) {
                break; // Headers fully read!
            }
        }

        // 2. Parse out Content-Length from the header block if it exists
        size_t clPos = rawRequest.find("Content-Length:");
        if (clPos != std::string::npos && clPos < headerEnd) {
            size_t valStart = clPos + 15; // length of "Content-Length:"
            size_t valEnd = rawRequest.find("\r\n", valStart);
            if (valEnd != std::string::npos) {
                std::string clStr = rawRequest.substr(valStart, valEnd - valStart);
                // Trim potential spaces
                clStr.erase(0, clStr.find_first_not_of(" "));
                try {
                    contentLength = std::stoull(clStr);
                    hasContentLength = true;
                } catch (...) {
                    hasContentLength = false;
                }
            }
        }

        // 3. Keep reading from socket until the FULL body is buffered
        if (hasContentLength && contentLength > 0) {
            size_t headerBlockLength = headerEnd + 4; // Add the size of "\r\n\r\n"
            size_t totalExpectedBytes = headerBlockLength + contentLength;

            // Loop until our rawRequest string accumulates everything
            while (rawRequest.length() < totalExpectedBytes) {
                bytesRead = read(clientFd, chunk, sizeof(chunk));
                if (bytesRead <= 0) {
                    break; // Client disconnected or transmission failed
                }
                rawRequest.append(chunk, bytesRead);
            }
        }

        // 4. Feed the fully aggregated buffer to the rest of your server engine
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