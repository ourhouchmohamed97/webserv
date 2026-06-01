#pragma once

#include "HttpUtils.hpp"
#include "RequestParser.hpp"
#include "StaticFileServer.hpp"
#include "Config.hpp"
#include "ChunkDecoder.hpp"
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
        bool isChunked = false;

        // 1. Read until we have at least received the complete header block (\r\n\r\n)
        while (true) {
            bytesRead = read(clientFd, chunk, sizeof(chunk));
            if (bytesRead <= 0) {
                if (rawRequest.empty()) {
                    close(clientFd);
                    return;
                }
                break;
            }
            rawRequest.append(chunk, bytesRead);

            headerEnd = rawRequest.find("\r\n\r\n");
            if (headerEnd != std::string::npos) {
                break; // Headers fully read!
            }
        }

        size_t headerBlockLength = headerEnd + 4;
        std::string headersOnly = rawRequest.substr(0, headerBlockLength);

        // 2. Determine encoding specifications from headers
        // Check for Transfer-Encoding: chunked
        if (headersOnly.find("Transfer-Encoding: chunked") != std::string::npos) {
            isChunked = true;
        } else {
            // Check for normal Content-Length fallback
            size_t clPos = headersOnly.find("Content-Length:");
            if (clPos != std::string::npos) {
                size_t valStart = clPos + 15;
                size_t valEnd = headersOnly.find("\r\n", valStart);
                if (valEnd != std::string::npos) {
                    std::string clStr = headersOnly.substr(valStart, valEnd - valStart);
                    clStr.erase(0, clStr.find_first_not_of(" "));
                    try {
                        contentLength = std::stoull(clStr);
                        hasContentLength = true;
                    } catch (...) {
                        hasContentLength = false;
                    }
                }
            }
        }

        // 3. Read loop for standard Content-Length payload
        if (!isChunked && hasContentLength && contentLength > 0) {
            size_t totalExpectedBytes = headerBlockLength + contentLength;
            while (rawRequest.length() < totalExpectedBytes) {
                bytesRead = read(clientFd, chunk, sizeof(chunk));
                if (bytesRead <= 0) break;
                rawRequest.append(chunk, bytesRead);
            }
        }
        // 4. Read loop for Transfer-Encoding: chunked payload
        else if (isChunked) {
            // Read until we safely locate the terminating chunk marker sequences "0\r\n\r\n"
            while (rawRequest.find("\r\n0\r\n\r\n") == std::string::npos && 
                   rawRequest.substr(rawRequest.length() >= 5 ? rawRequest.length() - 5 : 0) != "0\r\n\r\n") {
                bytesRead = read(clientFd, chunk, sizeof(chunk));
                if (bytesRead <= 0) break;
                rawRequest.append(chunk, bytesRead);
            }

            // Separate the headers block from the raw chunked body
            std::string rawBody = rawRequest.substr(headerBlockLength);
            
            // Decode the raw chunk stream back into plain text data
            std::pair<bool, std::string> decodeResult = ChunkDecoder::decode(rawBody);
            if (!decodeResult.first) {
                HttpResponse errRes(400, "Bad Request (Malformed Chunked Stream)");
                errRes.setHeader("Content-Type", "text/html");
                errRes.setBody(ErrorPageFactory::getErrorPage(400));
                write(clientFd, errRes.toString().c_str(), errRes.toString().length());
                close(clientFd);
                return;
            }

            // Normalization Step: Rebuild the rawRequest string into a standard HTTP format
            // Remove the 'Transfer-Encoding: chunked' header line
            size_t tePos = headersOnly.find("Transfer-Encoding: chunked\r\n");
            if (tePos != std::string::npos) {
                headersOnly.erase(tePos, 28);
            }

            // Inject an accurate Content-Length reflecting our compiled body block length
            std::string newClHeader = "Content-Length: " + std::to_string(decodeResult.second.length()) + "\r\n";
            headersOnly.insert(headerEnd, newClHeader);

            // Stitch the updated header blocks back directly to our decrypted flat text body payload
            rawRequest = headersOnly + decodeResult.second;
        }

        // 5. Feed the normalized data bundle straight through to your routing pipeline
        try {
            HttpRequest req = RequestParser::parse(rawRequest);
            HttpResponse res = StaticFileServer::serveFile(req, config, redirects);
            std::string serializedOutput = res.toString();
            write(clientFd, serializedOutput.c_str(), serializedOutput.length());
        } catch (const std::exception& e) {
            HttpResponse errRes(400, "Bad Request");
            errRes.setHeader("Content-Type", "text/html");
            errRes.setBody(ErrorPageFactory::getErrorPage(400));
            write(clientFd, errRes.toString().c_str(), errRes.toString().length());
        }

        close(clientFd);
    }

};