#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <unistd.h>
#include <vector>
#include <sys/stat.h>
#include <dirent.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>

struct HttpRequest
{
    std::string method;
    std::string path;
    std::string version;

    // We use a map for headers because they are key: value pairs.
    // unordered_map gives us the fast O(1) lookups.
    std::unordered_map<std::string, std::string> headers;

    std::string body; // The optional payload.

    void print() const
    {
        std::cout << "=== HTTP REQUEST ===\n";
        std::cout << "Method: " << method << "\n";
        std::cout << "Path: " << path << "\n";
        std::cout << "Version: " << version << "\n";
        std::cout << "Headers:\n";
        for (const auto &[key, value] : headers)
        {
            std::cout << " [" << key << "] -> " << value << "\n";
        }

        std::cout << "Body: " << body << "\n";
        std::cout << "=========================\n";
    }
};

class RequestParser
{
private:
    // Helper function to trim trailing '\r' and leading/trailing spaces
    static std::string trim(std::string str)
    {
        if (!str.empty() && str.back() == '\r')
        {
            str.pop_back();
        }
        // Basic space trimming
        size_t first = str.find_first_not_of(" ");
        if (first == std::string::npos)
            return "";
        size_t last = str.find_last_not_of(" ");
        return str.substr(first, (last - first + 1));
    }

public:
    static HttpRequest parse(const std::string &rawRequest)
    {
        HttpRequest req;
        std::stringstream requestStream(rawRequest);
        std::string line;
        // Parse Request Line
        if (std::getline(requestStream, line))
        {
            line = trim(line);
            std::stringstream lineStream(line);
            lineStream >> req.method >> req.path >> req.version;
        }
        // Parse Headers up to Blank Line
        while (std::getline(requestStream, line))
        {
            line = trim(line);

            if (line.empty())
                break;

            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos)
            {
                std::string key = trim(line.substr(0, colonPos));
                std::string value = trim(line.substr(colonPos + 1));
                req.headers[key] = value;
            }
        }

        // POST Validation: Check Content-Length
        if (req.method == "POST")
        {
            auto it = req.headers.find("Content-Length");
            if (it == req.headers.end())
            {
                throw std::runtime_error("404 Bad Request: Missing Content-Length");
            }

            int contentLength = std::stoi(it->second);

            if (contentLength > 0)
            {
                std::string remainingContent;
                std::ostringstream remainder;
                remainder << requestStream.rdbuf();
                remainingContent = remainder.str();

                if (remainingContent.length() >= static_cast<size_t>(contentLength))
                {
                    req.body = remainingContent.substr(0, contentLength);
                }
                else
                {
                    throw std::runtime_error("404 Bad Request: Body size less than Content-Length.");
                }
            }
        }
        return req;
    }
};

class HttpResponse
{
private:
    std::string version = "HTTP/1.1";
    int statusCode;
    std::string statusMessage;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

public:
    HttpResponse(int code, std::string message) : statusCode(code), statusMessage(message) {}

    void setHeader(const std::string &key, const std::string &value)
    {
        headers[key] = value;
    }

    // Set Content-Length automatically from body size
    void setBody(const std::string &b)
    {
        body = b;
        setHeader("Content-Length", std::to_string(body.length()));
    }
    // Serialize response to raw bytes: status + headers + \r\n\r\n + body

    std::string toString() const
    {
        std::stringstream ss;
        ss << version << " " << statusCode << " " << statusMessage << "\r\n";

        for (const auto &[key, value] : headers)
        {
            ss << key << ": " << value << "\r\n";
        }

        ss << "\r\n"
           << body;
        return ss.str();
    }
};

class MimeTypeHelper
{
private:
    static std::unordered_map<std::string, std::string> mimeMap;

public:
    static std::string getMimeType(const std::string &extension)
    {
        auto it = mimeMap.find(extension);
        if (it != mimeMap.end())
        {
            return it->second;
        }
        return "application/octet-stream"; // Default for unknown/binary files
    }
};

std::unordered_map<std::string, std::string> MimeTypeHelper::mimeMap = {
    {".html", "text/html"},
    {".css", "text/css"},
    {".js", "application/javascript"},
    {".png", "image/png"},
    {".jpeg", "image/jpeg"},
    {".ico", "image/x-icon"},
    {".txt", "image/plain"},
    {".pdf", "applicaiton/pdf"},
};


class ErrorPageFactory {
private:
    // A reusable master template to avoid duplicating CSS styles
    static std::string buildTemplate(const std::string& code, const std::string& title, const std::string& description) {
        return R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Error )" + code + R"(: )" + title + R"(</title>
    <style>
        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
            background-color: #121214;
            color: #e1e1e6;
            display: flex;
            align-items: center;
            justify-content: center;
            height: 100vh;
            margin: 0;
        }
        .container {
            text-align: center;
            max-width: 450px;
            padding: 20px;
        }
        h1 {
            font-size: 6rem;
            margin: 0;
            color: #ff5555;
            font-weight: 800;
            letter-spacing: -2px;
        }
        h2 {
            font-size: 1.5rem;
            margin: 10px 0 20px 0;
            color: #ff79c6;
        }
        p {
            color: #8b92a5;
            line-height: 1.6;
            margin-bottom: 30px;
        }
        .button {
            display: inline-block;
            background-color: #44475a;
            color: #f8f8f2;
            text-decoration: none;
            padding: 10px 20px;
            border-radius: 6px;
            font-size: 0.9rem;
            transition: background 0.2s;
        }
        .button:hover {
            background-color: #6272a4;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>)" + code + R"(</h1>
        <h2>)" + title + R"(</h2>
        <p>)" + description + R"(</p>
        <a href="/" class="button">Back to Home</a>
    </div>
</body>
</html>)";
    }
public:
    static std::string getErrorPage(int statusCode) {
        switch (statusCode) {
            case 400:
                return buildTemplate("400", "Bad Request", "The server could not understand the request due to malformed syntax.");
            case 403:
                return buildTemplate("403", "Forbidden", "You don't have permission to access this resource.");
            case 404:
                return buildTemplate("404", "Not Found", "The requested URL was not found on this server.");
            case 405:
                return buildTemplate("405", "Method Not Allowed", "The HTTP method used is not supported for this URL.");
            case 500:
                return buildTemplate("500", "Internal Server Error", "The server encountered an error and could not complete your request.");
            default:
                return buildTemplate(std::to_string(statusCode), "Error", "An unexpected error occurred.");
        }
    }
};


class PathResolver {
    public:
        // Decodes 20% to space, etc
        static std::string urlDecode(std::string str) {
            std::string ret;
            for (size_t i = 0; i < str.length(); ++i) {
                if (str[i] == '%' && i + 2 < str.length()) {
                    int value = std::stoi(str.substr(i + 1, 2), NULL, 16);
                    ret += static_cast<char>(value);
                    i += 2;
                }
                else {
                    ret += str[i];
                }
            }
            return ret;
        }

        // Resolves . and .. compenents
        static std::string normalize(std::string path) {
            std::vector<std::string> parts;
            std::stringstream ss(path);
            std::string segment;
            while (std::getline(ss, segment, '/')) {
                if (segment == "" || segment == ".")
                    continue;
                if (segment == "..") {
                    if (!parts.empty())
                        parts.pop_back();
                }
                else
                    parts.push_back(segment);
            }
            std::string result = "/";
            for (const auto& p : parts)
                result += p + "/";
            if (result.length() > 1)
                result.pop_back();
            return result;
        }
};


class AutoIndex {
    public:
        static std::string generate(const std::string& path, const std::string& dirPath) {
            std::string html = "<html><body><h1>Index of " + dirPath + "</h1><hr><ul>";
            DIR* dir = opendir(path.c_str());
            if (dir == NULL)
                return "";

            struct dirent* entry;
            while((entry = readdir(dir)) != NULL) {
                std::string name = entry->d_name;
                if (name == ".")
                    continue; // Skip current dir link

                    // Generate a simple link for eack file/folder
                    html += "<li><a href=\"" + name + "\">" + name + "</a></li>";
            }

            closedir(dir);
            html += "</ul><hr></body></html>";
            return html;
        }
};

struct RedirectRule {
    int statusCode; // 301 or 302
    std::string location; // The URL to redirect to
};

class StaticFileServer {
    public:
    // Upgraded signature: Takes the raw path AND your server configuration rules
    static HttpResponse serveFile(const std::string& rawPath, const ServerConfig& config, const std::unordered_map<std::string, RedirectRule>& redirectConfig) {
        
        // 1. Normalize and URL-decode the client input path
        std::string decodedPath = PathResolver::urlDecode(rawPath);
        std::string normalized = PathResolver::normalize(decodedPath);

        // 2. CHECK REDIRECTS (301/302) BEFORE HITTING THE DISK
        if (redirectConfig.count(normalized)) {
            const RedirectRule& rule = redirectConfig.at(normalized);
            HttpResponse res(rule.statusCode, (rule.statusCode == 301 ? "Moved Permanently" : "Found"));
            res.setHeader("Location", rule.location);
            return res;
        }

        // 3. LONGEST-PREFIX ROUTE MATCHING
        std::string matchedPrefix = RouteMatcher::match(normalized, config);
        if (matchedPrefix.empty()) {
            // Drop a fallback 404 error page if no prefix matches at all
            HttpResponse errorRes(404, "Not Found");
            errorRes.setHeader("Content-Type", "text/html");
            errorRes.setBody(ErrorPageFactory::getErrorPage(404));
            return errorRes;
        }

        const RouteConfig& matchedRoute = config.routes.at(matchedPrefix);

        // 4. TRANSLATE URL PATH TO PHYSICAL HARD DRIVE PATH
        // Strip the matched URL prefix and combine it with the configured root folder
        std::string relativePath = normalized.substr(matchedPrefix.length());
        std::string fullPath = matchedRoute.root;
        if (!relativePath.empty() && relativePath.front() != '/' && fullPath.back() != '/') {
            fullPath += "/";
        }
        fullPath += relativePath;

        std::string mimeLookupPath = normalized;

        // 5. SECURITY GUARD: Block Path Traversal Outside Configured Root
        if (fullPath.find(matchedRoute.root) != 0) {
            HttpResponse errorRes(403, "Forbidden");
            errorRes.setHeader("Content-Type", "text/html");
            errorRes.setBody(ErrorPageFactory::getErrorPage(403));
            return errorRes;
        }

        // 6. PROCESS FILES AND DIRECTORIES USING OS STATS
        struct stat s;
        if (stat(fullPath.c_str(), &s) != 0) {
            HttpResponse errorRes(404, "Not Found");
            errorRes.setHeader("Content-Type", "text/html");
            errorRes.setBody(ErrorPageFactory::getErrorPage(404));
            return errorRes;
        }

        // Handle Directory Request
        if (s.st_mode & S_IFDIR) {
            if (fullPath.back() != '/') fullPath += '/';
            
            std::string targetIndex = matchedRoute.indexFile.empty() ? "index.html" : matchedRoute.indexFile;
            std::string indexPath = fullPath + targetIndex;

            // Check if the route's configured index file actually exists on disk
            if (stat(indexPath.c_str(), &s) == 0) {
                fullPath = indexPath;
                mimeLookupPath = (normalized.back() == '/') ? normalized + targetIndex : normalized + "/" + targetIndex;
            } 
            // DYNAMIC AUTOINDEX GENERATION (If no index file exists and feature is turned ON)
            else if (matchedRoute.autoindex) {
                HttpResponse res(200, "OK");
                res.setHeader("Content-Type", "text/html");
                res.setBody(AutoIndex::generate(fullPath, normalized)); // 'fullPath' here is the directory path
                return res;
            } 
            // If no index file and autoindex is OFF -> Return 403 Forbidden
            else {
                HttpResponse errorRes(403, "Forbidden");
                errorRes.setHeader("Content-Type", "text/html");
                errorRes.setBody(ErrorPageFactory::getErrorPage(403));
                return errorRes;
            }
        }

        // 7. READ PERMISSIONS GUARD (Check if readable)
        if (access(fullPath.c_str(), R_OK) == -1) {
            HttpResponse errorRes(403, "Forbidden");
            errorRes.setHeader("Content-Type", "text/html");
            errorRes.setBody(ErrorPageFactory::getErrorPage(403));
            return errorRes;
        }

        // 8. PASS VERIFIED SECURITY DATA TO WORKER FOR STREAMING
        return serveFileContent(fullPath, mimeLookupPath);
    }

    private:
    static HttpResponse serveFileContent(const std::string& fullPath, const std::string& pathForMime) {
        std::ifstream file(fullPath, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            HttpResponse errorRes(404, "Not Found");
            errorRes.setHeader("Content-Type", "text/html");
            errorRes.setBody(ErrorPageFactory::getErrorPage(404));
            return errorRes;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        HttpResponse res(200, "OK");
        
        size_t dotPos = pathForMime.find_last_of('.');
        std::string extension = (dotPos != std::string::npos) ? pathForMime.substr(dotPos) : "";
        
        res.setHeader("Content-Type", MimeTypeHelper::getMimeType(extension));
        res.setBody(buffer.str());
        return res;
    }
};


class HttpServer {
    private:
        int serverFd;
        int port;
    public:
        HttpServer(int portNum) : serverFd(-1), port(portNum) {}
        ~HttpServer() {
            if (serverFd != -1) {
                close(serverFd);
            }
        }

        void init() {
            // Create Socket: AF_INET (IPv4), SOCK_STREAM (TCP)
            serverFd = socket(AF_INET, SOCK_STREAM, 0);
            if (serverFd < 0) {
                perror("Socket generation failed");
                exit(EXIT_FAILURE);
            }
            // Forcefully attaching socket to the port to avoid "Address already in use" errors
            int opt = 1;
            setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
            // Bind Socket to IP and port
            struct sockaddr_in address;
            std::memset(&address, 0, sizeof(address));
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = INADDR_ANY; // listen on all network interfaces
            address.sin_port = htons(port);  // convert host byte order to network byte order

            if (bind(serverFd, (struct sockaddr*)&address, sizeof(address)) < 0) {
                perror("Bind operator failed");
                exit(EXIT_FAILURE);
            }

            // Listen for incoming connections (Backlog queue length = 10)
            if (listen(serverFd, 10) < 0) {
                perror("Listen operation failed");
                exit(EXIT_FAILURE);
            }

            std::cout << "Server initialized safely. Listening on port " << port << "...\n";
        }

        void start() {
            struct sockaddr_in clientAddress;
            socklen_t clientLen = sizeof(clientAddress);

            // The Master Server Infinite Event Loop
            while (true) {
                // Accept client connection block
                int clientFd = accept(serverFd, (struct sockaddr*)&clientAddress, &clientLen);
                if (clientFd < 0) {
                    perror("Accepting connection failed");
                    continue;
                }
                // Handle the I/O transactions
                handleClient(clientFd);
            }
        }
    private:
        void handleClient(int clientFd) {
            char buffer[4096] = {0};

            // Read raw incoming network traffic bytes
            ssize_t bytesRead = read(clientFd, buffer, sizeof(buffer) - 1);
            if (bytesRead <= 0) {
                close(clientFd);
                return ;
            }

            std::string rawRequest(buffer, bytesRead);

            // ---- Integration point with my engine ----
            // Parse the incoming string using phase 1 code
            HttpRequest req = RequestParser::parse(rawRequest);

            // Process asset discovery using phase 3 code
            HttpResponse res = StaticFileServer::serveFile(req.path);

            // Serialize response structure using phase 2 code
            std::string serializedOutput = res.toString();

            // Send raw serialized bytes straight onto the socket wire
            write(clientFd, serializedOutput.c_str(), serializedOutput.length());

            // Standard HTTP/1.1 Non-persistent close strategy for now
            close(clientFd);
        }
};

struct RouteConfig {
    std::string root; // e.g., "./www/images_folder" or "./www/root_folder"
    bool autoindex; // On or Off
    std::string indexFile; // Defualt to "index.html"
};

// A map representing our server configuration 
// Key: the url prefix (route) => Value: the specific rutes for the route
class ServerConfig {
    public:
        std::unordered_map<std::string, RouteConfig> routes;

        // Helper to add routes easily during initialization
        void addRoute(const std::string& path, const RouteConfig& config) {
            routes[path] = config;
        }
};

class RouteMatcher {
    public:
        // Returns the matching route key (prefix string) for a given request path 
        static std::string match(const std::string& requestPath, const ServerConfig& config) {
            std::string bestMatch = "";
            size_t longestMatchLength = 0;

            for (const auto& [routePrefix, routeOpts] : config.routes) {
                // Check if the request path starts with the route prefix
                if (requestPath.rfind(routePrefix, 0) == 0) {
                    // If it's a match, and is longer than our previous best match, update it
                    if (routePrefix.length() > longestMatchLength) {
                        longestMatchLength = routePrefix.length();
                        bestMatch = routePrefix;
                    }
                }
            }
            return bestMatch;
        }
};


int main()
{
    // const int PORT = 8080;
    // HttpServer server(PORT);
    // server.init();
    // server.start();

    // Setup our configuration rules
    ServerConfig config;

    // Route 1: The general fallback root route
    RouteConfig rootRoute;
    rootRoute.root = "./www/main_site";
    rootRoute.autoindex = false;
    rootRoute.indexFile = "index.html";
    config.addRoute("/", rootRoute);

    // Route 2: The more specific images directory asset route
    RouteConfig imageRoute;
    imageRoute.root = "./www/global_images";
    imageRoute.autoindex = true; // allow directory browsing here!
    imageRoute.indexFile = "index.html";
    config.addRoute("/images/", imageRoute);

    // Test Case A: User requests a basic path
    std::string testPathA = "/about.html";
    std::string matchA = RouteMatcher::match(testPathA, config);
    std::cout << "Path: " << testPathA << " -> Matches Prefix: '" << matchA << "'\n"; 
    // Output: Matches Prefix: '/'

    // Test Case B: User requests an asset path (Longest Prefix wins!)
    std::string testPathB = "/images/gallery/avatar.png";
    std::string matchB = RouteMatcher::match(testPathB, config);
    std::cout << "Path: " << testPathB << " -> Matches Prefix: '" << matchB << "'\n"; 
    // Output: Matches Prefix: '/images/' because 8 characters beats 1 character ('/')!
    return 0;
}
