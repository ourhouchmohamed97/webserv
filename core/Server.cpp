#include "Server.hpp"

Server::Server(std::vector<int> _port, const std::vector<ServerConfig>& configs) 
    : ports(_port), _configs(configs)
{
    for(size_t i = 0; i < ports.size(); i++)
        setupSocket(ports[i]);
}

Server::~Server()
{
    for (size_t i = 0; i < fds.size(); i++)
        close(fds[i].fd);
}

void  Server::setupSocket(int port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        throw std::runtime_error("Socket failed");
    
    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
        throw std::runtime_error("SetSockopt failed");
    
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        throw std::runtime_error("Bind failed");
    if (listen(fd, 10) < 0)
        throw std::runtime_error("Listen failed");
    
    if (fcntl(fd, F_SETFL, O_NONBLOCK))
        throw std::runtime_error("fcntl F_SETFL failed");

    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    fds.push_back(pfd);
    server_fd.push_back(fd);

    std::cout << "Server start listen on port " << port << std::endl;
}

bool Server::is_serverFd(int fd)
{
    for (size_t i = 0; i < server_fd.size(); i++)
    {
        if (server_fd[i] == fd)
            return true;
    }
    return false;
}

void    Server::closeClient(size_t i)
{
    std::cout << "Client Closed: " << fds[i].fd << std::endl;
    close(fds[i].fd);
    clients.erase(fds[i].fd);
    fds.erase(fds.begin() + i);
}

void Server::acceptClient(int serverfd)
{
    while (true)
    {
        int client_fd = accept(serverfd, NULL, NULL);
        if (client_fd < 0)
            return ;

        if (fcntl(client_fd, F_SETFL, O_NONBLOCK))
            throw std::runtime_error("fcntl of client failed");

        pollfd client_poll;
        client_poll.fd = client_fd;
        client_poll.events = POLLIN;
        client_poll.revents = 0;

        fds.push_back(client_poll);
        clients.insert(std::make_pair(client_fd, Client(client_fd)));
        std::cout << "New client: " << client_fd << std::endl;
    }
}

bool Server::isReqComplete(const std::string &req)
{
    size_t header_end = req.find("\r\n\r\n");
    if (header_end == std::string::npos)
        return false;

    if (req.find("Transfer-Encoding: chunked") != std::string::npos || 
        req.find("transfer-encoding: chunked") != std::string::npos) {
        return (req.rfind("0\r\n\r\n") == req.length() - 5);
    }

    size_t pos = req.find("Content-Length:");
    if (pos == std::string::npos)
        pos = req.find("content-length:");
    
    if (pos != std::string::npos && pos < header_end)
    {
        size_t value_start = pos + 15;
        size_t value_end = req.find("\r\n", value_start);
        if (value_end != std::string::npos)
        {
            std::string len_str = req.substr(value_start, value_end - value_start);
            std::stringstream ss(len_str);
            size_t content_length = 0;
            ss >> content_length;

            size_t body_received = req.length() - (header_end + 4);
            return (body_received >= content_length);
        }
    }
    
    return true;
}

void Server::readFromClient(size_t i)
{
    int fd  = fds[i].fd;
    char buffer[4100];

    std::memset(buffer, 0, sizeof(buffer));
    ssize_t bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes == 0 || bytes == -1)
    {
        closeClient(i);
        return;
    }

    Client &cli = clients.find(fd)->second;
    cli.requestBuffer.append(buffer, bytes);

    if (!isReqComplete(cli.requestBuffer))
        return;

    try {
        std::string processedBuffer = cli.requestBuffer;
        if (processedBuffer.find("Transfer-Encoding: chunked") != std::string::npos ||
            processedBuffer.find("transfer-encoding: chunked") != std::string::npos) {
            
            size_t header_end = processedBuffer.find("\r\n\r\n");
            std::string headers = processedBuffer.substr(0, header_end + 4);
            std::string rawBody = processedBuffer.substr(header_end + 4);

            std::pair<bool, std::string> decodeResult = ChunkDecoder::decode(rawBody);
            processedBuffer = headers + decodeResult.second;
        }

        HttpRequest req = RequestParser::parse(processedBuffer);

        cli.requestBuffer.clear(); 

        ServerConfig activeConfig;
        if (!_configs.empty()) {
            activeConfig = _configs[0]; 
        }

        LocationConfig loc = RouteMatcher::match(req.path, activeConfig);
        std::map<std::string, std::string> cgiMap = loc.getCgi();
        // handle max body clients size ----->

        size_t maxBodySize = activeConfig.getClientMaxBodySize();
        
        // std::cout << "max maxBodySize:  " << maxBodySize << std::endl;
        // std::cout << "loc.getClientMaxBodySize():   " << loc.getClientMaxBodySize() << std::endl;

        if (loc.getClientMaxBodySize() != 0)
            maxBodySize = loc.getClientMaxBodySize();

        if (req.body.size() > maxBodySize)
        {
            HttpResponse res;
            res.statusCode = 413;
            res.headers["Content-Type"] = "text/html";
            res.body = "<h1>413 Payload Too Large</h1>";

            cli.responseBuffer = res.toString();
            cli.state = WRITING;
            fds[i].events = POLLOUT;
            return;
        }


        // 1. Resolve relative and absolute paths early
        std::string relative = req.path;
        if (relative.find(loc.getPath()) == 0)
            relative = relative.substr(loc.getPath().size());

        if (!relative.empty() && relative[0] != '/')
            relative = "/" + relative;

        std::string scriptPath = loc.getRoot() + relative;

        
        size_t dotPos = scriptPath.find_last_of(".");
        std::string ext = (dotPos != std::string::npos) ? scriptPath.substr(dotPos) : "";

        
        bool isCgiRequest = (!cgiMap.empty() && !ext.empty() && cgiMap.find(ext) != cgiMap.end());

        // handle session here ------------>
        if (req.path == "/session")
        {
            SessionManager Session_manage;
            HttpResponse res = Session_manage.handle(req);
            cli.responseBuffer = res.toString();

            cli.state = WRITING;
            fds[i].events = POLLOUT;
            return;
        }
        // handle upload here ------------>
        else if (req.method == "POST" && req.path == "/upload") 
        {
            HttpResponse res = UploadHandler::handle(req, loc, activeConfig);
            cli.responseBuffer = res.toString();
        }
       // handle CGI here ------------>
        else if (isCgiRequest) {
            std::string interpreterPath = "";
            if (cgiMap.find(ext) != cgiMap.end()) {
                interpreterPath = cgiMap[ext];
            } else if (cgiMap.find("default") != cgiMap.end()) {
                interpreterPath = cgiMap["default"];
            } else {
                interpreterPath = scriptPath;
            }

            std::cout << "[DEBUG SERVER] Executing matching CGI target script: " << scriptPath << std::endl;

            CGI cgiHandler;
            std::string cgiOutput = cgiHandler.execute(interpreterPath, scriptPath, req.method, req.body, req.headers);
            
            size_t headerEnd = cgiOutput.find("\r\n\r\n");
            if (headerEnd == std::string::npos)
                headerEnd = cgiOutput.find("\n\n");

            std::string cgiHeaders;
            std::string cgiBody;

            if (headerEnd != std::string::npos)
            {
                size_t sepLen = (cgiOutput.find("\r\n\r\n") != std::string::npos) ? 4 : 2;
                cgiHeaders = cgiOutput.substr(0, headerEnd);
                cgiBody = cgiOutput.substr(headerEnd + sepLen);
            }
            else
            {
                cgiHeaders = "Content-Type: text/html";
                cgiBody = cgiOutput;
            }
            std::stringstream ss;
            ss << "HTTP/1.1 200 OK\r\n"
               << cgiHeaders << "\r\n"
               << "Content-Length: " << cgiBody.length() << "\r\n"
               << "\r\n"
               << cgiBody;

            cli.responseBuffer = ss.str();
        } 
        // Branch 3: Default static file server logic paths (GET / DELETE operations)
        else {
            HttpResponse res = StaticFileServer::serveFile(req, loc, activeConfig);
            cli.responseBuffer = res.toString();
        }
    } catch (const std::exception& e) {
        std::cerr << "Parser Engine Fail Alert: " << e.what() << std::endl;
        cli.responseBuffer = "HTTP/1.1 400 Bad Request\r\nContent-Length: 15\r\n\r\n400 Bad Request";
    }

    cli.state = WRITING;
    fds[i].events = POLLOUT;
}

void    Server::writeToClient(size_t i)
{
    int fd = fds[i].fd;
    Client &cli = clients.find(fd)->second;

    ssize_t bytes_sent = send(fd, cli.responseBuffer.c_str(), cli.responseBuffer.size(), 0);
    if (bytes_sent == -1)
    {
        closeClient(i);
        return;
    }
    cli.responseBuffer.erase(0, bytes_sent);
    if (cli.responseBuffer.empty())
        closeClient(i);
}

void    Server::run()
{
    while (true)
    {
        if(poll(&fds[0], fds.size(), -1) < 0)
            throw std::runtime_error("poll failed");
        for (size_t i = 0; i < fds.size(); i++)
        {
            if (fds[i].revents == 0)
                continue;
            if (fds[i].revents & POLLHUP)
            {
                if (!is_serverFd(fds[i].fd))
                {
                    closeClient(i);
                    i--;
                }
                continue;
            }

            if (is_serverFd(fds[i].fd))
            {
                if (fds[i].revents & POLLIN)
                    acceptClient(fds[i].fd);
            }
            else
            {
                if (fds[i].revents & POLLIN)
                    readFromClient(i);
                else if (fds[i].revents & POLLOUT)
                    writeToClient(i);
            }
        }
    }
}